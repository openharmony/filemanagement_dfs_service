> 标题:clouddiskservice 占位符水合优先级(Hydrate Priority)NDK 接口设计 | 目录:`D:\code\dfs\filemanagement_dfs_service`
> 状态:设计稿,未实现。本设计为**纯规格演进**——对外结构体约束草案,供跨领域对齐讨论;**不可执行**,执行前将重新细化设计。**前置依赖** v2 占位符重构(`docs/sdd/placeholder-marking-refactor-sdd.md`)、v3 customInfo(`docs/sdd/placeholder-custom-info-sdd.md`)、v4 state-only 转换(`docs/sdd/placeholder-state-only-conversion-sdd.md`)、v5 脱水(`docs/sdd/placeholder-dehydrate-sdd.md`)、回调表基础设施(独立变更集)、水合主体(状态机推进/取消/回调往返/`PlaceholderTaskManager`,另案 SDD)均须先落地。

# Placeholder Hydrate Priority SDD

## 1. 背景

`CloudDiskService`(三方网盘 SA)的占位符体系经 v2(`filesyncstate` 高 3 位 `PlaceholderState`)、v3(customInfo)、v4(state-only Mark/Unmark)、v5(脱水 `DehydrateFile` + 回调表强制授权)演进后,占位态已可表达"未水合(1)/部分水合(2)/完全水合(3)"并完成脱水落地。但 v5 §5.3 的 `OH_CloudDisk_HydratePlaceholder`(水合,`1→2→3` 推进)仍标为"设计态、未落地",其对外结构体约束(回调数据契约)仅承载裸文件路径(`CloudDisk_CallbackContext.fetchData` 为 `CloudDisk_PathInfo *`),无优先级维度。

外部诉求:水合本质是异步分块下载,多个文件的水合请求会并发到达。若无优先级,SA 只能按到达顺序派发 `FETCH_DATA` 回调,应用侧也无优先级信息调度下载资源——用户主动打开的文件与后台预取竞争同一队列,体验差。需给水合加上优先级,让 SA 按优先级排队派发、应用按优先级调度下载响应。

本设计在 v5 §1.3 回调数据契约结构体上叠加优先级维度,作为对外结构体约束草案供跨领域对齐讨论:新增枚举 `CloudDisk_HydratePriority`(三级 `LOW/NORMAL/HIGH`)、新增结构体 `CloudDisk_FetchDataRequest`(`filePath`+`priority`)、将 `CloudDisk_CallbackContext.fetchData` 变体类型由裸 `CloudDisk_PathInfo *` 升级为 `CloudDisk_FetchDataRequest *`、`OH_CloudDisk_HydratePlaceholder` 签名新增第 4 参值传递 `CloudDisk_HydratePriority priority`。水合主体实现(状态机推进、取消语义、回调往返、`PlaceholderTaskManager` 任务管理、并发护栏)另案 SDD,本设计不定。

## 2. 目标

1. 新增 NDK `OH_CloudDisk_HydratePlaceholder` 第 4 参 `CloudDisk_HydratePriority priority`(值传递,强制显式,不提供无 priority 重载),`@since 26.1.0`(签名沿用 v5 §5.3 既定指针入参 + 新增第 4 参)。
2. 新增枚举 `CloudDisk_HydratePriority`(三级 `LOW(0)/NORMAL(1)/HIGH(2)`,值从 0 起,append-only,不复用既有码段)。
3. 新增结构体 `CloudDisk_FetchDataRequest { CloudDisk_PathInfo filePath; CloudDisk_HydratePriority priority; }`(`filePath`+`priority` 均为服务→应用入参,应用只读),作为 `FETCH_DATA` 回调的 context 变体载体。
4. `CloudDisk_CallbackContext` union 的 `fetchData` 变体类型由 `CloudDisk_PathInfo *` 改为 `CloudDisk_FetchDataRequest *`(union 大小不变,指针变体)。`cancelFetchData` 维持 `CloudDisk_PathInfo *`(取消不带优先级)。
5. 优先级经回调表透传到应用:服务在派发 `FETCH_DATA` 回调时填 `reqContext.fetchData->priority`,应用在 `OnCallback` 中读取据此按优先级调度下载响应。
6. SA 内语义(占位描述):`PlaceholderTaskManager` 按优先级排队派发 `FETCH_DATA` 回调(`HIGH`→`NORMAL`→`LOW`),**不抢占**(高优先级不中断低优先级在途请求)。任务管理器结构属水合主体另案 SDD。
7. 取消(`CANCEL_FETCH_DATA`)不带优先级:取消按文件取消(取消该文件所有在途水合,不论优先级);`priority` 仍须传入(签名强制),取消路径不消费,调用方对取消调用可传 `NORMAL` 占位。
8. 优先级不持久化:不入 `filesyncstate`/`custominfo` xattr、不进 dentry 占位字段、不进 RDB;仅在 `FETCH_DATA` 回调派发时随 `CloudDisk_FetchDataRequest` 携带。
9. **不新增错误码**:优先级值非法(`priority` 不在 `LOW/NORMAL/HIGH`)属入参校验类失败,复用既有通用码 `CLOUD_DISK_INVALID_ARG`(service 侧 `E_INVALID_ARG`),与 customInfo 超限、路径非法同类。
10. 本设计为纯规格演进、不可执行,执行前将重新细化设计;**不落测试**(等水合主体 SDD 一起定测试目标)。
11. 本设计**仅**约束对外结构体层(NDK 头声明 + inner `cloud_disk_common.h` 结构体/枚举/变体),**不**写 framework 编解码(`WriteCallbackParcel`/`WriteCallbackReply`/`ReadCallbackReply` 的 `FETCH_DATA` 优先级分支)、IDL、service override、`PlaceholderCallbackManager::DispatchFetchData` 改造、mock、测试——那些依赖水合主体与回调表(另案),等水合主体 SDD 一起写。

## 3. 总体决策

| 项 | 决策 |
| --- | --- |
| 优先级语义 | 三级枚举 `LOW/NORMAL/HIGH`;SA 内 `PlaceholderTaskManager` 按优先级排队派发 `FETCH_DATA`、**不抢占** |
| 优先级载体 | 新增 `CloudDisk_FetchDataRequest` 作 `FETCH_DATA` context 变体(类比脱水 `CloudDisk_DehydrateInfo`);`fetchData` 变体类型由 `PathInfo*` 升级为 `FetchDataRequest*` |
| 优先级作用范围 | 透传到应用回调(服务填 `reqContext.fetchData->priority`,应用据此调度下载);非仅 SA 内 |
| 入口签名 | NDK `OH_CloudDisk_HydratePlaceholder(syncFolderPath*, filePath*, type, priority)`(第 4 参值传递,强制显式);水合主体 IDL/inner manager 虚函数属水合主体另案 |
| 优先级值域 | 枚举 `CloudDisk_HydratePriority`(LOW=0/NORMAL=1/HIGH=2,值从 0 起,不复用既有码段);无 URGENT(不抢占,URGENT 暗示抢占) |
| 默认值 | 无默认(强制显式传入);NORMAL 为常见值但调用方须显式传 |
| 取消与优先级 | 取消(`CANCEL_FETCH_DATA`)**不带优先级**;`cancelFetchData` 变体维持 `PathInfo*`;取消按文件取消(不区分优先级) |
| `Execute` 关系 | `Execute`/`CloudDisk_CallbackResponse` 不变;优先级是入参(服务→应用),不经 `Execute` 回写(`Execute` 仍仅 `FETCH_DATA` 数据响应) |
| customInfo | 不读写(v3 §2.6 正交);`FetchDataRequest`/`HydratePriority` 不进 xattr |
| 脱水关系 | 正交;`DEHYDRATE` 回调不携带优先级,脱水经 `DehydrateFile` 触发、不经 `HydratePlaceholder` |
| count 副作用 | 无(优先级不持久化、不改 state,无 delta) |
| 持久化 | 不持久化(不入 xattr/dentry/RDB);仅运行期随 `FetchDataRequest` 携带 |
| 权限 | 沿用 `HydratePlaceholder` 既有 bundleName 归属(水合主体定,本设计不碰) |
| 入参风格 | NDK 指针(`syncFolderPath*`/`filePath*`,承 §5.3)+ 值(`type`/`priority` 枚举) |
| 新 Parcelable | 无(仅扩 context union 变体类型 + 新增结构体 `CloudDisk_FetchDataRequest` + 新增枚举 `CloudDisk_HydratePriority`) |
| 特性开关 | `dfs_service_feature_enable_cloud_disk` / `SUPPORT_CLOUD_DISK_SERVICE`,各处 `#ifdef` 一致(水合主体落地时核对) |
| 错误码新增 | **无**;非法 `priority` 复用 `CLOUD_DISK_INVALID_ARG`/`E_INVALID_ARG`;NDK/内部枚举/`innerToNErrTable` 映射表均不动 |
| 与既有接口关系 | 与 v5 脱水并存;优先级只挂 `FETCH_DATA`(水合),不挂 `DEHYDRATE`(脱水);`cancelFetchData`/`fetchRangeData`/`dehydrateData` 变体不变 |
| 前置依赖 | v2 重构(state 模型)+ v3 customInfo + v4 state-only + v5 脱水 + 回调表基础设施 + 水合主体(`PlaceholderTaskManager`/状态机/取消/回调往返)均须先落地 |
| 产物范围 | 仅 NDK 头声明 + inner `cloud_disk_common.h` 结构体层;framework/IDL/service/mock/test 等水合主体 SDD |
| 文档定位 | 对外结构体约束草案,跨领域对齐讨论用;不可执行,执行前重细化 |

## 4. 状态语义与错误码

### 4.1 状态翻转模型

本设计**不引入新占位态、不改 state 转换契约**。优先级不并入 `filesyncstate` 字节、不进 xattr、不进 dentry 占位字段、不进 RDB——优先级是 `FETCH_DATA` 回调的运行期入参,仅在回调派发时随 `CloudDisk_FetchDataRequest` 携带,不持久化。

水合主体落地时 state 推进仍走 v5 §2.2 的 `UNHYDRATED(1)→PARTIALLY_HYDRATED(2)→FULLY_HYDRATED(3)`(`1↔2↔3` 子态切换 delta=0,占位语义不变,不改 count、不祖先刷新)。优先级只影响 `FETCH_DATA` 派发顺序、不影响子态切换的 count 契约。

### 4.2 错误码

> 事实基线(实际代码树,非 spec 累积态):
> - NDK 枚举 `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h`:**现有最大 `OH_CLOUD_DISK_FILE_TOO_LARGE = 34400026`**(line 221)。`OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`(34400027,v3)/`OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`(34400028,v4)/`OH_CLOUD_DISK_DEHYDRATE_DENIED`(34400029,v5)**均未落地**。
> - 内部枚举 `utils/clouddiskservice/include/cloud_disk_service_error.h`(`CloudDiskServiceErrCode`,稀疏):**最大 `E_FILE_TOO_LARGE = 34400026`**;34400027/28/29 均未定义;17→19 间 34400018 空缺未补;21/22 空缺。
> - NDK↔inner 映射表 `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_utils.h`(`innerToNErrTable`):无 27/28/29 行,无 18/21/22 行。

**本设计不新增错误码。** 优先级值非法(`priority` 不在 `LOW/NORMAL/HIGH`)属入参校验类失败,复用既有通用码:

- NDK 侧:`CLOUD_DISK_INVALID_ARG`(34400001,`@since 21`)。
- service 侧:`E_INVALID_ARG`(34400001,既有)。
- 映射表 `innerToNErrTable`:已有 `E_INVALID_ARG → CLOUD_DISK_INVALID_ARG` 行(既有),不动。

理由:
- 优先级值非法与 customInfo 超限、路径非法、空参同类,脱水 SDD/state-only SDD 对校验类失败均复用既有通用码不新增,本设计保持一致。
- SA 内排队调度不产生失败码(排队只影响派发顺序,非错误)。
- 下一个专属错误码应留给真正的新语义(如水合主体在途护栏启用 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`(34400019,既有 reserved)时若需新增),不为优先级新增 34400030,避免污染 v3(27)→v4(28)→v5(29) 的错误码连续链叙事。

`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`(34400019)仍维持"保留,当前无接口产生",待水合主体落地由在途护栏产生(与 v5 §3 注一致)。

## 5. API / 各层改动(仅对外结构体约束层)

本设计**仅**约束对外结构体层(NDK 头声明 + inner `cloud_disk_common.h` 结构体/枚举/变体),**不**写 framework 编解码 / IDL / service / mock / test。理由:framework 的 `WriteCallbackParcel`/`WriteCallbackReply`/`ReadCallbackReply` 扩 `FETCH_DATA` 优先级分支、IDL 的 `HydratePlaceholderInner`、service override、`PlaceholderCallbackManager::DispatchFetchData` 改造、mock、测试均依赖回调表基础设施(未落地)与水合主体(另案 SDD),现在写了也是悬空约束,且越界到水合主体 SDD 的职责。等水合主体 SDD 一起写。

### 5.1 NDK — `interfaces/kits/ndk/clouddiskmanager/`

- `include/oh_cloud_disk_manager.h`:`OH_CloudDisk_HydratePlaceholder` 声明加第 4 参,`@since 26.1.0`/`@kit CoreFileKit`/`@syscap SystemCapability.FileManagement.CloudDiskManager`/`@library libohclouddiskmanager.so`,紧邻 §5.3 水合脱水段(v5 既有声明位置):

```c
CloudDisk_ErrorCode OH_CloudDisk_HydratePlaceholder(
    const CloudDisk_SyncFolderPath *syncFolderPath,
    const CloudDisk_PathInfo *filePath,
    CloudDisk_CallbackType type,
    CloudDisk_HydratePriority priority);  // v6 新增第 4 参
```

> 注:NDK 头里 `CloudDisk_HydratePriority`/`CloudDisk_FetchDataRequest`/`CloudDisk_CallbackContext`/`CloudDisk_CallbackType` 等回调契约类型的声明位置由水合主体 SDD 定(可能在 NDK 头或 inner 头,取决于回调表变更集);本设计假定这些类型已在 v5 §1.3 契约下声明,本设计只改 `CloudDisk_CallbackContext.fetchData` 变体类型 + 新增 `CloudDisk_FetchDataRequest`/`CloudDisk_HydratePriority`。

- `liboh_cloud_disk_manager.ndk.json`:`OH_CloudDisk_HydratePlaceholder` 若已登记符号(水合主体 SDD 定),则 `first_introduced` 维持 `26.1.0`;若尚未登记,等水合主体 SDD 一起加。本设计不单独改 ndk.json(符号在水合主体落地时一起加,避免半成品符号)。
- `src/oh_cloud_disk_manager.cpp`:NDK shim 属水合主体(空指针校验→ `CloudDiskServiceManager::GetInstance().HydratePlaceholder(..., priority)` → `ConvertToErrorCode`),本设计不定;`priority` 非法校验在 shim 或 inner manager(水合主体 SDD 定)。
- `include/cloud_disk_error_code.h`:**不动**(不新增错误码)。
- `include/oh_cloud_disk_utils.h` + `src/oh_cloud_disk_utils.cpp`:映射表 `innerToNErrTable` **不动**(无新内部枚举值,无新映射行;`priority` 非法复用既有 `E_INVALID_ARG → CLOUD_DISK_INVALID_ARG`)。
- `BUILD.gn`:**不动**(不新增公开 header,`CloudDisk_HydratePriority`/`CloudDisk_FetchDataRequest` 加进既有回调契约头,该头已在 `ohos_ndk_headers` 列表,水合主体 SDD 核对)。

### 5.2 inner API — `interfaces/inner_api/native/clouddiskservice_kit_inner/`

- `cloud_disk_common.h`:§1.3 水合脱水数据契约新增(范式照 v5 脱水加 `CloudDisk_DehydrateInfo` 的既有风格):
  - 新增枚举 `CloudDiskHydratePriority`(实现层无下划线,承既有 `CloudDiskCallbackType`/`CloudDiskRangeInfo` 同族命名):
    ```cpp
    enum CloudDiskHydratePriority : uint32_t {
        CLOUD_DISK_HYDRATE_PRIORITY_LOW = 0,
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL = 1,
        CLOUD_DISK_HYDRATE_PRIORITY_HIGH = 2,
    };
    ```
  - 新增结构体 `CloudDiskFetchDataRequest`:
    ```cpp
    struct CloudDiskFetchDataRequest {
        CloudDiskPathInfo filePath;        // 同步根内相对文件路径(服务→应用入参)
        CloudDiskHydratePriority priority; // 水合优先级(服务→应用入参)
    };
    ```
  - `CloudDiskCallbackContext` union 的 `fetchData` 变体类型由 `CloudDiskPathInfo *` 改为 `CloudDiskFetchDataRequest *`:
    ```cpp
    union CloudDiskCallbackContext {
        CloudDiskFetchDataRequest *fetchData;     // v6 变:原 CloudDiskPathInfo *
        CloudDiskPathInfo *cancelFetchData;       // 不变(取消不带优先级)
        CloudDiskRangeInfo *fetchRangeData;       // 不变
        CloudDiskDehydrateInfo *dehydrateData;   // 不变(v5)
    };
    ```
  - 命名桥接:spec `clouddisk-placeholder-v6.md` §1.3 作 `CloudDisk_FetchDataRequest`/`CloudDisk_HydratePriority`(下划线,承 v5 `CloudDisk_DehydrateInfo`/`CloudDisk_RangeInfo` 约定);实现层 `cloud_disk_common.h` 依既有无下划线惯例(`CloudDiskCallbackContext`/`CloudDiskRangeInfo`/`CloudDiskDehydrateInfo` 同族)作 `CloudDiskFetchDataRequest`/`CloudDiskHydratePriority`,二者为同一类型;桥接范式同 v5 脱水 SDD §5.2。下文代码片段沿用实现层无下划线命名。
- `cloud_disk_service_manager.h`:`HydratePlaceholder` 虚函数属水合主体(签名加 `CloudDiskHydratePriority priority` 参数),本设计不定;但为对外结构体约束的完整性,记录目标签名(水合主体 SDD 落地时核对):
  ```cpp
  virtual int32_t HydratePlaceholder(const std::string &syncFolder,
                                     const std::string &relativePath,
                                     CloudDiskCallbackType type,
                                     CloudDiskHydratePriority priority) = 0;
  ```
  本设计**不**在 `cloud_disk_service_manager.h` 加此虚函数(等水合主体 SDD 一起加,避免半成品虚函数导致 manager 不可编译)。
- `clouddiskservice_kit_inner.map`:本设计**不动**(`HydratePlaceholder` 符号导出等水合主体 SDD)。

### 5.3 不写的层(等水合主体 SDD)

以下层本设计**不**写,因依赖回调表基础设施(未落地)与水合主体(另案):

- framework `cloud_disk_common.cpp`:扩 `WriteCallbackParcel`(请求写 `filePath`+`priority`)/`WriteCallbackReply`/`ReadCallbackReply` 的 `FETCH_DATA` 分支(当前 `FETCH_DATA` context 是裸 `PathInfo`,改为 `FetchDataRequest` 后编解码要变)。
- framework `cloud_disk_service_manager_impl.{h,cpp}`:`HydratePlaceholder` override impl。
- IDL `ICloudDiskService.idl`:`HydratePlaceholderInner`。
- service `cloud_disk_service.{h,cpp}`:`HydratePlaceholderInner` override impl + 优先级入 `PlaceholderTaskManager` 队列。
- service `placeholder_callback_manager.{h,cpp}`:`DispatchFetchData` 改造(填 `reqContext.fetchData->priority`)——注:该文件本身随 v5 脱水 SDD 落地时新建,本设计依赖其存在。
- mock `test/unittests/clouddiskservice/mock/`:`HydratePlaceholder` mock 方法。
- test `test/unittests/clouddiskservice/`:NDK shim + SA 单测。

## 6. 行为语义

### 6.1 优先级语义(占位描述,任务管理器结构属水合主体另案)

```
1. 应用调 OH_CloudDisk_HydratePlaceholder(syncFolderPath*, filePath*, type, priority)
   - type == FETCH_DATA:发起水合(带优先级)
   - type == CANCEL_FETCH_DATA:取消水合(priority 不消费,可传 NORMAL 占位)
2. priority 校验(NDK shim 或 inner manager,水合主体 SDD 定位置):
   priority 不在 {LOW, NORMAL, HIGH} → E_INVALID_ARG(→ NDK CLOUD_DISK_INVALID_ARG)
3. SA 内(水合主体 SDD 定):
   - type == FETCH_DATA:PlaceholderTaskManager 把 (filePath, priority) 入队,
     按优先级排队(HIGH 先于 NORMAL 先于 LOW),不抢占(高优先级不中断低优先级在途)
   - 派发 FETCH_DATA 回调时:填 reqContext.fetchData->filePath = filePath,
     reqContext.fetchData->priority = priority,派发给应用
   - type == CANCEL_FETCH_DATA:取消该文件所有在途水合(不区分优先级),
     派发 CANCEL_FETCH_DATA 回调(reqContext.cancelFetchData = filePath,不带 priority)
4. 应用侧 OnCallback(reqHead, reqContext):
   - callbackType == FETCH_DATA:读 reqContext.fetchData->priority,
     按优先级调度下载(分配带宽/优先处理);完成后经 Execute 填 rsp.fetchData 回写数据
   - callbackType == CANCEL_FETCH_DATA:停止该文件下载(reqContext.cancelFetchData,无 priority)
```

- 优先级不持久化:不入 xattr/dentry/RDB,仅运行期随 `CloudDiskFetchDataRequest` 携带。
- 不抢占:高优先级请求到达只排在低优先级在途请求之后(若低优先级在途则等其完成或被显式取消),不中断在途。抢占(中断在途 + 状态回退 + 数据清理)属水合主体后续扩展,本设计不取。
- 取消不带优先级:取消按文件取消(取消该文件所有在途水合,不论优先级);`cancelFetchData` 变体维持 `CloudDiskPathInfo *`。若未来需"取消特定优先级在途",留待水合主体 SDD 升级 `cancelFetchData` 变体。

### 6.2 回调契约(透传,不经 Execute)

- 服务侧:水合主体落地后,`PlaceholderCallbackManager::DispatchFetchData`(随 v5 脱水 SDD 落地时新建的 `placeholder_callback_manager.{h,cpp}` 内)派发 `FETCH_DATA` 回调时,填 `reqContext.fetchData->filePath` 与 `reqContext.fetchData->priority`(`CloudDiskFetchDataRequest`),经 `CloudDiskServiceCallbackTableProxy::SendCallback` 同步 `SendRequest(SERVICE_CMD_ON_CALLBACK)` 派发给应用。
- 应用侧:`CloudDiskServiceCallbackTableStub::HandleOnCallback` 调 `OnCallback(reqHead, reqContext)`,app 在 `OnCallback` 中按 `callbackType==FETCH_DATA` 读 `reqContext.fetchData->priority` 据此调度下载,完成后调 `OH_CloudDisk_Execute` 填 `rsp.fetchData`(`CloudDisk_FetchData`,含 offset/size/totalSize/data/isComplete)回写数据。
- **不**经 `Execute` 传优先级:`Execute`/`CloudDisk_CallbackResponse` 仅用于 `FETCH_DATA` 的数据响应(`rsp.fetchData`);优先级是请求侧入参(`reqContext.fetchData->priority`,服务→应用),与响应侧数据(`rsp.fetchData`,应用→服务)分立。`FetchDataRequest`(请求)与 `FetchData`(响应)严格分立。
- 嵌套 IPC 线程安全:`HydratePlaceholderInner`(服务 binder 线程)→ `SendCallback`(同步 IPC 到 app)→ app callback stub 跑在 app 的另一 binder 线程,无自死锁(范式同 v5 脱水 SDD §6.2)。

## 7. 校验与错误处理

| 场景 | 码 | 位置 |
| --- | --- | --- |
| `priority` 不在 {LOW,NORMAL,HIGH} | `E_INVALID_ARG`(→NDK `CLOUD_DISK_INVALID_ARG` 34400001) | NDK shim 或 inner manager(水合主体 SDD 定) |
| `syncFolder` 未注册 / bundle 不匹配 | `E_SYNC_FOLDER_NOT_REGISTERED`/`E_SYNC_FOLDER_PATH_UNAUTHORIZED` | service(水合主体 SDD) |
| 无权限 | `E_PERMISSION_DENIED` | service(水合主体 SDD) |
| 前置态拒绝(占位符/普通文件等) | 水合主体定(另案) | service(水合主体 SDD) |
| 无 callback table | `E_LISTENER_NOT_REGISTERED`(→NDK `CALLBACK_NOT_REGISTERED` 34400021) | service(`DispatchCallback`,水合主体 SDD) |
| 路径父目录非目录 / 名过长 | `E_NOT_A_DIRECTORY`(34400023)/`E_NAME_TOO_LONG`(34400025) | service(既有校验链) |
| `GetCallerBundleName` 失败(含 twin app `instIndex!=0`) | `E_TRY_AGAIN` | service |
| `ServiceProxy` 为空 | `E_IPC_FAILED` | framework(水合主体 SDD) |
| 特性开关关闭 | `E_NOT_SUPPORTED` | framework `#else`(水合主体 SDD) |

- 校验位置(`priority` 非法)在水合主体 SDD 定(NDK shim 或 inner manager);本设计只约束"非法复用 `CLOUD_DISK_INVALID_ARG`、不新增码"。
- 校验须在派发 `FETCH_DATA` 回调之前(避免无效优先级进入队列)。

## 8. 安全与约束

1. 不绕过 `CloudDiskServiceAccessToken` 的 token/userId/bundleName 校验;水合走与既有占位操作同一 `CheckSyncFolderBundleName` + `CheckPermissions` 访问链(水合主体 SDD 定)。
2. `GetCallerBundleName` 拒 twin app(`instIndex != 0`),与既有 per-file 占位操作一致。
3. `GetUserId` 由 `callingUid / BASE_USER_RANGE` 派生,多用户隔离不可绕过。
4. 路径校验不绕过(`..` 逃逸走 `ReplacePathPrefix`/`GetHmdfsPath`,路径须落在已注册 sync folder 范围内)。
5. 不改权限模型/token 校验/多用户隔离/uid-gid 归属/RDB schema。
6. 不读写 customInfo xattr(v3 §2.6 正交);`FetchDataRequest`/`HydratePriority` 不进 xattr。
7. 不引入新第三方依赖、无常驻缓存;优先级不持久化,无内存常驻结构(任务管理器属水合主体另案)。
8. xattr 资源归属不变(占位符文件类资产归属各应用,uid/gid 逻辑不动)。
9. 优先级是 SA 内调度 hint + 应用侧下载调度 hint,不构成安全边界(不影响权限/隔离/数据归属)。

## 9. 契约 / ABI / 兼容影响

- **非破坏面**:本设计为**纯新增**——新增 1 NDK 函数第 4 参(`HydratePlaceholder` 签名变)、1 inner 枚举(`CloudDiskHydratePriority`)、1 inner 结构体(`CloudDiskFetchDataRequest`)、`CloudDiskCallbackContext` union 1 变体类型改(`fetchData` 由 `PathInfo*` 改 `FetchDataRequest*`)。不改 `PlaceholderInfo`/`PlaceholderCustomInfo` Parcelable 布局;不改既有 `filesyncstate`/`custominfo` xattr 语义;既有 `ConvertPlaceholderToFile`/`MarkFileAsPlaceholder`/`UnmarkPlaceholderFile`/`DehydrateFile` 签名与行为不变;`cancelFetchData`/`fetchRangeData`/`dehydrateData` 变体不变;`CloudDisk_CallbackType`/`CloudDisk_CallbackResponse`/`CloudDisk_FetchData` 不变。
- **回调 ABI**:`CloudDiskCallbackContext` union `fetchData` 变体类型变(union 大小不变,指针变体;`PathInfo*` 与 `FetchDataRequest*` 均为指针,union 尺寸不变);`FETCH_DATA` 的请求侧 context 由裸路径升级为 `FetchDataRequest`(响应侧 `CloudDisk_FetchData` 与 `Execute` 不变);回执包格式对既有 `CANCEL_FETCH_DATA`/`FETCH_RANGE_DATA`/`DEHYDRATE` 类型不变。
- **错误码新增面**:**无**。NDK 枚举、内部枚举、`innerToNErrTable` 映射表均不动。`priority` 非法复用既有 `CLOUD_DISK_INVALID_ARG`/`E_INVALID_ARG`。
- **特性开关**:`dfs_service_feature_enable_cloud_disk` 关闭时,framework `#ifdef SUPPORT_CLOUD_DISK_SERVICE` 走 `#else return E_NOT_SUPPORTED`(水合主体 SDD 落地);BUILD 与代码条件一致(AGENTS.md 硬约束)。
- **前置依赖**:v2 重构(`SetHighBits`/读高3 helper)、v3 customInfo(34400027)、v4 state-only(34400028 + state 3 写入路径)、v5 脱水(34400029 + 回调表强制授权 + `placeholder_callback_manager.{h,cpp}`)、回调表基础设施(`ICloudDiskServiceCallbackTable`/`OnCallback`/`WriteCallbackParcel`/`WriteCallbackReply`/`ReadCallbackReply`)、水合主体(`PlaceholderTaskManager`/状态机推进/取消/回调往返)均未落地时本设计无法实现。变更集顺序:①v2 重构先合,②v3 customInfo + v4 state-only 次之,③v5 脱水后叠,④水合主体 + 本设计(优先级)最后。
- `clouddiskservice` IDL 接口码不在 `CODEOWNERS` 额外评审清单,无额外 IPC 评审路径;但内部仍同步 proxy/stub/manager/mock/测试(水合主体 SDD)。
- 26.1.0 未商用:新增符号属 clean break 范畴,无既有消费方二进制兼容负担。

## 10. 测试要求

本设计为纯规格演进、不可执行,**不落测试**。测试目标随水合主体 SDD 一起定:

- NDK shim 单测:`priority` 非法 → `CLOUD_DISK_INVALID_ARG`;`priority` 透传到 `reqContext.fetchData->priority`(mock callback table 校验)。
- SA 单测:`HydratePlaceholder` 带 priority 入 `PlaceholderTaskManager` 队列;按优先级排队派发 `FETCH_DATA`(HIGH 先于 NORMAL 先于 LOW);不抢占(高优先级不中断低优先级在途);取消不带优先级(取消该文件所有在途,`cancelFetchData` 无 priority)。
- IPC:`FETCH_DATA` 的 `WriteCallbackParcel`(请求写 `filePath`+`priority`)往返编解码(`priority` 正确回填)。
- 测试**仅**落 `test/unittests/clouddiskservice/`(无下划线那棵),不改 `test/unittests/clouddisk_service/`(下划线那棵,另一棵测试树)。
- mock 同步:`mock/icloud_disk_service.h` 加 `HydratePlaceholderInner` 接口码 + virtual(带 priority 参数);`mock/cloud_disk_service_manager_mock.{h,cpp}` 加 `MOCK_METHOD`(`HydratePlaceholder`,带 priority)。

以上均等水合主体 SDD 一起落,本设计只记录测试意图。

## 11. 实现落点(未实现,sign-off + 水合主体 SDD 后进行)

| 内容 | 路径 | 本设计是否落 |
| --- | --- | --- |
| NDK `HydratePlaceholder` 声明(加第 4 参 `priority`) | `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h` | 是(对外结构体约束) |
| NDK `CloudDisk_HydratePriority` 枚举 + `CloudDisk_FetchDataRequest` 结构体声明 | 同上(或回调契约头,水合主体 SDD 定位置) | 是 |
| NDK `CloudDisk_CallbackContext.fetchData` 变体类型改 | 同上 | 是 |
| NDK shim(`priority` 校验 + 转 manager) | `interfaces/kits/ndk/clouddiskmanager/src/oh_cloud_disk_manager.cpp` | 否(水合主体 SDD) |
| NDK 符号清单 | `interfaces/kits/ndk/clouddiskmanager/liboh_cloud_disk_manager.ndk.json` | 否(水合主体 SDD,避免半成品符号) |
| NDK 错误码 + 映射 | `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h` + `oh_cloud_disk_utils.{h,cpp}` | 否(不动,无新错误码) |
| 回调契约类型 + context 变体(实现层) | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_common.h` | 是(对外结构体约束) |
| inner manager 虚函数 + `.map` | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_service_manager.h` + `clouddiskservice_kit_inner.map` | 否(水合主体 SDD,避免半成品虚函数) |
| framework 回调编解码 + manager impl | `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_common.cpp` + `cloud_disk_service_manager_impl.{h,cpp}` | 否(水合主体 SDD) |
| IDL | `services/clouddiskservice/ICloudDiskService.idl` | 否(水合主体 SDD) |
| service override + impl + `DispatchFetchData` 改造 | `services/clouddiskservice/ipc/{include/cloud_disk_service.h,src/cloud_disk_service.cpp,include/placeholder_callback_manager.h,src/placeholder_callback_manager.cpp}` | 否(水合主体 SDD) |
| mock | `test/unittests/clouddiskservice/mock/` | 否(水合主体 SDD) |
| 测试 | `test/unittests/clouddiskservice/{ndk/,ipc/}` | 否(水合主体 SDD) |

> 行号为当前代码树快照(取自探索期),实现时以最新树为准。`placeholder_callback_manager.{h,cpp}` 当前不存在(随 v5 脱水 SDD 落地时新建),本设计依赖其存在。

## 12. Ask-before / 评审项

- **新增 inner API 方法签名 + 结构体/枚举**:属 AGENTS.md "修改 public/inner API 签名"范畴,需维护者 sign-off。本方案以"API 尚未商用 + 纯规格演进草案"为前提接受新增,评审时确认该前提。
- **不新增错误码**:本设计不新增 NDK/内部错误码,`priority` 非法复用 `CLOUD_DISK_INVALID_ARG`/`E_INVALID_ARG`;评审确认"不新增 34400030"以保持 v3(27)→v4(28)→v5(29) 连续链叙事不被优先级打断。
- **`CloudDiskCallbackContext.fetchData` 变体类型变**:v5 该变体为 `CloudDiskPathInfo *`,本设计改为 `CloudDiskFetchDataRequest *`;属回调 ABI 变更(clean break,union 大小不变),需评审知悉。
- **强制显式 `priority` + 无默认值**:`HydratePlaceholder` 第 4 参强制显式,不提供无 priority 重载;评审确认"无双函数名入口"取舍可接受。
- **取消不带优先级**:`cancelFetchData` 维持 `PathInfo *`,取消按文件取消(不区分优先级);评审确认"取消不消费 priority 但签名强制传入"的取舍可接受(调用方对取消调用传 `NORMAL` 占位)。
- **不抢占**:高优先级不中断低优先级在途,仅影响排队顺序;评审确认"首版不取抢占"取舍可接受(抢占留待后续扩展)。
- **纯规格演进 + 不可执行**:本设计为对外结构体约束草案,不落代码、不落测试;评审确认"执行前将重新细化设计"的前提。
- **前置依赖链未落地**:v2 重构/v3 customInfo/v4 state-only/v5 脱水/回调表/水合主体均未落地(见 §1 事实基线);本设计无法先行,须待前置全落。评审知悉。
- 不改权限模型 / token 校验 / 多用户隔离 / uid-gid 归属 / RDB schema。
- DCO / `Signed-off-by`:按上游合入标准,commit 须含 `Signed-off-by`。

## 13. 后续扩展

1. **水合主体落地**(另案 SDD):`HydratePlaceholder` 的 state 推进(`1→2→3`)、`PlaceholderTaskManager` 任务结构、在途护栏(`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`,34400019)、`CANCEL_FETCH_DATA` 取消语义、framework 编解码(`WriteCallbackParcel`/`WriteCallbackReply`/`ReadCallbackReply` 扩 `FETCH_DATA` 优先级分支)、IDL `HydratePlaceholderInner`、service override、`DispatchFetchData` 改造、mock、测试。本设计的对外结构体约束(`FetchDataRequest`/`HydratePriority`/`fetchData` 变体)在水合主体落地时直接消费。
2. **抢占调度**:若未来需"高优先级中断低优先级在途",需处理在途任务取消 + state 回退(`PARTIALLY_HYDRATED`→`UNHYDRATED`)+ 已下载数据清理;可能新增 `URGENT` 优先级或独立抢占接口;v6 不取。
3. **取消带优先级**:若未来需"取消特定优先级在途",升级 `cancelFetchData` 变体为 `FetchDataRequest *`(带 priority);v6 不取(取消按文件取消)。
4. **流读(`FETCH_RANGE_DATA`)与优先级**:流读当前不带优先级(`fetchRangeData` 变体为 `RangeInfo *`,无 priority 字段);若未来流读也需优先级,可给 `RangeInfo` 加 `priority` 字段或新增 `FetchRangeDataRequest`;v6 不取(流读延后)。
5. **批量/目录水合 + 批量优先级**:本设计单文件;批量水合的优先级可作为后续扩展(IPC Parcelable 与签名需变)。
6. **DFX 打点**:水合优先级分布、排队等待时延、按优先级的下载完成率可纳入 `clouddiskservice-dfx-sdd` 的 `FSC_PLACEHOLDER` scenario(水合主体落地时)。
7. **优先级持久化**:若未来需"跨重启保留请求优先级"(如重启后继续排队),需把未完成请求的 priority 入 RDB 或 xattr;v6 不取(优先级是运行期 hint)。
