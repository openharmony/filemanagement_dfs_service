> 标题:clouddiskservice 占位符脱水(Dehydrate)NDK 接口设计 | 目录:`D:\code\dfs\filemanagement_dfs_service`
> 状态:设计稿,未实现。实现待本方案 sign-off 后单独进行;**前置依赖** v2 占位符重构(`docs/sdd/placeholder-marking-refactor-sdd.md`)、v3 customInfo(`docs/sdd/placeholder-custom-info-sdd.md`)与 v4 state-only 转换(`docs/sdd/placeholder-state-only-conversion-sdd.md`)落地。

# Placeholder Dehydrate SDD

## 1. 背景

`CloudDiskService`(三方网盘 SA)的占位符体系经 v2(合并 `filesyncstate` 高 3 位 `PlaceholderState`)、v3(customInfo)、v4(state-only Mark/Unmark,首次启用 `FULLY_HYDRATED(3)`)演进后,占位态已可表达"未水合(1)/部分水合(2)/完全水合(3)"。但 v4 §5.3 的 `OH_CloudDisk_DehydrateFile`(完全水合→未水合,回收本地数据)与 `OH_CloudDisk_HydratePlaceholder` 仍标为"设计态、未落地",当前无任何脱水实现路径。

外部诉求:三方网盘应用需在云端已持久化的前提下,把本地完整水合的占位符(数据全本地)回退为未水合占位符(纯 stub 稀疏空洞),回收本地空间。脱水的本质是**丢本地数据**,一旦云端尚未持久化即脱水将造成数据丢失,故脱水必须由掌握云端持久化状态的**应用侧授权**方可执行。

本设计在 v2~v4 占位态模型与 v4 state-only 转换的 `placeholder_helper`(`SetHighBits` 返回 old)基础上,落地 v4 §5.3 的 `OH_CloudDisk_DehydrateFile`,并通过**回调表**引入"应用授权出参":服务派发 `DEHYDRATE` 回调,应用在同步回执中回写 `bool allow` 决定是否执行脱水。回调表基础设施(`ICloudDiskServiceCallbackTable`/`OnCallback`/`WriteCallbackReply`/`ReadCallbackReply`)由独立变更集提供,本设计为其新增 `DEHYDRATE` 回调类型与 context 变体。

## 2. 目标

1. 新增 NDK `OH_CloudDisk_DehydrateFile(const CloudDisk_SyncFolderPath *syncFolderPath, const CloudDisk_PathInfo *filePath)`,`@since 26.1.0`(签名沿用 v4 §5.3 既定契约,指针入参,与 §4.1~4.7 值传递不同)。
2. 新增内层 IDL `void DehydrateInner([in] String syncFolder, [in] String relativePath)`,镜像 `ConvertPlaceholderToFileInner`;`void`→C++ `ErrCode DehydrateInner(...)` 返回最终结果码。
3. **强制回调授权**:每次脱水都派发 `DEHYDRATE` 回调到已注册 callback table;已注册表为硬前置,未注册→`OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`;应用回写 `allow=false`→不脱、返 `OH_CLOUD_DISK_DEHYDRATE_DENIED`,态/数据不变。
4. 回调契约新增:`CloudDiskCallbackType` 加 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE = 3`;`CloudDiskCallbackContext` 加变体 `CloudDiskDehydrateInfo *dehydrateData`(载 `filePath` 入参 + `allow` 出参);扩 `WriteCallbackParcel`(请求写 filePath)、`WriteCallbackReply`/`ReadCallbackReply`(回执写读 `allow`)、`IsValidCallbackType`。
5. 物理操作(整文件,前置 `state==FULLY_HYDRATED(3)`):`ftruncate(0)`→`ftruncate(logicalSize)`(重延伸失败重试)→`SetHighBits(UNHYDRATED(1))`→`fsync`,全程持 per-文件串行锁;不读写 `user.clouddisk.custominfo` xattr;`3→1` 子态切换不改目录 count、不祖先刷新(§2.2)。
6. 前置态:`state==3` 进行;`state==1`(已未水合)幂等返 `E_OK`(不操作);`state==0`(普通)→`OH_CLOUD_DISK_NOT_A_PLACEHOLDER`;`state==2`(部分)→`OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`(镜像 v4 Unmark 闸门)。
7. 并发:v1 仅 per-`(syncFolder,filePath)` 串行化锁;在途水合护栏(查 `PlaceholderTaskManager` 同文件 task→`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`)随水合落地一并加,不进脱水 v1。
8. 新增错误码:NDK 枚举 +1 项(34400029);内部枚举 +1 项(34400029);映射表 +1 行。详见 §4.2。
9. 测试**仅**落 `test/unittests/clouddiskservice/`(无下划线那棵),不改 `test/unittests/clouddisk_service/`。

## 3. 总体决策

| 项 | 决策 |
| --- | --- |
| 脱水语义 | 完全水合占位(3)→未水合占位(1),丢本地数据、保 logicalSize |
| 入口签名 | NDK `OH_CloudDisk_DehydrateFile(syncFolderPath*, filePath*)`(指针,§5.3);IDL `void DehydrateInner([in] String, [in] String)`(镜像 Convert) |
| 回调授权 | 强制:每次派发 `DEHYDRATE` 回调,已注册表为硬前置;`allow=false`→不脱、`DEHYDRATE_DENIED`;`allow` 为裸 bool(不带 reason) |
| 回调通道 | 同步 `OnCallback` 回执包(扩 `WriteCallbackReply`/`ReadCallbackReply`),**不**走 §5.2 `OH_CloudDisk_Execute`(Execute 仅 FETCH_DATA 数据回写) |
| 回调类型 | 新增 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE = 3`;新 context 变体 `CloudDiskDehydrateInfo{filePath; allow}` |
| 前置态 | `==3` 进行;`==1` 幂等 `E_OK`;`==0`→`NOT_A_PLACEHOLDER`(17);`==2`→`PLACEHOLDER_NOT_FULLY_HYDRATED`(28) |
| 物理操作 | `ftruncate(0)→ftruncate(logicalSize)`(重延伸重试)→`SetHighBits(1)`→`fsync`;数据先丢、state 末步 `3→1` |
| 顺序/回滚 | `ftruncate` 先(可重试;失败则数据仍在、state 仍 3、可重试)→`SetHighBits(1)` 末步;无瞬态 dehydrating |
| 读安全窗口 | `ftruncate` 与 `SetHighBits` 间 state==3+数据已空;v1 维持现状,随水合落地/FUSE 可见性一并处理(§13) |
| 触发模型 | 同步,单次 IPC(回调往返在 `DehydrateInner` 内同步完成);无 `TaskStateManager`、无 outputFd、无独立 Execute |
| customInfo | 不读写(v3 §2.6 正交) |
| count 副作用 | `3→1` 子态,占位语义不变,**不改 count、不祖先刷新**(§2.2) |
| 权限 | 经 `ResolvePlaceholderStatePath` / `CheckSyncFolderBundleName` 校验调用者身份、用户范围、目录归属和路径;不要求 `ACCESS_CLOUD_DISK_INFO` 或系统应用身份 |
| 入参风格 | NDK 指针(`const *`);IDL 值(`String`)—与 §5.3 一致,与 §4.1~4.7 值传递不同 |
| 新 Parcelable | 无(仅扩 context union 变体 + 新结构体 `CloudDiskDehydrateInfo`) |
| 特性开关 | `dfs_service_feature_enable_cloud_disk` / `SUPPORT_CLOUD_DISK_SERVICE`,各处 `#ifdef` 一致 |
| 错误码新增 | NDK `OH_CLOUD_DISK_DEHYDRATE_DENIED = 34400029`;内部 `E_DEHYDRATE_DENIED = 34400029`;映射表 +1 行 |
| 与既有接口关系 | 与 v4 Mark/Unmark 并存;脱水丢数据(3→1)、Mark 不动数据(0→3)、Unmark 不动数据(3→0)三者语义互补 |
| 前置依赖 | v2 重构(state 模型 + `SetHighBits`/读高3 helper)+ v3 customInfo(34400027 连续)+ v4 state-only(34400028 连续 + state 3 写入路径)先/同期落地 |

## 4. 状态语义与错误码

### 4.1 状态翻转模型

本设计不引入新占位态,复用 v2 `filesyncstate` 高 3 位 `PlaceholderState`(详见 `clouddisk-placeholder-v5.md` §2.2、`placeholder-marking-refactor-sdd.md` §3.2):

| 变迁 | old → new | delta | 说明 |
| --- | --- | --- | --- |
| Dehydrate | `FULLY_HYDRATED(3)` → `UNHYDRATED(1)` | `0` | 丢本地数据、保 logicalSize;`3→1` 子态切换,占位语义不变 |

- `IsPlaceholder(s) = (s != 0)`;`1` 与 `3` 均为占位符 → `3→1` 不改计数、不触发祖先刷新。
- 脱水是 state 1 的非创建性写入路径(既有写入者:`Create`/`Update` 置 1);不与 v4 Mark(0→3)冲突。

### 4.2 错误码

> 事实基线:
> - NDK 枚举 `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h`:v4 新增 `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028`(尚未落地);本设计紧接其后新增 34400029。
> - 内部枚举 `utils/clouddiskservice/include/cloud_disk_service_error.h`:v4 将补 `E_IS_A_PLACEHOLDER = 34400018`、`E_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028`(尚未落地);本设计在末尾新增 34400029。
> - NDK↔inner 映射表 `oh_cloud_disk_utils.h`(`innerToNErrTable`):v4 将增 2 行(18/28);本设计再增 1 行(29)。

**内部枚举** `utils/clouddiskservice/include/cloud_disk_service_error.h` 新增 1 项(末尾,沿用 `344xxxxx` 前缀):

```cpp
E_DEHYDRATE_DENIED = 34400029,  // 应用回调授权拒绝脱水(allow=false)
```

**NDK 枚举** `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h` 新增 1 项:

```c
OH_CLOUD_DISK_DEHYDRATE_DENIED = 34400029,
```

**NDK↔inner 映射表** `oh_cloud_disk_utils.h` + `oh_cloud_disk_utils.cpp` 增 1 行:

```cpp
{OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_DEHYDRATE_DENIED,
    OH_CLOUD_DISK_DEHYDRATE_DENIED},
```

其余错误码全部复用既有码:

- 前置态拒绝:`state==0`→`OH_CLOUD_DISK_NOT_A_PLACEHOLDER`(34400017);`state==2`→`OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`(34400028,v4);`state==1`→`E_OK`(幂等)。
- 无 callback table:`E_LISTENER_NOT_REGISTERED`(inner)→`OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`(34400021)——与框架 `PlaceholderCallbackManager::DispatchCallback` 既有行为一致。
- `ftruncate` 重延伸耗尽:`E_TRY_AGAIN`。
- 通用返回码:`CLOUD_DISK_OK`/`CLOUD_DISK_INVALID_ARG`/`CLOUD_DISK_PERMISSION_DENIED`/`CLOUD_DISK_IPC_FAILED`/`CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED`/`CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED`/`CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST`/`CLOUD_DISK_TRY_AGAIN`(均 `@since 21`)。
- 路径类:`OH_CLOUD_DISK_NOT_A_DIRECTORY`(34400023)/`OH_CLOUD_DISK_NAME_TOO_LONG`(34400025)——与既有 Convert/Update 校验链一致。
- `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`(34400019)维持"保留,当前无接口产生",随水合落地由在途水合护栏产生。

> 连续性(已定,见 §12):34400029 紧接 v4 的 34400028(尚未落地)。**v4 state-only 必须先或同期落地**,以保持 NDK 枚举 16~34400029、内部枚举 34400028→34400029 连续;不接受 34400028→34400030 的 29 空缺。连同 v3 customInfo(34400027),完整链为 v3(27)→v4(28)→v5(29)。

## 5. API / IDL / 各层改动

贯穿 NDK → inner API → framework → IDL/proxy → IPC → stub → service 全链。范式照搬既有 `ConvertPlaceholderToFile`(`cloud_disk_service_manager_impl.cpp:260-278`、`cloud_disk_service.cpp:1187-1233`),回调派发范式照搬 `PlaceholderCallbackManager::DispatchFetchData`。

### 5.1 NDK — `interfaces/kits/ndk/clouddiskmanager/`

- `include/oh_cloud_disk_manager.h`:新增函数声明,`@since 26.1.0`/`@kit CoreFileKit`/`@syscap SystemCapability.FileManagement.CloudDiskManager`/`@library libohclouddiskmanager.so`,紧邻 §5.3 水合脱水段:

```c
CloudDisk_ErrorCode OH_CloudDisk_DehydrateFile(
    const CloudDisk_SyncFolderPath *syncFolderPath,
    const CloudDisk_PathInfo *filePath);
```

- `liboh_cloud_disk_manager.ndk.json`:加符号 + `first_introduced: "26.1.0"`。
- `src/oh_cloud_disk_manager.cpp`:shim——空指针校验→ `CloudDiskServiceManager::GetInstance().DehydrateFile(...)` → `ConvertToErrorCode`。
- `include/cloud_disk_error_code.h`:加 `OH_CLOUD_DISK_DEHYDRATE_DENIED = 34400029`。
- `include/oh_cloud_disk_utils.h` + `src/oh_cloud_disk_utils.cpp`:映射表 `innerToNErrTable` 增 1 行(`E_DEHYDRATE_DENIED`→`OH_CLOUD_DISK_DEHYDRATE_DENIED`)。

### 5.2 inner API — `interfaces/inner_api/native/clouddiskservice_kit_inner/`

- `cloud_disk_common.h`:§1.3 水合脱水数据契约新增:
  - `CloudDiskCallbackType` 加 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE = 3`。
  - 新增 `CloudDiskDehydrateInfo { CloudDisk_PathInfo filePath; bool allow; }`(`filePath`=服务→应用入参,`allow`=应用→服务出参,app 在 `OnCallback` 中填充)。
  - 命名桥接:spec §1.3 作 `CloudDisk_DehydrateInfo`(承 v4 回调类型 `CloudDisk_RangeInfo` 等下划线约定);实现层 `cloud_disk_common.h` 依既有无下划线惯例(`CloudDiskCallbackContext`/`CloudDiskRangeInfo`/`CloudDiskFetchData` 同族)作 `CloudDiskDehydrateInfo`,二者为同一类型。下文代码片段沿用实现层无下划线命名。
  - `CloudDiskCallbackContext` union 加变体 `CloudDiskDehydrateInfo *dehydrateData`。
- `cloud_disk_service_manager.h`:紧邻 `ConvertPlaceholderToFile` 新增虚函数:

```cpp
virtual int32_t DehydrateFile(const std::string &syncFolder,
                              const std::string &relativePath) = 0;
```

- `clouddiskservice_kit_inner.map`:补新符号导出(`DehydrateFile`)。

### 5.3 framework — `frameworks/native/clouddiskservice_kit_inner/`

- `src/cloud_disk_common.cpp`:扩回调编解码(范式照搬既有 `FETCH_RANGE` 分支):
  - `IsValidCallbackType`:加 `DEHYDRATE` 判定。
  - `WriteCallbackParcel`:`DEHYDRATE` 分支写 `filePath`。
  - `WriteCallbackReply`:`DEHYDRATE` 分支写 `allow`(bool)。
  - `ReadCallbackReply`:`DEHYDRATE` 分支读 `allow` 回填 `dehydrateData->allow`。
- `include/cloud_disk_service_manager_impl.h`:`DehydrateFile` override。
- `src/cloud_disk_service_manager_impl.cpp`:实现,范式照搬 `ConvertPlaceholderToFile`(`:260-278`):`#ifdef SUPPORT_CLOUD_DISK_SERVICE` → `ServiceProxy::GetInstance()` 空检 → `E_IPC_FAILED` → `serviceProxy->DehydrateInner(syncFolder, relativePath)` → `SetDeathRecipient` → `#else return E_NOT_SUPPORTED`。

### 5.4 IDL — `services/clouddiskservice/ICloudDiskService.idl`

紧邻 `ConvertPlaceholderToFileInner` 新增(写法与既有 Convert 一致,`void` 返回 + 两个 `[in] String`):

```
void DehydrateInner([in] String syncFolder, [in] String relativePath);
```

`services/clouddiskservice/BUILD.gn` 的 `idl_gen_interface("cloud_disk_service")` 自动重生成 stub/proxy;接口码由 IDL 工具链自动递增。**`clouddiskservice` 接口码不在 `CODEOWNERS` 额外评审清单**(CODEOWNERS 仅覆盖 distributedfiledaemon / cloud_daemon_kit / cloud_file_sync_service 三处),故无额外 IPC 评审路径。

### 5.5 service — `services/clouddiskservice/ipc/`

- `include/cloud_disk_service.h`:紧邻 `ConvertPlaceholderToFileInner` 新增 override 声明:

```cpp
ErrCode DehydrateInner(const std::string &syncFolder, const std::string &relativePath) override;
```

- `src/cloud_disk_service.cpp`:`DehydrateInner` impl,行为见 §6。
- `include/placeholder_callback_manager.h` + `src/placeholder_callback_manager.cpp`:新增 `DispatchDehydrate`,范式照搬 `DispatchFetchData`:

```cpp
int32_t DispatchDehydrate(const std::string &bundleName, uint32_t syncFolderIndex,
                         CloudDiskCallbackReqHead &reqHead, CloudDisk_PathInfo &filePath)
{
    reqHead.callbackType = CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE;
    CloudDiskCallbackContext reqContext{};
    reqContext.dehydrateData = &dehydrateInfo;  // dehydrateInfo.filePath = filePath; allow 待 app 填
    return DispatchCallback(bundleName, syncFolderIndex, reqHead, reqContext);
}
```

`DispatchCallback` 同步 `SendRequest(SERVICE_CMD_ON_CALLBACK)` + `ReadCallbackReply`(由 `CloudDiskServiceCallbackTableProxy::SendCallback` 完成),读回 `dehydrateInfo.allow`。

## 6. 行为语义

### 6.1 DehydrateInner(完全水合→未水合,gate state==3 + allow)

入口先拒绝空 `syncFolder` / `filePath`(`E_INVALID_ARG`)。以下步骤 1~3 由 `ResolvePlaceholderStatePath` 完成,不调用 `CheckPermissions`。

```
1. GetUserId(); userId == 0 时尝试 GetAccountId(); GetCallerBundleName(bundleName)
   GetCallerBundleName 失败 → E_TRY_AGAIN
2. CheckSyncFolderBundleName(syncFolder, userId, bundleName)
   未注册 → E_SYNC_FOLDER_NOT_REGISTERED;归属不匹配 → E_SYNC_FOLDER_PATH_UNAUTHORIZED
   同步目录物理路径映射失败 → 返回对应错误
3. GetHmdfsPath(syncFolder, filePath, userId, hmdfsPath) + GetSyncRootContext(...)
   校验相对路径并解析文件、同步根上下文;失败 → 返回对应错误
4. 取 per-(syncFolder,filePath) 串行锁(防并发脱水交错)
5. 读 high3(v2 helper):
   state == 0  → E_NOT_A_PLACEHOLDER (34400017)
   state == 1  → E_OK (幂等,不操作,释放锁返回)
   state == 2  → E_PLACEHOLDER_NOT_FULLY_HYDRATED (34400028)
   state == 3  → 放行
6. DispatchDehydrate(bundleName, syncFolderIndex, reqHead, filePath):
   - 无 callback table → E_LISTENER_NOT_REGISTERED (→ NDK CALLBACK_NOT_REGISTERED 34400021)
   - 同步派发 OnCallback(DEHYDRATE),app 填 dehydrateData->allow
   - 读回执 allow:
     allow == false → E_DEHYDRATE_DENIED (34400029),不脱、态/数据不变,释放锁返回
     allow == true  → 继续
7. 打开文件(O_RDWR | O_NOFOLLOW | O_CLOEXEC),取 logicalSize = 当前 size(state==3 ⇒ 数据全本地,size==logicalSize)
8. ftruncate(fd, 0)                    // 丢本地数据
   失败 → 经 ConvertErrnoToCloudDiskError 归一;释放锁返回
9. ftruncate(fd, logicalSize)          // 重建稀疏空洞(保 logicalSize);失败重试此步
   重试耗尽 → E_TRY_AGAIN(此时文件 0 字节、state 仍 3;调用方可重试 DehydrateFile,state==3 仍进行、ftruncate 幂等)
10. SetHighBits(file, UNHYDRATED(1))   // v2 helper,返回 old(应 ==3);末步翻态 3→1
    失败 → 释放锁返回错误(state 仍 3、数据已丢;重试见步骤 9 注)
11. fsync(fd)
12. 不读写 user.clouddisk.custominfo xattr(v3 §2.6 正交)
13. 3→1 子态切换,不调祖先刷新 helper(§2.2 占位语义不变、不改 count)
14. 释放锁,返回 E_OK
```

- `ftruncate` 顺序(数据先丢、state 末步翻)的可重试性:若 `SetHighBits` 失败,重试 `DehydrateFile` 见 `state==3`(未翻)、`ftruncate` 幂等(0→0、logicalSize→logicalSize 均无副作用)、再置 1。
- **读安全窗口**(已知,§13):步骤 8~10 之间 state 仍 `==3`(声称完全水合、本地有数据)但数据已被打空;此间 FUSE/hmdfs 读按"本地有数据"直读稀疏空洞→读到全零。v1 维持现状,per-文件串行锁只串行化 `DehydrateFile`、不挡 FUSE 读路径;随水合落地与 FUSE 可见性一并处理。
- 决策方(应用云同步 handler)≠ 调用方(如文件管理器)时,`allow` 裸 bool 使调用方只能盲重试或放弃;本设计接受该取舍(不引 reason 字段,见 §12)。

### 6.2 回调契约(同步回执出参)

- 服务侧:`CloudDiskServiceCallbackTableProxy::SendCallback` 已有同步 `SendRequest(SERVICE_CMD_ON_CALLBACK)` + `ReadCallbackReply`;脱水复用,只需 `ReadCallbackReply` 支持 `DEHYDRATE` 分支读 `allow`。
- 应用侧:`CloudDiskServiceCallbackTableStub::HandleOnCallback` 调 `OnCallback(reqHead, reqContext)`(可写 `reqContext`),app 在 `OnCallback` 中按 `callbackType==DEHYDRATE` 填 `dehydrateData->allow`,stub 再 `WriteCallbackReply` 写 `allow` 入回执。
- **不**经 §5.2 `OH_CloudDisk_Execute`:`Execute`/`CloudDiskCallbackResponse` 仅用于 `FETCH_DATA` 数据回写;脱水决定是单 bool,走 `OnCallback` 回执包(与 `FETCH_RANGE` 的 range data 回执同机制)。
- 嵌套 IPC 线程安全:`DehydrateInner`(服务 binder 线程)→ `SendCallback`(同步 IPC 到 app)→ app callback stub 跑在 app 的另一 binder 线程(非阻塞 `DehydrateInner` 的调用线程),无自死锁。

## 7. 校验与错误处理

| 场景 | 码 | 位置 |
| --- | --- | --- |
| `syncFolder` 未注册 / bundle 不匹配 | `E_SYNC_FOLDER_NOT_REGISTERED`/`E_SYNC_FOLDER_PATH_UNAUTHORIZED` | service(`CheckSyncFolderBundleName`) |
| `state==0`(普通文件) | `E_NOT_A_PLACEHOLDER`(34400017) | service |
| `state==2`(部分水合) | `E_PLACEHOLDER_NOT_FULLY_HYDRATED`(34400028,v4) | service |
| `state==1`(已未水合) | `E_OK`(幂等) | service |
| 无 callback table | `E_LISTENER_NOT_REGISTERED`(→NDK `CALLBACK_NOT_REGISTERED` 34400021) | service(`DispatchCallback`) |
| `allow==false` | `E_DEHYDRATE_DENIED`(34400029,新) | service |
| `ftruncate` 重延伸耗尽 | `E_TRY_AGAIN` | service |
| 路径父目录非目录 / 名过长 | `E_NOT_A_DIRECTORY`(34400023)/`E_NAME_TOO_LONG`(34400025) | service(既有校验链) |
| `GetCallerBundleName` 失败(含 twin app `instIndex!=0`) | `E_TRY_AGAIN` | service |
| xattr 读/写 IO 失败 | 经 `ConvertErrnoToCloudDiskError` 归一 | service |
| `ServiceProxy` 为空 | `E_IPC_FAILED` | framework |
| 特性开关关闭 | `E_NOT_SUPPORTED` | framework `#else` |

- 校验须在 `ftruncate`/`SetHighBits` 之前,避免半成品需回滚(`state==1` 幂等、`state==0/2` 前置拒绝均在步骤 5,先于数据操作)。
- `ftruncate(0)` 后 `ftruncate(logicalSize)` 失败不回滚到原数据(数据已丢,但云端有 app 授权 `allow==true` 时的持久化);返回 `E_TRY_AGAIN`,调用方可重试(`state` 仍 3)。

## 8. 安全与约束

1. `DehydrateInner` 不调用 `CheckPermissions`,不要求系统权限或系统应用身份;仍经 `ResolvePlaceholderStatePath` 获取调用者 userId/bundleName,由 `CheckSyncFolderBundleName` 校验目录归属,并保留路径校验和 `DEHYDRATE` 回调授权。
2. `GetCallerBundleName` 拒 twin app(`instIndex != 0`),与既有 per-file 占位操作一致。
3. `GetUserId` 由 `callingUid / BASE_USER_RANGE` 派生,多用户隔离不可绕过。
4. 路径校验不绕过(`..` 逃逸走 `ReplacePathPrefix`/`GetHmdfsPath`,路径须落在已注册 sync folder 范围内)。
5. 本次只移除网盘侧额外的 `ACCESS_CLOUD_DISK_INFO` 要求;不改调用者身份解析、多用户隔离、uid/gid 归属或 RDB schema。同步目录注册/注销及 ArkTS 系统接口鉴权保持不变,见 [spec §5、§7](../agent-knowledge/clouddisk-placeholder-spec.md)。
6. 不读写 customInfo xattr(v3 §2.6 正交);调用方需 customInfo 另行调既有 `UpdatePlaceholder`。
7. 不引入新第三方依赖、无常驻缓存;`PlaceholderCallbackManager` 既有 death recipient 覆盖回调表死亡清理。
8. xattr 资源归属不变(占位符文件类资产归属各应用,uid/gid 逻辑不动)。

## 9. 契约 / ABI / 兼容影响

- **非破坏面**:本设计为**纯新增**——新增 1 NDK 符号、1 inner 虚函数、1 IDL 方法、1 SA override、1 回调类型枚举值、1 context union 变体、1 结构体(`CloudDiskDehydrateInfo`)。不改 `PlaceholderInfo`/`PlaceholderCustomInfo` Parcelable 布局;不改既有 `filesyncstate`/`custominfo` xattr 语义;既有 `ConvertPlaceholderToFile`/`MarkFileAsPlaceholder`/`UnmarkPlaceholderFile` 签名与行为不变。
- **错误码新增面**:NDK 枚举 +1(34400029);内部枚举 +1(`E_DEHYDRATE_DENIED`=34400029);映射表 +1 行。均 append-only,不改既有值,不影响既有调用方与 DFX 上报码段。
- **回调 ABI**:`CloudDiskCallbackContext` union 加变体属新增(union 大小不变,指针变体);`CloudDiskCallbackType` 枚举 append(值 3);回执包格式对既有 `FETCH_*` 类型不变(`WriteCallbackReply`/`ReadCallbackReply` 按 `callbackType` 分支,既有分支不动)。
- **特性开关**:`dfs_service_feature_enable_cloud_disk` 关闭时,framework `#ifdef SUPPORT_CLOUD_DISK_SERVICE` 走 `#else return E_NOT_SUPPORTED`;BUILD 与代码条件一致(AGENTS.md 硬约束)。
- **前置依赖**:v2 重构(`SetHighBits`/读高3 helper)、v3 customInfo(34400027)、v4 state-only(34400028 + state 3 写入路径)未落地时本设计无法实现。变更集顺序:①v2 重构先合,②v3 customInfo + v4 state-only 次之,③本设计(脱水)后叠。
- `clouddiskservice` IDL 接口码不在 `CODEOWNERS` 额外评审清单,无额外 IPC 评审路径;但内部仍同步 proxy/stub/manager/mock/测试。
- 26.1.0 未商用:新增符号属 clean break 范畴,无既有消费方二进制兼容负担。

## 10. 测试要求

测试**仅**落 `test/unittests/clouddiskservice/`(无下划线那棵),**不**改 `test/unittests/clouddisk_service/`(下划线那棵,另一棵测试树)。mock callback table(返 `allow` true/false)与 v2 `SetHighBits`/读高3 helper。

1. **NDK shim 单测**(`ndk/oh_cloud_disk_manager_test.cpp`,target `oh_cloud_disk_manager_test`):空参 → `CLOUD_DISK_INVALID_ARG`;调 manager mock 返 `E_OK`/`E_DEHYDRATE_DENIED`/`E_NOT_A_PLACEHOLDER`/`E_PLACEHOLDER_NOT_FULLY_HYDRATED`/`E_LISTENER_NOT_REGISTERED` → 经 `ConvertToErrorCode` 映射校验(含新增 34400029 映射行)。
2. **SA 单测**(`ipc/cloud_disk_service_test.cpp`,target `cloud_disk_service_test`):
   - 决定:`allow=true`→脱水成功(state 3→1,`ftruncate(0)`+`ftruncate(logicalSize)` 调用、`SetHighBits(1)` 调用);`allow=false`→`DEHYDRATE_DENIED`,state/数据不变(断言无 `ftruncate`)。
   - 无 callback table→`E_LISTENER_NOT_REGISTERED`。
   - 前置态:`state==3`进行 / `state==2`→34400028 / `state==1`→幂等 `E_OK`(断言无 `ftruncate`、无 `SetHighBits`)/ `state==0`→34400017。
   - 物理:`ftruncate(logicalSize)` 重延伸失败→重试→`E_TRY_AGAIN`;`SetHighBits` 失败→返回错误(state 仍 3)。
   - customInfo xattr:脱水前后 `user.clouddisk.custominfo` 不变(正交用例)。
   - 并发:两并发同文件 `DehydrateFile` 串行化(不交错 `ftruncate`/`SetHighBits`)。
   - IPC:`DEHYDRATE` 的 `WriteCallbackParcel`/`WriteCallbackReply`/`ReadCallbackReply` 往返编解码(`allow` 正确回填)。
   - 归属:跨 bundle / 未注册 sync folder / 无法解析调用者身份 → 拒绝;不因缺少 `ACCESS_CLOUD_DISK_INFO` 而拒绝合法网盘调用,后续仍须通过文件状态和回调授权校验。
   - 权限移除回归:`cloud_disk_service_static_test.cpp` 中断言 `CheckCallerPermission(_)` 调用次数为 0,归属不匹配仍返回 `E_SYNC_FOLDER_PATH_UNAUTHORIZED`。
3. **mock 同步**(`test/unittests/clouddiskservice/mock/`):
   - `mock/icloud_disk_service.h`:加 `DehydrateInner` 接口码 + virtual 方法。
   - `mock/cloud_disk_service_stub.{h,cpp}`:加 `DehydrateInner` stub 分发。
   - `mock/cloud_disk_service_manager_mock.{h,cpp}`:加 `MOCK_METHOD`(`DehydrateFile`)。
   - `mock/icloud_disk_service_callback_table.h`(若存在):加 `DEHYDRATE` 类型;否则随回调表 mock 一并补。
   - **仅这一棵 mock 树**;`test/unittests/clouddisk_service/mock/` 不动。
4. 特性开关:`SUPPORT_CLOUD_DISK_SERVICE` 关闭路径返回 `E_NOT_SUPPORTED`(若既有测试覆盖该模式)。

## 11. 实现落点(未实现,sign-off 后进行)

| 内容 | 路径 |
| --- | --- |
| NDK 函数声明 | `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h` |
| NDK shim | `interfaces/kits/ndk/clouddiskmanager/src/oh_cloud_disk_manager.cpp` |
| NDK 符号清单 | `interfaces/kits/ndk/clouddiskmanager/liboh_cloud_disk_manager.ndk.json` |
| NDK 错误码 + 映射 | `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h`(加 34400029)+ `include/oh_cloud_disk_utils.h` + `src/oh_cloud_disk_utils.cpp`(映射表加 1 行) |
| 回调契约类型 + context 变体 | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_common.h`(加 `DEHYDRATE` 类型、`CloudDiskDehydrateInfo`、context 变体) |
| inner manager 虚函数 + `.map` | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_service_manager.h` + `clouddiskservice_kit_inner.map` |
| framework 回调编解码 + manager impl | `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_common.cpp` + `include/cloud_disk_service_manager_impl.h` + `src/cloud_disk_service_manager_impl.cpp` |
| IDL | `services/clouddiskservice/ICloudDiskService.idl` |
| service override + impl | `services/clouddiskservice/ipc/{include/cloud_disk_service.h,src/cloud_disk_service.cpp}` |
| 回调派发 | `services/clouddiskservice/ipc/{include/placeholder_callback_manager.h,src/placeholder_callback_manager.cpp}`(加 `DispatchDehydrate`) |
| 错误码枚举 | `utils/clouddiskservice/include/cloud_disk_service_error.h`(加 `E_DEHYDRATE_DENIED=34400029` 于末尾) |
| mock | `test/unittests/clouddiskservice/mock/{icloud_disk_service.h,cloud_disk_service_stub.{h,cpp},cloud_disk_service_manager_mock.{h,cpp}}` |
| 测试 | `test/unittests/clouddiskservice/{ndk/oh_cloud_disk_manager_test.cpp,ipc/cloud_disk_service_test.cpp}` |

> 行号为当前代码树快照(取自探索期),实现时以最新树为准。

## 12. Ask-before / 评审项

- **新增 inner API 方法签名 + 新错误码语义**:属 AGENTS.md "修改 public/inner API 签名、错误码语义"范畴,需维护者 sign-off。本方案以"API 尚未商用"为前提接受新增,评审时确认该前提。
- **新增错误码** `E_DEHYDRATE_DENIED`(34400029,末尾):append-only 到 shared `utils/clouddiskservice` 错误头,低风险;按 utils/AGENTS 检查无调用方冲突。NDK 枚举同步加 `OH_CLOUD_DISK_DEHYDRATE_DENIED = 34400029`。
- **前置依赖 v2/v3/v4**:`filesyncstate` 高 3 位重解释 + `placeholder_helper`(`SetHighBits`/读高3)来自 v2;34400027(v3)/34400028(v4)保错误码连续;v4 state-only 提供 state 3 写入路径(Mark)。三者须先/同期落地,本设计不可先行。
- **回调授权强制 + 裸 bool 出参**:脱水丢本地数据,授权由应用(掌握云端持久化)给出;`allow` 不带 reason 字段——决策方(应用 handler)≠ 调用方(文件管理器等)时调用方只能盲重试/放弃,本设计接受该取舍以避免复杂化(详见 §6.1)。评审知悉。
- **读安全窗口**(§6.1):`ftruncate` 与 `SetHighBits` 间 state==3+数据已空,per-文件锁不挡 FUSE 读;v1 维持现状,随水合落地与 FUSE 可见性一并处理(§13)。评审知悉,不作为 v1 阻塞项。
- **在途水合护栏延后**:`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`(34400019)由脱水 v1 产生面维持"无";护栏随水合落地补(§13),与 v4 §5.3 v4 注"水合落地时复核二者协调"一致。
- 已按维护者确认移除网盘侧额外系统权限要求;调用者身份解析 / 目录归属 / 多用户隔离 / uid-gid 归属 / RDB schema 不变。后续调整权限边界仍须确认。
- DCO / `Signed-off-by`:按上游合入标准,commit 须含 `Signed-off-by`。

## 13. 后续扩展

1. **水合落地**:`OH_CloudDisk_HydratePlaceholder`(spec §5.3)落地时:①补在途水合护栏(脱水前查 `PlaceholderTaskManager` 同文件 task→`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`);②复核与脱水 state 3 写入路径协调(Mark 直置 3 vs 水合 1→2→3 推进);③读安全窗口与 FUSE 对"脱水/水合进行中文件"的可见性一并处理(可能引入瞬态或 FUSE 锁,届时重审 §6.1 顺序)。
2. **拒绝原因细化**:若未来调用方需区分"未持久化(重试)vs 在用(放弃)",可给 `CloudDiskDehydrateInfo` 加 `reason` 字段(复用 `CloudDisk_ErrorReason` 或新建枚举),并扩 `WriteCallbackReply`;v1 不引。
3. **批量/目录脱水**:本设计单文件;批量/目录递归可作为后续扩展(IPC Parcelable 与签名需变)。
4. **DFX 打点**:脱水成功/`DEHYDRATE_DENIED`/前置拒绝可纳入 `clouddiskservice-dfx-sdd` 的 `FSC_PLACEHOLDER` scenario。
