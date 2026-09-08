> 标题：clouddiskservice 占位符异步水合数据回写（Execute）NDK 接口详细设计
>
> 状态：设计已签核并实现。本文修订覆盖旧版“Execute 必须在 OnCallback 栈内完成”的约束。
>
> 规格依据：`docs/agent-knowledge/clouddisk-placeholder-spec.md`；task 创建、通知、取消和超时见
> `docs/sdd/placeholder-hydrate-main-sdd.md`。

# Placeholder Execute SDD

## 1. 背景

`OH_CloudDisk_Execute` 是网盘 provider 向 CloudDiskService 回写水合数据的独立同步 IPC。
Hydrate callback 改为 one-way 通知后，应用在 callback 中只深拷贝请求并投递异步下载，callback
返回后再调用 Execute。

Execute 的函数签名和 `CallbackExecuteRequest` Parcelable 不变，但 task 生命周期、参数所有权、
完成顺序、超时刷新和取消后查询语义发生变化：

- callback 返回不再清理 task；
- app 传入的异步请求内存由 app 管理，NDK 在 Execute 返回前完成同步复制；
- 最终数据先 fsync，再标记 FULLY_HYDRATED；
- 可重试错误保留 task；
- 取消后 Execute 可在60秒内通过 tombstone 得到 `E_CANCELLED`。

## 2. 目标

1. 保持现有 NDK、inner manager、IDL 和 Parcelable 形状，不新增函数。
2. 明确 callback 参数为借用内存，异步 Execute 必须使用应用深拷贝。
3. 对 NDK 和 service 实施一致的数据边界校验。
4. 同一 reqKey 的 Execute 由应用串行，service per-task mutex 再次串行化。
5. 成功且非空的 Execute 刷新5分钟空闲 deadline。
6. pwrite、fsync、xattr 的可重试失败不删除 task。
7. 最终顺序固定为 `pwrite → fsync → 非空文件确保 state=2 → state=3 → progress → close/erase`。
8. service 信任 `isComplete`，不跟踪区间覆盖；进度只做有界累计。
9. Execute 先查 active task，再查取消 tombstone。

## 3. 接口和所有权

### 3.1 NDK 签名

```c
CloudDisk_ErrorCode OH_CloudDisk_Execute(
    OH_CloudDisk_CallbackReqHead reqHead,
    OH_CloudDisk_CallbackContext reqContext,
    OH_CloudDisk_CallbackResponse rsp);
```

不新增 request handle、Copy 或 Destroy API。

### 3.2 应用内存契约

SDK 传给 callback 的下列内存只在 callback 调用期间有效：

- `reqHead.syncFolderPath.value`；
- `reqHead.reqKey.data`；
- `reqContext.fetchData` 及其 `filePath.value`；
- 其他 union 变体和内部数据 buffer。

应用要异步 Execute，必须在 callback 返回前深拷贝需要的字符串、reqKey 和结构字段。应用随后用
自己的存储重建 reqHead/reqContext/rsp。

Execute 是同步函数。应用必须保证以下内存在 Execute 返回前有效且不被并发修改：

- reqHead/reqContext 中的路径和 reqKey；
- rsp.fetchData；
- rsp.fetchData.data 指向的分块 buffer。

NDK `BuildCallbackExecuteRequest` 在发起 inner IPC 前将全部字段复制到 C++ owned object；
Execute 返回后 service 不再引用应用指针。

### 3.3 CallbackExecuteRequest

字段和 Marshalling 顺序不变：

```cpp
struct CallbackExecuteRequest final : public Parcelable {
    std::vector<uint8_t> reqKey;
    std::string syncFolder;
    std::string filePath;
    int32_t callbackType = 0;
    uint64_t offset = 0;
    uint64_t size = 0;
    uint64_t totalSize = 0;
    std::vector<uint8_t> data;
    bool isComplete = false;
};
```

顺序：reqKey → syncFolder → filePath → callbackType → offset → size → totalSize → data →
isComplete。Unmarshalling 逐项检查返回值。

## 4. NDK 校验

### 4.1 FETCH_DATA

```cpp
constexpr uint64_t MAX_EXECUTE_DATA_SIZE = 128 * 1024; // 128 KiB
```

`BuildExecuteFetchData` 必须校验：

1. `rsp.fetchData != nullptr`。
2. `size == data.dataSize`。
3. `size <= MAX_EXECUTE_DATA_SIZE`，其中上限固定为128 KiB（131072字节）。
4. `offset <= totalSize` 且 `size <= totalSize - offset`，避免溢出。
5. `size > 0` 时 `data.data != nullptr`。
6. `totalSize > 0 && isComplete=true` 时 `size > 0`。
7. `totalSize == 0` 时只接受 `offset=0,size=0,isComplete=true`。

非最终的零长度请求无业务效果，返回 `CLOUD_DISK_INVALID_ARG`，不能用于刷新空闲超时。

### 4.2 请求头和上下文

- reqKey data 非空，大小不超过既有上限。
- syncFolder 和 filePath 合法。
- FETCH_DATA 的 filePath 从 `reqContext.fetchData->filePath` 复制。
- `totalSize` 首次合法请求后固定，后续不得改变。
- `FETCH_RANGE_DATA` 拒绝。
- 现有 CANCEL_FETCH_DATA Execute 解析能力保留，但 provider 主动放弃任务的规范入口是
  `OH_CloudDisk_HydratePlaceholder(..., CANCEL_FETCH_DATA, ...)`。

## 5. Service 流程

### 5.1 ExecuteInner

```text
ExecuteInner(request)
  → reqKey/基础参数校验
  → ResolveOwnedSyncFolder
       失败统一返回 E_CALLBACK_NOT_REGISTERED
  → 确认 callback 仍注册
  → PlaceholderTaskManager::Execute(bundleName, syncFolderIndex, request)
```

`ResolveOwnedSyncFolder` 的具体失败原因只写内部脱敏日志，对外不暴露目录存在性或其他 provider
信息。

### 5.2 Active task / tombstone 查找

```text
Execute
  → FindActiveTask(reqKey)
       找到：校验 bundle/index/syncFolder/filePath → task mutex
       未找到：FindCancelledTombstone(reqKey)
                   身份和路径匹配 → E_CANCELLED
                   不匹配 → 原有安全错误
                   未命中/已过期 → E_NO_HYDRATION_IN_PROGRESS
```

COMPLETED 不留 tombstone。最终 Execute 返回成功后，后续重复调用返回
`E_NO_HYDRATION_IN_PROGRESS`；应用必须保存首次成功结果。

取消 tombstone 保存60秒，每应用最多16条、全局最多32条。它不持有 fd，不占 active task
名额。

### 5.3 Task 状态校验

| 状态 | Execute 结果 |
| --- | --- |
| PENDING | `E_TRY_AGAIN`；FETCH 尚未成功投递 |
| IN_PROGRESS | 执行数据回写 |
| CANCELLED | 正常路径已移入 tombstone；竞态窗口返回 `E_CANCELLED` |
| COMPLETED | 仅可能出现在终态清理竞态窗口；返回 `E_NO_HYDRATION_IN_PROGRESS` |

应用契约要求同一 reqKey 串行调用 Execute。service 的 per-task mutex 是并发安全兜底，不承诺将
并发请求按应用发起顺序排序。

### 5.4 数据写入

在 task mutex 内执行：

1. 重复执行 NDK 的 size、offset、totalSize、零长度和 callbackType 校验。
2. 检查 outputFd 有效。
3. 首次合法请求设置 `task.totalSize`；后续要求一致。
4. `pwrite` 循环处理短写；EINTR 重试。
5. 非最终非空请求若尚未进入部分水合，将 file state 从1推进到2。
6. 非最终块：有界更新 processedSize、刷新 deadline、推送节流进度并返回。
7. 最终块不提前修改 state，执行 §5.5；空文件保持 state=1 到最终状态写入。

service 不校验 offset 单调、不校验分块不重叠、不维护覆盖区间。provider 必须保证所有文件数据
均已写入，并仅在最后一个数据块设置 `isComplete=true`。

### 5.5 最终完成顺序

```text
pwrite(final block)
  → fsync(outputFd)
  → 非空文件确保 state 至少为 PARTIALLY_HYDRATED；空文件保持 UNHYDRATED
  → SetFilePlaceholderState(FULLY_HYDRATED)  // 1/2→3
  → task state = COMPLETED
  → progress(COMPLETED)
  → close fd + erase active task + 释放容量
```

只有上述步骤全部成功才返回 `E_OK`。fsync 或 xattr 失败时：

- 返回 `E_TRY_AGAIN`；
- task 保持 IN_PROGRESS；
- file state 保持 UNHYDRATED/PARTIALLY_HYDRATED，不暴露完全水合；
- 不关闭 fd、不创建 tombstone；
- 本次失败不刷新 deadline，应用可以在超时前重试。

若 pwrite 已成功而后续步骤失败，应用可以用相同 offset/data 重试。数据覆盖是幂等的，进度仅在
整次 Execute 成功后累计，避免一次失败写入被重复计数。

### 5.6 进度和 deadline

- `processedSize = min(totalSize, processedSize + successfulRequest.size)`。
- 重叠块和成功请求的重复提交由应用避免；service 不做去重。
- 只有返回 `E_OK` 且 `size > 0` 的 Execute 刷新
  `deadline = steady_clock::now() + 5min`。
- 非法请求、I/O/xattr 失败、零长度请求不刷新。
- 空文件的零长度最终 Execute 直接完成，不需要刷新。

## 6. 错误处理

| 条件 | 返回码 | task 是否保留 |
| --- | --- | --- |
| task 不存在且无 tombstone | `E_NO_HYDRATION_IN_PROGRESS` | — |
| 命中匹配的取消 tombstone | `E_CANCELLED` | — |
| task=PENDING | `E_TRY_AGAIN` | 是 |
| task=CANCELLED 竞态 | `E_CANCELLED` | 正在清理 |
| task=COMPLETED 竞态 | `E_NO_HYDRATION_IN_PROGRESS` | 正在清理 |
| size/data/offset/totalSize/zero-final 非法 | `E_INVALID_ARG` | 是 |
| pwrite ENOSPC | `E_NO_SPACE_LEFT` | 是 |
| pwrite EINTR | 内部重试 | 是 |
| pwrite 其他失败 | `E_TRY_AGAIN` | 是 |
| fsync/xattr 失败 | `E_TRY_AGAIN` | 是 |
| outputFd 内部失效 | `E_TRY_AGAIN`，随后内部取消 | 否 |
| callback 已注销 | `E_CALLBACK_NOT_REGISTERED` | 注销流程负责清理 |

可重试失败不会主动发送 CANCEL；task 只在 provider/System 主动取消、滑动超时、callback 死亡、
注销、用户切换、SA 停止或内部不可恢复错误时进入 CANCELLED。

## 7. 并发和锁

### 7.1 应用侧

- 同一 reqKey 使用单一 Execute 提交队列。
- 下载线程可以并行，但提交数据块时串行。
- 最终请求必须等待所有非最终 Execute 返回。
- 不得在 Execute 进行中修改或释放请求/响应 buffer。

### 7.2 service 侧

- mapMutex：active map、pending queue、deadline map、容量计数和 tombstone 的短临界区。
- task mutex：单 task 状态、fd、totalSize、进度和终态；deadline 刷新由该状态串行触发。
- 不持 mapMutex 执行 pwrite、fsync、xattr 或 callback IPC。
- Execute、主动取消和 timeout 以 task mutex 的首个终态转换为准。
- task 从 map 删除后，持有 shared_ptr 的竞态调用仍必须检查终态，不能再次 close 或重复推送。

## 8. SA 和 callback 生命周期

- Execute 不再依赖原始 callback 栈，回调返回后可以正常调用。
- callback table 必须仍处于注册状态；Unregister 会取消并删除对应 active task。
- provider death 由 service death recipient 清 task/fd。
- SA death 不在 NDK 合成 CANCEL，也不持久化 task。应用后续遇到 `IPC_FAILED`、
  `CALLBACK_NOT_REGISTERED` 或 `NO_HYDRATION_IN_PROGRESS` 时终止旧请求。
- SA 重启后 callback 不自动重新注册。

## 9. 安全

1. NDK 与 service 双重校验数据长度、offset、totalSize 和分块上限。
2. Execute 必须通过 syncFolder 的 bundleName 归属和 callback 注册校验。
3. active task 与 tombstone 均校验 bundleName、syncFolderIndex、syncFolder 和 filePath。
4. reqKey 对应用是 opaque byte sequence，应用只能复制和回传，不得解析或修改。
5. 路径和身份错误日志使用私有格式；错误码继续按最终 spec 脱敏。
6. 任务容量限制、滑动超时和 tombstone 上限共同约束 fd/内存占用。

## 10. 实现落点

| 路径 | 修改 |
| --- | --- |
| `interfaces/kits/ndk/clouddiskmanager/src/oh_cloud_disk_manager.cpp` | 异步所有权校验、非最终零长度/最终空块规则；保持同步深拷贝 |
| `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h` | 补充 callback 借用生命周期与 Execute buffer 生命周期 |
| `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_service_manager_impl.cpp` | 保持同步 Execute IPC |
| `services/clouddiskservice/ipc/src/cloud_disk_service.cpp` | Execute 前置归属/callback 校验不变 |
| `services/clouddiskservice/ipc/include/placeholder_task_manager.h` | deadline/tombstone 接口和终态字段 |
| `services/clouddiskservice/ipc/src/placeholder_task_manager.cpp` | active→tombstone 查找、重试保活、deadline 刷新、最终顺序 |
| `utils/clouddiskservice/include/cloud_disk_service_error.h` | 容量错误码由 Hydrate 主流程使用 |

IDL 和 `CallbackExecuteRequest` 字段顺序不变，不新增 public CAPI。

## 11. 测试要求

1. callback 返回、callback storage 释放后，应用深拷贝仍可异步 Execute。
2. 同一 task 多次非最终 Execute 后最终完成，数据、state、fd和进度正确。
3. 非空文件零长度 final 拒绝；空文件零长度 final 成功；非最终零长度拒绝。
4. totalSize 中途改变、offset 溢出、data mismatch 拒绝；128 KiB 边界成功，128 KiB + 1字节拒绝。
5. pwrite 短写/EINTR/ENOSPC/其他失败。
6. fsync 和 partial/full xattr 失败时 task 保留、state 不错误升级，可重试成功。
7. 成功非空请求刷新 deadline；失败请求不刷新。
8. overlapping/retry 不让 processedSize 超过 totalSize。
9. 取消后60秒内返回 CANCELLED，过期/淘汰后返回 NO_HYDRATION。
10. 完成不留 tombstone，重复最终 Execute 返回 NO_HYDRATION。
11. Execute/Cancel、Execute/timeout、两个 Execute 竞态只产生一次终态、一次 close和一次终态进度。
12. 跨 bundle/index/path 的 active 与 tombstone 请求均拒绝。

## 12. 不在本次范围

- 不增加覆盖区间、chunk sequence 或重复块去重。
- 不增加独立 Commit/Finish CAPI。
- 不增加 NDK request handle、Copy/Destroy 或 service-death 合成 CANCEL。
- 不改变 FETCH_RANGE_DATA 数据返回方式。
