# CloudDisk 占位符（Placeholder）接口规格

本文件是 CloudDisk 占位符特性的最终规格，整合 v2~v8 规格演进和已签核的异步水合回调修订，以版本无关的形式呈现。内部实现细节不复述：目录占位聚合见 `docs/sdd/placeholder-marking-refactor-sdd.md`；customInfo 接线见 `docs/sdd/placeholder-custom-info-sdd.md`；state-only 转换见 `docs/sdd/placeholder-state-only-conversion-sdd.md`；脱水接线见 `docs/sdd/placeholder-dehydrate-sdd.md`；水合优先级见 `docs/sdd/placeholder-hydrate-priority-sdd.md`；异步水合主体见 `docs/sdd/placeholder-hydrate-main-sdd.md`；Execute 接线见 `docs/sdd/placeholder-execute-sdd.md`；精确状态查询见 `docs/sdd/placeholder-state-query-capi-sdd.md`。版本间变化见 `clouddisk-placeholder-changelog.md`。本文件与历史 v1~v8 快照冲突时，以本文件为准。

水合（hydrate）将占位符填充为完整内容，脱水（dehydrate）将完整文件回退为占位符。**脱水为同步单次 IPC；水合为异步触发 + 异步 callback 通知 + 独立同步 Execute 回写**：`HydratePlaceholder` 创建 task 后立即返回，FFRT worker 通过 one-way `FETCH_DATA` 通知应用，应用深拷贝请求并立即从 callback 返回，之后异步、多次调用 `Execute` 回写分块。callback 返回不结束 task。水合进度通常经 `UNHYDRATED`→`PARTIALLY_HYDRATED`→`FULLY_HYDRATED`（1→2→3）推进；空文件允许在最终 Execute 中由1直接到3。高 3 位子态切换不改变占位计数。customInfo 与水合脱水正交：水合/脱水不读写 customInfo xattr。Mark/Unmark 只翻高 3 位状态、不动数据；脱水丢本地数据并翻 `3→1`；水合下载数据并翻 `1→2→3`（空文件 `1→3`）；三者语义互补、并存。优先级只随 `FETCH_DATA` 回调透传、不影响 state 模型/count/customInfo/脱水；脱水（`DEHYDRATE` 回调）不携带优先级。

`@since 26.1.0`，clean break（pre-release，无既有消费方二进制兼容负担）。本次修订不新增 CAPI 函数、不改变 callback/Execute 签名或 Parcelable 字段顺序，仅新增容量错误码并修订生命周期语义。spec 层下划线命名 `CloudDisk_*`；实现层 `cloud_disk_common.h` 无下划线 `CloudDisk*`，二者为同一类型（桥接范式）。

## 1. 数据结构

### 1.1 路径与元数据

```c
typedef struct CloudDisk_PathInfo {
    char *value;       // null-terminated 路径字符串
    size_t length;     // 路径长度（不含 '\0'）
} CloudDisk_PathInfo;
typedef CloudDisk_PathInfo CloudDisk_FileIdInfo;
typedef CloudDisk_PathInfo CloudDisk_SyncFolderPath;

typedef struct CloudDisk_PlaceholderInfo {
    uint64_t logicalSize;  // 逻辑文件大小（稀疏空洞大小）
    uint64_t atimeMs;      // 访问时间（毫秒）
    uint64_t mtimeMs;      // 修改时间（毫秒）
} CloudDisk_PlaceholderInfo;

typedef struct CloudDisk_PlaceholderCustomInfo {
    const uint8_t *data;     // 自定义信息原始字节
    size_t dataLength;       // 字节长度，上限 4 KiB
} CloudDisk_PlaceholderCustomInfo;
```

### 1.2 同步状态

`CloudDisk_FileSyncState`（`uint8_t`）：高 3 位 = PlaceholderState（§2.2），低 5 位 = SyncState（文件同步状态，独立于占位语义）。

### 1.3 水合脱水数据契约

```c
typedef struct CloudDisk_DataBuf {
    uint8_t *data;
    uint64_t dataSize;
} CloudDisk_DataBuf;

typedef enum CloudDisk_CallbackType {
    CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA = 0,         // 全文件水合数据请求
    CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA = 1,  // 取消水合
    CLOUD_DISK_CALLBACK_TYPE_FETCH_RANGE_DATA = 2,   // 按需 range 读取（demand hydration，预留）
    CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE = 3,          // 脱水授权请求
} CloudDisk_CallbackType;

typedef enum CloudDisk_HydratePriority {
    CLOUD_DISK_HYDRATE_PRIORITY_LOW = 0,
    CLOUD_DISK_HYDRATE_PRIORITY_NORMAL = 1,
    CLOUD_DISK_HYDRATE_PRIORITY_HIGH = 2,
} CloudDisk_HydratePriority;

typedef struct CloudDisk_CallbackReqHead {
    CloudDisk_SyncFolderPath syncFolderPath;
    CloudDisk_CallbackType callbackType;
    CloudDisk_DataBuf reqKey;       // 请求标识，service 生成，app 透传给 Execute
} CloudDisk_CallbackReqHead;

// FETCH_DATA 回调入参（服务→应用）
typedef struct CloudDisk_FetchDataRequest {
    CloudDisk_PathInfo filePath;
    CloudDisk_HydratePriority priority;
} CloudDisk_FetchDataRequest;

// FETCH_RANGE_DATA 回调入参（预留）
typedef struct CloudDisk_RangeInfo {
    CloudDisk_PathInfo filePath;
    uint64_t offset;
    uint64_t size;
    CloudDisk_DataBuf data;     // app 预分配缓冲，service 在 reply 中回填
} CloudDisk_RangeInfo;

// DEHYDRATE 回调入参（服务→应用）
typedef struct CloudDisk_DehydrateInfo {
    CloudDisk_PathInfo filePath;
    bool allow;     // app 填充：是否允许脱水
} CloudDisk_DehydrateInfo;

typedef union CloudDisk_CallbackContext {
    CloudDisk_FetchDataRequest *fetchData;        // FETCH_DATA
    CloudDisk_PathInfo *cancelFetchData;           // CANCEL_FETCH_DATA
    CloudDisk_RangeInfo *fetchRangeData;           // FETCH_RANGE_DATA（预留）
    CloudDisk_DehydrateInfo *dehydrateData;       // DEHYDRATE
} CloudDisk_CallbackContext;

// FETCH_DATA 响应（app→service，经 Execute 回写）
typedef struct CloudDisk_FetchData {
    uint64_t offset;
    uint64_t size;
    uint64_t totalSize;    // 文件总大小，app 回填
    CloudDisk_DataBuf data;
    bool isComplete;       // 是否为末次分块
} CloudDisk_FetchData;

typedef union CloudDisk_CallbackResponse {
    CloudDisk_FetchData *fetchData;   // 当前仅 FETCH_DATA
} CloudDisk_CallbackResponse;
```

`FETCH_DATA`/`CANCEL_FETCH_DATA` 的 one-way callback 不承载同步 reply。应用在 callback 返回后，
使用深拷贝的请求字段和自有数据构造 `CloudDisk_CallbackResponse`，再调用独立的
`OH_CloudDisk_Execute`。`DEHYDRATE`/`FETCH_RANGE_DATA` 仍在同步 callback 期间写回各自上下文。

### 1.4 Execute 请求 Parcelable

```c
// ExecuteInner IDL 的单参 sequenceable，app 侧 NDK shim 从 reqHead + rsp 构建
typedef struct CallbackExecuteRequest {
    CloudDisk_DataBuf reqKey;
    CloudDisk_SyncFolderPath syncFolderPath;
    CloudDisk_PathInfo filePath;
    CloudDisk_CallbackType callbackType;   // FETCH_DATA 或 CANCEL_FETCH_DATA
    uint64_t offset;
    uint64_t size;                          // 须 == data.dataSize
    uint64_t totalSize;                     // app 回填，首次回填后校验一致性
    CloudDisk_DataBuf data;
    bool isComplete;
} CallbackExecuteRequest;
```

Marshalling 顺序：reqKey → syncFolderPath → filePath → callbackType → offset → size → totalSize → data → isComplete。

## 2. 存储模型

### 2.1 filesyncstate 字节布局

`uint8_t`：高 3 位 = PlaceholderState（§2.2），低 5 位 = SyncState。二者独立读写（`SetHighBits` 只改高 3 位，保留低 5 位）。

### 2.2 PlaceholderState 编码

| 值 | 常量 | 含义 | counted |
| --- | --- | --- | --- |
| 0 | NONE | 普通文件 | 否 |
| 1 | UNHYDRATED | 未水合占位符（稀疏空洞，读零） | 是 |
| 2 | PARTIALLY_HYDRATED | 部分水合占位符（有部分本地数据 + 空洞） | 是 |
| 3 | FULLY_HYDRATED | 完全水合占位符（数据全本地） | 是 |
| 4~7 | — | reserved | — |

子态切换（`1↔2↔3`、`3→1`）delta=0，**不改目录 count、不祖先刷新**。只有 `0↔{1,2,3}`（普通↔占位）触发 count 变化（delta=±1）。

- `SetHighBits(2)`（state 2 写入路径）：首次非空 `Execute` 完成该次必要步骤时由 service 调，翻 `1→2`；最终首块在 `fsync` 后、置 state 3 前短暂经过 state 2。
- `SetHighBits(3)`（state 3 写入路径）：末次 `Execute`（`isComplete=true`）的 `pwrite` 和 `fsync` 均成功后调，通常翻 `2→3`；空文件可翻 `1→3`；`MarkFileAsPlaceholder`（§4.6）直接置 3。
- `SetHighBits(1)`（脱水回退）：`DehydrateFile`（§5.3）末步翻 `3→1`。
- `SetHighBits(0)`（取消占位）：`UnmarkPlaceholderFile`（§4.7）翻 `3→0`。

### 2.3 文件属性与创建

占位符文件创建时按 `logicalSize` 建稀疏空洞（`ftruncate`），写入 `atime`/`mtime`，高 3 位置 `UNHYDRATED(1)`。

### 2.4 customInfo 持久化

`user.clouddisk.custominfo` xattr，原始字节，上限 4 KiB。Create 时写、Update 时覆写或保留、Get 时读、Convert/Mark/Unmark/Hydrate/Dehydrate 不读写（正交）。

### 2.5 兼容性

26.1.0 未商用，clean break。`CloudDisk_CallbackContext` union 变体类型（`fetchData` 从 `PathInfo*`→`FetchDataRequest*`）大小不变（指针变体），ABI 兼容。

## 3. 错误码

### 3.1 占位符专属错误码

| 枚举名 | 数值 | 语义 | 产生接口 |
| --- | --- | --- | --- |
| `OH_CLOUD_DISK_FILE_ALREADY_EXISTS` | 34400016 | 目标路径已存在同名文件 | Create |
| `OH_CLOUD_DISK_NOT_A_PLACEHOLDER` | 34400017 | 目标路径不是占位符文件 | Convert、GetPlaceholderCustomInfo、Unmark、Dehydrate、Hydrate |
| `OH_CLOUD_DISK_IS_A_PLACEHOLDER` | 34400018 | 目标路径是占位符文件 | Mark |
| `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS` | 34400019 | 文件已有在途水合 task | Hydrate(FETCH_DATA)、Dehydrate(在途护栏) |
| `OH_CLOUD_DISK_NO_SPACE_LEFT` | 34400020 | 磁盘可用空间不足 | Create、Update、Execute(pwrite ENOSPC) |
| `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED` | 34400021 | 回调表未注册 / 安全脱敏 | UnregisterCallbackTable、Hydrate、Dehydrate、Execute(ResolveOwnedSyncFolder 失败) |
| `OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED` | 34400022 | 回调表已注册 | RegisterCallbackTable |
| `OH_CLOUD_DISK_NOT_A_DIRECTORY` | 34400023 | 目标路径的父路径不是目录 | Create、Convert、Update、Mark、Unmark |
| `OH_CLOUD_DISK_FILE_NOT_EXIST` | 34400024 | 目标路径不存在 | IsPlaceholderFile、GetPlaceholderCustomInfo、GetPlaceholderState |
| `OH_CLOUD_DISK_NAME_TOO_LONG` | 34400025 | 文件名或路径过长 | Create、Convert、Update、Mark、Unmark |
| `OH_CLOUD_DISK_FILE_TOO_LARGE` | 34400026 | 文件过大 | Create、Update |
| `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND` | 34400027 | 占位符存在但无 customInfo xattr | Get |
| `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED` | 34400028 | 占位符存在但未完全水合(state 1/2) | Unmark、Dehydrate |
| `OH_CLOUD_DISK_DEHYDRATE_DENIED` | 34400029 | 应用回调授权拒绝脱水(allow=false) | Dehydrate |
| `OH_CLOUD_DISK_CANCELLED` | 34400030 | task 已被取消，Execute 拒绝 | Execute |
| `OH_CLOUD_DISK_ALREADY_HYDRATED` | 34400031 | 文件已完全水合(state=3)，无需再水合 | Hydrate(FETCH_DATA) |
| `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS` | 34400032 | 无在途 task，取消/Execute 无对象 | Hydrate(CANCEL_FETCH_DATA)、Execute(task 不存在) |
| `OH_CLOUD_DISK_INVALID_PLACEHOLDER_STATE` | 34400033 | 持久化占位状态为保留值 4～7 | GetPlaceholderState |
| `OH_CLOUD_DISK_HYDRATION_TASK_LIMIT_REACHED` | 34400034 | 每应用5个或全局10个常驻水合 task 已满 | Hydrate(FETCH_DATA) |

### 3.2 通用错误码

`CLOUD_DISK_OK`、`CLOUD_DISK_INVALID_ARG`、`CLOUD_DISK_PERMISSION_DENIED`、`CLOUD_DISK_NOT_SUPPORTED`、`CLOUD_DISK_IPC_FAILED`、`CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED`、`CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED`、`CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST`、`CLOUD_DISK_TRY_AGAIN`（均 `@since 21`）。校验类失败（空参、路径非法、`priority` 非法、`syncFolder` 未注册/`bundleName` 不匹配）复用通用码，不新增。

### 3.3 Execute 错误码映射

| 触发条件 | 返回码 |
| --- | --- |
| task 不存在（已完成清理/未创建）且无取消 tombstone | `NO_HYDRATION_IN_PROGRESS`(32) |
| 命中身份和路径匹配的取消 tombstone（TTL 60秒） | `CANCELLED`(30) |
| task = CANCELLED（终态清理竞态窗口） | `CANCELLED`(30) |
| task = COMPLETED（终态清理竞态窗口） | `NO_HYDRATION_IN_PROGRESS`(32) |
| task = PENDING（FETCH 尚未成功投递） | `TRY_AGAIN`(14) |
| task = IN_PROGRESS（正常） | 进入写入流程；本次请求全部必要步骤成功后返回 `OK` |
| pwrite ENOSPC | `NO_SPACE_LEFT`(20) |
| pwrite EINTR | 自动重试（不返回） |
| pwrite 其他失败 | `TRY_AGAIN`(14) |
| fsync / placeholder state xattr 失败 | `TRY_AGAIN`(14)，task 保留 |
| size != data.dataSize / size > 128 KiB / offset+size > totalSize / totalSize 中途变更 | `INVALID_ARG`(1) |
| 非最终零长度请求 | `INVALID_ARG`(1) |
| totalSize > 0 且最终请求 size = 0 | `INVALID_ARG`(1) |
| totalSize = 0 且 offset=0,size=0,isComplete=true | `OK` |
| callbackType = FETCH_RANGE_DATA | `INVALID_ARG`(1)（NDK 入口拒绝） |
| reqKey 跨 provider 伪造 | `CALLBACK_NOT_REGISTERED`(21) |
| reqKey 跨文件伪造 | `INVALID_ARG`(1) |
| ResolveOwnedSyncFolder 失败 | `CALLBACK_NOT_REGISTERED`(21)（安全脱敏） |

### 3.4 安全脱敏

`ExecuteInner` 调 `ResolveOwnedSyncFolder` 失败时（路径不存在 / bundleName 不匹配 / syncFolder 未注册）对外统一返回 `CALLBACK_NOT_REGISTERED`(34400021)，不暴露失败原因（防探测）。内部 log 区分，用 `%{private}` 脱敏。与脱水路径（返 `SYNC_FOLDER_PATH_UNAUTHORIZED` 等具体码）不同——Execute 使用应用保存的异步请求和 service 生成的 opaque reqKey，安全脱敏更谨慎。

## 4. 占位符文件接口

参数 `syncFolderPath` 为已注册同步文件夹路径；`relativePathInfo`/`path` 为相对路径，须落在 syncFolder 范围内。权限走 `bundleName` 归属（`CheckSyncFolderBundleName`）。

### 4.1 OH_CloudDisk_CreatePlaceholder

```c
CloudDisk_ErrorCode OH_CloudDisk_CreatePlaceholder(
    CloudDisk_SyncFolderPath syncFolderPath,
    CloudDisk_PathInfo relativePathInfo,
    CloudDisk_PlaceholderInfo placeholderInfo,
    const CloudDisk_PlaceholderCustomInfo *customInfo);
```

行为：在 syncFolder 内创建占位符文件，按 `logicalSize` 建稀疏空洞，写 `atime`/`mtime`，高 3 位置 `UNHYDRATED(1)`。`customInfo` 非空时写 `user.clouddisk.custominfo` xattr。同名文件存在→`FILE_ALREADY_EXISTS`。count delta=+1。

### 4.2 OH_CloudDisk_IsPlaceholderFile

```c
CloudDisk_ErrorCode OH_CloudDisk_IsPlaceholderFile(
    CloudDisk_SyncFolderPath syncFolderPath,
    CloudDisk_PathInfo path,
    bool *isPlaceholder);
```

行为：读高 3 位，`!= 0` 返回 `true`（不区分子态 1/2/3）。目录路径拒绝。

保留兼容行为：若持久化高 3 位意外为 4～7，本接口仍按 `!= 0` 返回成功和 `true`；严格合法性检查只由 §4.8 新接口执行。

### 4.3 OH_CloudDisk_ConvertPlaceholderToFile

```c
CloudDisk_ErrorCode OH_CloudDisk_ConvertPlaceholderToFile(
    CloudDisk_SyncFolderPath syncFolderPath,
    CloudDisk_PathInfo relativePathInfo);
```

行为：**数据破坏型**——`ftruncate(0)` 后高 3 位置 `NONE(0)`。前置：高 3 位 `!= 0`（任意占位态可转换）。count delta=−1。若需保留数据用 §4.7 `UnmarkPlaceholderFile`。

### 4.4 OH_CloudDisk_UpdatePlaceholder

```c
CloudDisk_ErrorCode OH_CloudDisk_UpdatePlaceholder(
    CloudDisk_SyncFolderPath syncFolderPath,
    CloudDisk_PathInfo relativePathInfo,
    CloudDisk_PlaceholderInfo placeholderInfo,
    const CloudDisk_PlaceholderCustomInfo *customInfo);
```

行为：`ftruncate(0)` → 按 `logicalSize` 重设大小、重写 `atime`/`mtime`、高 3 位置 `UNHYDRATED(1)`。`customInfo` 非空时覆写 xattr；NULL/`dataLength==0` 时保留既有。count delta = `IsPlaceholder(new) − IsPlaceholder(old)`。

### 4.5 OH_CloudDisk_GetPlaceholderCustomInfo

```c
CloudDisk_ErrorCode OH_CloudDisk_GetPlaceholderCustomInfo(
    CloudDisk_SyncFolderPath syncFolderPath,
    CloudDisk_PathInfo relativePathInfo,
    uint8_t *dataBuf,
    size_t *inOutDataLength);
```

行为：读 `user.clouddisk.custominfo` xattr。出参走 caller 预分配：`inOutDataLength` 入参为容量、出参为实际长度。缓冲不足→`INVALID_ARG`（回填 actual）。占位符存在但无 xattr→`PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`(34400027)。

### 4.6 OH_CloudDisk_MarkFileAsPlaceholder

```c
CloudDisk_ErrorCode OH_CloudDisk_MarkFileAsPlaceholder(
    CloudDisk_SyncFolderPath syncFolderPath,
    CloudDisk_PathInfo relativePathInfo);
```

行为：**只翻状态、不动数据**——高 3 位置 `FULLY_HYDRATED(3)`。不 `ftruncate`、不写 `atime`/`mtime`、不写 customInfo。前置：高 3 位 `== 0`（普通文件）；已是占位符→`IS_A_PLACEHOLDER`(34400018)。count delta=+1。同步、单次 IPC、无 callback。

### 4.7 OH_CloudDisk_UnmarkPlaceholderFile

```c
CloudDisk_ErrorCode OH_CloudDisk_UnmarkPlaceholderFile(
    CloudDisk_SyncFolderPath syncFolderPath,
    CloudDisk_PathInfo relativePathInfo);
```

行为：**只翻状态、保留数据**——高 3 位置 `NONE(0)`，不 `ftruncate`。前置：高 3 位 `== 3`（`FULLY_HYDRATED`）。state 1/2→`PLACEHOLDER_NOT_FULLY_HYDRATED`(34400028)。count delta=−1。同步、单次 IPC、无 callback。

### 4.8 OH_CloudDisk_GetPlaceholderState

```c
typedef enum OH_CloudDisk_PlaceholderState {
    OH_CLOUD_DISK_PLACEHOLDER_STATE_NONE = 0,
    OH_CLOUD_DISK_PLACEHOLDER_STATE_UNHYDRATED = 1,
    OH_CLOUD_DISK_PLACEHOLDER_STATE_PARTIALLY_HYDRATED = 2,
    OH_CLOUD_DISK_PLACEHOLDER_STATE_FULLY_HYDRATED = 3,
} OH_CloudDisk_PlaceholderState;

CloudDisk_ErrorCode OH_CloudDisk_GetPlaceholderState(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo,
    OH_CloudDisk_PlaceholderState *state);
```

行为：同步读取 `filesyncstate` 高 3 位的瞬时快照，返回普通文件 `NONE(0)`、未水合 `UNHYDRATED(1)`、部分水合 `PARTIALLY_HYDRATED(2)`或完全水合 `FULLY_HYDRATED(3)`。xattr 不存在视为 `NONE` 并返回成功；4～7 为保留值，返回 `INVALID_PLACEHOLDER_STATE`(34400033)。

`state` 不得为 NULL；入口先初始化为 `NONE`，仅返回 `CLOUD_DISK_OK` 时结果有效。接口只支持文件，目录拒绝；沿用 `IsPlaceholderFile` 的相对路径、userId、bundleName 和已注册同步目录校验，不要求 `ACCESS_CLOUD_DISK_INFO` 或系统应用身份。不创建任务、不修改 xattr/customInfo/目录 count，不返回任务状态。`@since 26.1.0`。

详细设计见 [Placeholder 状态查询 CAPI SDD](../sdd/placeholder-state-query-capi-sdd.md)。

## 5. 水合脱水接口

本节网盘应用入口与文件管理器系统入口的权限边界不同。`PERM_CLOUD_DISK_SERVICE` 对应 `ohos.permission.ACCESS_CLOUD_DISK_INFO`。

| 入口 | 系统权限 / 系统应用身份要求 | 保留的访问边界 |
| --- | --- | --- |
| 网盘应用 `StartHydrationInner` / `CancelHydrationInner` / `ExecuteInner` / `DehydrateInner` | 不要求上述系统权限，不要求系统应用身份 | 调用者身份、用户隔离、已注册同步目录的 bundleName 归属、路径校验，以及各操作的回调/任务校验 |
| UFS 调用 DFS 的 `RegisterSyncFolderInner` / `UnregisterSyncFolderInner` | 保留 `CheckPermissions(PERM_CLOUD_DISK_SERVICE, true)`；该 helper 当前只检查权限，不检查系统应用身份 | 既有同步目录注册/注销校验不变；这是 UFS→DFS 内部入口，不是 callback table 注册/注销 |
| §7 `CloudDiskSystemAccessor` 的水合/取消、脱水、进度订阅/取消订阅 | 客户端和 SA 均检查上述系统权限与系统应用身份，见 §7.2 | 在当前用户范围内解析已注册目录，不要求文件管理器与网盘 provider 的 bundleName 相同 |

移除网盘侧系统权限要求不等于取消归属校验，也不改变同步目录注册/注销及 ArkTS 系统接口的鉴权。

### 5.1 OH_CloudDisk_RegisterCallbackTable / OH_CloudDisk_UnregisterCallbackTable

```c
CloudDisk_ErrorCode OH_CloudDisk_RegisterCallbackTable(
    CloudDisk_SyncFolderPath syncFolderPath,
    void (*callback)(CloudDisk_CallbackReqHead reqHead,
                     CloudDisk_CallbackContext reqContext));

CloudDisk_ErrorCode OH_CloudDisk_UnregisterCallbackTable(
    CloudDisk_SyncFolderPath syncFolderPath);
```

行为：为 syncFolder 注册/反注册回调表。`callback` 接收 `FETCH_DATA`（水合数据请求）、`CANCEL_FETCH_DATA`（取消水合）、`DEHYDRATE`（脱水授权）回调。同一 syncFolder 重复注册→`CALLBACK_ALREADY_REGISTERED`(34400022)。反注册未注册→`CALLBACK_NOT_REGISTERED`(34400021)。`@since 26.1.0`。

- `FETCH_DATA`/`CANCEL_FETCH_DATA` 是 one-way 通知；`DEHYDRATE`/`FETCH_RANGE_DATA` 继续同步 request/reply。
- callback 的 reqHead、reqContext 及其内部指针均为 SDK 借用内存，只在本次 callback 期间有效。
- 应用需要异步 Execute 时，必须在 callback 返回前手工深拷贝 syncFolder、reqKey、filePath 等字段，并自行管理副本；不新增 Copy/Destroy API。
- callback table client 崩溃时，service death recipient 清理 callback 注册和对应 active task/fd，文件保持未水合或部分水合。
- Unregister 会取消并清理该同步根全部 active task。Unregister 成功返回后不再进入应用 callback；应用将其视为本地异步请求的批量终止信号。

### 5.2 OH_CloudDisk_Execute

```c
CloudDisk_ErrorCode OH_CloudDisk_Execute(
    CloudDisk_CallbackReqHead reqHead,
    CloudDisk_CallbackContext reqContext,
    CloudDisk_CallbackResponse rsp);
```

| 参数 | 说明 |
| --- | --- |
| `reqHead` | 回调请求头（含 `reqKey`，service 据此查 task + fd） |
| `reqContext` | 回调上下文（FETCH_DATA 时为 `FetchDataRequest*`，含 filePath+priority；CANCEL_FETCH_DATA 时为 `PathInfo*`） |
| `rsp` | 回调响应（填充 `fetchData`：offset/size/totalSize/data/isComplete） |

**行为**：应用在 `FETCH_DATA` callback 中深拷贝请求并立即返回，之后使用应用自有内存调用本函数，经独立同步 IPC（`ExecuteInner` IDL，单参 `CallbackExecuteRequest` sequenceable）将分块写回 service。callback 返回不影响 task 生命周期。

- **参数生命周期**：应用构造的 reqHead/reqContext/rsp 及其内部 buffer 在 Execute 返回前必须保持有效且不可修改。NDK 在 inner IPC 前同步深拷贝；返回后 service 不再引用应用内存。
- **数据回写路径**：独立 IPC（app→service），不走 `OnCallback` reply。FETCH callback 本身为 one-way。
- **分块驱动**：app 自行下载并决定 offset/size。下载可以并行，但同一 reqKey 的 Execute 必须由应用串行提交；最终 Execute 必须等待全部非最终 Execute 返回。
- **完整性责任**：service 信任应用的 `isComplete`，不校验 offset 单调、分块重叠或 [0,totalSize) 覆盖。应用必须保证完整数据已经写入。
- **最终块**：`totalSize > 0` 时，`isComplete=true` 的请求必须携带非空数据；`totalSize=0` 的空文件允许 `offset=0,size=0,isComplete=true`。非最终零长度请求非法。
- **`MAX_EXECUTE_DATA_SIZE`**：单次 Execute 数据固定上限为128 KiB（131072字节）；`size` 或 `data.dataSize` 超过该值返回 `INVALID_ARG`。NDK shim和 service 双重校验，应用必须将更大数据拆分为多次 Execute。
- **service 行为**：`ExecuteInner` → `ResolveOwnedSyncFolder`（失败安全脱敏为 `CALLBACK_NOT_REGISTERED`）→ callback 注册校验 → 先查 active task、再查取消 tombstone → 校验 bundleName/syncFolderIndex/syncFolder/filePath → per-task mutex → pwrite。
- **状态和落盘**：首次成功非空写入推进 `1→2`；最终请求严格执行 `pwrite → fsync → 非空文件确保 state=2 → state=3 → COMPLETED progress → close/erase`。只有空文件最终请求允许直接 `state 1→3`。fsync/state 写入失败不得提前暴露 state=3。
- **重试**：pwrite、fsync、xattr 的可重试失败保留 task；ENOSPC→`NO_SPACE_LEFT`，EINTR内部重试，其他→`TRY_AGAIN`。成功且非空的 Execute 刷新5分钟滑动超时。
- **进度**：processedSize按成功请求 size 累加并限制不超过 totalSize；重叠块和成功请求重放由应用避免。
- **完成清理**：COMPLETED 不留 tombstone。最终 Execute 成功后的重复调用返回 `NO_HYDRATION_IN_PROGRESS`。
- **取消查询**：CANCELLED task 清理后留下60秒 tombstone；同一应用、同步根和路径的迟到 Execute 返回 `CANCELLED`。
- **权限**：不调用 `CheckPermissions`，不要求系统应用身份。即使 task 由文件管理器发起，Execute 仍由 provider 以自身身份回写，并通过目录归属、callback和 task/tombstone 身份校验。
- **并发竞态**：Execute 与主动取消/timeout 由 task mutex 线性化，首个完成终态转换者生效。
- 返回：`CLOUD_DISK_OK` 或 §3.3 所列错误码。`@since 26.1.0`。

### 5.3 OH_CloudDisk_HydratePlaceholder / OH_CloudDisk_DehydrateFile

```c
CloudDisk_ErrorCode OH_CloudDisk_HydratePlaceholder(
    const CloudDisk_SyncFolderPath *syncFolderPath,
    const CloudDisk_PathInfo *filePath,
    CloudDisk_CallbackType type,
    CloudDisk_HydratePriority priority);

CloudDisk_ErrorCode OH_CloudDisk_DehydrateFile(
    const CloudDisk_SyncFolderPath *syncFolderPath,
    const CloudDisk_PathInfo *filePath);
```

**HydratePlaceholder**（`@since 26.1.0`）：

| 参数 | 说明 |
| --- | --- |
| `syncFolderPath` | 已注册同步文件夹路径（指针） |
| `filePath` | 相对路径（指针） |
| `type` | `FETCH_DATA`（启动水合）或 `CANCEL_FETCH_DATA`（取消水合）；`FETCH_RANGE_DATA`/`DEHYDRATE` 拒绝 |
| `priority` | 水合优先级（`LOW`/`NORMAL`/`HIGH`，强制显式传入）。取消路径不消费（可传 `NORMAL` 占位） |

- **异步触发**：`HydratePlaceholder(FETCH_DATA)` 在 callback 已注册、路径/state、per-file 护栏和容量校验成功后创建 PENDING task，并立即返回 `CLOUD_DISK_OK`。该成功只表示任务已入队，不表示应用已经接收 callback 或水合成功。
- **one-way 通知**：FFRT worker按优先级取 PENDING task，通过 one-way Binder投递 `FETCH_DATA`。发送被接受后 task 进入 IN_PROGRESS并启动5分钟空闲计时，worker立即处理下一任务，不等待应用 callback 返回。
- **应用行为**：应用在 callback 中手工深拷贝 syncFolder、reqKey、filePath等字段，投递异步下载后立即返回；之后按 §5.2串行 Execute。callback 返回不取消、不清理 task。
- **完成通知**：CAPI应用通过最终 Execute 返回值感知完成。跨进程 file manager通过 §7 ArkTS `hydrateProgress` 感知。
- **状态机**：
  - task：`PENDING` → `IN_PROGRESS` → `COMPLETED` / `CANCELLED`；终态立即关闭 fd并从 active map删除。
  - file：首次成功非空写入推进 `1→2`；最终请求在 pwrite、fsync都成功后推进 `1/2→3`，空文件可走 `1→3`。
- **空闲超时**：FETCH投递成功后开始5分钟滑动计时；每次成功且非空的 Execute刷新，不设绝对总时长。超时走统一取消。
- **active 限额**：按 bundleName统计每应用最多5个，全局最多10个；PENDING/IN_PROGRESS及尚未完成清理且仍持有fd的record占名额。容量满返回 `HYDRATION_TASK_LIMIT_REACHED`(34400034)，HIGH不抢占已有任务。
- **取消**（`CANCEL_FETCH_DATA`）：主动取消、System API取消、空闲超时和投递后的内部终止统一将 task改为CANCELLED、推送终态进度、close/erase，并对已投递FETCH的任务最多发送一次one-way CANCEL。provider自己取消也会收到该通知；PENDING未投递则不发。取消不脱水，file state保持1或2。
- **取消 tombstone**：只为CANCELLED保留60秒；每应用最多16条、全局最多32条。迟到Execute返回CANCELLED；tombstone不占active名额、不阻止同文件立即重新水合。COMPLETED不留tombstone。
- **per-file 在途护栏**：`FETCH_DATA` 时查同 `(syncFolder,filePath)` 已有 PENDING/IN_PROGRESS task → `HYDRATE_IN_PROGRESS`(34400019)。不同文件可并发。`CANCEL` 不受此护栏拦截。
- **优先级排队**：`HIGH`>`NORMAL`>`LOW`，同优先级 FIFO，不抢占。优先级不持久化。
- **前置态**：
  - `FETCH_DATA`：state=0→`NOT_A_PLACEHOLDER`(17)；state=3 且无 task→`ALREADY_HYDRATED`(31)；state=1/2 且无 task→创建 task；有在途 task→`HYDRATE_IN_PROGRESS`(19)。
  - `CANCEL_FETCH_DATA`：无 task→`NO_HYDRATION_IN_PROGRESS`(32)；有 task→标 CANCELLED，返回 `OK`。
- **callback 前置**：provider CAPI和System API均须在创建task前确认对应callback table已注册，否则返回 `CALLBACK_NOT_REGISTERED`。callback注册/注销与task创建按同步根生命周期串行，避免孤儿task。
- **权限**：启动和取消均通过 `ResolvePlaceholderStatePath` 获取调用者 userId/bundleName，再由 `CheckSyncFolderBundleName` 校验已注册目录的归属，并解析、校验文件路径；不调用 `CheckPermissions`，不要求系统应用身份。
- **`totalSize` 可信边界**：首次合法 Execute 提供的 totalSize 作为该 task 的声明总大小，service 信任该值、不与完整覆盖区间比对；后续 Execute 必须保持 totalSize 一致，并继续执行 offset/size 边界校验。
- **FUSE 读安全窗口**：水合期间 state=1（全空洞）或 2（部分数据+空洞），FUSE 读直接读文件、读到空洞返回零。仅标注，随 demand hydration 落地处理。

**生命周期清理**：

- Unregister取消并清理该同步根active task，且成功返回后不再进入应用callback；注销本身是应用侧批量终止信号，不逐task发送CANCEL。
- provider死亡由service death recipient清理callback和active task。
- 用户切换时整体清理active task、callback、progress subscriber和tombstone。SA同一时刻仅服务当前用户，因此限额不增加userId维度。
- SA停止/重启不持久化或恢复task/tombstone，不自动重新注册callback，也不在NDK合成CANCEL。应用从后续IPC/注册/任务错误或自身下载超时清理。

**DehydrateFile**（`@since 26.1.0`）：

- **权限**：`DehydrateInner` 同样通过 `ResolvePlaceholderStatePath` / `CheckSyncFolderBundleName` 保留调用者身份、用户隔离、目录归属和路径校验；不要求上述系统权限或系统应用身份。此校验与下述 `DEHYDRATE` 回调授权是两层独立约束。
- **前置**（读高 3 位）：state=3→放行；state=1→幂等返回 `OK`；state=0→`NOT_A_PLACEHOLDER`(17)；state=2→`PLACEHOLDER_NOT_FULLY_HYDRATED`(28)。
- **强制回调授权**：service 派发 `DEHYDRATE` 回调，app 在 `OnCallback` 同步回执中填 `allow`。`allow=false`→`DEHYDRATE_DENIED`(29)，不脱水。未注册 callback table→`CALLBACK_NOT_REGISTERED`(21)。
- **物理操作**：`ftruncate(fd, 0)` → `ftruncate(fd, logicalSize)`（重延伸失败重试）→ `SetHighBits(UNHYDRATED(1))`（末步翻 `3→1`）→ `fsync`。数据先丢、state 末步翻。
- **并发**：per-file 串行锁防并发脱水；**在途水合护栏**：脱水前查 `PlaceholderTaskManager` 同文件 task → 有 PENDING/IN_PROGRESS → `HYDRATE_IN_PROGRESS`(19)。
- **读安全窗口**：`ftruncate(0)` 与 `SetHighBits(1)` 之间 state 仍 `==3` 但数据已空 → FUSE 读零。随 demand hydration 落地处理。
- customInfo 不读写；`3→1` delta=0，不改 count；不携带优先级。

**关联**：水合 `1→2→3`、脱水 `3→1`，子态切换 delta=0 不改 count。`HYDRATE_IN_PROGRESS`(19) 由 per-file 在途护栏产生（Hydrate + Dehydrate 均查）。水合/脱水不读写 customInfo。优先级只影响 `FETCH_DATA` 派发顺序。

- 错误码：`NOT_A_PLACEHOLDER`(17)、`HYDRATE_IN_PROGRESS`(19)、`ALREADY_HYDRATED`(31)、`NO_HYDRATION_IN_PROGRESS`(32)、`PLACEHOLDER_NOT_FULLY_HYDRATED`(28)、`CALLBACK_NOT_REGISTERED`(21)、`DEHYDRATE_DENIED`(29) + 通用码。

## 6. 目录占位聚合（内部）

内部能力，**不新增对外 IPC/NDK 接口**。

- **数据**：`user.clouddisk.phcount` xattr（目录直接占位子项数，`uint32` 小端）+ 目录 `filesyncstate` 高 3 位（`HAS_PLACEHOLDER`/`NONE`）。
- **维护**：文件占位态变化（create +1 / convert −1 / update 按 before-after / Mark +1 / Unmark −1）沿祖先链增量更新（count RMW 逐级到 syncRoot，仅跨 0 边界翻目录 high3 并上溯）。观察事件删除/重命名经 dentry 占位字段 best-effort 算 delta。recount 兜底修正 best-effort 漂移。
- **子态切换不触发**：`1↔2↔3`（水合）、`3→1`（脱水）delta=0，不触发 count 变化、不祖先刷新。
- **重启鲁棒**：count/high3 在 xattr、dentry 字段在 metafile，全盘持久化。

## 7. 文件管理器 ArkTS API

文件管理器不直接调 CAPI，而是通过 NAPI 模块 `file.cloudDiskManager` 的 `CloudDiskSystemAccessor` 类访问 SA。`@permission ohos.permission.ACCESS_CLOUD_DISK_INFO`，`@systemapi`，`@since 26 dynamic`。

### 7.1 类型

```typescript
enum HydratePriority { LOW = 0, NORMAL = 1, HIGH = 2 }
enum CallbackType { FETCH_DATA = 0, CANCEL_FETCH_DATA = 1 }
enum HydrateProgressState {
    PENDING = 0,      // task 已创建,FFRT worker 尚未 dispatch
    IN_PROGRESS = 1,  // one-way FETCH 已投递,等待异步 Execute
    COMPLETED = 2,
    CANCELLED = 3,    // 主动取消/空闲超时/provider death/内部终止
}
interface HydrateProgress {
    filePath: string;         // 原始绝对路径(与 hydratePlaceholder 传入的一致)
    state: HydrateProgressState;
    processedSize: number;    // 已下载字节数(IN_PROGRESS 时有效)
    totalSize: number;        // 文件总大小
}
```

`HydrateProgressState` 对齐 CAPI TaskState（PENDING/IN_PROGRESS/COMPLETED/CANCELLED，无 FAILED）。可重试的 pwrite/fsync/xattr 失败保持 IN_PROGRESS；主动取消、空闲超时、provider death 和不可恢复内部错误进入 CANCELLED。

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

- **构造**：无参，仅创建访问器；实际操作在方法调用时鉴权。
- **鉴权**：`hydratePlaceholder`（含取消）、`dehydrateFile`、`on`、`off` 在 UFS 客户端 `CloudDiskSystemManager::CheckAccess` 和 DFS SA 的 `CheckSystemAccessorPermission` 两侧校验。先检查 `ACCESS_CLOUD_DISK_INFO`（缺少则返回 201），再检查系统应用身份（不满足则返回 202）。HAP 必须是系统应用；native token 可通过身份检查，但仍须具有该权限；shell token 不放行。
- **路径范围**：`filePath` 为 `/storage/Users/currentUser/` 下的完整绝对路径（沙箱路径）。SA 的 `ResolveSystemAccessorPath` 按当前用户映射物理路径，选择包含目标路径的最长已注册同步目录，解析为 (syncFolder, relativePath)，并使用该目录登记的 provider bundleName；不要求其与文件管理器 bundleName 相同。路径格式非法返回 `INVALID_ARG`(34400001)；用户解析、路径映射或已注册目录解析失败返回 `CALLBACK_NOT_REGISTERED`(34400021)。后续文件状态、I/O 等错误按具体操作返回。
- **`hydratePlaceholder(filePath, callbackType, priority)`**：`FETCH_DATA` 启动水合，`CANCEL_FETCH_DATA` 取消。`priority` 强制传入（file manager 通常 `HIGH`）。异步：callback、路径/state、per-file和容量校验通过并创建task后立即resolve `Promise<void>`；resolve不表示provider已经收到FETCH或水合完成。容量满映射 `HYDRATION_TASK_LIMIT_REACHED`(34400034)。映射到SA IDL `StartHydrationByPathInner(absolutePath, type, priority)`。
- **`dehydrateFile(filePath)`**：返回 `Promise<void>`。映射到 SA IDL `DehydrateFileByPathInner(absolutePath)`。行为同 §5.3 `DehydrateFile`。
- **`on('hydrateProgress', callback)`**：注册当前用户范围内的进度回调（不按文件订阅，由 `filePath` 区分）。SA 侧 `PlaceholderProgressManager` 持 callback proxy，task 状态变化/processedSize增长时经 `ICloudDiskProgressCallback::OnProgress` 推送。processedSize按成功Execute的size累加并限制不超过totalSize；时间节流500ms，末次(COMPLETED/CANCELLED)必推。
- **`off('hydrateProgress', callback?)`**：注销指定 callback，无参注销全部。
- **progress callback death recipient**：file manager 崩溃→`OnRemoteDied`→清理 proxy。
- **推送范围**：仅与订阅者 userId 相同的 task（含 CAPI 发起的），file manager 再按 `filePath` 过滤；不跨用户推送。
- **`PlaceholderTaskRecord`** 内部加 `cachedSize`（整次成功 Execute 后累加）+ `absolutePath`（推送时用原始路径）。
- **totalSize**：首次 fstat logicalSize，Execute 回填后用 app totalSize。

详见 `docs/sdd/placeholder-arkts-api-sdd.md`。
