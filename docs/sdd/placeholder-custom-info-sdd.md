> **opencode session**:`ses_fb398cd8fffegoRHIP4rImck79`
> slug:`gentle-otter` | 标题:clouddiskservice占位符自定义信息入参设计 | 目录:D:\code\dfs\filemanagement_dfs_service
> 续接:`opencode resume ses_fb398cd8fffegoRHIP4rImck79`(或在 TUI session 列表选 `gentle-otter`)
> 状态:设计稿,未实现。实现待本方案 sign-off 后单独进行。

# Placeholder Custom Info SDD

## 1. 背景

`CloudDiskService`(三方网盘 SA)在创建与更新占位符时,只通过 `PlaceholderInfo`(`interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_common.h:106`)携带 `logicalSize`/`atimeMs`/`mtimeMs` 三个 `uint64_t`,**没有**任何调用方自定义的标识信息字段。参考 Windows `CF_PLACEHOLDER_BASIC_INFO` 的 `FileIdentityLength` + `FileIdentity[1]`,同步引擎/适配器通常需要一个随占位符持久化、调用方自定义、服务不解析的不透明 blob,以便后续变更事件触发时直接关联本地占位符与云端实体,免去额外查询。

本设计为 create/update 各新增一个**可选**的"自定义信息"入参,并配套新增一条**读取**接口。内容用途不明确、由调用方自定义、服务永不解析。

## 2. 目标

1. create/update 占位符时,可携带调用方自定义的不透明信息(下称 customInfo),服务不解析、原样持久化。
2. customInfo 为**可选**入参,可不传(create 不传=不写 customInfo;update 不传=保留旧 customInfo)。
3. 新增读取接口,可把已持久化的 customInfo 读回。
4. **不动 `PlaceholderInfo`**(其 Parcelable 布局与既有 wire 格式保持不变),customInfo 作为独立新入参 + 独立新结构体。
5. 复用现有 xattr 持久化范式(与 `user.clouddisk.placeholder` 同级新增一个 xattr),不引入 RDB / schema / 迁移。
6. 遵循仓内不透明字节过 IPC 的既有范式(`XattrResult`,`std::vector<uint8_t>` + `WriteUInt8Vector/ReadUInt8Vector`)。

## 3. 总体决策

| 项 | 决策 |
| --- | --- |
| 用途 | 不透明、调用方自定义、服务不解析 |
| 入参形状 | 独立新入参 + 新结构体,**不动 `PlaceholderInfo`** |
| 内层 C++ 类型 | `PlaceholderCustomInfo { std::vector<uint8_t> data; }`,长度走 `data.size()`,无冗余 length 字段 |
| NDK C 类型 | `OH_CloudDisk_PlaceholderCustomInfo { size_t dataLength; const uint8_t *data; }`,校验 `dataLength` 与实际缓冲长度一致 |
| API 策略 | **在原 create/update 方法上新增入参**,全层改签名;NDK + inner API 尚未商用,破坏可接受 |
| 持久化 | 新增 xattr `"user.clouddisk.custominfo"`,写在 `SetPlaceholderFileAttributes`(`cloud_disk_service.cpp:180`) |
| "可选"编码 | in-band:空=缺省;create 空=不写 customInfo xattr;**update 空=保留旧 customInfo**;无显式清空 |
| 大小上限 | ≤ 4 KiB(受单 xattr 值上限约束);NDK 入口与 service 入口各校验一次 |
| 校验失败码 | 复用 `E_INVALID_ARG`(34400001),不新增 |
| 读取接口 | 新增 `GetPlaceholderCustomInfo`(全层、非破坏),复用 create/update 同款 `syncFolder/bundleName/token` 校验;占位符在但无 customInfo → 新错误码。NDK 出参走 caller 预分配(`uint8_t *dataBuf` + `size_t *inOutDataLength`,capacity 入/actual 出),见 §6.3 |
| 未找到错误码 | 新增 `E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND = 34400027`(append-only,沿用 `344xxxxx` 前缀) |

## 4. 数据模型

### 4.1 内层 C++ Parcelable

`interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_common.h`(紧邻 `PlaceholderInfo` @ `:106`),命名空间 `OHOS::FileManagement::CloudDiskService`:

```cpp
struct PlaceholderCustomInfo : public Parcelable {
    std::vector<uint8_t> data;   // 长度 == data.size();空表示"未提供"
    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static PlaceholderCustomInfo *Unmarshalling(Parcel &parcel);
};
```

parcel 实现位于 `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_common.cpp`(紧邻 `:348-391`),用 `parcel.WriteUInt8Vector(data)` / `parcel.ReadUInt8Vector(&data)`,范式抄 `XattrResult`(`frameworks/native/cloudsync_kit_inner/src/cloud_sync_common.cpp:914-940`)。读写逐项检查返回值,失败返回 false 并打日志。

### 4.2 NDK C 结构体

`interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h`(紧邻 `OH_CloudDisk_PlaceholderInfo` @ `:366`):

```c
typedef struct OH_CloudDisk_PlaceholderCustomInfo {
    size_t dataLength;        // 镜像 Windows FileIdentityLength
    const uint8_t *data;      // 镜像 Windows FileIdentity[1]
} OH_CloudDisk_PlaceholderCustomInfo;
```

NDK 入口校验:`dataLength` 与 `data` 实际可读长度一致、`dataLength <= 4096`;`data == NULL` 或 `dataLength == 0` 视为"未提供"(等价于内层空 vector)。

### 4.3 持久化 xattr

`services/clouddiskservice/ipc/src/cloud_disk_service.cpp`(`:53` 附近):

```cpp
constexpr const char *CLOUD_DISK_CUSTOM_INFO_XATTR = "user.clouddisk.custominfo";
```

- 仅在 `data` 非空时 `fsetxattr` 写入原始字节;`data` 为空时**不写、不删**(实现 update 保留语义)。
- 与既有 `user.clouddisk.filesyncstate`(`:52`)等占位符 xattr 同级,互不干扰。
- 值即原始字节,xattr 自带长度,无额外 length 前缀。

### 4.4 错误码

`utils/clouddiskservice/include/cloud_disk_service_error.h` 枚举末尾追加(append-only,不影响既有值与调用方):

```cpp
E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND = 34400027,   // 占位符存在但无 customInfo xattr
```

沿用 `344xxxxx` 前缀(现有最大 `E_FILE_TOO_LARGE = 34400026`)。校验类失败全部复用既有 `E_INVALID_ARG`,不新增。

## 5. API / IDL / 各层改动

customInfo 与查询方法需贯穿 NDK → inner API → framework → IDL/proxy → IPC → stub → service 全链。

### 5.1 IDL — `services/clouddiskservice/ICloudDiskService.idl`

- `:19` 附近注册新 sequenceable:`sequenceable cloud_disk_common..OHOS.FileManagement.CloudDiskService.PlaceholderCustomInfo;`
- `:30` `CreatePlaceholderFileInner`、`:36` `UpdatePlaceholderInner` 各追加 `[in] PlaceholderCustomInfo customInfo`(**破坏性签名变更,已接受**)。
- 新增查询方法(返回机制 `[out]` vs 返回 sequenceable 留作实现期确认,以 IDL 生成器支持为准):
  ```
  void GetPlaceholderCustomInfoInner([in] String syncFolder, [in] String relativePath, [out] PlaceholderCustomInfo info);
  ```
- `services/clouddiskservice/BUILD.gn:34` 的 `idl_gen_interface("cloud_disk_service")` 自动重生成 stub/proxy。

### 5.2 inner API — `interfaces/inner_api/native/clouddiskservice_kit_inner/`

- `cloud_disk_service_manager.h:41,50`:`CreatePlaceholderFile`/`UpdatePlaceholder` 加 `const PlaceholderCustomInfo &customInfo`;新增 `GetPlaceholderCustomInfo` 虚函数。
- `clouddiskservice_kit_inner.map`:补新查询方法符号导出;create/update 因参类型变化致 C++ name mangling 变更 → 符号变更。

### 5.3 framework — `frameworks/native/clouddiskservice_kit_inner/`

- `include/cloud_disk_service_manager_impl.h:45,54` + `src/cloud_disk_service_manager_impl.cpp:149,280`:加参并透传 proxy;新增 `GetPlaceholderCustomInfo` 实现。
- 结构体 parcel 代码见 §4.1(`cloud_disk_common.cpp`)。
- `src/service_proxy.cpp` 经生成 proxy(`cloud_disk_service_proxy.h` @ `:19`)发起 IPC。

### 5.4 NDK — `interfaces/kits/ndk/clouddiskmanager/`

- `include/oh_cloud_disk_manager.h:534,574`:`OH_CloudDisk_CreatePlaceholder`/`OH_CloudDisk_UpdatePlaceholder` 加 `const OH_CloudDisk_PlaceholderCustomInfo *customInfo`(可空,空=未提供);新增 `OH_CloudDisk_GetPlaceholderCustomInfo`。
- `src/oh_cloud_disk_manager.cpp:299,554`:Create/Update NDK→inner 拷贝(`dataLength`/`data` → `std::vector<uint8_t>`);**入口校验** `dataLength` 一致性与 ≤4 KiB,违例返 `CLOUD_DISK_INVALID_ARG`(NDK 表面;inner 侧对应 `E_INVALID_ARG`)。
- Get NDK 桥接(Model 3):`OH_CloudDisk_GetPlaceholderCustomInfo(syncFolder, path, uint8_t *dataBuf, size_t *inOutDataLength)`;shim 调 inner `GetPlaceholderCustomInfo`(inner 返回 `PlaceholderCustomInfo`/vector 与 `E_*` 码),按 `*inOutDataLength`(入参 capacity)拷贝进 `dataBuf`、回填 actual;capacity < actual → shim 返 `CLOUD_DISK_INVALID_ARG`(不部分拷贝);命中返 `CLOUD_DISK_OK`,inner 未命中码经映射返 `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`/`OH_CLOUD_DISK_NOT_A_PLACEHOLDER`/`OH_CLOUD_DISK_FILE_NOT_EXIST`。

### 5.5 service — `services/clouddiskservice/ipc/`

- `include/cloud_disk_service.h:59,69`:override 加 `const PlaceholderCustomInfo &customInfo`;新增 `GetPlaceholderCustomInfoInner` override。
- `src/cloud_disk_service.cpp`:
  - `SetPlaceholderFileAttributes`(`:180`,create @ `:221` 与 update @ `:1262` 的汇聚点):扩展为接收 customInfo,`data` 非空时 `fsetxattr(fd, CLOUD_DISK_CUSTOM_INFO_XATTR, data.data(), data.size(), ...)`,空时不写不动。
  - `CreatePlaceholderFileInner`(`:798`)/`UpdatePlaceholderInner`(`:1275`):收 customInfo 透传;**service 再校验** `data.size() <= 4096`,违例返 `E_INVALID_ARG`(防绕过 NDK)。
  - 新增 `GetPlaceholderCustomInfoInner`:同款 `CloudDiskServiceAccessToken::GetUserId`/`CheckSyncFolderBundleName` 校验 → `fgetxattr` 读 customInfo;无 xattr → `E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`;非占位符 → `E_NOT_A_PLACEHOLDER`;文件不存在 → `E_FILE_NOT_EXIST`。

## 6. 行为语义

### 6.1 create

| customInfo | 行为 |
| --- | --- |
| 非空(≤4 KiB) | 写 size/atime/mtime + 占位符 xattr + customInfo xattr |
| 空 / 未提供 | 写 size/atime/mtime + 占位符 xattr,**不写** customInfo xattr |
| 非法(超限 / length 不符) | 整个 create 失败,不产生半成品占位符(校验先于 `openat`) |

### 6.2 update

| customInfo | 行为 |
| --- | --- |
| 非空 | `SetPlaceholderFileAttributes` 覆写 customInfo xattr(原值被替换) |
| 空 / 未提供 | **保留**既有 customInfo xattr 不动(只更新 size/atime/mtime) |

> 不提供"显式清空 customInfo"能力(少见需求);若将来需要,另设专用路径,不重载"空"语义以免破坏 §6.2 的保留约定。

### 6.3 query

服务侧行为(inner,返回 `PlaceholderCustomInfo`/vector 与 `E_*` 码;NDK 表面契约与缓冲拷贝见 §5.4 Model 3):

- 命中:返回 `E_OK` + customInfo(`data` 为 xattr 原始字节)。
- 占位符存在但无 customInfo xattr:返回 `E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`(`data` 空)。
- 非 placeholder 文件:`E_NOT_A_PLACEHOLDER`。
- 文件不存在:`E_FILE_NOT_EXIST`。
- 越权:`E_ACCES` / `E_SYNC_FOLDER_NOT_REGISTERED`(同 create/update 校验链)。
- NDK shim 经映射(`E_*` → `CLOUD_DISK_*`/`OH_CLOUD_DISK_*`)返回;缓冲不足(capacity<actual)由 shim 判定、返 `CLOUD_DISK_INVALID_ARG`(不部分拷贝、回填 actual),inner 不涉及此码,见 §5.4。

## 7. 校验与错误处理

| 场景 | 码 | 位置 |
| --- | --- | --- |
| `dataLength` 与实际缓冲长度不符 | `CLOUD_DISK_INVALID_ARG` | NDK 入口 |
| `dataLength`/size > 4096 | NDK:`CLOUD_DISK_INVALID_ARG`;service:`E_INVALID_ARG` | NDK 入口 + service 入口(双重) |
| Get 缓冲不足(capacity<actual) | `CLOUD_DISK_INVALID_ARG` | NDK shim |
| `syncFolder`/`relativePath` 非法 | `E_INVALID_ARG` | service(既有逻辑) |
| 越权 / 未注册 / bundle 不匹配 | `E_ACCES`/`E_SYNC_FOLDER_NOT_REGISTERED` | service(既有 `CheckSyncFolderBundleName`) |
| 占位符在但无 customInfo | `E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`(新) | service 查询 |
| xattr 写/读 IO 失败 | 经 `ConvertErrnoToCloudDiskError` 归一 | service |

- 校验失败必返明确错误码并打日志(热路径注意 buffer 限流)。
- create 校验失败须在 `openat` 之前,避免产生半成品占位符需回滚。
- update 覆写 customInfo xattr 失败不回滚 size/time(与既有 `SetPlaceholderFileAttributes` 失败处理一致)。

## 8. 安全与约束

1. 不绕过 `CloudDiskServiceAccessToken` 的 token/userId/bundleName 校验;查询与 create/update 走同一访问链。
2. customInfo 内容服务不解析、不执行,仅原样存取,无注入面。
3. 大小硬上限 4 KiB,NDK 与 service 双重校验,防 xattr 撑爆与内存放大。
4. 不动 `PlaceholderInfo` Parcelable 布局;新结构体独立,既有 wire 兼容不受影响。
5. 不引入 RDB / schema / 迁移 / 新第三方依赖。
6. 路径校验不绕过(`..` 逃逸仍走 `ReplacePathPrefix`/`GetHmdfsPath`)。
7. xattr 资源归属不变(占位符文件类资产归属各应用,uid/gid 逻辑不动)。

## 9. 契约 / ABI / 兼容影响

- **破坏面(已接受,API 未商用)**:NDK `OH_CloudDisk_CreatePlaceholder/UpdatePlaceholder` 签名;inner API `CloudDiskServiceManager::CreatePlaceholderFile/UpdatePlaceholder` 签名(name mangling 变 → `clouddiskservice_kit_inner.map` 变更);mock 元数 `MOCK_METHOD3`→`MOCK_METHOD4`;既有 `EXPECT_CALL(...CreatePlaceholder(_,_,_))` 站点。
- **非破坏面**:`PlaceholderInfo` Parcelable 布局不动;既有 `user.clouddisk.placeholder`/`filesyncstate` xattr 不动;查询方法为纯新增。
- 无 CODEOWNERS 额外评审的 clouddiskservice interface-code 头(stub/proxy 由 IDL 生成,本仓 `CODEOWNERS` 不含 clouddiskservice 接口码文件)。
- 新错误码 append-only,不改既有值,不影响既有调用方与 DFX 上报码段。

## 10. 测试要求

1. `PlaceholderCustomInfo` parcel 往返:非空、空、边界长度(0、4096、4097)成功/失败路径(范式抄 `cloud_sync_common_test.cpp:1894-2063` 的 `WriteUInt8Vector` 成败用例)。
2. create:非空 customInfo → xattr 落盘可读回;空 customInfo → 无 customInfo xattr;超限/length 不符 → 失败且无半成品占位符。
3. update:非空 → 覆写(原值替换);空 → **保留**旧 customInfo(关键用例)。
4. 查询:命中 / 缓冲不足(NDK capacity<actual → `CLOUD_DISK_INVALID_ARG`,不部分拷贝、回填 actual)/ 未命中(`E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`)/ 非占位符(`E_NOT_A_PLACEHOLDER`)/ 文件不存在(`E_FILE_NOT_EXIST`)/ 越权。
5. mock 元数:`cloud_disk_service_manager_mock.h:43,48` 改 `MOCK_METHOD4`;新增 `GetPlaceholderCustomInfo` mock;既有 NDK/service 测试的 `EXPECT_CALL(_,_,_)` 同步改 `(_,_,_,_)`。
6. 双重校验:NDK 入口拦非法 `dataLength`;绕过 NDK 直调 service 时 service 入口仍拦超限。
7. 访问控制:跨 bundle / 未注册 sync folder / token 不匹配 → 拒绝(create/update/query 一致)。

## 11. 实现落点(未实现,sign-off 后进行)

| 内容 | 路径 |
| --- | --- |
| 内层 Parcelable 结构体 | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_common.h`(@ `:106` 附近) |
| 结构体 parcel 实现 | `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_common.cpp`(@ `:348` 附近) |
| inner API manager 虚函数 + `.map` | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_service_manager.h`(`:41,50`)+ `clouddiskservice_kit_inner.map` |
| framework manager impl | `frameworks/native/clouddiskservice_kit_inner/{include/cloud_disk_service_manager_impl.h,src/cloud_disk_service_manager_impl.cpp,src/service_proxy.cpp}`(`:45,54,149,280,19`) |
| NDK C 结构体 + 函数签名 | `interfaces/kits/ndk/clouddiskmanager/{include/oh_cloud_disk_manager.h,src/oh_cloud_disk_manager.cpp}`(`:366,534,574,299,554`) |
| IDL | `services/clouddiskservice/ICloudDiskService.idl`(`:19,30,36`)+ 新查询方法 |
| service override + xattr + 查询 | `services/clouddiskservice/ipc/{include/cloud_disk_service.h,src/cloud_disk_service.cpp}`(`:53,59,69,180,221,798,1262,1275`) |
| 错误码 | `utils/clouddiskservice/include/cloud_disk_service_error.h`(枚举末尾) |
| mock | `test/unittests/clouddiskservice/mock/cloud_disk_service_manager_mock.{h,cpp}`(`:43,48`) |
| 测试 | `test/unittests/{ndk/cloud_disk_manager_test.cpp,clouddiskservice/ndk/oh_cloud_disk_manager_test.cpp,clouddiskservice/ipc/*}` |

> 行号为当前代码树快照,实现时以最新树为准。

## 12. Ask-before / 评审项

- **inner API + NDK 签名变更**:属 AGENTS.md "修改 public/inner API 签名"范畴,需维护者确认设计/评审路径。本方案以"API 尚未商用"为前提接受破坏性变更,评审时确认该前提。
- **新增错误码**:`E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND` 新增到 shared `utils/clouddiskservice` 错误头(append-only,低风险),按 utils/AGENTS 检查无调用方冲突;若评审希望零 shared 改动,可退回"查询返回 `E_OK`+空 data、空=无 customInfo"的等价方案(与 §6.1 空语义自洽)。
- **新增 xattr**:`user.clouddisk.custominfo` 属盘上持久化格式范畴,按 `services/clouddiskservice/AGENTS.md` 补读写测试;clean break(全新 key,无历史兼容负担)。
- **Get 出参缓冲所有权(已定 Model 3)**:NDK 出参走 caller 预分配——`uint8_t *dataBuf` + `size_t *inOutDataLength`(capacity 入/actual 出),不新增 destroy 函数、不改结构体 `const`、无隐藏 malloc。IDL/inner 仍返回 `PlaceholderCustomInfo`/vector,NDK shim 拷贝进调用方缓冲(见 v3 规格 §4.5)。
- 不改权限模型 / token 校验 / 多用户隔离 / uid-gid 归属 / RDB schema。
- DCO / `Signed-off-by`:按上游合入标准,commit 须含 `Signed-off-by`。

## 13. 后续扩展

1. 显式清空 customInfo:若出现需求,新增专用路径(如 `ClearPlaceholderCustomInfo` 或 update 带 has 标志),不重载"空=保留"语义。
2. 大小上限调优:4 KiB 为保守值;若实际载荷需要更大,评估 xattr 容量与内存影响后上调,并同步更新双重校验阈值。
3. 查询返回机制:实现期确认 IDL `[out]` vs 返回 sequenceable 的取舍,以 IDL 生成器支持为准。
4. DFX 打点:customInfo 落盘/查询失败可纳入 `clouddiskservice-dfx-sdd` 的 `FSC_PLACEHOLDER` scenario。
