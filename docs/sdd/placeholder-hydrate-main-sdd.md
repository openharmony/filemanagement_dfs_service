> 标题：clouddiskservice 占位符异步水合主体 NDK 接口详细设计
>
> 状态：设计已签核并实现。本文修订覆盖旧版“应用必须在 OnCallback 返回前完成 Execute”的同步回调语义。
>
> 规格依据：`docs/agent-knowledge/clouddisk-placeholder-spec.md`；Execute 细节见
> `docs/sdd/placeholder-execute-sdd.md`。

# Placeholder Hydrate Main SDD

## 1. 背景

`OH_CloudDisk_HydratePlaceholder(FETCH_DATA)` 是异步触发接口：service 创建水合任务后立即返回，
网盘应用收到 `FETCH_DATA` 回调后下载文件，并通过 `OH_CloudDisk_Execute` 分块写回。

旧设计虽然让 Hydrate 入口异步返回，但 service 使用同步 Binder 回调，并在回调返回后执行
`FinishDispatch`：任务仍为 `IN_PROGRESS` 时立即取消和删除。这要求应用在回调栈内完成全部下载和
Execute，与网盘应用“回调只负责投递异步工作”的使用模型不一致，也让长时间网络下载占用固定
FFRT worker。

本修订将 `FETCH_DATA`/`CANCEL_FETCH_DATA` 定义为 one-way 通知。应用可以立即从回调返回，
task 在 service 中继续存活，直到最终 Execute、主动取消、空闲超时、callback 死亡、注销或 SA
停止。

## 2. 目标

1. Hydrate 创建 task 后立即返回；成功只表示任务已入队。
2. `FETCH_DATA` 和 `CANCEL_FETCH_DATA` 使用 one-way Binder；`DEHYDRATE` 和
   `FETCH_RANGE_DATA` 保持同步。
3. 回调返回不改变 task 状态，不关闭 fd，不调用旧 `FinishDispatch` 自动取消逻辑。
4. 应用在回调内深拷贝请求字段，回调返回后异步、串行调用 Execute。
5. 引入5分钟滑动空闲超时；成功且非空的 Execute 刷新超时，不设绝对总时长。
6. 限制常驻任务：每个应用最多5个，全局最多10个。
7. 取消后保留轻量 tombstone：每个应用最多16条、全局最多32条、TTL 60秒。
8. 对所有正常取消来源统一发送最多一次 `CANCEL_FETCH_DATA`。
9. 保留原有优先级、per-file 在途护栏、目录归属、状态机和进度推送。

## 3. 总体决策

| 项 | 决策 |
| --- | --- |
| Hydrate 入口 | 创建 PENDING task 后立即返回；`CLOUD_DISK_OK` 不代表 callback 已执行或水合完成 |
| FETCH/CANCEL callback | one-way、best-effort；同一同步根内串行投递，保证 FETCH 先于对应 CANCEL |
| DEHYDRATE/RANGE callback | 继续使用同步 request/reply |
| 回调参数所有权 | SDK 所有，仅在回调期间有效；应用必须手工深拷贝并自行释放，不新增 Copy/Destroy CAPI |
| task 生命周期 | callback 返回后保持 IN_PROGRESS，直到完成、取消、超时、callback 死亡、注销或 SA 停止 |
| 空闲超时 | FETCH 成功投递后开始5分钟计时；成功且 `size > 0` 的 Execute 刷新；无绝对上限 |
| active 限额 | 按当前 SA 服务的 `bundleName` 计数：每应用5个、全局10个；不按 userId 分片 |
| 限额错误 | 新增 `OH_CLOUD_DISK_HYDRATION_TASK_LIMIT_REACHED = 34400034` |
| 优先级 | HIGH > NORMAL > LOW，同优先级 FIFO；容量满时不抢占、不突破上限 |
| Execute 顺序 | 下载可并行；同一 reqKey 的 Execute 由应用串行提交，最终 Execute 必须最后调用 |
| Execute 分块上限 | 固定128 KiB（131072字节）；NDK 与 service 双重校验，超出返回 `INVALID_ARG` |
| 完成可信边界 | service 信任应用的 `isComplete`，不跟踪或校验写入区间完整性 |
| 取消通知 | 已成功投递 FETCH 的 task 首次进入 CANCELLED 时，统一发送最多一次 one-way CANCEL |
| tombstone | 仅 CANCELLED 保留；每应用16条、全局32条、TTL 60秒；COMPLETED 不保留 |
| 跨重启 | task、tombstone 和 callback 注册均不持久化；SA 重启不恢复、不自动重注册 |
| 用户切换 | SA 同一时刻只服务当前用户；切换时整体清理 task/callback/progress/tombstone |
| 权限 | provider 入口保留目录归属校验，不增加系统权限；System API 继续双侧系统权限/身份校验 |

## 4. 状态与数据结构

### 4.1 PlaceholderTaskState

| 状态 | 含义 | 进入条件 | 离开条件 |
| --- | --- | --- | --- |
| PENDING | task 已创建，FETCH 尚未成功投递 | CreateHydrateTask | one-way FETCH 成功→IN_PROGRESS；取消/失败→清理 |
| IN_PROGRESS | FETCH 已投递，等待异步 Execute | callback proxy 接受 one-way 请求 | 最终 Execute→COMPLETED；取消/超时/死亡→CANCELLED |
| COMPLETED | 最终写入、fsync、state=3 已成功 | 最终 Execute | 立即 close fd、erase；不留 tombstone |
| CANCELLED | task 被终止，file state 保持1或2 | 取消、超时、callback 死亡等 | 立即 close fd、erase，并按规则留 tombstone |

`COMPLETED`/`CANCELLED` 是终态转换，不作为长期 active record 驻留。

### 4.2 PlaceholderTaskRecord

现有字段保留，并增加异步生命周期字段：

```cpp
struct PlaceholderTaskRecord {
    RequestKey reqKey;
    std::string syncFolder;
    std::string filePath;
    std::string bundleName;
    uint32_t syncFolderIndex = 0;
    CloudDiskCallbackType callbackType = CloudDiskCallbackType::FETCH_DATA;
    CloudDiskHydratePriority priority;
    UniqueFd outputFd;
    std::atomic<PlaceholderTaskState> state = PlaceholderTaskState::PENDING;

    int32_t userId = -1;
    uint64_t totalSize = 0;
    bool totalSizeInitialized = false;
    uint64_t cachedSize = 0;
    bool hasPartialState = false;
    std::string absolutePath;

    bool fetchDispatched = false;
    bool cancelCallbackSent = false;
    bool terminalProgressSent = false;
    uint64_t createSeq = 0;
    std::mutex mutex;
};
```

deadline 由 manager 的 `deadlineMap_` 按 reqKey 保存，并只使用单调时钟。active task 全局上限为10，
因此超时线程直接扫描这个有界 map，不引入最小堆和 version。

### 4.3 CancelledTaskTombstone

```cpp
struct CancelledTaskTombstone {
    RequestKey reqKey;
    std::string bundleName;
    uint32_t syncFolderIndex = 0;
    std::string syncFolder;
    std::string filePath;
    std::chrono::steady_clock::time_point expiresAt;
    uint64_t createSeq = 0;
};
```

tombstone 不保存 fd、task mutex、文件数据或进度对象。由于 reqKey 是 service 生成的 opaque 标识，
Execute 命中 tombstone 时仍须校验 bundleName、syncFolderIndex、syncFolder 和 filePath。
生成新 reqKey 时同时避开 active map 与 tombstone map。

### 4.4 常量

```cpp
CLOUD_DISK_HYDRATE_WORKER_COUNT = 4
MAX_EXECUTE_DATA_SIZE = 128KiB
HYDRATION_IDLE_TIMEOUT = 5min
MAX_ACTIVE_TASKS_PER_APP = 5
MAX_ACTIVE_TASKS_GLOBAL = 10
MAX_TOMBSTONES_PER_APP = 16
MAX_TOMBSTONES_GLOBAL = 32
TOMBSTONE_TTL = 60s
```

## 5. 行为设计

### 5.1 启动水合

```text
app / SystemAccessor
  → StartHydrationInner / StartHydrationByPathInner
  → 权限、调用者归属和路径解析
  → per-file 在途、文件类型和 placeholder state 校验
  → open(O_RDWR | O_CLOEXEC | O_NOFOLLOW)
  → 在 callback registry 生命周期临界区确认 callback table 已注册
  → 在同一 task map 临界区检查 active 限额并插入 PENDING task
       taskMap_[reqKey] = PENDING
       pendingQueue.push(priority, FIFO)
       progress(PENDING)
  → 返回 CLOUD_DISK_OK
```

provider CAPI 与 System API 都必须在 task 创建前确认 callback 已注册；未注册返回
`OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`。callback registry mutex 同时承载注册/注销和“确认注册后创建
task”的生命周期临界区，避免“校验已注册后刚好注销”产生孤儿 task。

active 限额检查和 task 插入在同一 map 临界区完成。PENDING、IN_PROGRESS 以及正在完成终态清理但
仍持有 fd 的 record 均占名额；erase 后立即释放名额。容量满时不按优先级抢占，返回
`OH_CLOUD_DISK_HYDRATION_TASK_LIMIT_REACHED`。

### 5.2 one-way FETCH 投递

worker 仍从优先级队列取 PENDING task，但只负责短时通知，不等待应用下载：

```text
worker
  → 取最高优先级 PENDING task
  → 在 task mutex 下确认未取消
  → callback proxy 使用 MessageOption::TF_ASYNC 发送 FETCH_DATA
  → 发送接受成功：
       state = IN_PROGRESS
       fetchDispatched = true
       deadline = steady_clock::now() + 5min
       progress(IN_PROGRESS)
  → 立即处理下一个 task
```

发送失败或 remote 已死亡：task 进入 CANCELLED、关闭 fd并清理。由于 FETCH 未成功投递，应用没有
见过 reqKey，不发送 CANCEL，也无需保留 tombstone。

旧 `FinishDispatch` 语义删除：callback 返回不会触发 task 取消或清理。

### 5.3 应用回调契约

应用 callback 必须快速完成以下工作后返回：

1. 判断 callbackType。
2. 对 FETCH_DATA 深拷贝 `syncFolderPath.value`、`reqKey.data`、
   `reqContext.fetchData`、`filePath.value` 和业务需要的 priority。
3. 将深拷贝后的请求投递到应用异步下载队列。
4. 不保存 SDK 提供的任何裸指针。

应用拥有深拷贝资源。异步 Execute 调用期间，构造出的 reqHead/reqContext/rsp 及其指向的内存必须
保持有效；Execute 返回后可以释放或复用本次传入内存。

下载可并行，但同一 reqKey 的 Execute 必须经应用侧串行队列提交。最终 `isComplete=true`
请求只能在该 reqKey 的所有非最终 Execute 返回后调用。

### 5.4 Execute 与完成

Execute 详细校验、错误处理和落盘顺序见 Execute SDD。水合主体只规定：

- 第一次成功非空写入将 file state 从1推进到2；空文件最终 Execute 可由1直接到3。
- service 不验证区间覆盖，完整性由 provider 保证。
- `totalSize > 0` 时，最终 Execute 必须携带非空数据；空文件允许零长度最终 Execute。
- 最终顺序为 `pwrite → fsync → 非空文件确保 state=2 → SetFilePlaceholderState(3) →
  progress(COMPLETED) → close/erase`；空文件允许 `state=1→3`。
- COMPLETED 不留 tombstone；清理后重复 Execute 返回 `E_NO_HYDRATION_IN_PROGRESS`。
- pwrite、fsync、xattr 的可重试失败不删除 task，等待应用重试或5分钟空闲超时。

### 5.5 取消

正常取消来源统一进入同一状态转换：

- provider 调 `HydratePlaceholder(CANCEL_FETCH_DATA)`；
- SystemAccessor 取消；
- 5分钟空闲超时；
- FETCH 已投递后的内部终止。

取消与 Execute 由同一 task mutex 线性化，首个完成终态转换的一方生效：

- 最终 Execute 先完成：取消返回 `E_NO_HYDRATION_IN_PROGRESS`。
- 取消先完成：迟到 Execute 命中 tombstone，返回 `E_CANCELLED`。

取消步骤：

1. 将 task 从 PENDING/IN_PROGRESS 改为 CANCELLED。
2. 推送一次终态进度。
3. 创建/更新取消 tombstone，保证 active record 删除后迟到 Execute 仍可识别取消结果。
4. close outputFd，从 active map 删除并释放限额。
5. 若 `fetchDispatched=true`、callback 仍有效且 `cancelCallbackSent=false`，通过同一同步根的串行
   通知队列发送一次 one-way `CANCEL_FETCH_DATA`，携带原 reqKey 和 filePath。

provider 主动取消也会收到 CANCEL 回调，应用统一在 CANCEL 分支停止下载并释放资源。
发送是 at-most-once、best-effort，不等待应用确认。

例外：

- PENDING task 在 FETCH 投递前取消：应用从未看到请求，不发 CANCEL。
- Unregister：注销本身是应用侧批量取消信号，不逐 task 回调。
- provider 进程死亡、用户切换、SA 停止：没有有效接收方，不发 CANCEL。
- SA 异常死亡：不在 NDK 维护请求镜像，不合成 CANCEL；应用从后续 IPC 错误或自身下载超时清理。

### 5.6 Tombstone

取消 tombstone 的行为：

- TTL 60秒；每应用最多16条，全局最多32条。
- 插入前先删除过期项；达到上限时淘汰最老项。
- 命中且身份/路径匹配的 Execute 返回 `E_CANCELLED`。
- 过期、被淘汰或从未存在时返回 `E_NO_HYDRATION_IN_PROGRESS`。
- tombstone 不参与 per-file 在途护栏，不占 active 名额，不阻止同一文件立即创建新 task。
- 新 task 使用新的 reqKey。

### 5.7 滑动空闲超时

PlaceholderTaskManager 使用一个基于 `steady_clock` 的有界 deadline map 和一个清理任务：

1. FETCH one-way 投递成功后设置首个 deadline。
2. 成功且 `size > 0` 的非最终 Execute 覆盖更新 reqKey 对应的 deadline。
3. 非法参数、pwrite/fsync/xattr 失败和零长度非最终请求不刷新。
4. 清理任务扫描最多10项的 deadline map，取到期 reqKey 后在 task mutex 下重新检查当前 deadline。
5. 确认超时后走 §5.5 的统一取消路径。

不设置绝对任务时长；只要应用持续成功回写非空数据，超长下载可以继续。

### 5.8 注销、死亡、用户切换和 SA 生命周期

**UnregisterCallbackTable**

- 按同步根生命周期锁阻止新 task。
- 取消并清理该同步根全部 active task，创建取消 tombstone，但不发送 CANCEL。
- 移除 callback 注册。
- 客户端 callback table 设置 inactive；已排队的 one-way 通知到达 stub 时直接丢弃。
- Unregister 成功返回后，不再进入应用 callback。应用必须将 Unregister 视为本地异步任务批量终止。

**provider death**

- death recipient 清 callback 注册并取消对应 active task。
- 关闭 fd并保留 file state；不回调已经死亡的应用。

**用户切换**

- SA 同时只服务当前用户，限额仅按 bundleName 统计。
- 切换时整体取消 active task并清理 callback、progress subscriber 和 tombstone。

**SA 停止/重启**

- 停止 worker、deadline 清理任务，关闭全部 fd，清空内存状态。
- 不持久化、不恢复 task/tombstone，不自动重新注册 callback。
- 应用后续可能收到 `IPC_FAILED`、`CALLBACK_NOT_REGISTERED` 或
  `NO_HYDRATION_IN_PROGRESS`，应据此终止旧请求并重新注册。

## 6. 锁与竞态

建议锁职责：

| 锁 | 保护内容 |
| --- | --- |
| callback registry mutex | callback map/death recipient，以及注册/注销与 task 创建的生命周期串行 |
| mapMutex | active map、pending queue、deadline map、限额计数、tombstone map/顺序 |
| task mutex | 单 task state、fd、totalSize、进度、终态转换；deadline 刷新由该状态串行触发 |
| 每同步根 dispatch mutex | FETCH/CANCEL 提交顺序；不在持有 registry mutex 时等待 |

不得在持有 callback registry mutex 时等待 task mutex或发 Binder 请求。mapMutex 只用于短临界区，
不得跨 pwrite/fsync/xattr 或 callback IPC 持有。取消与 Execute 先取得 task shared_ptr，再以 task
mutex 决定终态，终态清理需保证 close/erase/progress/tombstone 各执行一次。

## 7. 错误码

| 场景 | 返回码 |
| --- | --- |
| callback 未注册 | `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`(34400021) |
| 同文件已有 active task | `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`(34400019) |
| 每应用5个或全局10个 active task 已满 | `OH_CLOUD_DISK_HYDRATION_TASK_LIMIT_REACHED`(34400034) |
| 取消无 active task | `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`(34400032) |
| Execute 命中取消 tombstone | `OH_CLOUD_DISK_CANCELLED`(34400030) |
| 完成清理后的 Execute | `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`(34400032) |
| task 尚未成功投递 FETCH | `CLOUD_DISK_TRY_AGAIN`(34400014) |

其他状态、路径、权限和 I/O 错误沿用最终 spec。

## 8. 实现落点

| 路径 | 修改 |
| --- | --- |
| `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h` | 新增 34400034 |
| `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h` | 补充 callback 借用生命周期、应用深拷贝、异步 Execute 与 Hydrate 成功语义 |
| `interfaces/kits/ndk/clouddiskmanager/src/oh_cloud_disk_manager.cpp` | 保持现有 CAPI；Execute 同步复制应用内存 |
| `frameworks/native/clouddiskservice_kit_inner/**` | FETCH/CANCEL one-way；callback table inactive 护栏 |
| `utils/clouddiskservice/include/cloud_disk_service_error.h` | 新增内部容量错误码 |
| `services/clouddiskservice/ipc/include/placeholder_task_manager.h` | deadline、限额、tombstone、异步终态字段 |
| `services/clouddiskservice/ipc/src/placeholder_task_manager.cpp` | 删除 callback-return 自动取消；实现滑动超时、容量、tombstone和统一取消 |
| `services/clouddiskservice/ipc/src/placeholder_callback_manager.cpp` | one-way FETCH/CANCEL 顺序及 callback death 清理 |
| `services/clouddiskservice/ipc/src/cloud_disk_service.cpp` | 两类 Hydrate 入口统一 callback 前置检查；注销/用户切换清理 |
| `services/clouddiskservice/ipc/src/cloud_disk_service_callback_proxy.cpp` | 按 callback type 选择 one-way 或同步 MessageOption |

不新增公开函数，不改变 IDL 的 Hydrate/Cancel/Execute 方法签名，不改变
`CallbackExecuteRequest` Parcelable 字段顺序。

## 9. 测试要求

至少覆盖：

1. callback 返回后 task 仍为 IN_PROGRESS，fd 未关闭。
2. 回调返回后异步多次 Execute，最终数据和 state 正确。
3. FETCH/CANCEL 使用 one-way；DEHYDRATE/RANGE 保持同步。
4. 应用侧深拷贝后原 callback storage 销毁，Execute 仍正确。
5. 单次 Execute 在128 KiB边界成功，128 KiB + 1字节由NDK和service拒绝。
6. 5分钟超时、成功非空 Execute 刷新、失败/空请求不刷新。
7. 每应用第6个、全局第11个 task 返回容量错误；完成/取消后名额释放。
8. 容量满时 HIGH 不抢占已有 LOW。
9. provider/System/timeout 取消均最多发送一次 CANCEL；未投递 FETCH 不发 CANCEL。
10. FETCH/CANCEL 顺序以及 Execute/Cancel、Execute/timeout 竞态。
11. 取消 tombstone 的命中、身份校验、TTL、每应用16和全局32淘汰。
12. 完成不留 tombstone，重复最终 Execute 返回 NO_HYDRATION_IN_PROGRESS。
13. Unregister 后无应用 callback，active task 全清；重新注册可创建新 task。
14. provider death、用户切换和 SA stop 清理 fd/map/queue/timer。
15. callback 未注册时 provider 与 System Hydrate 均立即失败。

## 10. 不在本次范围

- 不新增 NDK request handle 或 Copy/Destroy API。
- 不在 NDK 保存 active request 镜像，不在 SA death 时合成 CANCEL。
- 不持久化或跨 SA 重启恢复水合任务。
- 不验证写入区间覆盖、不去重重叠块。
- 不实现 FETCH_RANGE_DATA demand hydration。
- 不改变 ArkTS 进度 API 形状。
