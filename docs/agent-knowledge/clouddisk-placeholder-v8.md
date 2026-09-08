# CloudDisk 占位符（Placeholder）接口规格 v8

本文件是 v7（`clouddisk-placeholder-v7.md`）的修订版，反映**Execute 数据回写详细规格落地**——v7 的 `OH_CloudDisk_Execute` 行为只定了"独立 IPC、不走 reply、pwrite 到 outputFd"的高层框架，本规格将 Execute SDD（`docs/sdd/placeholder-execute-sdd.md`）的规格级决策并入：`CallbackExecuteRequest` sequenceable Parcelable、`ExecuteInner` IDL 形状、`MAX_EXECUTE_DATA_SIZE` 分块上限、death recipient 行为（崩溃→CANCELLED→fd 清理）、`ResolveOwnedSyncFolder` 失败的错误码安全脱敏。v7 的水合主体框架（异步触发、状态机、取消、护栏、优先级排队）、v6 的优先级增量、v5 的脱水、v4 的 Mark/Unmark、v3 的 customInfo、v2 的 state 模型全部保留。内部实现细节不复述：水合主体各层接线见 `docs/sdd/placeholder-hydrate-main-sdd.md`；Execute 各层接线见 `docs/sdd/placeholder-execute-sdd.md`。v7 保留以记录 Execute 详细规格落地前的快照。

> v8 变化：本次为 **Execute 详细规格落地**——v7 的 §5.2 Execute 行为只定了"独立 IPC + pwrite 到 outputFd"高层框架，Execute SDD E1-E6 细化后本规格并入以下规格级变更：(1) `ExecuteInner` IDL 形状定为 `void ExecuteInner([in] CallbackExecuteRequest request)` 单参 sequenceable；(2) 新增 `CallbackExecuteRequest` Parcelable（§1.4）；(3) `MAX_EXECUTE_DATA_SIZE` 定为 4MB 可配置宏（§5.2 行为约束）；(4) Execute 的错误码映射详定（§5.2：task 状态码 + pwrite 失败码 + 安全脱敏码）；(5) death recipient 行为明确（§5.3：callback table client 崩溃→task CANCELLED→fd 清理→state 保持）；(6) `ResolveOwnedSyncFolder` 失败对外统一 `E_CALLBACK_NOT_REGISTERED`，内部 log 区分（§3 安全脱敏）。v7 的水合主体框架、结构体约束、脱水/Mark/Unmark/customInfo/state 模型全部不变。

## 1. 结构体

### 1.1~1.3（同 v7，不改动）

v7 §1.1（`CloudDisk_PathInfo`/`CloudDisk_SyncFolderPath`/`CloudDisk_FileIdInfo`）、§1.2（`CloudDisk_PlaceholderInfo`/`CloudDisk_PlaceholderCustomInfo`）、§1.3（水合脱水数据契约：`CloudDisk_CallbackType`/`CloudDisk_CallbackReqHead`/`CloudDisk_RangeInfo`/`CloudDisk_CallbackContext`/`CloudDisk_FetchDataRequest`/`CloudDisk_HydratePriority`/`CloudDisk_FetchData`/`CloudDisk_CallbackResponse`）全部保留不变。详见 v7 §1 / v6 §1。

### 1.4 `CallbackExecuteRequest`（v8 新增）

Execute 数据回写的 IPC 载荷，作为 `ExecuteInner` IDL 的单参 sequenceable。app 侧 NDK shim 从 `reqHead.reqKey` + `rsp.fetchData` 字段构建，经 inner manager → framework proxy → `ExecuteInner` IDL → service stub。

```c
// 实现层命名（无下划线）；spec 层用 CloudDisk_CallbackExecuteRequest
typedef struct CallbackExecuteRequest {
    CloudDiskDataBuf reqKey;          // 请求标识，service 据此查 task + fd
    CloudDisk_SyncFolderPath syncFolderPath;  // 同步文件夹路径
    CloudDisk_PathInfo filePath;       // 目标文件相对路径
    CloudDiskCallbackType callbackType; // FETCH_DATA 或 CANCEL_FETCH_DATA（RANGE 拒绝）
    uint64_t offset;                   // 当前分块在文件中的偏移（FETCH_DATA 有效）
    uint64_t size;                     // 当前分块大小（须 == data.dataSize）
    uint64_t totalSize;                // 文件总大小（app 回填，首次回填后校验一致性）
    CloudDiskDataBuf data;             // 分块数据（FETCH_DATA 有效）
    bool isComplete;                   // 是否为末次分块
} CallbackExecuteRequest;
```

Marshalling 顺序：reqKey → syncFolderPath → filePath → callbackType → offset → size → totalSize → data → isComplete。Unmarshalling 逆序，逐项校验。`data` 走 `WriteUInt8Vector`/`ReadUInt8Vector`（binder 支持 ≤4MB vector）。

> 实现细节（Marshalling/Unmarshalling 代码、NDK shim `BuildCallbackExecuteRequest` 校验逻辑、`pwrite` 循环实现）见 Execute SDD §4.1/§5.1/§5.3。

## 2. 存储实现（同 v7，不改动）

v7 §2.1~§2.6（filesyncstate 字节布局、PlaceholderState 编码 0/1/2/3、文件属性与创建、目录级占位聚合、兼容性、customInfo 持久化）全部保留不变。详见 v7 §2 / v6 §2。

> v7 补注（§2.2）保留：v7 水合主体落地 `SetHighBits(2)` 写入路径——首次 `Execute` 的 `pwrite` 成功后翻 `1→2`；末次 `Execute`（`isComplete=true`）的 `pwrite` 成功后翻 `2→3`。

## 3. 错误码（v8 更新）

v7 §3 的全部错误码保留。v8 补充 Execute 路径的错误码语义：

### 3.1 v7 既有错误码（不变）

v7 §3 表（34400016~34400032）全部保留不变。详见 v7 §3。

### 3.2 Execute 错误码映射（v8 补充）

| 触发条件 | 返回码 | 语义 |
| --- | --- | --- |
| Execute：task 不存在（已清理/未创建） | `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`(34400032) | 无在途 task |
| Execute：task = CANCELLED | `OH_CLOUD_DISK_CANCELLED`(34400030) | task 已被取消 |
| Execute：task = COMPLETED | `OH_CLOUD_DISK_ALREADY_HYDRATED`(34400031) | task 已完成 |
| Execute：task = PENDING（极短竞态） | `CLOUD_DISK_TRY_AGAIN`(34400014) | FFRT worker 尚未 dispatch |
| Execute：task = IN_PROGRESS（正常） | `CLOUD_DISK_OK` | pwrite 成功 |
| Execute：pwrite `ENOSPC` | `CLOUD_DISK_NO_SPACE_LEFT`(34400020) | 磁盘满 |
| Execute：pwrite `EINTR` | 自动重试（不返回） | — |
| Execute：pwrite 其他失败 | `CLOUD_DISK_TRY_AGAIN`(34400014) | 暂时性错误 |
| Execute：`size != data.dataSize` | `CLOUD_DISK_INVALID_ARG`(34400001) | 参数不一致 |
| Execute：`size > MAX_EXECUTE_DATA_SIZE` | `CLOUD_DISK_INVALID_ARG`(34400001) | 分块超限 |
| Execute：`offset + size > totalSize` | `CLOUD_DISK_INVALID_ARG`(34400001) | 越界 |
| Execute：`totalSize` 中途变更 | `CLOUD_DISK_INVALID_ARG`(34400001) | 与首次回填不一致 |
| Execute：`callbackType = FETCH_RANGE_DATA` | `CLOUD_DISK_INVALID_ARG`(34400001) | NDK 入口拒绝 |
| Execute：`reqKey` 跨 provider 伪造 | `CLOUD_DISK_CALLBACK_NOT_REGISTERED`(34400021) | bundleName 不匹配 |
| Execute：`reqKey` 跨文件伪造 | `CLOUD_DISK_INVALID_ARG`(34400001) | filePath 不匹配 |
| Execute：`ResolveOwnedSyncFolder` 失败 | `CLOUD_DISK_CALLBACK_NOT_REGISTERED`(34400021) | **安全脱敏**（见 §3.3） |

### 3.3 安全脱敏（v8 新增）

`ExecuteInner` 调 `ResolveOwnedSyncFolder`（`PathToPhysicalPath`→`GetCallerBundleName`→`DentryHash`→`GetSyncFolderValueByIndex`→比对 bundleName）。失败原因可能是：
- 路径不存在 / 非法格式
- `bundleName` 不匹配（跨应用伪造）
- syncFolder 未注册

**对外统一返回 `CLOUD_DISK_CALLBACK_NOT_REGISTERED`(34400021)**，不向调用方暴露失败原因（防探测）。内部 log 区分具体原因，用 `%{private}` 脱敏。

> 与脱水（`DehydrateFile`）路径不同：脱水返 `CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED` 等具体码（脱水是对授权应用的正常调用，暴露失败原因可接受）；Execute 是嵌套回调（在 `OnCallback` 内调），安全脱敏更谨慎。详见 Execute SDD §6。

## 4. 占位符文件接口（同 v7，不改动）

v7 §4.1~§4.7（Create、Is、Convert、Update、Get customInfo、Mark、Unmark）全部保留不变。详见 v7 §4 / v6 §4。

## 5. 水合脱水接口

### 5.1 `OH_CloudDisk_RegisterCallbackTable` / `OH_CloudDisk_UnregisterCallbackTable`（同 v7，不改动）

v7 §5.1 全部保留不变。详见 v7 §5.1。

### 5.2 `OH_CloudDisk_Execute`（v8 更新）

签名不变（v7 §5.2 / v6 §5.2）：

```c
CloudDisk_ErrorCode OH_CloudDisk_Execute(
    const CloudDisk_CallbackReqHead reqHead,
    CloudDisk_CallbackContext reqContext,
    CloudDisk_CallbackResponse rsp);
```

v8 补充规格级细节：

- **IDL 形状（v8 定）**：`void ExecuteInner([in] CallbackExecuteRequest request)`（单参 sequenceable，v7 未定形状）。`CallbackExecuteRequest` 为新增 Parcelable（§1.4），IDL 声明 `sequenceable cloud_disk_common..OHOS.FileManagement.CloudDiskService.CallbackExecuteRequest`。
- **NDK shim（v8 定）**：`OH_CloudDisk_Execute` 的 NDK shim（`BuildCallbackExecuteRequest`）从 `reqHead.reqKey` 提取请求标识、从 `rsp.fetchData` 提取 `offset`/`size`/`totalSize`/`data`/`isComplete`，构建 `CallbackExecuteRequest`，调 `CloudDiskServiceManager::Execute(request)` → `serviceProxy->ExecuteInner(request)`。NDK shim 校验：`callbackType ∈ {FETCH_DATA, CANCEL_FETCH_DATA}`（RANGE 拒绝→`CLOUD_DISK_INVALID_ARG`）、`rsp.fetchData != nullptr`、`size == data.dataSize`、`size <= MAX_EXECUTE_DATA_SIZE`、`offset + size <= totalSize`。
- **`MAX_EXECUTE_DATA_SIZE`（v8 定）**：单次 Execute 分块上限，可配置宏，默认 4MB（`EXECUTE_DATA_SIZE_MB * 1024 * 1024`）。NDK shim + service 双重校验。app 应按此上限分块。
- **`filePath` 来源（v8 定）**：NDK shim 从 `reqContext` 按 `callbackType` 取——FETCH_DATA 时 `reqContext.fetchData->filePath`，CANCEL_FETCH_DATA 时 `reqContext.cancelFetchData`。service 侧用此与 task record 的 filePath 比对（防 reqKey 跨文件伪造）。
- **service 行为（v7 已定 + v8 补充错误码映射）**：`ExecuteInner` → `ResolveOwnedSyncFolder`（失败→`CLOUD_DISK_CALLBACK_NOT_REGISTERED`，安全脱敏 §3.3）→ `IsCallbackRegistered`（未注册→`CLOUD_DISK_CALLBACK_NOT_REGISTERED`）→ `PlaceholderTaskManager::Execute`：查 task by `reqKey` → 校验 `bundleName`/`syncFolderIndex`/`syncFolder`/`filePath` 匹配 → `pwrite(fd, data, size, offset)`（EINTR 重试，ENOSPC→`CLOUD_DISK_NO_SPACE_LEFT`）→ 首次 pwrite 成功翻 state `1→2` → `isComplete=true` 末次 pwrite 成功翻 state `2→3`、`close(outputFd)`、erase task。完整错误码映射见 §3.2。
- **数据回写路径（v7 已定，不变）**：Execute 是独立 IPC（app→service），**不**走 `OnCallback` reply。`WriteCallbackReply`/`ReadCallbackReply` 对 `FETCH_DATA` 维持 no-op。
- **嵌套 IPC 线程安全（v7 已定，不变）**：FFRT worker 阻塞在 `SendCallback`，`ExecuteInner` 由 service 另一 binder 线程处理，无自死锁。
- **分块驱动（v7 已定，不变）**：app 驱动。一次 `OnCallback`/文件，app 自行下载、自行决定 `offset`/`size`，多次调 `Execute`。`totalSize` 由 app 在 Execute 回填。
- **并发模型（v8 定）**：per-task 锁——不同文件的 Execute 可并发（不同 per-task mutex），同文件的 Execute 串行（同 per-task mutex）。`taskMap_` 增删用短临界区 `mapMutex_`，pwrite 只持 per-task 锁，不阻塞其他 task。
- **death recipient（v8 定行为）**：callback table client 崩溃 → `OnRemoteDied` → 标记该 provider 的所有 task 为 CANCELLED → FFRT worker 从 `DispatchFetchData` 返回（`SendRequest` 失败）→ 查 task=CANCELLED → `close(outputFd)` → erase task → file state 保持。PENDING task（未 dispatch）在 worker 取到时感知（callback 不存在→清理）。death recipient 的三表联动实现见 Execute SDD §5.7。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回 §3.2 所列错误码。
- 错误码：见 §3.2。

> v7 变化（§5.2）保留不变。v6 注（FetchDataRequest 请求 vs FetchData 响应严格分立）保留不变。v5 注（脱水不经 Execute，走 OnCallback reply）保留不变。

### 5.3 `OH_CloudDisk_HydratePlaceholder` / `OH_CloudDisk_DehydrateFile`（v8 微调）

签名不变（v7 §5.3 / v6 §5.3）。v8 补充 death recipient 行为明确：

- **death recipient 行为（v8 明确）**：callback table client 崩溃时（不论是 `OnCallback` 执行中还是 PENDING 阶段）：
  - `OnRemoteDied` 触发 → 标记该 provider 的所有 task 为 `CANCELLED`。
  - IN_PROGRESS task：FFRT worker 从 `DispatchFetchData` 返回（`SendRequest` 失败）→ 查 task=CANCELLED → `close(outputFd)` → erase task → file state 保持（1 或 2）。
  - PENDING task：worker 取到时 `GetCallback` 返回 null → `CLOUD_DISK_LISTENER_NOT_REGISTERED` → 标 CANCELLED → 清理。
  - `callbackMap_` 中的 stale 条目由 `OnRemoteDied` 清理（三表联动）。
- 其余 HydratePlaceholder/DehydrateFile 行为（异步触发、状态机、取消语义、在途护栏、优先级排队、FUSE 读安全窗口、totalSize 校验、权限）**全部同 v7，不变**。详见 v7 §5.3。

> v4/v5/v6/v7 注全部保留不变。

## 6. 目录占位聚合（同 v7，不改动）

v7 §6（目录级占位聚合）全部保留不变。详见 v7 §6 / v6 §6。

> v7 补注保留：水合 `1→2→3` 子态切换 delta=0，不触发目录 count 变化。`SetHighBits(2)` 由水合主体首次启用。

## 7. 文件管理器 ArkTS API（v8 新增）

文件管理器不直接调 CAPI，而是通过 NAPI 模块 `file.cloudDiskManager` 的 `CloudDiskSystemAccessor` 类访问 SA。本节规格并入 ArkTS API SDD（`docs/sdd/placeholder-arkts-api-sdd.md`）的规格级决策。

### 7.1 类型定义

```typescript
enum HydratePriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
}

enum HydrateProgressState {
    PENDING = 0,      // task 已创建,FFRT worker 尚未 dispatch
    IN_PROGRESS = 1,
    COMPLETED = 2,
    CANCELLED = 3,    // 用户取消/crash/app 放弃
}

interface HydrateProgress {
    filePath: string;         // 原始绝对路径(与 hydratePlaceholder 传入的一致)
    state: HydrateProgressState;
    processedSize: number;    // 已下载字节数(IN_PROGRESS 时有效)
    totalSize: number;        // 文件总大小
}
```

> v8 变化（PR #35509）：PR 原始版本无 `HydratePriority`、`hydratePlaceholder` 无 `priority` 参、`HydrateProgressState` 有 `IN_PROGRESS`/`COMPLETED`/`FAILED`。v8 改：加 `HydratePriority`、`hydratePlaceholder` 加 `priority` 参、`HydrateProgressState` 删 `FAILED` 加 `PENDING`+`CANCELLED`（对齐 CAPI TaskState）。

### 7.2 CloudDiskSystemAccessor

```typescript
class CloudDiskSystemAccessor {
    constructor();
    on(type: 'hydrateProgress', callback: Callback<HydrateProgress>): void;
    off(type: 'hydrateProgress', callback?: Callback<HydrateProgress>): void;
    hydratePlaceholder(filePath: string, callbackType: CallbackType, priority: HydratePriority): Promise<void>;
    dehydrateFile(filePath: string): Promise<void>;
}
```

- **`@permission`** `ohos.permission.ACCESS_CLOUD_DISK_INFO`，`@systemapi`，`@since 26 dynamic`。
- **构造**：无参。创建 `CloudDiskSystemManager`（ufs inner API）实体。
- **`filePath`**：完整绝对路径（沙箱路径）。SA 内部 `ResolveOwnedSyncFolderByPath` 解析为 (syncFolder, relativePath)。
- **`hydratePlaceholder(filePath, callbackType, priority)`**：`callbackType = FETCH_DATA` 启动水合，`CANCEL_FETCH_DATA` 取消。`priority` 强制传入（file manager 通常用 `HIGH`——用户主动操作）。异步：返回 `Promise<void>`，SA 创建 task 立即返回 resolve。SA 新 IDL `StartHydrationByPathInner(absolutePath, type, priority)` 传绝对路径，SA 内部解析。
- **`dehydrateFile(filePath)`**：返回 `Promise<void>`。SA 新 IDL `DehydrateFileByPathInner(absolutePath)`。
- **`on('hydrateProgress', callback)`**：注册进度回调。SA 侧 `PlaceholderProgressManager` 持有 callback proxy，task 状态变化/cachedSize 增长时经 `ICloudDiskProgressCallback::OnProgress(HydrateProgress)` 推送。全局订阅（收所有文件进度，`filePath` 区分）。时间节流 500ms（暂定可讨论），末次(COMPLETED/CANCELLED)必推。
- **`off('hydrateProgress', callback?)`**：注销指定 callback，无参注销全部。

### 7.3 进度推送机制

- **TaskManager push**：TaskManager 在 task 状态变化（PENDING→IN_PROGRESS、→COMPLETED/CANCELLED）和 Execute pwrite 成功（cachedSize 增长）后调 `ProgressManager::OnTaskProgress(reqKey)`。ProgressManager 读 task record（absolutePath/state/cachedSize/totalSize）→ 节流 → 经 `ICloudDiskProgressCallback` broker 推 `OnProgress` 到 file manager。
- **推送范围**：所有 task（含 CAPI 发起的），file manager 按 `filePath` 自行过滤。
- **death recipient**：progress callback 挂 `SvcDeathRecipient`，file manager 崩溃→`OnRemoteDied`→清理 proxy。
- **`PlaceholderTaskRecord`** 加 `cachedSize`（Execute pwrite 累加）+ `absolutePath`（推送时用原始绝对路径）。
- **totalSize**：首次 fstat logicalSize，Execute 回填后用 app totalSize。

> 本节是对 CAPI 侧 D10"跨进程通知后续单独设计"的落地。详见 ArkTS API SDD。
