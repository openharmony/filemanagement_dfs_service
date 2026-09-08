# CloudDisk 占位符（Placeholder）接口规格 v7

本文件是 v6（`clouddisk-placeholder-v6.md`）的修订版，反映**水合主体（Hydrate Main Body）落地**——v6 标为"另案 SDD、未落地"的水合主体执行路径由 `docs/sdd/placeholder-hydrate-main-sdd.md` 定稿设计，本规格将 SDD 的关键决策并入规格层：`OH_CloudDisk_HydratePlaceholder` 异步触发模型、`OH_CloudDisk_Execute` 独立 IPC 数据回写、状态机 `1→2→3` 推进时机、`CANCEL_FETCH_DATA` 取消语义、per-file 在途护栏、FFRT 优先级排队。v6 的水合优先级增量、v5 的脱水（已落地规格）、v4 的 Mark/Unmark、v3 的 customInfo、v2 的 state 模型全部保留。内部细节不复述：目录占位聚合见 `docs/sdd/placeholder-marking-refactor-sdd.md`；customInfo 各层接线见 `docs/sdd/placeholder-custom-info-sdd.md`；state-only 转换见 `docs/sdd/placeholder-state-only-conversion-sdd.md`；脱水各层接线见 `docs/sdd/placeholder-dehydrate-sdd.md`；水合优先级单点变化见 `docs/sdd/placeholder-hydrate-priority-sdd.md`；水合主体各层接线见 `docs/sdd/placeholder-hydrate-main-sdd.md`。下文中"SDD §x"指 marking-refactor SDD。v6 保留以记录水合主体落地前的规格快照。

> v7 变化：本次为**水合主体落地**——v6 标为"设计态、另案 SDD、未落地"的水合主体由 `docs/sdd/placeholder-hydrate-main-sdd.md` 定稿，本规格将 SDD 决策并入规格层。关键变化：(1) `OH_CloudDisk_HydratePlaceholder` 定为**异步**（创建 task 后立即返回，FFRT worker 派发 OnCallback），与脱水（同步单次 IPC）显著不同；(2) `OH_CloudDisk_Execute` 定为**独立 IPC**（app→service，新 `ExecuteInner` IDL），不走 OnCallback reply（reply 对 FETCH_DATA 维持 no-op），service 侧 `pwrite` 到 `PlaceholderTaskRecord.outputFd`；(3) 状态机 `1→2→3` 推进时机定（首次 `Execute` pwrite→2，末次 `isComplete=true` pwrite→3）；(4) `CANCEL_FETCH_DATA` 取消语义定（标 task CANCELLED + 不发 OnCallback(CANCEL) + app 经 Execute 返回 `E_CANCELLED` 感知 + 不脱水）；(5) per-file 在途护栏启用 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`(34400019)；(6) 新增错误码 `OH_CLOUD_DISK_CANCELLED`(34400030)/`OH_CLOUD_DISK_ALREADY_HYDRATED`(34400031)/`OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`(34400032)；(7) FFRT worker pool（默认 4）+ `PlaceholderTaskManager` 改优先级队列。v6 的优先级增量、结构体约束、脱水/Mark/Unmark/customInfo/state 模型全部不变。

> 事实基线（代码为准，非文档）：v7 编写时 clouddiskservice 实际代码仍停在 **v1 模型**——`cloud_disk_service.cpp` 仍是 v1 两态（`'1'`/`'2'` 字符），独立 `user.clouddisk.placeholder` xattr 仍是唯一占位判据，`filesyncstate` 仍整体存 `SyncState`（无高 3 位/低 5 位拆分）。v2 重构、v3 customInfo、v4 state-only、v5 脱水、v6 优先级、回调表基础设施（PR #4353 `RegisterCallbackTable`/`UnregisterCallbackTable`/`ICloudDiskServiceCallbackTable`/`PlaceholderCallbackManager`/`PlaceholderTaskManager` 骨架）、水合主体（PR #4217 框架参考，未合入且代码陈旧）**均未落地**。PR #4353 提供 callback table 基础设施骨架（未合入），PR #4217 提供水合框架参考（未合入，与 v6/v7 有冲突：无 priority、同步阻塞、cancel 含 ioctl 脱水、无状态机推进、无 Execute）。v7/SDD 如实标注前置依赖未落地，不假装代码已存在。规格链演进不应被代码现状阻塞，但落地须按 SDD 既定顺序逐级实现：①v2 重构先合，②v3 customInfo + v4 state-only 次之，③v5 脱水后叠，④v6 优先级 + 回调表基础设施 + 水合主体最后。

> 水合（hydrate）将占位符填充为完整内容，脱水（dehydrate）将完整文件回退为占位符。**v7 关键差异：脱水为同步单次 IPC（回调往返在 `DehydrateInner` 内同步完成），水合为异步（`HydratePlaceholder` 立即返回，FFRT worker 派发 OnCallback，app 在 OnCallback 内经 `Execute` 独立 IPC 多次回写分块数据）**。水合进度经 `UNHYDRATED`→`PARTIALLY_HYDRATED`→`FULLY_HYDRATED`（1→2→3）推进（高 3 位子态），子态切换不改变占位计数。customInfo 与水合脱水正交：水合/脱水不读写 customInfo xattr。v5 的 Dehydrate 落地、v4 的 Mark/Unmark 与水合脱水亦正交：Mark/Unmark 只翻高 3 位状态、不动数据；脱水丢本地数据并翻 `3→1`（子态切换，不改计数）；水合下载数据并翻 `1→2→3`（子态切换，不改计数）；三者语义互补、并存。v6 的优先级增量与上述全部正交：优先级只随 `FETCH_DATA` 回调透传、不影响 state 模型/count/customInfo/脱水；脱水（`DEHYDRATE` 回调）不携带优先级。

## 1. 结构体

**v7 不改动 §1。** v6 §1.1（`OH_CloudDisk_PlaceholderInfo`）、§1.2（`OH_CloudDisk_PlaceholderCustomInfo`）、§1.3（水合脱水数据契约，含 v6 新增的 `CloudDisk_HydratePriority`/`CloudDisk_FetchDataRequest`/`CloudDisk_FetchData`/`CloudDisk_CallbackResponse`）全部保留不变。详见 v6 §1。

## 2. 存储实现

**v7 不改动 §2。** v6 §2.1（filesyncstate 字节布局）、§2.2（PlaceholderState 编码 0/1/2/3，4-7 reserved）、§2.3（文件属性与创建）、§2.4（目录级占位聚合）、§2.5（兼容性）、§2.6（customInfo 持久化）全部保留不变。详见 v6 §2。

> v7 补注（§2.2）：v2/v6 标"`state==2`（`PARTIALLY_HYDRATED`）仍无写入者，留待水合落地"——**v7 水合主体落地 `SetHighBits(2)` 写入路径**：首次 `Execute` 的 `pwrite` 成功后由 service 调 `placeholder_helper::SetHighBits(PARTIALLY_HYDRATED(2))` 翻 `1→2`；末次 `Execute`（`isComplete=true`）的 `pwrite` 成功后调 `SetHighBits(FULLY_HYDRATED(3))` 翻 `2→3`。`1→2→3` 子态切换 delta=0，不改目录 count、不祖先刷新（§2.2 不变）。

## 3. 错误码

占位符特性随 `@since 26.1.0` 新增的错误码（`34400016`~`34400032`）。"产生接口"列使用简称，对应第 4、5 节接口。

| 枚举名 | 数值 | 语义 | 产生接口 |
| --- | --- | --- | --- |
| `OH_CLOUD_DISK_FILE_ALREADY_EXISTS` | 34400016 | 目标路径已存在同名文件 | Create |
| `OH_CLOUD_DISK_NOT_A_PLACEHOLDER` | 34400017 | 目标路径不是占位符文件 | Convert、Get、Unmark、Dehydrate（v5） |
| `OH_CLOUD_DISK_IS_A_PLACEHOLDER` | 34400018 | 目标路径是占位符文件 | Mark（v4） |
| `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS` | 34400019 | 文件已有在途水合 task（PENDING/IN_PROGRESS） | **HydratePlaceholder（FETCH_DATA）（v7 启用）**、**Dehydrate（v7 在途护栏）** |
| `OH_CLOUD_DISK_NO_SPACE_LEFT` | 34400020 | 磁盘可用空间不足 | Create、Update |
| `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED` | 34400021 | 回调表未注册 | UnregisterCallbackTable、Dehydrate（v5） |
| `OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED` | 34400022 | 回调表已注册 | RegisterCallbackTable |
| `OH_CLOUD_DISK_NOT_A_DIRECTORY` | 34400023 | 目标路径的父路径不是目录 | Create、Convert、Update、Mark、Unmark（v4） |
| `OH_CLOUD_DISK_FILE_NOT_EXIST` | 34400024 | 目标路径不存在 | Is、Get |
| `OH_CLOUD_DISK_NAME_TOO_LONG` | 34400025 | 文件名或路径过长 | Create、Convert、Update、Mark、Unmark（v4） |
| `OH_CLOUD_DISK_FILE_TOO_LARGE` | 34400026 | 文件过大 | Create、Update |
| `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND` | 34400027 | 占位符存在但无 customInfo xattr | Get |
| `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED` | 34400028 | 占位符存在但未完全水合（state 1/2），不可 Unmark/Dehydrate | Unmark（v4）、Dehydrate（v5） |
| `OH_CLOUD_DISK_DEHYDRATE_DENIED` | 34400029 | 应用回调授权拒绝脱水（allow=false） | Dehydrate（v5） |
| `OH_CLOUD_DISK_CANCELLED` | 34400030 | task 已被取消，Execute 拒绝 | **Execute（v7 新增）** |
| `OH_CLOUD_DISK_ALREADY_HYDRATED` | 34400031 | 文件已完全水合（state=3），无需再水合 | **HydratePlaceholder（FETCH_DATA）（v7 新增）** |
| `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS` | 34400032 | 无在途 task，取消/Execute 无对象 | **HydratePlaceholder（CANCEL_FETCH_DATA）、Execute（v7 新增）** |

> v7 变化：
> - `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）由 v6 的"保留，当前无接口产生"改为 **`HydratePlaceholder`（FETCH_DATA）产生**——per-file 在途护栏：`HydratePlaceholder`（FETCH_DATA） 时查 `PlaceholderTaskManager` 同 `(syncFolder,filePath)` 已有 PENDING/IN_PROGRESS task 即返回此码。v2 将 19 由"Convert 产生"改为"保留"是因为 v1 `'2'` 重定义为稳定态后阻塞分支移除；v7 重新启用，产生面变为水合在途护栏（非 Convert）。
> - 新增 `OH_CLOUD_DISK_CANCELLED`（34400030）：`Execute` 产生（task 已被 `CancelHydration` 或 app 崩溃标记 CANCELLED，app 下次调 `Execute` 时 service 返回此码感知取消）。语义为"别再试"（非"暂时性重试"），与 `CLOUD_DISK_TRY_AGAIN` 区分。
> - 新增 `OH_CLOUD_DISK_ALREADY_HYDRATED`（34400031）：`HydratePlaceholder`（FETCH_DATA）产生（文件 `state==3` 且无在途 task，无需再水合）。与 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（19）区分：19 = 有在途 task（进行中），31 = 已完成（无在途 task 但 state 已 3）。
> - 新增 `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`（34400032）：`HydratePlaceholder`（CANCEL_FETCH_DATA）和 `Execute` 产生（无在途 task 可取消/可回写）。取消无对象、Execute 的 task 已被清理（COMPLETED 后自动 erase）均产生此码。
> - 完整错误码决策表见 `docs/sdd/placeholder-hydrate-main-sdd.md` §4.3。
> - 校验类失败（空参、路径非法、`priority` 非法、`syncFolder` 未注册/`bundleName` 不匹配）复用既有通用码 `CLOUD_DISK_INVALID_ARG`/`CLOUD_DISK_SYNC_FOLDER_*`，不新增。
> - 启用前同步：NDK 枚举（`cloud_disk_error_code.h`）新增 30/31/32（19 已存在）；内部枚举（`cloud_disk_service_error.h`）新增 `E_CANCELLED`/`E_ALREADY_HYDRATED`/`E_NO_HYDRATION_IN_PROGRESS`（`E_HYDRATE_IN_PROGRESS` 已存在，补产生者）；NDK↔inner 映射表 `innerToNErrTable` 增 4 行（19/30/31/32）。
> - 连续性：34400030 紧接 v5 的 34400029；v6 明确"不为优先级新增 34400030"是为保持 v3(27)→v4(28)→v5(29) 连续链叙事不被打断——v7 的 30/31/32 是水合主体语义，在 29 之后连续追加，不破坏既有链。

各接口还可能返回通用返回码：`CLOUD_DISK_OK`、`CLOUD_DISK_INVALID_ARG`、`CLOUD_DISK_PERMISSION_DENIED`、`CLOUD_DISK_NOT_SUPPORTED`、`CLOUD_DISK_IPC_FAILED`、`CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED`、`CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED`、`CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST`、`CLOUD_DISK_TRY_AGAIN`（均 `@since 21`，见 `cloud_disk_error_code.h`）。Create/Update/Get 的 customInfo 校验失败均走 `CLOUD_DISK_INVALID_ARG`。Mark/Unmark/Dehydrate/Hydrate 的 sync folder 校验失败走 `CLOUD_DISK_SYNC_FOLDER_*` 系列。HydratePlaceholder 的 `priority` 校验失败走 `CLOUD_DISK_INVALID_ARG`。

## 4. 占位符文件接口

**v7 不改动 §4。** v6 §4.1（Create）、§4.2（Is）、§4.3（Convert）、§4.4（Update）、§4.5（Get customInfo）、§4.6（Mark）、§4.7（Unmark）全部保留不变。详见 v6 §4。

## 5. 水合脱水接口

### 5.1 `OH_CloudDisk_RegisterCallbackTable` / `OH_CloudDisk_UnregisterCallbackTable`

**v7 不改动 §5.1。** v6 §5.1（注册/反注册回调表）全部保留不变。详见 v6 §5.1。

### 5.2 `OH_CloudDisk_Execute`

```c
CloudDisk_ErrorCode OH_CloudDisk_Execute(
    const CloudDisk_CallbackReqHead reqHead,
    CloudDisk_CallbackContext reqContext,
    CloudDisk_CallbackResponse rsp);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `reqHead` | `CloudDisk_CallbackReqHead` | 回调请求头（含 `reqKey`，service 据此查 task + fd） |
| `reqContext` | `CloudDisk_CallbackContext` | 回调上下文，按 `callbackType` 取对应成员（FETCH_DATA 时为 `FetchDataRequest*`，含 filePath+priority；service 已从 task 知悉，不用于 IPC） |
| `rsp` | `CloudDisk_CallbackResponse` | 回调响应，填充 `fetchData` 返回数据 |

- 行为：app 在 `OnCallback` 内调此函数，经**独立 IPC**（app→service，新 `ExecuteInner` IDL）将 `FETCH_DATA` 数据分块回写到 service。NDK shim 从 `reqHead.reqKey` 提取请求标识、从 `rsp.fetchData` 提取 `offset`/`size`/`totalSize`/`data`/`isComplete`，经 inner manager → framework proxy → `ExecuteInner` IDL → service stub → service impl。service 侧用 `reqKey` 从 `PlaceholderTaskManager` 查 task（不存在→`OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`；CANCELLED→`OH_CLOUD_DISK_CANCELLED`；COMPLETED→`OH_CLOUD_DISK_ALREADY_HYDRATED`；PENDING→`CLOUD_DISK_TRY_AGAIN` 极短竞态），从 `PlaceholderTaskRecord.outputFd` 取 fd → `pwrite(fd, data, size, offset)`。首次 `pwrite` 成功翻 state `1→2`；`isComplete=true` 的末次 `pwrite` 成功翻 state `2→3`、close fd、移除 task。
- **数据回写路径**：Execute 是独立 IPC（app→service），**不**走 `OnCallback` 的 reply parcel。`WriteCallbackReply`/`ReadCallbackReply` 对 `FETCH_DATA` 维持 no-op（reply 不承载数据）。这与 `FETCH_RANGE_DATA`（range data 经 reply 回写）和 `DEHYDRATE`（`allow` 经 reply 回写）不同——`FETCH_DATA` 数据量大、需分块多次回写，reply parcel 不适合，故走独立 IPC。
- **嵌套 IPC 线程安全**：FFRT worker 调 `DispatchFetchData` → `SendCallback`（同步 `SendRequest`，FFRT worker 阻塞）；app 在 `OnCallback` 内调 `Execute` → `ExecuteInner` IPC 到达 service，由 service 另一 binder 线程处理（非 FFRT worker 所在线程）。无自死锁（范式同脱水 SDD §6.2）。
- **分块驱动**：app 驱动分块。一次 `OnCallback`/文件，app 自行下载整个文件、自行决定 `offset`/`size`，多次调 `Execute` 交付分块。`totalSize`（云端文件总大小）由 app 在 `Execute` 的 `rsp.fetchData.totalSize` 回填（service 不预填，信任 app）。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_CANCELLED`：task 已被取消（v7 新增，34400030）
  - `OH_CLOUD_DISK_ALREADY_HYDRATED`：task 已完成（v7 新增，34400031）
  - `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`：task 不存在/已清理（v7 新增，34400032）
  - 通用返回码（见第 3 节；`CLOUD_DISK_TRY_AGAIN` 用于 task PENDING 极短竞态，app 可重试）

> v7 变化（§5.2）：v6 §5.2 说"在回调中同步响应请求"——v7 明确 `Execute` 是**独立 IPC**（app→service，新 `ExecuteInner` IDL），**不**走 `OnCallback` reply。v6 的 "Execute 仍仅用于 `FETCH_DATA` 的数据响应（`rsp.fetchData`，`CloudDisk_FetchData`，含 offset/size/totalSize/data/isComplete）"不变——Execute 仍仅用于 FETCH_DATA 数据回写，优先级是入参不经 Execute 回写（v6 不变）。v6 注"FetchDataRequest（请求）与 FetchData（响应）严格分立"不变。本节的 v5 注（脱水不经 Execute，走 OnCallback reply）亦不变。

> v5 注（不变）：`OH_CloudDisk_Execute`/`CloudDisk_CallbackResponse` 仅用于 `FETCH_DATA` 数据回写。脱水（`DEHYDRATE`）的授权决定（`bool allow`）**不**经 `Execute`，而在 `OnCallback` 同步回执包中回写（应用在 `OnCallback` 中填充 `dehydrateData->allow`，由 stub 经 `WriteCallbackReply` 写入回执、服务经 `ReadCallbackReply` 读取），与 `FETCH_RANGE_DATA` 的 range data 回执同机制。

> v6 注（不变）：`Execute`/`CloudDisk_CallbackResponse` 不变。优先级是 `FETCH_DATA` 回调的**入参**（随 `reqContext.fetchData->priority` 由服务→应用携带），**不**经 `Execute` 回写——`Execute` 仍仅用于 `FETCH_DATA` 的数据响应（`rsp.fetchData`，`CloudDisk_FetchData`，含 offset/size/totalSize/data/isComplete）。`FetchDataRequest`（请求，含 priority）与 `FetchData`（响应）严格分立：前者服务→应用、后者应用→服务。应用按 `reqContext.fetchData->priority` 调度下载，完成后经 `Execute` 填 `rsp.fetchData` 回写数据。

### 5.3 `OH_CloudDisk_HydratePlaceholder` / `OH_CloudDisk_DehydrateFile`

```c
CloudDisk_ErrorCode OH_CloudDisk_HydratePlaceholder(
    const CloudDisk_SyncFolderPath *syncFolderPath,
    const CloudDisk_PathInfo *filePath,
    CloudDisk_CallbackType type,
    CloudDisk_HydratePriority priority);  // v6 新增第 4 参

CloudDisk_ErrorCode OH_CloudDisk_DehydrateFile(
    const CloudDisk_SyncFolderPath *syncFolderPath,
    const CloudDisk_PathInfo *filePath);
```

`OH_CloudDisk_HydratePlaceholder`：

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `const CloudDisk_SyncFolderPath *` | 已注册的同步文件夹路径（指针） |
| `filePath` | `const CloudDisk_PathInfo *` | 相对同步文件夹的相对路径（指针） |
| `type` | `CloudDisk_CallbackType` | 请求类型：`FETCH_DATA`（水合）或 `CANCEL_FETCH_DATA`（取消水合）；`FETCH_RANGE_DATA`/`DEHYDRATE` 拒绝 |
| `priority` | `CloudDisk_HydratePriority` | 水合优先级（v6 新增；`LOW/NORMAL/HIGH`，强制显式传入，无默认） |

`OH_CloudDisk_DehydrateFile`（v5 落地）：

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `const CloudDisk_SyncFolderPath *` | 已注册的同步文件夹路径（指针） |
| `filePath` | `const CloudDisk_PathInfo *` | 相对同步文件夹的相对路径（指针） |

- **`HydratePlaceholder` 行为（v7 落地水合主体）**：
  - 入参 `syncFolderPath`/`filePath` 为指针（与 §4.1~§4.7 的 `syncFolderPath`/`relativePathInfo` 值传递不同）。`type` 区分水合（`FETCH_DATA`）与取消（`CANCEL_FETCH_DATA`）。`priority` 强制显式传入，不提供无 priority 重载（C 无默认参数，双入口需两函数名污染 NDK 符号表，不取）。`priority` 取值须在 `LOW(0)/NORMAL(1)/HIGH(2)` 范围，非法返 `CLOUD_DISK_INVALID_ARG`。取消（`CANCEL_FETCH_DATA`）不带优先级：`priority` 仍须传入（签名强制），但取消路径不消费 `priority`（`cancelFetchData` 变体维持 `CloudDisk_PathInfo *`），取消按文件取消（取消该文件所有在途水合，不论优先级）。调用方对取消调用可传 `NORMAL` 占位。
  - **异步触发模型（v7 核心决策）**：`HydratePlaceholder`（FETCH_DATA）创建 task（PENDING）后**立即返回 `CLOUD_DISK_OK`**，不等待 OnCallback 执行或文件下载完成。FFRT worker（固定池，宏 `CLOUD_DISK_HYDRATE_WORKER_COUNT` 默认 4）从 `PlaceholderTaskManager` 优先级队列取 task → 标 IN_PROGRESS → 同步派发 `OnCallback(FETCH_DATA)` 到已注册 callback table（FFRT worker 阻塞在 `SendCallback`）。app 在 `OnCallback` 内下载分块、多次调 `OH_CloudDisk_Execute` 回写数据。与脱水（同步单次 IPC，回调往返在 `DehydrateInner` 内同步完成）显著不同——水合是长时数据传输，异步避免阻塞调用方。
  - **完成通知**：不新增服务侧通知。调用方 = callback table 注册方（同进程），本地追踪 `Execute` 的 `isComplete` 标签感知完成。跨进程场景（如文件管理器调 `HydratePlaceholder`、网盘应用注册 callback table 做下载）暂不考虑，后续单独设计进度回调。
  - **状态机推进（v7 定时机）**：
    - task 状态机：`PENDING`（创建）→ `IN_PROGRESS`（FFRT worker 取 task 派发 OnCallback）→ `COMPLETED`（末次 `Execute` `isComplete=true` 处理完毕）/ `CANCELLED`（被取消或 app 崩溃）。`COMPLETED`/`CANCELLED` 的 task 在处理后从 `taskMap_` 自动移除。
    - 文件 PlaceholderState：首次 `Execute` 的 `pwrite` 成功 → `SetHighBits(2)` 翻 `1→2`（`PARTIALLY_HYDRATED`，首次启用 state 2 写入路径）；末次 `Execute`（`isComplete=true`）的 `pwrite` 成功 → `SetHighBits(3)` 翻 `2→3`（`FULLY_HYDRATED`）。`1→2→3` 子态切换 delta=0，不改目录 count、不祖先刷新（§2.2）。
  - **取消语义（v7 定）**：`HydratePlaceholder`（CANCEL_FETCH_DATA）→ 标 task CANCELLED（不物理 erase、不 close fd）→ 返回 `CLOUD_DISK_OK`。**不**发 `OnCallback(CANCEL_FETCH_DATA)`（避免同步模型下向同一 app 发嵌套 OnCallback 的重入风险）。app 感知取消：下次调 `Execute` 时 service 查 task=CANCELLED → 返回 `OH_CLOUD_DISK_CANCELLED`(34400030) → app 停止下载、从 `OnCallback` 返回。**不**调 `PerformDehydration`/ioctl（脱水是 `DehydrateFile` 职责，取消只停水合）。文件 state：取消后保持当前值（1 或 2），不回退——已下载的数据保留（诚实反映部分本地数据）。`DispatchCancelFetchData`（PR #4353/#4217 已实现）在本异步模型下**不被调用**，标注为"为其他模型预留"。
  - **per-file 在途护栏（v7 启用 34400019）**：`HydratePlaceholder`（FETCH_DATA） 时查 `PlaceholderTaskManager` 同 `(syncFolder,filePath)` 已有 PENDING/IN_PROGRESS task → 返回 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`(34400019)。不同文件可并发（不同 FFRT worker 可同时 dispatch 不同文件的 OnCallback）。`CANCEL_FETCH_DATA` 不受此护栏拦截（取消可对在途 task 进行）。
  - **优先级排队（v6 占位描述 → v7 定结构）**：`PlaceholderTaskManager` 内部改优先级队列（`HIGH`>`NORMAL`>`LOW`，同优先级 FIFO 按创建序，**不抢占**——高优先级不中断低优先级在途请求，仅影响排队顺序）。FFRT worker 按优先级取 task 派发。优先级不持久化：不入 `filesyncstate`/`custominfo` xattr、不进 dentry 占位字段、不进 RDB；仅在 `FETCH_DATA` 回调派发时随 `CloudDisk_FetchDataRequest` 携带。
  - 优先级经回调表透传到应用：服务在派发 `FETCH_DATA` 回调时，将其填入 `reqContext.fetchData->priority`（`CloudDisk_FetchDataRequest`），应用在 `OnCallback` 中读取 `reqContext.fetchData->priority` 据此按优先级调度下载响应。
  - 优先级与 customInfo 正交（不读写 `user.clouddisk.custominfo`）、与脱水正交（`DEHYDRATE` 回调不携带优先级，脱水经 `DehydrateFile` 触发，不经 `HydratePlaceholder`）。
  - **`totalSize` 校验**：信任 app 在 `Execute` 的 `rsp.fetchData.totalSize` 回填的值，service 不校验。`pwrite` 自然延伸文件（超出现有 size 时内核自动扩展）。
  - **权限**：`ResolveOwnedSyncFolder`（bundleName 归属校验）+ `CheckPermissions(PERM_CLOUD_DISK_SERVICE)`，同 Convert/Dehydrate 校验链。
  - **FUSE 读安全窗口**：水合期间文件 state=1（全空洞）或 2（部分数据+空洞），hmdfs FUSE 读直接读文件、读到空洞返回零。本规格**仅标注**，不处理——随 demand hydration（`FETCH_RANGE_DATA` 按需下载）落地一并处理。

- **前置态（HydratePlaceholder FETCH_DATA）**：
  - 高 3 位 `== 0`（`NONE`，普通文件）→ `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`（34400017）。
  - 高 3 位 `== 3`（`FULLY_HYDRATED`）且无在途 task → `OH_CLOUD_DISK_ALREADY_HYDRATED`（34400031，v7 新增）。
  - 高 3 位 `== 1` 或 `== 2` 且无在途 task → 创建 task，返回 `CLOUD_DISK_OK`（state=2 时重新全量下载覆盖，不做断点续传，后续扩展）。
  - 任意 state 且有在途 task（PENDING/IN_PROGRESS）→ `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）。

- **前置态（HydratePlaceholder CANCEL_FETCH_DATA）**：
  - 无在途 task → `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`（34400032，v7 新增）。
  - 有在途 task（PENDING/IN_PROGRESS）→ 标 CANCELLED，返回 `CLOUD_DISK_OK`。

- **`DehydrateFile` 完整行为（v5 落地，v7 补在途水合护栏，其余不变）**：
  - 前置（读 `filesyncstate` 高 3 位）：
    - 高 3 位 `== 3`（`FULLY_HYDRATED`）→ 放行。
    - 高 3 位 `== 1`（`UNHYDRATED`，已未水合）→ 幂等返回 `CLOUD_DISK_OK`（不操作）。
    - 高 3 位 `== 0`（`NONE`，普通文件）→ `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`（34400017）。
    - 高 3 位 `== 2`（`PARTIALLY_HYDRATED`）→ `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028）。
  - **强制回调授权**：服务对已注册 callback table 派发 `DEHYDRATE` 回调（`callbackType = CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE`，`dehydrateData->filePath` = 目标文件相对路径）；已注册 callback table 为硬前置，未注册 → `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`（34400021）。应用在 `OnCallback` 同步回执中填充 `dehydrateData->allow`：
    - `allow == false` → 不脱水、返回 `OH_CLOUD_DISK_DEHYDRATE_DENIED`（34400029），态/数据不变。
    - `allow == true` → 继续。
  - 物理操作（整文件）：`ftruncate(fd, 0)` → `ftruncate(fd, logicalSize)`（重延伸失败重试）→ `SetHighBits(file, UNHYDRATED(1))`（末步翻态 `3→1`）→ `fsync`。`logicalSize` = 当前文件 size（`state==3` ⇒ 数据全本地，二者相等）。数据先丢、state 末步 `3→1`；无瞬态 dehydrating。
  - 不读写 `user.clouddisk.custominfo` xattr（v3 §2.6 正交）；`3→1` 子态切换不改目录 count、不祖先刷新（§2.2）；脱水不携带优先级（v6 正交）。
  - 并发：per-`(syncFolder,filePath)` 串行化锁防并发脱水交错；**在途水合护栏（v7 已落地）**：脱水前查 `PlaceholderTaskManager` 同文件 task → 有 PENDING/IN_PROGRESS task → `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）。
  - 读安全窗口（已知，v5 维持现状）：`ftruncate(0)` 与 `SetHighBits(1)` 之间 state 仍 `==3`（声称完全水合、本地有数据）但数据已被打空；此间 FUSE/hmdfs 读按"本地有数据"直读稀疏空洞 → 读到全零。per-文件串行锁只串行化 `DehydrateFile`、不挡 FUSE 读路径；随 demand hydration 落地与 FUSE 可见性一并处理（见 `docs/sdd/placeholder-hydrate-main-sdd.md` §12）。
- v2/v3/v4/v5/v6/v7 关联：水合推进时高 3 位经 `UNHYDRATED(1)`→`PARTIALLY_HYDRATED(2)`→`FULLY_HYDRATED(3)`；脱水回退为 `UNHYDRATED(1)`。这些子态切换占位语义不变，不触发目录 count 变化。"水合进行中"的检测/阻塞（`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`，34400019）**v7 已落地**——由 per-file 在途护栏产生。水合/脱水不读写 customInfo xattr。v6：优先级只影响 `FETCH_DATA` 派发顺序、不影响 state 推进与 count 契约。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`：目标路径不是占位符文件（state==0）——`Dehydrate`（v5）、`Hydrate`（v7）
  - `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`：文件已有在途水合 task——`Hydrate`（FETCH_DATA）（v7 启用）、`Dehydrate`（v7 在途护栏）
  - `OH_CLOUD_DISK_ALREADY_HYDRATED`：文件已完全水合（state==3），无需再水合——`Hydrate`（FETCH_DATA）（v7 新增）
  - `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`：无在途 task，取消无对象——`Hydrate`（CANCEL_FETCH_DATA）（v7 新增）
  - `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`：占位符存在但未完全水合（state==2）——`Dehydrate`（v5）
  - `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`：回调表未注册（脱水强制授权前置）——`Dehydrate`（v5）
  - `OH_CLOUD_DISK_DEHYDRATE_DENIED`：应用回调授权拒绝脱水（allow=false）——`Dehydrate`（v5 新增）
  - 通用返回码（见第 3 节；`syncFolder` 未注册/`bundleName` 不匹配走 `CLOUD_DISK_SYNC_FOLDER_*`；`ftruncate` 重延伸耗尽走 `CLOUD_DISK_TRY_AGAIN`；`priority` 非法走 `CLOUD_DISK_INVALID_ARG`）

> v4 注（不变）：本节水合/脱水仍为设计态、未落地。v4 的 §4.6 `MarkFileAsPlaceholder` 直接置 `FULLY_HYDRATED(3)`，与本节水合路径（`1→2→3` 推进）在 `0↔{1,2,3}` 计数契约上等价；水合落地时复核二者协调（见 state-only-conversion SDD §13）。

> v5 注（不变）：**脱水（`DehydrateFile`）落地**；水合（`HydratePlaceholder`）仍为设计态、未落地。脱水经回调表强制授权（应用 `allow` 出参决定是否执行），区别于"直接脱水"。脱水各层接线见 `docs/sdd/placeholder-dehydrate-sdd.md`。

> v6 注（不变）：**水合优先级增量**（对外结构体约束草案）。`HydratePlaceholder` 加第 4 参 `CloudDisk_HydratePriority priority`、`CloudDisk_CallbackContext.fetchData` 变体升级为 `CloudDisk_FetchDataRequest *`、新增 `CloudDisk_HydratePriority` 枚举。优先级经回调表透传到应用，SA 内 `PlaceholderTaskManager` 按优先级排队派发 `FETCH_DATA`、不抢占。取消不带优先级。优先级不新增错误码（非法值复用 `CLOUD_DISK_INVALID_ARG`）。水合优先级各层接线见 `docs/sdd/placeholder-hydrate-priority-sdd.md`。

> v7 注：**水合主体落地**。v6 标为"设计态、另案 SDD、未落地"的水合主体由 `docs/sdd/placeholder-hydrate-main-sdd.md` 定稿设计，本规格将 SDD 关键决策并入规格层：(1) 异步触发模型（HydratePlaceholder 立即返回，FFRT worker 池派发 OnCallback）；(2) `OH_CloudDisk_Execute` 定为独立 IPC（app→service，新 `ExecuteInner` IDL），不走 OnCallback reply；(3) 状态机 `1→2→3` 推进时机（首次 pwrite→2，末次 isComplete=true pwrite→3）；(4) 取消语义（标 CANCELLED + 不发 OnCallback(CANCEL) + Execute 返回 E_CANCELLED + 不脱水）；(5) per-file 在途护栏启用 34400019；(6) 新增错误码 34400030/31/32；(7) FFRT worker pool（默认 4）+ 优先级队列。水合主体各层接线（NDK shim / inner manager 虚函数 / IDL `StartHydrationInner`/`CancelHydrationInner`/`ExecuteInner` / framework 编解码 / SA override / `PlaceholderTaskManager` 扩展 / `PlaceholderTaskRecord.outputFd` + close-on-erase / `CloudDiskServiceSyncFile` 改异步 / mock / 测试）见 `docs/sdd/placeholder-hydrate-main-sdd.md`。**前置依赖**：v2 重构 + v3 customInfo + v4 state-only + v5 脱水 + v6 优先级 + 回调表基础设施（PR #4353）均须先落地（见文档头"事实基线"）。

## 6. 目录占位聚合（内部）

**v7 不改动 §6。** v6 §6（目录级占位聚合，`phcount` xattr + 目录 `filesyncstate` 高 3 位 + 增量维护 + recount 兜底）全部保留不变。详见 v6 §6。

> v7 补注：水合 `1→2→3` 子态切换 delta=0，不触发目录 count 变化、不祖先刷新——与 v5 脱水 `3→1` 同契约（§2.2）。`placeholder_helper` 的 `SetHighBits(2)`（state 2 写入路径）由本规格水合主体首次启用。
