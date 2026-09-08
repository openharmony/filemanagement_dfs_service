# CloudDisk 占位符规格修订记录（Changelog）

## README：文档导航

本文件是 CloudDisk 占位符（Placeholder）特性的变更日志。以下是与占位符相关的全部设计文档及其用途：

### 规格文档（`docs/agent-knowledge/`）

| 文件 | 用途 |
| --- | --- |
| `clouddisk-placeholder-spec.md` | **完整规格（版本无关）**——整合 v2~v8 及后续确认的异步水合生命周期，以单一文档呈现最终规格。**如需了解"规格是什么"，读此文件。** |
| `clouddisk-placeholder-v1.md` ~ `v8.md` | 版本快照——每版相对前版的修订记录。v1 是初始规格，v2~v8 逐步叠加（v2 重构、v3 customInfo、v4 Mark/Unmark、v5 脱水、v6 优先级、v7 水合主体、v8 Execute 详细 + ArkTS API）。**如需了解"某版改了什么"，读对应 vN 文件。** |
| `clouddisk-placeholder-changelog.md` | **本文件**——按版本记录变化摘要，含每版的关键决策、错误码、ABI、前置依赖。 |

### 详细设计文档（`docs/sdd/`）

SDD 是规格的下级实现设计，定各层接线（NDK/NAPI/inner API/IDL/framework/service/BUILD/test）。

| 文件 | 覆盖版本 | 内容 |
| --- | --- | --- |
| `placeholder-marking-refactor-sdd.md` | v2 | filesyncstate 高 3 位拆分、placeholder_helper、目录 count 聚合、recount 兜底 |
| `placeholder-custom-info-sdd.md` | v3 | customInfo xattr、GetPlaceholderCustomInfo |
| `placeholder-state-only-conversion-sdd.md` | v4 | MarkFileAsPlaceholder / UnmarkPlaceholderFile（只翻状态不动数据） |
| `placeholder-dehydrate-sdd.md` | v5 | DehydrateFile + 回调表强制授权 |
| `placeholder-hydrate-priority-sdd.md` | v6 | HydratePriority enum + FetchDataRequest 结构体约束（纯规格演进，不可执行） |
| `placeholder-hydrate-main-sdd.md` | v7 + 追加设计 | 水合主体：one-way callback、异步 Execute、滑动超时、容量限制、取消通知和 tombstone |
| `placeholder-execute-sdd.md` | v8 + 追加设计 | Execute 数据回写：应用内存所有权、分块校验、重试、最终落盘顺序和终态竞态 |
| `placeholder-arkts-api-sdd.md` | v8 | 文件管理器 ArkTS API：CloudDiskSystemAccessor、进度回调、PlaceholderProgressManager |
| `placeholder-state-query-capi-sdd.md` | 追加设计 | provider CAPI 精确查询占位符四态、保留值错误处理与兼容策略 |

### 阅读建议

- **快速了解最终规格**：读 `clouddisk-placeholder-spec.md`（版本无关）。
- **了解某功能的实现方案**：读对应 SDD。
- **了解版本间差异**：读本文件对应版本的 section。
- **了解代码现状**：读 `clouddisk-placeholder-spec.md` 或最新版 `v8.md` 的"事实基线"段落（标注了哪些已落地、哪些未落地）。

### 仓库分布

- **dfs 仓**（`filemanagement_dfs_service`）：SA 实现、CAPI（NDK）、dfs inner API、framework、IDL、SDD、规格文档。
- **ufs 仓**（`filemanagement_user_file_service`）：ArkTS NAPI 入口、ufs inner API（`CloudDiskPlaceholderManager`/`CloudDiskSystemManager`）。
- **interface_sdk-js 仓**：ArkTS .d.ts 类型定义（PR #35509）。

---

简要记录每版相对前版的变化。详细规格见对应版本文件（`clouddisk-placeholder-v1.md` / `clouddisk-placeholder-v2.md` / `clouddisk-placeholder-v3.md` / `clouddisk-placeholder-v4.md` / `clouddisk-placeholder-v5.md` / `clouddisk-placeholder-v6.md` / `clouddisk-placeholder-v7.md` / `clouddisk-placeholder-v8.md`），内部设计见 `docs/sdd/placeholder-marking-refactor-sdd.md`（v2 重构）、`docs/sdd/placeholder-custom-info-sdd.md`（v3 customInfo）、`docs/sdd/placeholder-state-only-conversion-sdd.md`（v4 state-only 转换）、`docs/sdd/placeholder-dehydrate-sdd.md`（v5 脱水）、`docs/sdd/placeholder-hydrate-priority-sdd.md`（v6 水合优先级）、`docs/sdd/placeholder-hydrate-main-sdd.md`（v7 水合主体）、`docs/sdd/placeholder-execute-sdd.md`（v8 Execute 数据回写）、`docs/sdd/placeholder-arkts-api-sdd.md`（v8 文件管理器 ArkTS API）与 `docs/sdd/placeholder-state-query-capi-sdd.md`（精确状态查询 CAPI）。

## 2026-09-08：水合 callback 与 Execute 异步生命周期修订

本次更新最终规格、水合/Execute/ArkTS SDD及受影响的 ArkTS 使用说明，并在 dfs 仓完成源码与单元测试实现；不修改 v1～v8 历史快照。最终契约见 [完整规格 §5.2～§5.3](clouddisk-placeholder-spec.md)，实现设计见 [水合主体 SDD](../sdd/placeholder-hydrate-main-sdd.md) 和 [Execute SDD](../sdd/placeholder-execute-sdd.md)。

- `FETCH_DATA` / `CANCEL_FETCH_DATA` 改为 one-way 通知。应用在 callback 中深拷贝请求、立即返回，之后使用自有内存异步调用既有 `OH_CloudDisk_Execute`；不新增公开 API。
- 单次 Execute 数据固定上限改为128 KiB（131072字节），超过上限返回 `INVALID_ARG`；NDK和service双重校验，应用将更大数据拆分提交。该约束覆盖 v8 快照中的4MB历史设计。
- callback 返回不再结束或取消 task。task 持续到最终 Execute、主动取消、5分钟滑动空闲超时、callback 死亡、注销、用户切换或 SA 停止。
- 每个应用最多5个 active task、全局最多10个；只按当前 SA 服务用户下的 bundleName 计数。新增 `OH_CLOUD_DISK_HYDRATION_TASK_LIMIT_REACHED = 34400034`，优先级只影响排队，不抢占。
- CANCELLED task 保留60秒轻量 tombstone，每应用最多16条、全局最多32条；COMPLETED 不留 tombstone。迟到 Execute 可区分已取消与不存在任务。
- 已投递 FETCH 的正常取消最多发送一次、best-effort 的 one-way CANCEL；未投递 FETCH、Unregister、接收方死亡、用户切换和 SA 停止按各自生命周期规则清理。
- 最终 Execute 顺序固定为 `pwrite → fsync → 非空文件确保 PARTIALLY_HYDRATED → FULLY_HYDRATED → COMPLETED progress → close/erase`。pwrite/fsync/xattr 的可重试失败保留 task；service 信任应用 `isComplete`，完整分块覆盖由应用保证。
- task、tombstone 和 callback 注册均不持久化；SA 重启不恢复任务，也不由 NDK 合成 CANCEL。应用通过后续 IPC 失败或自身下载超时释放资源。
- 实现验证：`rk3568` 开启 `dfs_service_feature_enable_cloud_disk=true` 后，`clouddisk_service_test` 聚合目标及 `clouddiskservice_sa`、`clouddiskservice_kit_inner`、`ohclouddiskmanager` 三个生产目标均编译链接成功；测试产物为 ARM 二进制，未在无设备环境执行。

## 2026-09-07：新增 Placeholder 精确状态查询 CAPI 设计

本次只完成设计文档，不包含源码、测试、编译、提交或推送。详细方案见 [Placeholder 状态查询 CAPI SDD](../sdd/placeholder-state-query-capi-sdd.md)，最终契约同步到 [spec §4.8](clouddisk-placeholder-spec.md)。

- 新增 `OH_CloudDisk_GetPlaceholderState(syncFolderPath, relativePathInfo, state)` 和 `OH_CloudDisk_PlaceholderState`，精确返回 `NONE(0)`、`UNHYDRATED(1)`、`PARTIALLY_HYDRATED(2)`、`FULLY_HYDRATED(3)`。
- 普通文件及缺少 `filesyncstate` xattr 时返回成功和 `NONE`；查询仅支持文件，返回瞬时快照，不等待任务完成。
- 追加 `OH_CLOUD_DISK_INVALID_PLACEHOLDER_STATE = 34400033`；持久化高 3 位为保留值 4～7 时，新接口返回该错误且出参保持 `NONE`。
- 保留 `OH_CloudDisk_IsPlaceholderFile` 的兼容行为：对 4～7 仍按非零状态返回成功和 `true`。
- 新接口面向网盘 provider，沿用旧查询的 userId、bundleName、注册目录、相对路径及目录拒绝语义；不要求系统权限或系统应用身份。
- 实现期从 `IsPlaceholderFileInner` 抽取 `ResolvePlaceholderQueryPath`，并增加 `QueryPlaceholderStateByXattr`；继续复用 `GetRegisteredMntSyncFolder`、`GetFilePlaceholderState` 和 `IsValidPlaceholderState`，不新增承载类。
- CAPI/错误码/IDL/virtual/导出符号全部末尾追加，不修改 `PlaceholderInfo` 或 Parcelable，不新增源文件或 BUILD 依赖。


## 2026-09-04：同步网盘侧权限修正

对应已完成的 DFS 权限修正：`0239457a895aa0cc23cd70ec81fd37598798b703` → `02cb9a095a0848e5d6e7b803cb0a853f74e5eb16`。本节补记文档同步；历史 v1~v8 快照保持不变，当前权限边界以 [spec §5、§7](clouddisk-placeholder-spec.md) 为准。

- 按维护者确认，从网盘侧 `StartHydrationInner`、`CancelHydrationInner`、`ExecuteInner`、`DehydrateInner` 移除 `CheckPermissions(PERM_CLOUD_DISK_SERVICE, true)`。这些操作不要求 `ohos.permission.ACCESS_CLOUD_DISK_INFO` 或系统应用身份。
- 保留调用者身份解析、userId 隔离、同步目录注册状态、bundleName 归属、路径及回调/任务校验。Execute 目录解析失败仍统一返回 `E_CALLBACK_NOT_REGISTERED`；脱水的 `DEHYDRATE` 回调授权保持不变。
- UFS→DFS 的 `RegisterSyncFolderInner` / `UnregisterSyncFolderInner` 仍检查上述权限。ArkTS `CloudDiskSystemAccessor` 的水合/取消、脱水、进度订阅/取消订阅仍在客户端和 SA 双侧检查权限与系统应用身份（201 / 202）。
- 同步澄清已有 SystemAccessor 行为：在当前用户范围内解析已注册目录，使用该目录登记的 provider，不要求文件管理器与 provider 的 bundleName 相同；进度只推送给相同 userId 的订阅者。这些是既有实现说明，不是此次代码权限修正新增的行为。
- 同步文档：[spec](clouddisk-placeholder-spec.md)、[水合主体 SDD](../sdd/placeholder-hydrate-main-sdd.md)、[Execute SDD](../sdd/placeholder-execute-sdd.md)、[脱水 SDD](../sdd/placeholder-dehydrate-sdd.md)、[ArkTS API SDD](../sdd/placeholder-arkts-api-sdd.md)。
- 回归用例位于 `test/unittests/clouddiskservice/ipc/cloud_disk_service_static_test.cpp`：网盘侧入口断言 `CheckCallerPermission(_)` 调用次数为 0；目录归属失败仍拒绝；合法归属下取消不存在任务返回 `E_NO_HYDRATION_IN_PROGRESS`。
- 实现验证（2026-09-03）：`rk3568` 的 `dfs_service`、`cloudsyncunittests` 在默认配置及 `dfs_service_feature_enable_cloud_disk=true` 配置下，四个编译流程均成功；未在设备上执行 UT。本次仅更新文档，不重新编译。
- 权限修正的增量代码差异见 [13-dfs-provider-permission-fix.diff](../diff/13-dfs-provider-permission-fix.diff)，不含本次文档更新。

## v2（相对 v1）

占位符 xattr 重构：占位态并入 `filesyncstate` 高 3 位，移除独立 `user.clouddisk.placeholder` xattr；新增目录级占位聚合。对外 NDK 接口签名不变。

### 存储模型（§2）
- 占位符状态由独立 `user.clouddisk.placeholder`（char `'1'`/`'2'`/`'0'`）改为 `user.clouddisk.filesyncstate` 高 3 位（数值 `0`–`3`）；v1 独立 xattr 契约废弃、常量移除。
- `PlaceholderState` 4 态：`0 NONE`（普通）/`1 UNHYDRATED`（未水合稀疏 stub）/`2 PARTIALLY_HYDRATED`（部分水合）/`3 FULLY_HYDRATED`（完全水合），`4–7` reserved。`IsPlaceholder = (!=0)`，高位编码不写死。
- v1 的 `'2'`（hydrating-in-progress）重定义为 `PARTIALLY_HYDRATED`（稳定态，非进行中）。

### 目录级占位聚合（§2.4、§6，v2 新增）
- 新增 `user.clouddisk.phcount`（`uint32` 小端，目录直接占位子项数）+ 目录 `filesyncstate` 高 3 位 = `(count>0)?1:0`。
- 不对外暴露（NDK 对目录 Get 返 `NO_SYNC_STATE`）；内部增量维护 + recount 兜底。

### 错误码（§3）
- `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）由“Convert 产生”改为“保留，当前无接口产生”（`'2'` 重定义为稳定态后阻塞分支移除）；枚举值保留，待水合落地后重新产生。
- `OH_CLOUD_DISK_IS_A_PLACEHOLDER`（34400018）维持“保留，当前无接口产生”。

### 接口行为（§4）
- `IsPlaceholder`：底层判据由 `'1'`/`'2'` 改为高 3 位 `!=0`；对外仍返回 bool，不区分子态（行为不变）。
- `Convert`：前置由“xattr 为 `'1'`、`'2'` 不可转换”改为“高 3 位 `!=0` 均可转换”；不再产生 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`。
- `Create`/`Convert`/`Update`：xattr `'1'`/`'0'` 改为高 3 位 `UNHYDRATED(1)`/`NONE(0)`；新增内部祖先 count 增量刷新副作用（见 SDD §6/§7）。
- **行为变化（合并固有，仅占位符）**：`CreatePlaceholder` 现写 filesyncstate → 占位符带上 xattr → `GetFileSyncStates` 对占位符由 `NO_SYNC_STATE` 改返 `IDLE(0)`；**普通文件不受影响**（未调过 Set 的普通文件仍 `NO_SYNC_STATE`，调过的仍返其 `SyncState`）。

### 水合脱水（§5）
- 签名不变。补注水合进度经 `UNHYDRATED→PARTIALLY→FULLY`（`1→2→3`）推进，子态切换不触发 count 变化；“水合进行中”检测延后到水合落地（见 SDD §3.2）。

### 兼容性（§2.5）
- clean break：无真实用户、无迁移、无读回退；已有普通文件高 3 位恒 0 → `NONE`，天然兼容。

### 内部维护（本规格不复述，见 SDD）
- Model B 祖先刷新、recount 兜底、dentry 占位字段（借 `CloudDiskServiceDentry.reserved`）、filesyncstate 高/低位 util、全局锁。

## v3（相对 v2）

占位符自定义信息（customInfo）新增：create/update 各新增一个可选入参，携带调用方自定义、服务不解析的不透明 blob（参考 Windows `CF_PLACEHOLDER_BASIC_INFO` 的 `FileIdentityLength`+`FileIdentity`）；新增读取接口与对应错误码。对外 NDK 接口签名**变更**（Create/Update 加参、新增 Get），因 26.1.0 未商用发布，属 clean break。

### 结构体（§1）
- 新增 `OH_CloudDisk_PlaceholderCustomInfo { size_t dataLength; const uint8_t *data; }`（§1.2），customInfo 承载结构。原 v2 §1.2 水合脱水数据契约顺延为 §1.3。`OH_CloudDisk_PlaceholderInfo`（§1.1）不变。

### 存储模型（§2）
- 新增 `user.clouddisk.custominfo` xattr（§2.6），值为 customInfo 原始字节，4 KiB 上限，NDK 入口与 service 入口双重校验。仅在 `SetPlaceholderFileAttributes`（create/update 汇聚点）写入。
- customInfo 不参与目录占位聚合（§2.4 注）、不进 dentry 占位字段、不进 RDB；与水合脱水正交（水合/脱水不读写该 xattr）。

### 错误码（§3）
- 新增 `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`（34400027，append-only，沿用 `344xxxxx` 前缀），由 `Get` 产生；内部对应 `E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND = 34400027`。
- 校验类失败（`dataLength` 与实际不符、超 4 KiB）复用既有通用码 `CLOUD_DISK_INVALID_ARG`，不新增。

### 接口行为（§4）
- `Create`（§4.1）/`Update`（§4.4）：新增第 4 参数 `const OH_CloudDisk_PlaceholderCustomInfo *customInfo`（可选；NULL 或 `dataLength==0` = 不提供）。
  - Create：非空 → 写 customInfo xattr；不提供 → 不写。
  - Update：非空 → 覆写（原值替换）；不提供 → **保留**旧 customInfo（不写不删）。不提供"显式清空"能力。
  - customInfo 写/不写/保留均不影响占位 count delta 副作用。
- 新增 `Get`（§4.5）`OH_CloudDisk_GetPlaceholderCustomInfo`：读 customInfo xattr；NDK 出参走 caller 预分配（`uint8_t *dataBuf` + `size_t *inOutDataLength`，capacity 入/actual 出），不复用 `OH_CloudDisk_PlaceholderCustomInfo` 作出参（结构体保持 `const`、仅作 Create/Update 入参）。命中 / 缓冲不足（`CLOUD_DISK_INVALID_ARG`）/ 未命中（`OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`）/ 非占位符（`OH_CLOUD_DISK_NOT_A_PLACEHOLDER`）/ 不存在（`OH_CLOUD_DISK_FILE_NOT_EXIST`）。访问校验同 Create/Update。
- `IsPlaceholder`/`Convert` 签名不变，且与 customInfo 无关（Convert 不读写 customInfo）。

### 兼容性（§2.5）
- 26.1.0 未商用：Create/Update 签名变更、新增 Get 与错误码均为 clean break，无既有消费方二进制兼容负担。新增 `user.clouddisk.custominfo` 为全新 key，无迁移、无读回退。

### 内部维护（本规格不复述，见 SDD）
- customInfo 各层接线（inner `PlaceholderCustomInfo` Parcelable、IDL 新增入参/方法、parcel 读写、xattr 写读、mock 元数 `MOCK_METHOD3`→`MOCK_METHOD4`）见 `docs/sdd/placeholder-custom-info-sdd.md`。

### 出参契约（已定）
- Get NDK 出参采用 **Model 3**：caller 预分配 `uint8_t *dataBuf` + `size_t *inOutDataLength`（capacity 入/actual 出）；不复用 `OH_CloudDisk_PlaceholderCustomInfo` 作出参，结构体保持 `const`、仅作 Create/Update 入参；不新增 destroy 函数、无隐藏 malloc、单次 IPC。IDL/inner 仍返回 `PlaceholderCustomInfo`/vector（`[out]` vs 返回 sequenceable 见 SDD §13），NDK shim 拷贝进调用方缓冲。
- 仍待实现期确认：IDL 查询方法 `[out]` vs 返回 sequenceable 的取舍（见 `docs/sdd/placeholder-custom-info-sdd.md` §13）。

## v4（相对 v3）

占位符/普通文件 **state-only 双向转换**新增：新增两个 NDK 接口 `OH_CloudDisk_MarkFileAsPlaceholder`（普通→占位，置 `FULLY_HYDRATED(3)`）与 `OH_CloudDisk_UnmarkPlaceholderFile`（占位→普通，前置 `state==3`，置 `NONE(0)`），均为**只翻状态、不动数据**；既有数据破坏型 `OH_CloudDisk_ConvertPlaceholderToFile`（`ftruncate(0)`）保留，与新接口并存。新增错误码 `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028）。因 26.1.0 未商用，属 clean break。

### 状态语义（§2.2）
- **提前启用 `FULLY_HYDRATED(3)`**：v3 标"2/3 为未来水合预留(clouddiskservice 现无水合路径)"；v4 经 `MarkFileAsPlaceholder`（"标记既有本地文件"路径）写入 3，语义自洽（数据全本地 ⇔ `FULLY_HYDRATED`），与 v3 设想的下载回调路径（`1→2→3` 推进）在 `0↔{1,2,3}` 计数契约上等价，不冲突。`2`（`PARTIALLY_HYDRATED`）仍无写入者，留待水合落地。
- Mark/Unmark 触发祖先 count 刷新（`+1`/`−1`），复用 v2 重构的 `placeholder_helper`（Model B），不引入新刷新逻辑。

### 错误码（§3）
- 新增 `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028，append-only，沿用 `344xxxxx` 前缀），由 `Unmark` 产生（占位符在但 `state∈{1,2}`）；内部枚举新增 `E_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028`（末尾），NDK↔inner 映射表 `innerToNErrTable` 增对应行。
- `OH_CLOUD_DISK_IS_A_PLACEHOLDER`（34400018）由 v3 的"保留，当前无接口产生"改为 **`Mark` 产生**（目标已是占位符）。NDK 枚举 34400018 已存在（reserved，`cloud_disk_error_code.h:173`）；内部枚举（`utils/clouddiskservice/include/cloud_disk_service_error.h`，稀疏）当前 17→19 间空缺、无 `E_IS_A_PLACEHOLDER`，需新增 `E_IS_A_PLACEHOLDER = 34400018`（填空缺，非末尾）+ 映射表增对应行。
- `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）仍维持"保留，当前无接口产生"，待水合落地。
- `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`（34400017）增加 `Unmark` 产生（`state==0`）；`OH_CLOUD_DISK_NOT_A_DIRECTORY`（34400023）/`NAME_TOO_LONG`（34400025）增加 Mark/Unmark 产生。
- 校验类失败（空参、路径非法、`syncFolder` 未注册/`bundleName` 不匹配）复用既有通用码 `CLOUD_DISK_INVALID_ARG`/`CLOUD_DISK_SYNC_FOLDER_*`，不新增。

### 接口行为（§4）
- 新增 `MarkFileAsPlaceholder`（§4.6）：普通文件（`state==0`）→ 占位符（`state==3`）；**不 `ftruncate`、不写 size/atime/mtime、不写 customInfo xattr**；文件当前 size 即 `logicalSize`。前置 `state!=0` → `IS_A_PLACEHOLDER`（34400018）。入参传值（`syncFolderPath`/`relativePathInfo`），不带 `PlaceholderInfo`/`PlaceholderCustomInfo`。
- 新增 `UnmarkPlaceholderFile`（§4.7）：占位符 → 普通文件（`state==0`）；**不 `ftruncate`**（与 `ConvertPlaceholderToFile` 的关键差异，保留本地数据）。前置 `state==0` → `NOT_A_PLACEHOLDER`（34400017）；`state∈{1,2}` → `PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028）；`state==3` 放行。闸门避免 `UNHYDRATED`/`PARTIALLY_HYDRATED` 转"普通"产生"带空洞读零"的伪普通文件。
- 既有 Create/Is/Convert/Update/Get 签名与行为不变；customInfo（§2.6）与 Mark/Unmark 正交——Mark/Unmark 不读写 `user.clouddisk.custominfo` xattr，调用方需 customInfo 另行调既有 `UpdatePlaceholder`。
- 同步、单次 IPC、无 callback、无 `TaskStateManager`；权限走 bundleName 归属（`CheckSyncFolderBundleName`），不走 `ACCESS_CLOUD_DISK_INFO`。

### 兼容性（§2.5）
- 26.1.0 未商用：新增两接口与 34400028 错误码均为 clean break，无既有消费方二进制兼容负担。无新 xattr、无新 Parcelable、无新结构体；既有 `filesyncstate`/`custominfo` xattr 语义不变；既有 `ConvertPlaceholderToFile`（数据破坏型）签名与行为不变，与新接口并存。

### 前置依赖与两份变更集
- **前置依赖 v2 占位符重构**：本设计的 `placeholder_helper`（`SetHighBits` 返回 old + Model B 祖先刷新）来自 v2 重构，未落地时本设计无法实现。两份变更集分先后：①v2 重构（`placeholder-marking-refactor-sdd`，无新 IPC）先合；②本设计（新 IPC/IDL/NDK）后叠。
- **错误码连续性依赖 v3 customInfo**：34400028 紧接 v3 customInfo 的 `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`（34400027，尚未落地）；v3 customInfo 必须先或同期落地，以保持 NDK 枚举 16~34400028、内部枚举 34400027→34400028 连续，不接受 34400026→34400028 的 27 空缺。

### 内部维护（本规格不复述，见 SDD）
- Mark/Unmark 各层接线（NDK shim / inner manager 虚函数 / IDL `MarkFileAsPlaceholderInner`/`UnmarkPlaceholderFileInner` / framework impl / SA override / mock / 测试）见 `docs/sdd/placeholder-state-only-conversion-sdd.md`。
- 测试**仅**落 `test/unittests/clouddiskservice/`（无下划线那棵），不改 `test/unittests/clouddisk_service/`（下划线那棵，另一棵测试树）。

### 后续扩展（见 SDD §13）
- v3 §5.3 的 `HydratePlaceholder`/`DehydrateFile`（带数据下载语义、`1↔2↔3` 子态推进、异步 callback）仍待后续实现；落地时复核与 Mark 直置 state 3 路径的协调。

## v5（相对 v4）

占位符**脱水（Dehydrate）落地**：落地 v4 §5.3 的 `OH_CloudDisk_DehydrateFile`（完全水合占位 → 未水合占位，丢本地数据、保 `logicalSize`），并通过回调表引入"应用授权出参"——服务派发 `DEHYDRATE` 回调，应用在 `OnCallback` 同步回执中回写 `bool allow` 决定是否执行脱水（强制授权：未注册回调表或 `allow=false` 即拒绝脱水）。新增回调类型 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE`、context 变体 `CloudDisk_DehydrateInfo`、错误码 `OH_CLOUD_DISK_DEHYDRATE_DENIED`（34400029）。因 26.1.0 未商用，属 clean break。

### 状态语义（§2.2）
- 脱水落地 `FULLY_HYDRATED(3)` → `UNHYDRATED(1)` 这条子态切换。`3` 与 `1` 均为占位符 → 脱水**不改目录 count、不祖先刷新**（与水合进度切换 `1↔2↔3` 同契约，§2.2）。
- `state==1` 的既有写入者为 `Create`/`Update`（置 1）；脱水是其非创建性写入路径（`3→1`），不与 v4 Mark（`0→3`）冲突。
- `state==2`（`PARTIALLY_HYDRATED`）仍无写入者，留待水合落地。

### 水合脱水数据契约（§1.3）
- `CloudDisk_CallbackType` 新增 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE = 3`（append-only）。
- `CloudDisk_CallbackContext` union 新增 `dehydrateData` 变体（union 大小不变，指针变体）。
- 新增结构体 `CloudDisk_DehydrateInfo { CloudDisk_PathInfo filePath; bool allow; }`（`filePath` = 服务→应用入参，`allow` = 应用→服务出参，应用在 `OnCallback` 中填充）。
- `CloudDisk_CallbackResponse` 不变——脱水决定**不**经 `OH_CloudDisk_Execute`（Execute 仅 `FETCH_DATA` 数据回写），走 `OnCallback` 同步回执包（应用填 `dehydrateData->allow`，stub 经 `WriteCallbackReply` 写入回执、服务经 `ReadCallbackReply` 读取），与 `FETCH_RANGE_DATA` 的 range data 回执同机制。

### 错误码（§3）
- 新增 `OH_CLOUD_DISK_DEHYDRATE_DENIED`（34400029，append-only，沿用 `344xxxxx` 前缀），由 `Dehydrate` 产生（应用回调 `allow=false`）；内部枚举新增 `E_DEHYDRATE_DENIED = 34400029`（末尾），NDK↔inner 映射表 `innerToNErrTable` 增 1 行。
- `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`（34400017）/`OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028）/`OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`（34400021）增加 `Dehydrate` 产生（脱水前置态 `state==0`/`state==2`/无回调表）。
- `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）仍维持"保留，当前无接口产生"，待水合落地（届时由脱水/水合的在途护栏产生）。
- 校验类失败（空参、路径非法、`syncFolder` 未注册/`bundleName` 不匹配）复用既有通用码 `CLOUD_DISK_INVALID_ARG`/`CLOUD_DISK_SYNC_FOLDER_*`，不新增。
- 连续性：34400029 紧接 v4 的 34400028；完整链 v3（27）→v4（28）→v5（29），**v3/v4 必须先或同期落地**以保持连续。

### 接口行为（§5.3）
- 落地 `OH_CloudDisk_DehydrateFile(syncFolderPath*, filePath*)`（§5.3 既定指针入参，与 §4.1~§4.7 值传递不同）。内层 IDL `void DehydrateInner([in] String syncFolder, [in] String relativePath)`（镜像 `ConvertPlaceholderToFileInner`）。
- 前置（读高 3 位）：`==3` 进行 / `==1` 幂等 `E_OK`（不操作）/ `==0`→`NOT_A_PLACEHOLDER`（17）/ `==2`→`PLACEHOLDER_NOT_FULLY_HYDRATED`（28）。
- 强制回调授权：每次脱水派发 `DEHYDRATE` 回调到已注册 callback table，硬前置；未注册→`CALLBACK_NOT_REGISTERED`（21）；`allow=false`→`DEHYDRATE_DENIED`（29）、不脱、态/数据不变。
- 物理操作（整文件）：`ftruncate(0)`→`ftruncate(logicalSize)`（重延伸失败重试）→`SetHighBits(UNHYDRATED(1))`（末步翻态 `3→1`）→`fsync`，全程持 per-文件串行锁。数据先丢、state 末步 `3→1`，无瞬态 dehydrating。
- 不读写 `user.clouddisk.custominfo` xattr（v3 §2.6 正交）；`3→1` 不改 count、不祖先刷新（§2.2）。
- `HydratePlaceholder` 仍为设计态、未落地。

### 并发与读安全（见 dehydrate SDD §6/§13）
- v1 仅 per-`(syncFolder,filePath)` 串行化锁防并发脱水交错；在途水合护栏（查 `PlaceholderTaskManager` 同文件 task → `HYDRATE_IN_PROGRESS`）随水合落地补，不进脱水 v1。
- 读安全窗口（已知，v5 维持现状）：`ftruncate(0)` 与 `SetHighBits(1)` 间 state 仍 `==3`（声称完全水合）但数据已空，FUSE 读按"本地有数据"直读稀疏空洞→读到全零；per-文件锁只串行化 `DehydrateFile`、不挡 FUSE 读路径。随水合落地与 FUSE 可见性一并处理。

### 兼容性（§2.5）
- 26.1.0 未商用：落地 `DehydrateFile` 与新增 `DEHYDRATE` 回调类型、`CloudDisk_DehydrateInfo`、34400029 错误码均为 clean break，无既有消费方二进制兼容负担。`CloudDisk_CallbackContext` union 加变体（union 大小不变）；`CloudDisk_CallbackType` append 值 3；回执包格式对既有 `FETCH_*` 类型不变（按 `callbackType` 分支）。既有 Mark/Unmark/Convert 签名与行为不变，与脱水并存。

### 前置依赖与变更集顺序
- **前置依赖 v2/v3/v4**：v2 占位符重构（合并 `filesyncstate` 高 3 位 + `placeholder_helper` 的 `SetHighBits`/读高3）+ v3 customInfo（34400027 连续）+ v4 state-only（34400028 连续 + state 3 写入路径 Mark）。三者须先/同期落地，脱水后叠；变更集顺序：①v2 重构先合，②v3 customInfo + v4 state-only 次之，③v5 脱水后叠。
- 错误码连续性依赖 v3/v4：34400029 紧接 v4 的 34400028（紧接 v3 的 34400027）；v3/v4 必须先或同期落地，不接受 34400028→34400030 的 29 空缺。

### 内部维护（本规格不复述，见 SDD）
- 脱水各层接线（NDK shim / inner manager 虚函数 `DehydrateFile` / IDL `DehydrateInner` / framework impl / `cloud_disk_common.cpp` 扩 `WriteCallbackParcel`/`WriteCallbackReply`/`ReadCallbackReply`/`IsValidCallbackType` 的 `DEHYDRATE` 分支 / SA override / `PlaceholderCallbackManager::DispatchDehydrate` / mock / 测试）见 `docs/sdd/placeholder-dehydrate-sdd.md`。
- 测试**仅**落 `test/unittests/clouddiskservice/`（无下划线那棵），不改 `test/unittests/clouddisk_service/`（下划线那棵，另一棵测试树）。

### 后续扩展（见 dehydrate SDD §13）
- 水合（`HydratePlaceholder`）落地时：①补在途水合护栏（脱水前查 `PlaceholderTaskManager` 同文件 task→`HYDRATE_IN_PROGRESS`）；②复核与脱水 state 3 写入路径协调；③读安全窗口与 FUSE 对"脱水/水合进行中文件"的可见性一并处理（可能引入瞬态或 FUSE 锁，届时重审脱水物理操作顺序）。
- 拒绝原因细化：若未来调用方需区分"未持久化（重试）vs 在用（放弃）"，可给 `CloudDisk_DehydrateInfo` 加 `reason` 字段；v5 不引（`allow` 为裸 bool）。

## v6（相对 v5）

占位符**水合优先级（Hydrate Priority）增量**：给水合的对外结构体约束加上优先级维度——新增枚举 `CloudDisk_HydratePriority`（三级 `LOW/NORMAL/HIGH`）、新增结构体 `CloudDisk_FetchDataRequest`（`filePath` + `priority`）、将 `CloudDisk_CallbackContext.fetchData` 变体类型由裸 `CloudDisk_PathInfo *` 升级为 `CloudDisk_FetchDataRequest *`、`OH_CloudDisk_HydratePlaceholder` 签名新增第 4 参值传递 `CloudDisk_HydratePriority priority`（强制显式，不提供无 priority 重载）。本次为**纯规格演进**——对外结构体约束草案，供跨领域对齐讨论；**不可执行**，执行前将重新细化设计。因 26.1.0 未商用，属 clean break。

> 事实基线：v6 编写时 clouddiskservice 实际代码停在 v1 模型（`cloud_disk_service.cpp:55-58` 两态 `'1'`/`'2'`，独立 `user.clouddisk.placeholder` xattr 仍是唯一占位判据），v2→v5 规格链、回调表基础设施、`PlaceholderTaskManager`、per-file 串行锁、错误码 27/28/29 **均未落地**。v5 文档称脱水"落地"系规格层自述，与代码不符。v6/SDD 如实标注前置依赖未落地。

### 水合脱水数据契约（§1.3）
- 新增枚举 `CloudDisk_HydratePriority { CLOUD_DISK_HYDRATE_PRIORITY_LOW=0; NORMAL=1; HIGH=2 }`（append-only，值从 0 起，不复用既有码段；不设 URGENT，因不抢占）。
- 新增结构体 `CloudDisk_FetchDataRequest { CloudDisk_PathInfo filePath; CloudDisk_HydratePriority priority; }`（`filePath` 与 `priority` 均为服务→应用入参，应用只读；字段顺序 `filePath` 在前，与 `CloudDisk_DehydrateInfo{filePath; allow}` 一致）。命名与既有 `CloudDisk_FetchData`（响应）形成 Request/Response 对偶。
- `CloudDisk_CallbackContext` union 的 `fetchData` 变体类型由 v5 的 `CloudDisk_PathInfo *` 改为 `CloudDisk_FetchDataRequest *`（union 大小不变，指针变体；`FETCH_DATA` 回调 context 从裸路径升级为请求结构体，承载优先级）。`cancelFetchData` 变体维持 `CloudDisk_PathInfo *`（取消不带优先级）。`fetchRangeData`/`dehydrateData` 变体不变。
- `CloudDisk_CallbackResponse` 不变（优先级是入参、不经 `OH_CloudDisk_Execute` 回写）；`CloudDisk_CallbackType` 不变（不新增回调类型）。
- spec 层下划线命名 `CloudDisk_FetchDataRequest`/`CloudDisk_HydratePriority`；实现层 `cloud_disk_common.h` 无下划线 `CloudDiskFetchDataRequest`/`CloudDiskHydratePriority`（桥接范式同 v5 `CloudDisk_DehydrateInfo`/`CloudDiskDehydrateInfo`，见脱水 SDD §5.2）。

### 状态语义（§2.2）
- 优先级**不**并入 `filesyncstate` 字节、不进 xattr、不进 dentry 占位字段、不进 RDB——优先级是 `FETCH_DATA` 回调的运行期入参，仅在回调派发时随 `CloudDisk_FetchDataRequest` 携带，不持久化。
- 水合主体落地时 state 推进仍走 `1→2→3`（另案 SDD），优先级只影响 `FETCH_DATA` 派发顺序、不影响子态切换的 count 契约（`1↔2↔3` delta=0 不变）。
- 优先级不触发祖先 count 刷新（无 delta）。

### 错误码（§3）
- **不新增错误码**。优先级值非法（`priority` 不在 `LOW/NORMAL/HIGH`）属入参校验类失败，复用既有通用码 `CLOUD_DISK_INVALID_ARG`（service 侧 `E_INVALID_ARG`），与 customInfo 超限、路径非法同类。NDK 枚举、内部枚举、`innerToNErrTable` 映射表均不动。
- SA 内排队调度不产生失败码（排队只影响派发顺序，非错误）。
- `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）仍维持"保留，当前无接口产生"，待水合主体落地由在途护栏产生。
- 不为优先级新增 34400030，以保持 v3（27）→v4（28）→v5（29）连续链叙事不被打断；下一个专属码留给真正的新语义。

### 接口行为（§5.3）
- `OH_CloudDisk_HydratePlaceholder` 签名加第 4 参 `CloudDisk_HydratePriority priority`（值传递，强制显式，不提供无 priority 重载——C 无默认参数，双入口需两函数名污染 NDK 符号表，不取）。`priority` 取值须在 `LOW/NORMAL/HIGH` 范围，非法返 `CLOUD_DISK_INVALID_ARG`。
- 优先级经回调表透传到应用：服务派发 `FETCH_DATA` 回调时填 `reqContext.fetchData->priority`（`CloudDisk_FetchDataRequest`），应用在 `OnCallback` 中读取据此按优先级调度下载响应；完成后经 `OH_CloudDisk_Execute` 填 `rsp.fetchData`（`CloudDisk_FetchData`）回写数据。`FetchDataRequest`（请求，服务→应用）与 `FetchData`（响应，应用→服务）严格分立。
- SA 内语义（占位描述，任务管理器结构属水合主体另案）：`PlaceholderTaskManager` 按优先级排队派发 `FETCH_DATA` 回调（`HIGH`→`NORMAL`→`LOW`），**不抢占**——高优先级不中断低优先级在途请求，仅影响排队顺序。
- 取消（`CANCEL_FETCH_DATA`）**不带优先级**：`cancelFetchData` 变体维持 `CloudDisk_PathInfo *`，取消按文件取消（取消该文件所有在途水合，不论优先级）；`priority` 仍须传入（签名强制），取消路径不消费，调用方对取消调用可传 `NORMAL` 占位。
- `OH_CloudDisk_Execute`/`CloudDisk_CallbackResponse` 不变（优先级是入参，不经 Execute 回写）。
- 水合主体（`HydratePlaceholder` 的状态机 `1→2→3` 推进、`CANCEL_FETCH_DATA` 取消语义、回调往返、`PlaceholderTaskManager` 任务管理、并发护栏）仍为设计态、另案 SDD、未落地。v6 只给水合的对外结构体约束加上优先级。

### 兼容性（§2.5）
- 26.1.0 未商用：新增 `CloudDisk_HydratePriority`/`CloudDisk_FetchDataRequest`、`CloudDisk_CallbackContext.fetchData` 变体类型变、`HydratePlaceholder` 签名加第 4 参，均为 clean break，无既有消费方二进制兼容负担。`CloudDisk_CallbackContext` union 变体类型变（union 大小不变，指针变体）；既有 Mark/Unmark/Convert/Dehydrate 签名与行为不变；`cancelFetchData`/`fetchRangeData`/`dehydrateData` 变体不变；`CloudDisk_CallbackType`/`CloudDisk_CallbackResponse`/`CloudDisk_FetchData` 不变。回执包格式对既有 `CANCEL_FETCH_DATA`/`FETCH_RANGE_DATA`/`DEHYDRATE` 类型不变；`FETCH_DATA` 请求侧 context 由裸路径升级为 `FetchDataRequest`（响应侧 `FetchData` 与 `Execute` 不变）。

### 前置依赖与变更集顺序
- **前置依赖 v2/v3/v4/v5 + 回调表 + 水合主体**：v2 占位符重构（`filesyncstate` 高 3 位 + `placeholder_helper` 的 `SetHighBits`/读高3）+ v3 customInfo（34400027 连续）+ v4 state-only（34400028 连续 + state 3 写入路径 Mark）+ v5 脱水（34400029 + 回调表强制授权 + `placeholder_callback_manager.{h,cpp}`）+ 回调表基础设施（`ICloudDiskServiceCallbackTable`/`OnCallback`/`WriteCallbackParcel`/`WriteCallbackReply`/`ReadCallbackReply`）+ 水合主体（`PlaceholderTaskManager`/状态机推进/取消/回调往返）。均须先落地，本设计后叠。变更集顺序：①v2 重构先合，②v3 customInfo + v4 state-only 次之，③v5 脱水后叠，④水合主体 + v6 优先级最后。
- v6 是纯规格演进、不可执行，执行前将重新细化设计；落地须按上述顺序逐级实现。

### 内部维护（本规格不复述，见 SDD）
- 水合优先级各层接线（仅对外结构体约束层：NDK 头 `HydratePlaceholder` 声明加第 4 参 + inner `cloud_disk_common.h` 新增 `CloudDiskFetchDataRequest`/`CloudDiskHydratePriority` + `CloudDiskCallbackContext.fetchData` 变体类型改）见 `docs/sdd/placeholder-hydrate-priority-sdd.md`。framework 编解码 / IDL / service / mock / test 归水合主体 SDD（本设计不写，因依赖未落地的回调表与水合主体）。
- 测试**仅**落 `test/unittests/clouddiskservice/`（无下划线那棵），不改 `test/unittests/clouddisk_service/`（下划线那棵，另一棵测试树）；本设计不落测试，等水合主体 SDD。

### 后续扩展（见 hydrate-priority SDD §13）
- 水合主体落地（另案 SDD）：state 推进 `1→2→3`、`PlaceholderTaskManager` 任务结构、在途护栏（`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`，34400019）、`CANCEL_FETCH_DATA` 取消语义、framework 编解码、IDL `HydratePlaceholderInner`、service override、`DispatchFetchData` 改造、mock、测试。本设计的对外结构体约束在水合主体落地时直接消费。
- 抢占调度：若未来需"高优先级中断低优先级在途"，需处理在途任务取消 + state 回退 + 已下载数据清理，可能新增 `URGENT` 或独立抢占接口；v6 不取。
- 取消带优先级：若未来需"取消特定优先级在途"，升级 `cancelFetchData` 变体为 `FetchDataRequest *`；v6 不取。
- 流读（`FETCH_RANGE_DATA`）与优先级：当前流读不带优先级；若未来需要，可给 `RangeInfo` 加 `priority` 或新增 `FetchRangeDataRequest`；v6 不取（流读延后）。

## v7（相对 v6）

> 本节记录 v7 当时的设计快照；与 2026-09-08 异步生命周期修订冲突的内容，以本文件顶部修订记录和最终 spec 为准。

占位符**水合主体（Hydrate Main Body）落地**：v6 标为"设计态、另案 SDD、未落地"的水合主体由 `docs/sdd/placeholder-hydrate-main-sdd.md` 定稿设计，本规格将 SDD 关键决策并入规格层。核心变化：(1) `OH_CloudDisk_HydratePlaceholder` 定为**异步**（创建 task 后立即返回，FFRT worker 池派发 OnCallback），与脱水（同步单次 IPC）显著不同；(2) `OH_CloudDisk_Execute` 定为**独立 IPC**（app→service，新 `ExecuteInner` IDL），不走 OnCallback reply（reply 对 FETCH_DATA 维持 no-op），service 侧 `pwrite` 到 `PlaceholderTaskRecord.outputFd`；(3) 状态机 `1→2→3` 推进时机定（首次 `Execute` pwrite→2，末次 `isComplete=true` pwrite→3）；(4) `CANCEL_FETCH_DATA` 取消语义定（标 task CANCELLED + 不发 OnCallback(CANCEL) + app 经 Execute 返回 `E_CANCELLED` 感知 + 不脱水）；(5) per-file 在途护栏启用 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`(34400019)；(6) 新增错误码 34400030/31/32；(7) FFRT worker pool（默认 4）+ `PlaceholderTaskManager` 改优先级队列。因 26.1.0 未商用，属 clean break。

### 触发模型（§5.3，v7 核心决策）

- **异步触发**：`HydratePlaceholder`（FETCH_DATA）创建 task（PENDING）后**立即返回 `CLOUD_DISK_OK`**。FFRT worker（固定池，宏 `CLOUD_DISK_HYDRATE_WORKER_COUNT` 默认 4）从 `PlaceholderTaskManager` 优先级队列取 task → 标 IN_PROGRESS → 同步派发 `OnCallback(FETCH_DATA)`（FFRT worker 阻塞在 `SendCallback`）。app 在 `OnCallback` 内下载分块、多次调 `OH_CloudDisk_Execute` 回写数据。
- 与脱水（同步单次 IPC，回调往返在 `DehydrateInner` 内同步完成）显著不同——水合是长时数据传输，异步避免阻塞调用方。
- **完成通知**：不新增服务侧通知。调用方 = callback table 注册方（同进程），本地追踪 `Execute` 的 `isComplete` 标签感知完成。跨进程场景（如文件管理器调 HydratePlaceholder）后续单独设计进度回调。

### Execute 数据回写（§5.2，v7 定机制）

- v6 §5.2 说"在回调中同步响应请求"——v7 明确 `OH_CloudDisk_Execute` 是**独立 IPC**（app→service，新 `ExecuteInner` IDL），**不**走 `OnCallback` reply（`WriteCallbackReply`/`ReadCallbackReply` 对 FETCH_DATA 维持 no-op）。
- NDK shim 从 `reqHead.reqKey` 提取请求标识、从 `rsp.fetchData` 提取 `offset`/`size`/`totalSize`/`data`/`isComplete`，经 inner manager → framework proxy → `ExecuteInner` IDL → service stub → service impl。service 用 `reqKey` 从 `PlaceholderTaskManager` 查 task + 从 `PlaceholderTaskRecord.outputFd` 取 fd → `pwrite(fd, data, size, offset)`。
- **嵌套 IPC 线程安全**：FFRT worker 阻塞在 `SendCallback`，`ExecuteInner` 由 service 另一 binder 线程处理，无自死锁（范式同脱水 SDD §6.2）。
- **分块驱动**：app 驱动。一次 `OnCallback`/文件，app 自行下载整个文件、自行决定 `offset`/`size`，多次调 `Execute` 交付分块。`totalSize` 由 app 在 `Execute` 的 `rsp.fetchData.totalSize` 回填（service 不预填，信任 app，不校验）。
- v6 的"Execute 仍仅用于 FETCH_DATA 数据回写"不变；v5 的"脱水不经 Execute，走 OnCallback reply"不变；v6 的"FetchDataRequest（请求）与 FetchData（响应）严格分立"不变。

### 状态语义（§2.2，v7 首次启用 state 2 写入路径）

- v2/v6 标"`state==2`（`PARTIALLY_HYDRATED`）仍无写入者，留待水合落地"——**v7 水合主体落地 `SetHighBits(2)` 写入路径**：首次 `Execute` 的 `pwrite` 成功后翻 `1→2`；末次 `Execute`（`isComplete=true`）的 `pwrite` 成功后翻 `2→3`。`1→2→3` 子态切换 delta=0，不改目录 count、不祖先刷新（§2.2 不变）。
- task 状态机（v7 新增）：`PENDING`（创建）→ `IN_PROGRESS`（FFRT worker 派发 OnCallback）→ `COMPLETED`（末次 Execute isComplete=true）/ `CANCELLED`（被取消或 app 崩溃）。COMPLETED/CANCELLED 的 task 自动从 taskMap_ 移除。

### 取消语义（§5.3，v7 定）

- `HydratePlaceholder`（CANCEL_FETCH_DATA）→ 标 task CANCELLED（不物理 erase、不 close fd）→ 返回 `CLOUD_DISK_OK`。
- **不**发 `OnCallback(CANCEL_FETCH_DATA)`（避免同步模型下向同一 app 发嵌套 OnCallback 的重入风险）。app 经 `Execute` 返回 `OH_CLOUD_DISK_CANCELLED`(34400030) 感知取消。
- **不**调 `PerformDehydration`/ioctl（脱水是 `DehydrateFile` 职责，取消只停水合）。
- 文件 state：取消后保持当前值（1 或 2），不回退——已下载的数据保留。
- `DispatchCancelFetchData`（PR #4353/#4217 已实现）在本异步模型下**不被调用**，标注为"为其他模型预留"。

### 错误码（§3）

- `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）由 v6 的"保留，当前无接口产生"改为 **`HydratePlaceholder`（FETCH_DATA）产生**——per-file 在途护栏：同 `(syncFolder,filePath)` 已有 PENDING/IN_PROGRESS task 即返回此码。脱水亦查此护栏（v7 在脱水前加在途水合检查）。
- 新增 `OH_CLOUD_DISK_CANCELLED`（34400030）：`Execute` 产生（task 已被取消）。语义为"别再试"（与 `CLOUD_DISK_TRY_AGAIN` 区分）。
- 新增 `OH_CLOUD_DISK_ALREADY_HYDRATED`（34400031）：`HydratePlaceholder`（FETCH_DATA）产生（state==3 且无在途 task）。与 19（有在途 task）区分：19=进行中，31=已完成。
- 新增 `OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS`（34400032）：`HydratePlaceholder`（CANCEL_FETCH_DATA）和 `Execute` 产生（无在途 task）。
- v6 说"不为优先级新增 34400030"是为保持 v3(27)→v4(28)→v5(29) 连续链叙事——v7 的 30/31/32 是水合主体语义，在 29 之后连续追加，不破坏既有链。
- 校验类失败（空参、路径非法、`priority` 非法、`syncFolder` 未注册/`bundleName` 不匹配）复用既有通用码 `CLOUD_DISK_INVALID_ARG`/`CLOUD_DISK_SYNC_FOLDER_*`，不新增。
- 启用前同步：NDK 枚举新增 30/31/32（19 已存在）；内部枚举新增 `E_CANCELLED`/`E_ALREADY_HYDRATED`/`E_NO_HYDRATION_IN_PROGRESS`（`E_HYDRATE_IN_PROGRESS` 已存在，补产生者）；映射表增 4 行（19/30/31/32）。
- 连续性：34400030 紧接 v5 的 34400029；完整链 v3(27)→v4(28)→v5(29)→v7(30/31/32)，**v3/v4/v5/v6 必须先或同期落地**以保持连续。

### 并发与读安全（§5.3，见 hydrate-main SDD §6/§12）

- **per-file 在途护栏**：`HydratePlaceholder`（FETCH_DATA） 时查 `PlaceholderTaskManager` 同文件 task → 有 PENDING/IN_PROGRESS → `HYDRATE_IN_PROGRESS`(19)。不同文件可并发（上限=FFRT worker 池大小）。`CANCEL_FETCH_DATA` 不受此护栏拦截。
- **优先级排队**：`PlaceholderTaskManager` 改优先级队列（HIGH>NORMAL>LOW，同优先级 FIFO，不抢占）。FFRT worker 按优先级取 task 派发。
- **FUSE 读安全窗口**：水合期间 state=1（全空洞）或 2（部分数据+空洞），hmdfs FUSE 读直接读文件、读到空洞返回零。v7 **仅标注，不处理**——随 demand hydration（`FETCH_RANGE_DATA` 按需下载）落地一并处理。

### 权限（§5.3，v7 补齐 PR #4217 缺失）

- `StartHydrationInner`/`CancelHydrationInner`/`ExecuteInner` 均调 `ResolveOwnedSyncFolder`（bundleName 归属校验）。PR #4217 的 `StartHydration` 缺失此校验（只做 `GetCallerBundleName`+`DentryHash`），v7 补齐。

### 兼容性（§2.5）

- 26.1.0 未商用：水合主体落地（`HydratePlaceholder` 异步、`Execute` 独立 IPC、`ExecuteInner` IDL、FFRT worker pool、`PlaceholderTaskManager` 扩展、`PlaceholderTaskRecord.outputFd` + close-on-erase）与新增 34400030/31/32 错误码均为 clean break，无既有消费方二进制兼容负担。`CloudDisk_CallbackContext` union 变体类型变更（v6 已定）、新增 IDL 接口码（由 IDL 自动生成）、新增 NDK 导出符号（`OH_CloudDisk_Execute`、`HydratePlaceholder` 加第 4 参），均无兼容负担。既有 Mark/Unmark/Convert/Dehydrate 签名与行为不变，与水合并存。

### 前置依赖与变更集顺序

- **前置依赖 v2/v3/v4/v5/v6 + 回调表基础设施**：v2 占位符重构（`filesyncstate` 高 3 位 + `placeholder_helper` 的 `SetHighBits`/读高3）+ v3 customInfo（34400027 连续）+ v4 state-only（34400028 连续 + state 3 写入路径 Mark）+ v5 脱水（34400029 + 回调表强制授权 + `PlaceholderCallbackManager`）+ v6 优先级（`HydratePriority`/`FetchDataRequest` 结构体约束）+ 回调表基础设施（PR #4353 `RegisterCallbackTable`/`UnregisterCallbackTable`/`ICloudDiskServiceCallbackTable`/`PlaceholderCallbackManager`/`PlaceholderTaskManager` 骨架）。均须先落地，水合主体后叠。变更集顺序：①v2 重构先合，②v3 customInfo + v4 state-only 次之，③v5 脱水后叠，④v6 优先级 + 回调表基础设施 + v7 水合主体最后。
- PR #4217（三方网盘全链路集成，含水合框架参考）未合入且代码陈旧，与 v6/v7 有冲突（无 priority、同步阻塞、cancel 含 ioctl 脱水、无状态机推进、无 Execute），仅作框架参考，以 v6/v7 规格为准。

### 内部维护（本规格不复述，见 SDD）

- 水合主体各层接线（NDK shim / inner manager 虚函数 `StartHydration`/`CancelHydration`/`Execute` / IDL `StartHydrationInner`/`CancelHydrationInner`/`ExecuteInner` / framework 编解码 `WriteCallbackParcel`/`ReadCallbackParcel` 扩 FETCH_DATA priority 分支 / SA override / `PlaceholderTaskManager` 扩展（4 态 + 优先级队列 + FFRT worker pool）/ `PlaceholderTaskRecord.outputFd` + close-on-erase / `CloudDiskServiceSyncFile` 改异步 + `Execute` pwrite + state 翻转 / mock / 测试）见 `docs/sdd/placeholder-hydrate-main-sdd.md` 与 `docs/sdd/placeholder-execute-sdd.md`。
- 测试落 `test/unittests/clouddiskservice/`（无下划线那棵）与 `test/unittests/ndk/`。

### 后续扩展（见 hydrate-main SDD §12）

- 跨进程完成通知：调用方 ≠ callback table 注册方（如文件管理器调 HydratePlaceholder、网盘应用做下载）时，需单独的进度回调函数。v7 不实现，**v8 由 ArkTS API SDD 落地**（`on('hydrateProgress')` + `PlaceholderProgressManager`）。
- demand hydration（`FETCH_RANGE_DATA`）：FUSE 读未水合 range 时按需下载该 range，与本 SDD 的全文件水合（`HydratePlaceholder`+`Execute`）是独立路径。FUSE 读安全窗口随此落地。
- 断点续传：state=2 + 无 task 时重新 HydratePlaceholder，当前全量重下载覆盖；后续可让 app 查询已下载 range 后只下载缺失部分。
- 批量/目录水合：递归水合目录下所有占位符，当前单文件。
- 抢占调度：高优先级到达时中断低优先级在途水合，当前不抢占（v6 §5.3 明确）。
- DFX 打点：水合排队时延、按优先级的下载完成率、pwrite 失败率→纳入 `clouddiskservice-dfx-sdd` 的 `FSC_PLACEHOLDER` scenario。

## v8（相对 v7）

> 本节记录 v8 当时的设计快照；与 2026-09-08 异步生命周期修订冲突的内容，以本文件顶部修订记录和最终 spec 为准。

占位符**Execute 数据回写详细规格落地**：v7 的 §5.2 Execute 行为只定了"独立 IPC + pwrite 到 outputFd"高层框架，Execute SDD（`docs/sdd/placeholder-execute-sdd.md` E1-E6）细化后本规格并入以下规格级变更。因 26.1.0 未商用，属 clean break。

### ExecuteInner IDL 形状（§5.2，v8 定）

- v7 只说"新 `ExecuteInner` IDL"未定形状。v8 定为 `void ExecuteInner([in] CallbackExecuteRequest request)` 单参 sequenceable（E1）。
- v7 §9.4 的 flat params（`[in] uint8[], [in] uint64, ...`）方案由 Execute SDD E1 否决——sequenceable 更利于扩展（加字段只改 Parcelable 不改 IDL 签名）。

### CallbackExecuteRequest Parcelable（§1.4，v8 新增）

- 新增 `CallbackExecuteRequest` sequenceable，字段：`reqKey`/`syncFolderPath`/`filePath`/`callbackType`/`offset`/`size`/`totalSize`/`data`/`isComplete`。Marshalling 顺序固定，Unmarshalling 逆序逐项校验。
- IDL 声明 `sequenceable cloud_disk_common..OHOS.FileManagement.CloudDiskService.CallbackExecuteRequest`。

### MAX_EXECUTE_DATA_SIZE（§5.2，v8 定）

> 以下4MB为 v8 当时的历史决策；当前有效上限已由 2026-09-08 修订为固定128 KiB（131072字节）。

- 单次 Execute 分块上限，可配置宏 `MAX_EXECUTE_DATA_SIZE`，默认 4MB（`EXECUTE_DATA_SIZE_MB * 1024 * 1024`）。NDK shim + service 双重校验。app 应按此上限分块。
- binder transaction buffer 实际支持大小需在目标设备实测；若不支持 4MB 可调小宏。

### Execute 错误码映射（§3.2，v8 补充）

- v7 已定义 34400030/31/32 的语义和产生接口。v8 补充 Execute 的完整错误码映射表（§3.2）：task 状态码（CANCELLED/COMPLETED/不存在/PENDING/IN_PROGRESS）、pwrite 失败码（ENOSPC→NO_SPACE_LEFT、EINTR 重试、其他→TRY_AGAIN）、校验码（size 不一致/超限/越界/totalSize 中途变更→INVALID_ARG）、callbackType 限制（RANGE→INVALID_ARG）。

### 安全脱敏（§3.3，v8 新增）

- `ExecuteInner` 调 `ResolveOwnedSyncFolder` 失败时对外统一返回 `CLOUD_DISK_CALLBACK_NOT_REGISTERED`(34400021)，不向调用方暴露失败原因（防探测）。内部 log 区分（路径不存在/归属不匹配/未注册），用 `%{private}` 脱敏（E6）。
- 与脱水路径不同：脱水返 `CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED` 等具体码（脱水是对授权应用的正常调用，暴露失败原因可接受）；Execute 是嵌套回调，安全脱敏更谨慎。

### death recipient 行为（§5.2/§5.3，v8 明确）

- v7 §5.3 说"崩溃 = CANCELLED → fd 清理 → state 保持"但未明确 death recipient 机制。v8 明确：
  - callback table client 崩溃 → `OnRemoteDied` → 标记该 provider 所有 task 为 CANCELLED。
  - IN_PROGRESS task：FFRT worker 从 `DispatchFetchData` 返回（`SendRequest` 失败）→ 查 CANCELLED → `close(outputFd)` → erase task → state 保持。
  - PENDING task：worker 取到时 `GetCallback` 返回 null → 清理。
  - `callbackMap_` stale 条目由 `OnRemoteDied` 清理（三表联动）。
- 恢复 PR #4245 PATCH 1/3 的 death recipient 实现（PATCH 2/3 删除是缺陷）。实现见 Execute SDD §5.7。

### 并发模型（§5.2，v8 定）

- per-task 锁：不同文件的 Execute 可并发（不同 per-task mutex），同文件的 Execute 串行（同 per-task mutex）。`taskMap_` 增删用短临界区 `mapMutex_`，pwrite 只持 per-task 锁，不阻塞其他 task。
- 实现见 Execute SDD §5.5（`unordered_map<reqKey, unique_ptr<mutex>>`）。

### fd 生命周期（v7 已定，v8 确认）

- v7 + Execute SDD E3 已定：`outputFd` 留在 `PlaceholderTaskRecord`，close-on-erase（完成/取消/崩溃三路径统一 close），不引入 `CloudDiskSyncFileTempManager`。v8 确认此设计不变。

### 兼容性（§2.5）

- 26.1.0 未商用：Execute 详细规格落地（`CallbackExecuteRequest` sequenceable、`ExecuteInner` IDL、`MAX_EXECUTE_DATA_SIZE`、death recipient 恢复、per-task 锁）与 v7 的水合主体落地均为 clean break，无既有消费方二进制兼容负担。新增 `CallbackExecuteRequest` 导出符号、新增 IDL `sequenceable` 声明、新增 IDL 接口码，均无兼容负担。

### 前置依赖

- **v7 水合主体 + 回调表基础设施**：v7 的全部前置依赖（v2~v6 + PR #4353）均须先落地。Execute 在水合主体之后落地（Execute 消费 HydratePlaceholder 创建的 task + outputFd）。

### 内部维护（见 Execute SDD）

- Execute 各层接线（NDK shim `BuildCallbackExecuteRequest` / inner manager `Execute` / IDL `ExecuteInner` / framework `CallbackExecuteRequest` Marshalling / service `ExecuteInner` override / `PlaceholderTaskManager::Execute` + `ExecuteFetchDataLocked` pwrite 循环 / per-task 锁 / death recipient 三表联动 / mock / 测试）见 `docs/sdd/placeholder-execute-sdd.md`。

### 后续扩展（见 Execute SDD §11）

- `cachedSize` 已加（A5 修正，ArkTS 进度回调需要）。
- `FETCH_RANGE_DATA` 回应（demand hydration）。
- 进度查询接口（主动查 `GetHydrateStatus`，当前经 `on('hydrateProgress')` 被动收）。
- `pwrite` → `pwrite64` 显式化。
- 大块传输优化（shared memory/Ashmem）。

### 文件管理器 ArkTS API（v8 补充）

v8 在 Execute 详细规格之外，补充文件管理器 ArkTS API 规格落地。CAPI D10"跨进程通知后续单独设计"由本节落地。详见 `docs/sdd/placeholder-arkts-api-sdd.md`。

- **PR #35509 变更**：加 `HydratePriority` enum（LOW/NORMAL/HIGH）；`hydratePlaceholder` 加 `priority` 参（`hydratePlaceholder(filePath: string, callbackType: CallbackType, priority: HydratePriority): Promise<void>`）；`HydrateProgressState` 删 `FAILED`、加 `PENDING`+`CANCELLED`。当前修订下，可重试 pwrite/fsync/xattr 失败保持 IN_PROGRESS，生命周期终止才进入 CANCELLED。
- **新增 `CloudDiskSystemAccessor` 类**（无参构造，与既有 `CloudDiskAccessor(syncFolderPath)` 平行）：`hydratePlaceholder`/`dehydrateFile`/`on('hydrateProgress')`/`off('hydrateProgress')`。`@permission ohos.permission.ACCESS_CLOUD_DISK_INFO`，`@systemapi`，`@since 26 dynamic`。
- **filePath 语义**：完整绝对路径（沙箱路径）。SA 新 IDL `StartHydrationByPathInner(absolutePath, type, priority)` / `DehydrateFileByPathInner(absolutePath)` 传绝对路径，SA 内部 `ResolveOwnedSyncFolderByPath` 解析为 (syncFolder, relativePath)，一次 IPC。
- **进度回调机制**：SA 新 broker `ICloudDiskProgressCallback`（`OnProgress(HydrateProgress)`）+ `PlaceholderProgressManager` + SA IDL `RegisterProgressCallbackInner`/`UnregisterProgressCallbackInner`。TaskManager push 模型（状态变化/cachedSize 增长→`ProgressManager::OnTaskProgress`→节流 500ms 暂定→推送）。全局订阅（`on` 一次收所有文件进度）。death recipient 挂 `SvcDeathRecipient`（同 E5 范式）。推送所有 task 进度（含 CAPI 发起的），file manager 按 filePath 过滤。
- **`PlaceholderTaskRecord` 扩展**：加 `cachedSize`（当前修订下按成功 Execute 的 size 有界累加）+ `absolutePath`（推送时用原始绝对路径）。
- **totalSize 来源**：首次 fstat logicalSize，Execute 回填后用 app totalSize。
- **跨仓改动**：ufs 仓（NAPI `cloud_disk_system_access_n_exporter` + inner API `CloudDiskSystemManager`）+ dfs 仓（SA IDL + `PlaceholderProgressManager` + `HydrateProgress` Parcelable + `ICloudDiskProgressCallback` broker）。
