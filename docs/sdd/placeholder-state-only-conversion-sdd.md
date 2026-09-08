> 标题:clouddiskservice 占位符/普通文件 state-only 转换 NDK 接口设计 | 目录:`D:\code\dfs\filemanagement_dfs_service`
> 状态:设计稿,未实现。实现待本方案 sign-off 后单独进行;**前置依赖** v2 占位符重构(`docs/sdd/placeholder-marking-refactor-sdd.md`)落地。

# Placeholder State-only Conversion SDD

## 1. 背景

`CloudDiskService`(三方网盘 SA)已提供占位符能力,但既有"占位符→普通"接口 `OH_CloudDisk_ConvertPlaceholderToFile`(`interfaces/kits/ndk/clouddiskmanager`、`services/clouddiskservice/ipc/src/cloud_disk_service.cpp` 的 `ConvertPlaceholderToEmptyFile`)是**数据破坏型**:它 `ftruncate(fd, 0)` 清空本地内容后再置占位态为 `NONE(0)`。反向"普通→占位符"则只有 `CreatePlaceholder`(新建文件,`O_CREAT|O_EXCL`)与 `UpdatePlaceholder`(会 `ftruncate(0)` 重置)两条路径,均不适用于"把既有本地普通文件标记为占位符、且保留其本地数据"。

新的外部诉求:提供显式 NDK 接口,进行普通文件与占位符文件的**双向转换**,且**只动状态、不动数据**。典型场景——本地已完整下载的文件,要纳入云管(普通→占位符);或已完全水合的占位符要脱离云管变为纯本地普通文件(占位符→普通),两者均不应清空或下载数据。

本设计在 v2 占位符重构(高 3 位 `PlaceholderState`、4 态编码、祖先 count 刷新 helper)基础上,新增两个**纯状态翻转**的 NDK 接口,复用 v2 重构的祖先刷新能力,不引入新 Parcelable、不动 customInfo。

## 2. 目标

1. 新增两个 NDK 接口:`OH_CloudDisk_MarkFileAsPlaceholder`(普通→占位符)与 `OH_CloudDisk_UnmarkPlaceholderFile`(占位符→普通),均为**只翻状态、不动数据**。
2. 普通→占位符:目标态 = `FULLY_HYDRATED(3)`(数据全本地,即已水合占位符);**不** `ftruncate`、**不**写 size/atime/mtime、**不**写 customInfo xattr。
3. 占位符→普通:目标态 = `NONE(0)`;前置 `state == FULLY_HYDRATED(3)`,拒绝 `UNHYDRATED(1)`/`PARTIALLY_HYDRATED(2)`(避免产生"带稀疏空洞读零"的伪普通文件);**不** `ftruncate`(与既有数据破坏型 `ConvertPlaceholderToFile` 的关键差异)。
4. 双向均复用 v2 重构的 `placeholder_helper`:`SetHighBits` 返回 old state → `delta = IsPlaceholder(new) − IsPlaceholder(old)` → 调祖先刷新 helper(普通→占位 `+1`、占位符→普通 `−1`)→ 同步 `DentrySetPlaceholder`(SDD §8.1 IPC 路径要求)。
5. 同步、单次 IPC、无 callback、无 `TaskStateManager`。
6. 权限沿用 per-file 占位操作的 bundleName 归属模型,不走 `ACCESS_CLOUD_DISK_INFO`。
7. 不新增 Parcelable;不改 `PlaceholderInfo`/`PlaceholderCustomInfo` 布局;不动 `user.clouddisk.custominfo` xattr。
8. 新增错误码:NDK 枚举新增 1 项(34400028);内部枚举新增 2 项(填补 `E_IS_A_PLACEHOLDER`=34400018 空缺 + `E_PLACEHOLDER_NOT_FULLY_HYDRATED`=34400028);映射表新增 2 行。详见 §4.2。

## 3. 总体决策

| 项 | 决策 |
| --- | --- |
| 转换语义 | 只翻 `filesyncstate` 高 3 位,**无 `ftruncate`、无 size/customInfo 写** |
| 两方向 | 双向:普通→占位(→`FULLY_HYDRATED(3)`)+ 占位符→普通(→`NONE(0)`) |
| Mark 目标态 | `FULLY_HYDRATED(3)`——本接口为 state 3 的**首个写入路径**(v3 标 2/3"预留",本设计提前启用 3) |
| Unmark 前置 | `state==0` → `NOT_A_PLACEHOLDER`(34400017);`state∈{1,2}` → 新增 `PLACEHOLDER_NOT_FULLY_HYDRATED`(34400028);`state==3` 放行 |
| Mark 前置 | `state!=0` → `IS_A_PLACEHOLDER`(34400018)——NDK 枚举已存在(reserved),v4 启用产生;内部枚举需新增 `E_IS_A_PLACEHOLDER` |
| 入参风格 | 传值(`const CloudDisk_SyncFolderPath`/`const CloudDisk_PathInfo`),与 §4.1–4.4 Convert/Update 同族一致 |
| 触发模型 | 同步,单次 IPC;无 callback、无 `TaskStateManager` |
| 权限 | bundleName 归属(`CheckSyncFolderBundleName`),不走 `ACCESS_CLOUD_DISK_INFO` |
| customInfo | 不读写(v3 §2.6 正交) |
| PlaceholderInfo | 不带(文件自有 size/atime/mtime,`FULLY_HYDRATED` 下 logicalSize = 实际 size) |
| 新 Parcelable | 无 |
| 特性开关 | `dfs_service_feature_enable_cloud_disk` / `SUPPORT_CLOUD_DISK_SERVICE`,各处 `#ifdef` 一致 |
| 错误码新增 | NDK 枚举新增 `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028`(`OH_CLOUD_DISK_IS_A_PLACEHOLDER`=34400018 已存在,复用);内部枚举新增 `E_IS_A_PLACEHOLDER = 34400018`(填 17→19 间空缺)+ `E_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028`;映射表增 2 行。34400028 紧接 v3 customInfo 的 34400027,**v3 customInfo 必须先/同期落地**以保持连续,见 §12 |
| 与既有 Convert 关系 | **并存**(既有数据破坏型 `ConvertPlaceholderToFile` 保留,新接口为独立符号) |
| 前置依赖 | v2 占位符重构(`placeholder-marking-refactor-sdd`)先合,本设计后叠 |

## 4. 状态语义与错误码

### 4.1 状态翻转模型(无新数据结构)

本设计**不引入新结构体、不动 `PlaceholderInfo`/`PlaceholderCustomInfo`**。状态翻转复用 v2 `filesyncstate` 高 3 位 `PlaceholderState`(详见 `clouddisk-placeholder-v4.md` §2.2、`placeholder-marking-refactor-sdd.md` §3.2):

| 变迁 | old → new | delta | 说明 |
| --- | --- | --- | --- |
| Mark(普通→占位) | `NONE(0)` → `FULLY_HYDRATED(3)` | `+1` | 数据全本地,即已水合占位符;**不 `ftruncate`、不写 size/time/customInfo** |
| Unmark(占位→普通) | `FULLY_HYDRATED(3)` → `NONE(0)` | `−1` | **不 `ftruncate`**(保留本地数据);与既有 `ConvertPlaceholderToEmptyFile`(`ftruncate(0)`+`NONE`)的关键差异 |

- `IsPlaceholder(s) = (s != 0)`;`0↔{1,2,3}` 改变计数 → 触发祖先 count 刷新;`1↔2↔3` 不改计数(本设计不涉及)。
- Mark 是 state 3 的首个写入路径。v3 §3.2 注"2/3 为未来水合预留(clouddiskservice 现无水合路径)";本设计通过"标记既有本地文件"启用 state 3,语义自洽(数据全本地 ⇔ `FULLY_HYDRATED`),不违背 v2/v3 编码契约,仅提前启用预留态。

### 4.2 错误码

> 事实基线(实际代码树,非 spec 累积态):
> - NDK 枚举 `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h`:已有 `OH_CLOUD_DISK_IS_A_PLACEHOLDER = 34400018`(line 173,`@since 26.1.0`,reserved)、`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS = 34400019` 等;**现有最大 `OH_CLOUD_DISK_FILE_TOO_LARGE = 34400026`**(line 221)。`OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`(34400027,v3 customInfo)**尚未落地**。
> - 内部枚举 `utils/clouddiskservice/include/cloud_disk_service_error.h`(`CloudDiskServiceErrCode`):**稀疏**——有 `E_NOT_A_PLACEHOLDER = 34400017`、`E_HYDRATE_IN_PROGRESS = 34400019`(17→19 间**空缺 34400018**)、`E_FILE_TOO_LARGE = 34400026`(末尾);**无** `E_IS_A_PLACEHOLDER`、**无** `E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`、**无** `E_CALLBACK_*`(21/22)。
> - NDK↔inner 映射表 `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_utils.h`(`innerToNErrTable`):有 `E_NOT_A_PLACEHOLDER→OH_CLOUD_DISK_NOT_A_PLACEHOLDER` 等行;**无 34400018 行**(因内部无 `E_IS_A_PLACEHOLDER`)。

**内部枚举** `utils/clouddiskservice/include/cloud_disk_service_error.h` 新增 2 项(沿用 `344xxxxx` 前缀;`34400018` 填 17→19 间空缺、`34400028` 置末尾;实际现有最大为 `E_FILE_TOO_LARGE = 34400026`):

```cpp
E_IS_A_PLACEHOLDER = 34400018,               // 填 17→19 间空缺;目标已是占位符,Mark 产生
// ... 既有 E_HYDRATE_IN_PROGRESS = 34400019 ... E_FILE_TOO_LARGE = 34400026 ...
E_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028,  // 占位符存在但未完全水合(state 1/2),不可 Unmark
```

**NDK 枚举** `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h` 新增 1 项(`OH_CLOUD_DISK_IS_A_PLACEHOLDER = 34400018` 已存在 line 173,复用;只新增 34400028):

```c
OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028,
```

**NDK↔inner 映射表** `oh_cloud_disk_utils.h`(`innerToNErrTable`)+ `oh_cloud_disk_utils.cpp` 增 2 行:

```cpp
{OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_IS_A_PLACEHOLDER,
    OH_CLOUD_DISK_IS_A_PLACEHOLDER},
{OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_PLACEHOLDER_NOT_FULLY_HYDRATED,
    OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED},
```

其余错误码全部复用既有码:

- Mark 前置失败(已是占位符):`OH_CLOUD_DISK_IS_A_PLACEHOLDER`(34400018)——NDK 枚举已存在,v4 启用产生;内部枚举/映射表如上补。
- Unmark 前置失败(根本不是占位符,`state==0`):`OH_CLOUD_DISK_NOT_A_PLACEHOLDER`(34400017)——既有,无需新增。
- 通用返回码:`CLOUD_DISK_OK`/`CLOUD_DISK_INVALID_ARG`/`CLOUD_DISK_PERMISSION_DENIED`/`CLOUD_DISK_IPC_FAILED`/`CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED`/`CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED`/`CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST`/`CLOUD_DISK_TRY_AGAIN`(均 `@since 21`)。
- 路径类:`OH_CLOUD_DISK_NOT_A_DIRECTORY`(34400023)/`OH_CLOUD_DISK_NAME_TOO_LONG`(34400025)——与既有 Convert/Update 校验链一致。

> 连续性(已定,见 §12):34400028 紧接 v3 customInfo 的 34400027(尚未落地)。**v3 customInfo 必须先或同期落地**,以保持 NDK 枚举 16~34400028 与内部枚举 34400027→34400028 连续;不接受 34400026→34400028 的 27 空缺。

## 5. API / IDL / 各层改动

贯穿 NDK → inner API → framework → IDL/proxy → IPC → stub → service 全链。范式照搬既有 `ConvertPlaceholderToFile`(`cloud_disk_service_manager_impl.cpp:260-278`、`cloud_disk_service.cpp:1187-1233`)。

### 5.1 NDK — `interfaces/kits/ndk/clouddiskmanager/`

- `include/oh_cloud_disk_manager.h`:新增两函数声明,`@since 26.1.0`/`@kit CoreFileKit`/`@syscap SystemCapability.FileManagement.CloudDiskManager`/`@library libohclouddiskmanager.so`,紧邻 `OH_CloudDisk_ConvertPlaceholderToFile`/`OH_CloudDisk_UpdatePlaceholder`。

```c
CloudDisk_ErrorCode OH_CloudDisk_MarkFileAsPlaceholder(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo);

CloudDisk_ErrorCode OH_CloudDisk_UnmarkPlaceholderFile(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo);
```

- `liboh_cloud_disk_manager.ndk.json`:加两符号 + `first_introduced: "26.1.0"`。
- `src/oh_cloud_disk_manager.cpp`:两 shim——入参校验(空指针/路径)→ `CloudDiskServiceManager::GetInstance().MarkFileAsPlaceholder(...)`/`UnmarkPlaceholderFile(...)` → `ConvertToErrorCode`。范式照搬既有 Convert shim(`:525-552`)。
- `include/cloud_disk_error_code.h`:加 `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028`(`OH_CLOUD_DISK_IS_A_PLACEHOLDER`=34400018 line 173 已存在,复用)。
- `include/oh_cloud_disk_utils.h` + `src/oh_cloud_disk_utils.cpp`:映射表 `innerToNErrTable` 增 2 行(`E_IS_A_PLACEHOLDER`→`OH_CLOUD_DISK_IS_A_PLACEHOLDER`、`E_PLACEHOLDER_NOT_FULLY_HYDRATED`→`OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`)。
- `BUILD.gn`:不新增公开 header(函数加进既有 `oh_cloud_disk_manager.h`,该 header 已在 `ohos_ndk_headers` 列表),故 BUILD 多半不变;`.ndk.json` 必改。

### 5.2 inner API — `interfaces/inner_api/native/clouddiskservice_kit_inner/`

- `cloud_disk_service_manager.h`:紧邻 `ConvertPlaceholderToFile`(`:49`)新增两虚函数:

```cpp
virtual int32_t MarkFileAsPlaceholder(const std::string &syncFolder,
                                      const std::string &relativePath) = 0;
virtual int32_t UnmarkPlaceholderFile(const std::string &syncFolder,
                                      const std::string &relativePath) = 0;
```

- `clouddiskservice_kit_inner.map`:补两新符号导出。

### 5.3 framework — `frameworks/native/clouddiskservice_kit_inner/`

- `include/cloud_disk_service_manager_impl.h`:两 override。
- `src/cloud_disk_service_manager_impl.cpp`:两实现,范式照搬 `ConvertPlaceholderToFile`(`:260-278`):`#ifdef SUPPORT_CLOUD_DISK_SERVICE` → `ServiceProxy::GetInstance()` 空检 → `E_IPC_FAILED` → `serviceProxy->MarkFileAsPlaceholderInner/UnmarkPlaceholderFileInner(syncFolder, relativePath)` → `SetDeathRecipient` → `#else return E_NOT_SUPPORTED`。
- `src/service_proxy.cpp` 经生成 proxy(`cloud_disk_service_proxy.h`)发起 IPC。

### 5.4 IDL — `services/clouddiskservice/ICloudDiskService.idl`

紧邻 `ConvertPlaceholderToFileInner`(`:35`)新增(写法与既有 Convert 一致,`void` 返回 + 两个 `[in] String`):

```
void MarkFileAsPlaceholderInner([in] String syncFolder, [in] String relativePath);
void UnmarkPlaceholderFileInner([in] String syncFolder, [in] String relativePath);
```

`services/clouddiskservice/BUILD.gn` 的 `idl_gen_interface("cloud_disk_service")` 自动重生成 stub/proxy;接口码由 IDL 工具链自动递增。**`clouddiskservice` 接口码不在 `CODEOWNERS` 额外评审清单**(CODEOWNERS 仅覆盖 distributedfiledaemon / cloud_daemon_kit / cloud_file_sync_service 三处),故无额外 IPC 评审路径。

### 5.5 service — `services/clouddiskservice/ipc/`

- `include/cloud_disk_service.h`:紧邻 `ConvertPlaceholderToFileInner`(`:68`)新增两 override 声明。
- `src/cloud_disk_service.cpp`:两 impl,行为见 §6。
- 错误枚举**不在** `services/clouddiskservice/ipc/include/` 下(全仓唯一错误头在 shared `utils/clouddiskservice/include/cloud_disk_service_error.h`,已由 §4.2/§11 覆盖),故本目录无错误码改动。

## 6. 行为语义

### 6.1 MarkFileAsPlaceholder(普通→占位,→state 3)

```
1. GetUserId() / GetAccountId() / GetCallerBundleName(bundleName)  // 同 Convert 校验链
   失败 → E_TRY_AGAIN / E_INVALID_ARG
2. CheckSyncFolderBundleName(syncFolder, userId, bundleName)        // bundleName 归属
   失败 → E_SYNC_FOLDER_PATH_UNAUTHORIZED / E_SYNC_FOLDER_NOT_REGISTERED
3. GetHmdfsPath(...) → 读 high3:
   state != 0 → E_IS_A_PLACEHOLDER (34400018,内部枚举/映射待补,见 §4.2)
4. SetHighBits(file, FULLY_HYDRATED(3))  // 无 ftruncate、无 fsetxattr size、无 customInfo 写
   返回 old(应 = 0)
5. delta = IsPlaceholder(3) − IsPlaceholder(0) = +1
   → 调 placeholder_helper 祖先刷新(+1)(复用 v2 重构 Model B)
6. DentrySetPlaceholder(dentry, 3)  // SDD §8.1 IPC 路径要求
```

- 不 `ftruncate`、不写 `atime`/`mtime`、不写 `user.clouddisk.custominfo` xattr。
- 文件当前 size 即占位符 `logicalSize`(`FULLY_HYDRATED` 下二者相等)。

### 6.2 UnmarkPlaceholderFile(占位→普通,gate state==3)

```
1. bundleName 归属校验(同 Mark)
2. 读 high3:
   state == 0      → E_NOT_A_PLACEHOLDER (34400017,既有)
   state ∈ {1,2}   → E_PLACEHOLDER_NOT_FULLY_HYDRATED (34400028,新)
   state == 3      → 放行
3. SetHighBits(file, NONE(0))  // 无 ftruncate —— 与 ConvertPlaceholderToEmptyFile 的关键差异
   返回 old(= 3)
4. delta = IsPlaceholder(0) − IsPlaceholder(3) = −1
   → 调 placeholder_helper 祖先刷新(−1)
5. DentrySetPlaceholder(dentry, 0)
6. 不读写 user.clouddisk.custominfo xattr(v3 §2.6 正交)
```

- `state==3` 闸门避免 `UNHYDRATED(1,纯 stub 稀疏空洞)`/`PARTIALLY_HYDRATED(2,部分空洞)` 转"普通"产生"带空洞读零"的伪普通文件。
- 不 `ftruncate` 是本需求核心:保留本地数据,与既有数据破坏型 `ConvertPlaceholderToFile`(`ftruncate(0)`)并存,应用按"丢数据 vs 留数据"自选。

## 7. 校验与错误处理

| 场景 | 码 | 位置 |
| --- | --- | --- |
| `syncFolder` 未注册 / bundle 不匹配 | `E_SYNC_FOLDER_NOT_REGISTERED`/`E_SYNC_FOLDER_PATH_UNAUTHORIZED` | service(`CheckSyncFolderBundleName`) |
| Mark 时已是占位符(`state!=0`) | `E_IS_A_PLACEHOLDER`(34400018,内部枚举待补) | service |
| Unmark 时根本不是占位符(`state==0`) | `E_NOT_A_PLACEHOLDER`(34400017,既有) | service |
| Unmark 时 state ∈{1,2} | `E_PLACEHOLDER_NOT_FULLY_HYDRATED`(34400028,新) | service |
| 路径父目录非目录 / 名过长 | `E_NOT_A_DIRECTORY`(34400023)/`E_NAME_TOO_LONG`(34400025) | service(既有校验链) |
| `GetCallerBundleName` 失败(含 twin app `instIndex!=0`) | `E_TRY_AGAIN` | service |
| xattr 读/写 IO 失败 | 经 `ConvertErrnoToCloudDiskError` 归一 | service |
| `ServiceProxy` 为空 | `E_IPC_FAILED` | framework |
| 特性开关关闭 | `E_NOT_SUPPORTED` | framework `#else` |

- 校验失败必返明确错误码并打日志(热路径注意 buffer 限流、`%{private}` 敏感路径)。
- Mark/Unmark 校验须在 `SetHighBits` 之前,避免产生半成品状态需回滚。
- 祖先 count 刷新失败不回滚已完成文件状态翻转(v2 SDD §10.1 既有约定:count/high3 更新失败不回滚文件业务操作)。

## 8. 安全与约束

1. 不绕过 `CloudDiskServiceAccessToken` 的 token/userId/bundleName 校验;Mark/Unmark 走与 Convert/Update 同一 `CheckSyncFolderBundleName` 访问链。
2. `GetCallerBundleName` 拒 twin app(`instIndex != 0`),与既有 per-file 占位操作一致。
3. `GetUserId` 由 `callingUid / BASE_USER_RANGE` 派生,多用户隔离不可绕过。
4. 路径校验不绕过(`..` 逃逸仍走 `ReplacePathPrefix`/`GetHmdfsPath`,路径须落在已注册 sync folder 范围内)。
5. 不改权限模型/token 校验/多用户隔离/uid-gid 归属/RDB schema。
6. 不读写 customInfo xattr(v3 §2.6 正交);调用方需 customInfo 另行调既有 `UpdatePlaceholder`。
7. 不引入新第三方依赖、无常驻缓存。
8. xattr 资源归属不变(占位符文件类资产归属各应用,uid/gid 逻辑不动)。

## 9. 契约 / ABI / 兼容影响

- **非破坏面**:本设计为**纯新增**——新增两 NDK 符号、两 inner 虚函数、两 IDL 方法、两 SA override。不改 `PlaceholderInfo`/`PlaceholderCustomInfo` Parcelable 布局;不改既有 `user.clouddisk.filesyncstate`/`user.clouddisk.custominfo` xattr 语义;既有 `ConvertPlaceholderToFile`(数据破坏型)签名与行为不变,与新接口并存。
- **错误码新增面**:NDK 枚举新增 1 项(34400028;34400018 已存在复用);内部枚举新增 2 项(`E_IS_A_PLACEHOLDER`=34400018 填空缺 + `E_PLACEHOLDER_NOT_FULLY_HYDRATED`=34400028);映射表新增 2 行。均 append-only/填空缺,不改既有值,不影响既有调用方与 DFX 上报码段。
- **特性开关**:`dfs_service_feature_enable_cloud_disk` 关闭时,framework `#ifdef SUPPORT_CLOUD_DISK_SERVICE` 走 `#else return E_NOT_SUPPORTED`;BUILD 与代码条件一致(AGENTS.md 硬约束)。
- **前置依赖**:v2 占位符重构未落地时,`placeholder_helper`(`SetHighBits`/祖先刷新)不存在,本设计无法实现。两份变更集**必须分先后**:①v2 重构(无新 IPC)先合,②本设计(新 IPC)后叠。
- `clouddiskservice` IDL 接口码不在 `CODEOWNERS` 额外评审清单,无额外 IPC 评审路径;但内部仍同步 proxy/stub/manager/mock/测试。
- 26.1.0 未商用:新增符号属 clean break 范畴,无既有消费方二进制兼容负担。

## 10. 测试要求

测试**仅**落 `test/unittests/clouddiskservice/`(无下划线那棵),**不**改 `test/unittests/clouddisk_service/`(下划线那棵,另一棵测试树)。

1. **NDK shim 单测**(`ndk/oh_cloud_disk_manager_test.cpp`,target `oh_cloud_disk_manager_test`):空参 → `CLOUD_DISK_INVALID_ARG`;调 manager mock 返 `E_OK`/`E_IS_A_PLACEHOLDER`/`E_NOT_A_PLACEHOLDER`/`E_PLACEHOLDER_NOT_FULLY_HYDRATED` → 经 `ConvertToErrorCode` 映射校验(含新增的 34400018/34400028 两映射行)。
2. **SA 单测**(`ipc/cloud_disk_service_test.cpp`,target `cloud_disk_service_test`):
   - Mark:空参 / `GetBundleNameFail` / 已是占位符(`state!=0` → 34400018)/ 成功(state 0→3,`SetHighBits` 调用、helper(+1)调用、`DentrySetPlaceholder(dentry,3)` 调用)。
   - Unmark:空参 / `state==0` → 34400017 / `state==1` → 34400028 / `state==2` → 34400028 / `state==3` 成功(`SetHighBits(0)` 调用、**断言无 `ftruncate`**、helper(−1)调用、`DentrySetPlaceholder(dentry,0)` 调用)。
   - bundleName 归属:跨 bundle / 未注册 sync folder / token 不匹配 → 拒绝。
   - customInfo xattr:Unmark 前后 `user.clouddisk.custominfo` 不变(正交用例)。
3. **mock 同步**(`test/unittests/clouddiskservice/mock/`):
   - `mock/icloud_disk_service.h`:加两接口码枚举 + 两 virtual 方法。
   - `mock/cloud_disk_service_stub.{h,cpp}`:加两 stub 分发。
   - `mock/cloud_disk_service_manager_mock.{h,cpp}`:加两 `MOCK_METHOD`。
   - **仅这一棵 mock 树**;`test/unittests/clouddisk_service/mock/` 不动。
4. 无新 Parcelable → `cloud_disk_common_test.cpp` 不动;无 parcel 读写测试。
5. 特性开关:`SUPPORT_CLOUD_DISK_SERVICE` 关闭路径返回 `E_NOT_SUPPORTED`(若既有测试覆盖该模式)。

## 11. 实现落点(未实现,sign-off 后进行)

| 内容 | 路径 |
| --- | --- |
| NDK 函数声明 | `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h` |
| NDK shim | `interfaces/kits/ndk/clouddiskmanager/src/oh_cloud_disk_manager.cpp` |
| NDK 符号清单 | `interfaces/kits/ndk/clouddiskmanager/liboh_cloud_disk_manager.ndk.json` |
| NDK 错误码 + 映射 | `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h`(加 34400028;34400018 已存在)+ `include/oh_cloud_disk_utils.h` + `src/oh_cloud_disk_utils.cpp`(映射表加 2 行) |
| inner manager 虚函数 + `.map` | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_service_manager.h` + `clouddiskservice_kit_inner.map` |
| framework manager impl | `frameworks/native/clouddiskservice_kit_inner/{include/cloud_disk_service_manager_impl.h,src/cloud_disk_service_manager_impl.cpp,src/service_proxy.cpp}` |
| IDL | `services/clouddiskservice/ICloudDiskService.idl` |
| service override + 阶梯实现 | `services/clouddiskservice/ipc/{include/cloud_disk_service.h,src/cloud_disk_service.cpp}` |
| 错误码枚举 | `utils/clouddiskservice/include/cloud_disk_service_error.h`(加 `E_IS_A_PLACEHOLDER=34400018` 填 17→19 空缺 + `E_PLACEHOLDER_NOT_FULLY_HYDRATED=34400028` 于末尾) |
| mock | `test/unittests/clouddiskservice/mock/{icloud_disk_service.h,cloud_disk_service_stub.{h,cpp},cloud_disk_service_manager_mock.{h,cpp}}` |
| 测试 | `test/unittests/clouddiskservice/{ndk/oh_cloud_disk_manager_test.cpp,ipc/cloud_disk_service_test.cpp}` |

> 行号为当前代码树快照(取自探索期),实现时以最新树为准。

## 12. Ask-before / 评审项

- **新增 inner API 方法签名 + 新错误码语义**:属 AGENTS.md "修改 public/inner API 签名、错误码语义"范畴,需维护者 sign-off。本方案以"API 尚未商用"为前提接受新增,评审时确认该前提。
- **新增错误码** `E_IS_A_PLACEHOLDER`(34400018,填内部枚举空缺)+ `E_PLACEHOLDER_NOT_FULLY_HYDRATED`(34400028,末尾):append-only/填空缺到 shared `utils/clouddiskservice` 错误头,低风险;按 utils/AGENTS 检查无调用方冲突。NDK 枚举 34400018 已存在(reserved),只新增 34400028。
- **前置依赖 v2 重构**:`filesyncstate` 盘上格式重解释(高 3 位)+ `CloudDiskServiceDentry.reserved` 借用字节属持久化格式范畴,需维护者 sign-off(见 `placeholder-marking-refactor-sdd.md` §15)。本设计依赖其 `placeholder_helper` 落地,不可先行。
- **state 3 提前启用**:v3 标 2/3"预留(clouddiskservice 现无水合路径)";本设计通过"标记既有本地文件"启用 state 3,语义自洽但提前,评审知悉。
- **34400027 连续性(已定)**:v3 customInfo 的 `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`(34400027)尚未落地(NDK/内部枚举均无)。**v3 customInfo 必须先或同期落地**,以保持 NDK 枚举 16~34400028、内部枚举 34400027→34400028 连续;**不接受** 34400026→34400028 的 27 空缺。故本设计实际前置依赖为:v2 占位符重构(state 模型 + `placeholder_helper`)+ v3 customInfo(34400027 错误码)先/同期落地。
- 不改权限模型 / token 校验 / 多用户隔离 / uid-gid 归属 / RDB schema。
- DCO / `Signed-off-by`:按上游合入标准,commit 须含 `Signed-off-by`。

## 13. 后续扩展

1. **数据水合/脱水落地**:本设计仅落地 state-only 翻转;v3 §5.3 的 `OH_CloudDisk_HydratePlaceholder`/`OH_CloudDisk_DehydrateFile`(带数据下载语义、`1↔2↔3` 子态推进、异步 callback)仍待后续实现。落地时复核本设计的 state 3 写入路径与水合路径是否需协调(本设计直接置 3,水合路径经 1→2→3 推进)。
2. **批量/目录转换**:本设计单文件;批量/目录递归可作为后续扩展(IPC Parcelable 与签名需变)。
3. **Unmark 放宽**:若未来允许 `UNHYDRATED`/`PARTIALLY_HYDRATED` 转"普通"(产生稀疏普通文件),需重审 §6.2 闸门与错误码 34400028 适用面。
4. **DFX 打点**:Mark/Unmark 成功/前置拒绝可纳入 `clouddiskservice-dfx-sdd` 的 `FSC_PLACEHOLDER` scenario。
