# CloudDisk 占位符（Placeholder）接口规格 v5

本文件是 v4（`clouddisk-placeholder-v4.md`）的修订版，反映**脱水（Dehydrate）落地**新增：落地 v4 §5.3 的 `OH_CloudDisk_DehydrateFile`（完全水合→未水合，回收本地数据），并为其在回调表中新增 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE` 回调类型与 `CloudDisk_DehydrateInfo` context 变体，引入"应用授权出参" `bool allow`（服务感知、决定是否执行脱水）；新增错误码 `OH_CLOUD_DISK_DEHYDRATE_DENIED`（34400029）。v4 的 Mark/Unmark（state-only 转换）、customInfo 全部保留。内部细节不复述：目录占位聚合见 `docs/sdd/placeholder-marking-refactor-sdd.md`；customInfo 各层接线见 `docs/sdd/placeholder-custom-info-sdd.md`；state-only 转换见 `docs/sdd/placeholder-state-only-conversion-sdd.md`；脱水各层接线见 `docs/sdd/placeholder-dehydrate-sdd.md`。下文中"SDD §x"指 marking-refactor SDD。v4 保留以记录脱水落地前契约。

> v5 变化：因 26.1.0 尚未商用发布，落地 `OH_CloudDisk_DehydrateFile` 与新增 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE`、`CloudDisk_DehydrateInfo`、`OH_CLOUD_DISK_DEHYDRATE_DENIED`（34400029）均为 clean break（pre-release），无既有消费方二进制兼容负担。脱水经回调表强制授权：应用注册回调表后，服务派发 `DEHYDRATE` 回调，应用在 `OnCallback` 同步回执中回写 `bool allow`，服务据此决定是否执行脱水。脱水目标态 `UNHYDRATED(1)`，前置 `state==FULLY_HYDRATED(3)`；`state==1` 幂等、`state∈{0,2}` 拒绝。脱水与 customInfo 正交、与 Mark/Unmark 正交。

> 水合（hydrate）将占位符填充为完整内容，脱水（dehydrate）将完整文件回退为占位符。水合脱水经回调表驱动：应用注册回调接收数据请求，再同步响应；亦可显式触发。v3 下水合进度经 `UNHYDRATED`→`PARTIALLY_HYDRATED`→`FULLY_HYDRATED`（1→2→3）推进（高 3 位子态），子态切换不改变占位计数。customInfo 与水合脱水正交：水合/脱水不读写 customInfo xattr。**v5 的 Dehydrate 落地、Mark/Unmark 与水合脱水亦正交**：Mark/Unmark 只翻高 3 位状态、不动数据；脱水丢本地数据并翻 `3→1`（子态切换，不改计数）；三者语义互补、并存。

## 1. 结构体

### 1.1 `OH_CloudDisk_PlaceholderInfo`

占位符元数据，用于创建与更新占位符文件。**v3/v4/v5 不改动本结构体**（customInfo 作为独立入参，见 §1.2）。

```c
typedef struct OH_CloudDisk_PlaceholderInfo {
    uint64_t logicalSize; // 云端文件的逻辑大小（字节）。占位符按此大小建为稀疏空洞，本地不下载实际内容
    uint64_t atimeMs;      // 占位符访问时间视图（毫秒），写入文件 atime
    uint64_t mtimeMs;      // 云端文件修改时间视图（毫秒），写入文件 mtime
} OH_CloudDisk_PlaceholderInfo;
```

### 1.2 `OH_CloudDisk_PlaceholderCustomInfo`（v3 新增）

占位符自定义信息，作为 create/update 的可选入参，携带调用方自定义、服务不解析的不透明字节 blob。参考 Windows `CF_PLACEHOLDER_BASIC_INFO` 的 `FileIdentityLength` + `FileIdentity`。

```c
typedef struct OH_CloudDisk_PlaceholderCustomInfo {
    size_t dataLength;        // data 指向的字节数；0 表示"未提供"
    const uint8_t *data;      // 自定义信息缓冲；可为 NULL（等价于 dataLength==0）
} OH_CloudDisk_PlaceholderCustomInfo;
```

- `dataLength` 与 `data` 实际可读长度必须一致，否则返回 `CLOUD_DISK_INVALID_ARG`。
- `dataLength <= 4096`（受单 xattr 值上限约束），否则返回 `CLOUD_DISK_INVALID_ARG`。
- `data == NULL` 或 `dataLength == 0` 视为"未提供 customInfo"（create 不写 customInfo xattr；update 保留旧 customInfo）。
- 服务原样持久化、原样读回，不解析内容。
- 本结构体仅用于 Create/Update 入参；Get 读取走独立缓冲出参（`uint8_t *dataBuf` + `size_t *inOutDataLength`，见 §4.5），不复用本结构体。
- **v4/v5**：Mark/Unmark 不带本结构体、不读写 customInfo xattr（见 §4.6/§4.7）；**v5**：脱水亦不读写 customInfo xattr（见 §5.3）。

### 1.3 水合脱水数据契约

以下类型承载水合脱水的回调数据契约，引用 `@since 21` 的既有类型 `CloudDisk_PathInfo` 与 `CloudDisk_SyncFolderPath`（后者为前者的别名）。

```c
typedef struct CloudDisk_DataBuf {
    uint8_t *data;     // 数据缓冲区
    uint64_t dataSize; // 数据长度
} CloudDisk_DataBuf;

typedef enum CloudDisk_CallbackType {
    CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA = 0,        // 获取云端文件数据（水合）
    CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA = 1, // 取消获取云端文件数据（取消水合）
    CLOUD_DISK_CALLBACK_TYPE_FETCH_RANGE_DATA = 2,  // 获取云端文件指定范围数据（流读）
    CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE = 3,         // 脱水授权请求（v5 新增）
} CloudDisk_CallbackType;

typedef struct CloudDisk_CallbackReqHead {
    CloudDisk_SyncFolderPath syncFolderPath; // 回调所属同步根
    CloudDisk_CallbackType callbackType;       // 回调请求类型
    CloudDisk_DataBuf reqKey;                // 不透明请求标识
} CloudDisk_CallbackReqHead;

typedef struct CloudDisk_RangeInfo {
    CloudDisk_PathInfo filePath; // 同步根内相对文件路径
    uint64_t offset;            // 请求读取的起始偏移，单位字节
    uint64_t size;              // 请求读取的数据长度，单位字节
    CloudDisk_DataBuf data;     // 数据缓冲区，由provider在回调中直接写入
} CloudDisk_RangeInfo;

typedef struct CloudDisk_DehydrateInfo {  // v5 新增
    CloudDisk_PathInfo filePath; // 同步根内相对文件路径（服务→应用入参）
    bool allow;                   // 是否允许脱水（应用→服务出参，应用在 OnCallback 中填充）
} CloudDisk_DehydrateInfo;

typedef union CloudDisk_CallbackContext {
    CloudDisk_PathInfo *fetchData;         // callbackType 为 FETCH_DATA 时的文件路径
    CloudDisk_PathInfo *cancelFetchData;  // callbackType 为 CANCEL_FETCH_DATA 时的文件路径
    CloudDisk_RangeInfo *fetchRangeData;  // callbackType 为 FETCH_RANGE_DATA 时的范围信息
    CloudDisk_DehydrateInfo *dehydrateData; // callbackType 为 DEHYDRATE 时的脱水授权信息（v5 新增）
} CloudDisk_CallbackContext;

typedef struct CloudDisk_FetchData {
    uint64_t offset;        // 本次数据在文件中的起始偏移，单位字节
    uint64_t size;          // 本次下载数据长度，单位字节
    uint64_t totalSize;     // 云端文件总大小，单位字节
    CloudDisk_DataBuf data; // 文件数据缓冲区
    bool isComplete;        // 是否是最后一块数据
} CloudDisk_FetchData;

typedef union CloudDisk_CallbackResponse {
    CloudDisk_FetchData *fetchData; // 回调响应，当前仅用于 FETCH_DATA
} CloudDisk_CallbackResponse;
```

> v5 变化：`CloudDisk_CallbackType` 新增 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE = 3`（append-only）；`CloudDisk_CallbackContext` union 新增 `dehydrateData` 变体；新增结构体 `CloudDisk_DehydrateInfo`（载 `filePath` 入参 + `allow` 出参）。`CloudDisk_CallbackResponse` 不变（脱水决定不经 `OH_CloudDisk_Execute`，走 `OnCallback` 同步回执包，见 §5.2/§5.3）。

## 2. 存储实现

占位符是 hmdfs 挂载路径上的稀疏文件。占位符状态并入 `user.clouddisk.filesyncstate` 的高 3 位（低 5 位仍为 `SyncState` 0–5）。v3 在此基础上新增 `user.clouddisk.custominfo` xattr 承载 customInfo（见 §2.6），与既有 `filesyncstate`/`phcount` 互不干扰。

### 2.1 filesyncstate 字节布局

```
bit7 bit6 bit5 | bit4 bit3 bit2 bit1 bit0
<── 高 3 位 ──> <────── 低 5 位 ──────>
   PlaceholderState       SyncState
```

- **文件**：高 3 位 = `PlaceholderState`；低 5 位 = `SyncState`（0–5，bit3-4 保留 0）。
- **目录**：高 3 位 = `(count>0) ? 1 : 0`（`HAS_PLACEHOLDER`）；低 5 位 恒 0，不语义化。
- "是否占位"统一以高 3 位 `!= 0` 判（文件与目录一致）。
- NDK `GetFileSyncStates`/`SetFileSyncStates` 对外只暴露低 5 位（`SyncState` 语义不变）；高 3 位为内部状态。NDK `GetFileSyncStates` 对目录路径返 `NO_SYNC_STATE`（保持 v1 行为）。

### 2.2 PlaceholderState 编码

| 值 | 常量 | 含义 | counted |
| --- | --- | --- | --- |
| 0 | `NONE` | 普通文件 | 否 |
| 1 | `UNHYDRATED` | 未水合占位符（稀疏文件，纯云 stub） | 是 |
| 2 | `PARTIALLY_HYDRATED` | 部分水合占位符（数据部分下载本地） | 是 |
| 3 | `FULLY_HYDRATED` | 完全水合占位符（数据全下载本地，对外仍是占位符/仍受云管） | 是 |
| 4–7 | reserved | 预留 | — |

- `IsPlaceholder(s) = (s != 0)`；`1/2/3` 均计为占位符。
- NDK 不暴露子态：`IsPlaceholder` 仅返回 bool，不区分 `1/2/3`。
- v1 的 `'2'`（hydrating-in-progress）重定义为 `PARTIALLY_HYDRATED`（稳定态，非进行中）；`'0'`→`NONE`、`'1'`→`UNHYDRATED`。
- 水合进度切换 `1↔2↔3`：占位语义不变 → 不触发目录 count 变化；只有 `0 ↔ {1,2,3}` 才改变计数。

> v5 变化：脱水落地 `3→1` 这条子态切换（完全水合→未水合）。`3` 与 `1` 均为占位符 → 脱水不改目录 count、不触发祖先刷新（与水合进度切换同契约）。`state==1` 的既有写入者为 `Create`/`Update`（置 1）；脱水是其非创建性写入路径（`3→1`）。`state==2`（`PARTIALLY_HYDRATED`）仍无写入者，留待水合落地。

### 2.3 文件属性与创建

- 文件权限：`S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP`（`0660`）。
- 创建标志：`O_CREAT | O_EXCL | O_RDWR | O_NOFOLLOW | O_CLOEXEC`，同名已存在得到 `EEXIST` -> `OH_CLOUD_DISK_FILE_ALREADY_EXISTS`。
- 逻辑大小：通过 `ftruncate(fd, logicalSize)` 建稀疏空洞，不写入实际内容。
- 时间：`atime`/`mtime` 由 `futimens` 写入，毫秒转 `timespec`。
- 占位符创建时高 3 位置 `UNHYDRATED(1)`；convert 置 `NONE(0)`；update 重置为 `UNHYDRATED(1)`。**v4**：Mark 置 `FULLY_HYDRATED(3)`（无 `ftruncate`、不写 size/time）；Unmark 置 `NONE(0)`（无 `ftruncate`，前置 `state==3`）。**v5**：脱水置 `UNHYDRATED(1)`（`ftruncate(0)→ftruncate(logicalSize)` 丢数据后置 1，前置 `state==3`）。

### 2.4 目录级占位聚合

- `user.clouddisk.phcount`（`uint32_t` 小端，4 字节）：目录"直接占位子项"数（直系占位符文件 + 直系占位子目录）；缺失视为 0；仅 count≥1 的目录写该 xattr。
- 目录 `filesyncstate` 高 3 位 = `(count>0) ? HAS_PLACEHOLDER(1) : NONE(0)`；低 5 位 恒 0。
- 目录占位聚合**不对外暴露**（NDK 对目录 Get 返 `NO_SYNC_STATE`）；其维护为内部增量 + recount 兜底，算法见 SDD §6/§8/§9。
- customInfo 不参与目录占位聚合：customInfo xattr 的写/读不触发祖先 count 刷新，也不计入 `phcount`。
- **v4**：Mark/Unmark 触发祖先 count 刷新（Mark `delta=+1`、Unmark `delta=−1`）；**v5**：脱水 `3→1` 子态切换**不**触发祖先 count 刷新（`3` 与 `1` 均 counted，delta=0）。

### 2.5 兼容性

- clean break：无真实用户、无迁移、无读回退分支。
- 已有普通文件 high 3 位恒 0 → 读作 `NONE`，天然兼容，无需迁移。
- v1 的独立 `user.clouddisk.placeholder` xattr 及其常量移除。
- 行为变化：v1 下 `CreatePlaceholder` 不写 filesyncstate，NDK `GetFileSyncStates` 对占位符返 `NO_SYNC_STATE`；v2 下 `CreatePlaceholder` 写 filesyncstate（high3=`UNHYDRATED`、low5=`IDLE`），`GetFileSyncStates` 对占位符返 `IDLE(0)`。此为占位态并入 filesyncstate 字节的固有结果。

> v3 变化：因 26.1.0 尚未商用发布，`OH_CloudDisk_CreatePlaceholder`/`OH_CloudDisk_UpdatePlaceholder` 新增 `customInfo` 入参、新增 `OH_CloudDisk_GetPlaceholderCustomInfo` 接口与 `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND` 错误码，均为 clean break（pre-release），无既有消费方二进制兼容负担。新增 `user.clouddisk.custominfo` xattr 为全新 key，无历史兼容负担。

> v4 变化：新增 `OH_CloudDisk_MarkFileAsPlaceholder`/`OH_CloudDisk_UnmarkPlaceholderFile` 两接口与 `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028）错误码，均为 clean break（pre-release），无既有消费方二进制兼容负担。无新 xattr、无新 Parcelable、无新结构体；既有 `filesyncstate`/`custominfo` xattr 语义不变；既有 `ConvertPlaceholderToFile`（数据破坏型）签名与行为不变，与新接口并存。

> v5 变化：落地 `OH_CloudDisk_DehydrateFile` 与新增 `CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE`、`CloudDisk_DehydrateInfo`、`OH_CLOUD_DISK_DEHYDRATE_DENIED`（34400029）错误码，均为 clean break（pre-release），无既有消费方二进制兼容负担。`CloudDisk_CallbackContext` union 加 `dehydrateData` 变体（union 大小不变，指针变体）；`CloudDisk_CallbackType` append 值 3；既有 `filesyncstate`/`custominfo` xattr 语义不变；既有 Mark/Unmark/Convert 签名与行为不变，与脱水并存。回执包格式对既有 `FETCH_*` 类型不变（按 `callbackType` 分支）。

### 2.6 占位符自定义信息持久化（v3 新增）

- xattr key：`user.clouddisk.custominfo`；值为 customInfo 原始字节（xattr 自带长度，无额外 length 前缀）。
- 写入点：`SetPlaceholderFileAttributes`（create 与 update 的汇聚点），仅在 `data` 非空时 `fsetxattr` 写入；`data` 为空时**不写、不删**（实现 update 保留语义）。
- 大小上限 4 KiB；NDK 入口与 service 入口双重校验，超限返回 `CLOUD_DISK_INVALID_ARG`（service 侧对应 `E_INVALID_ARG`，见 §3）。
- 不参与目录占位聚合、不进 dentry 占位字段、不进 RDB；仅随占位符文件本身持久化。
- 水合/脱水不读写 customInfo xattr（customInfo 与水合脱水正交）。
- **v4**：Mark/Unmark 不读写 customInfo xattr（customInfo 与 state-only 转换正交）；调用方需 customInfo 另行调既有 `UpdatePlaceholder`。
- **v5**：脱水不读写 customInfo xattr（customInfo 与脱水正交）；调用方需 customInfo 另行调既有 `UpdatePlaceholder`。

## 3. 错误码

占位符特性随 `@since 26.1.0` 新增的错误码（`34400016`~`34400029`）。"产生接口"列使用简称，对应第 4、5 节接口。

| 枚举名 | 数值 | 语义 | 产生接口 |
| --- | --- | --- | --- |
| `OH_CLOUD_DISK_FILE_ALREADY_EXISTS` | 34400016 | 目标路径已存在同名文件 | Create |
| `OH_CLOUD_DISK_NOT_A_PLACEHOLDER` | 34400017 | 目标路径不是占位符文件 | Convert、Get、Unmark、**Dehydrate**（v5） |
| `OH_CLOUD_DISK_IS_A_PLACEHOLDER` | 34400018 | 目标路径是占位符文件 | Mark（v4） |
| `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS` | 34400019 | 文件正在水合中 | 保留，当前无接口产生 |
| `OH_CLOUD_DISK_NO_SPACE_LEFT` | 34400020 | 磁盘可用空间不足 | Create、Update |
| `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED` | 34400021 | 回调表未注册 | UnregisterCallbackTable、**Dehydrate**（v5） |
| `OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED` | 34400022 | 回调表已注册 | RegisterCallbackTable |
| `OH_CLOUD_DISK_NOT_A_DIRECTORY` | 34400023 | 目标路径的父路径不是目录 | Create、Convert、Update、Mark、Unmark（v4） |
| `OH_CLOUD_DISK_FILE_NOT_EXIST` | 34400024 | 目标路径不存在 | Is、Get |
| `OH_CLOUD_DISK_NAME_TOO_LONG` | 34400025 | 文件名或路径过长 | Create、Convert、Update、Mark、Unmark（v4） |
| `OH_CLOUD_DISK_FILE_TOO_LARGE` | 34400026 | 文件过大 | Create、Update |
| `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND` | 34400027 | 占位符存在但无 customInfo xattr | Get |
| `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED` | 34400028 | 占位符存在但未完全水合（state 1/2），不可 Unmark/Dehydrate | Unmark（v4）、**Dehydrate**（v5） |
| `OH_CLOUD_DISK_DEHYDRATE_DENIED` | 34400029 | 应用回调授权拒绝脱水（allow=false） | **Dehydrate**（v5 新增） |

> v3 变化：新增 `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`（34400027，append-only，沿用 `344xxxxx` 前缀），由 `Get` 产生。校验类失败（`dataLength` 与实际不符、超 4 KiB）复用既有通用码 `CLOUD_DISK_INVALID_ARG`，不新增。`34400018`/`34400019` 维持"保留，当前无接口产生"。启用前需同步内部错误枚举（`E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND = 34400027`）与映射。

> v4 变化：新增 `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028，append-only，沿用 `344xxxxx` 前缀），由 `Unmark` 产生（占位符在但 `state∈{1,2}`）。`34400018`（`IS_A_PLACEHOLDER`）由 v3 的"保留，当前无接口产生"改为 **`Mark` 产生**（目标已是占位符）。`34400019`（`HYDRATE_IN_PROGRESS`）仍维持"保留，当前无接口产生"，待水合落地。校验类失败（空参、路径非法）复用既有通用码 `CLOUD_DISK_INVALID_ARG`/`CLOUD_DISK_SYNC_FOLDER_*`，不新增。启用前需同步：NDK 枚举新增 `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028`；**内部枚举**（`utils/clouddiskservice/include/cloud_disk_service_error.h`，稀疏）新增 `E_IS_A_PLACEHOLDER = 34400018`（当前 17→19 间空缺，需填补）+ `E_PLACEHOLDER_NOT_FULLY_HYDRATED = 34400028`；NDK↔inner 映射表 `innerToNErrTable`（`oh_cloud_disk_utils.h`）增 2 行。`E_NOT_A_PLACEHOLDER` 等既有值不变。

> v5 变化：新增 `OH_CLOUD_DISK_DEHYDRATE_DENIED`（34400029，append-only，沿用 `344xxxxx` 前缀），由 `Dehydrate` 产生（应用回调 `allow=false`）。`34400019`（`HYDRATE_IN_PROGRESS`）仍维持"保留，当前无接口产生"，待水合落地（届时由脱水/水合的在途护栏产生）。`OH_CLOUD_DISK_NOT_A_PLACEHOLDER`（17）/`OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（28）/`OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`（21）增加 `Dehydrate` 产生（脱水前置态 `state==0`/`state==2`/无回调表）。校验类失败（空参、路径非法、`syncFolder` 未注册/`bundleName` 不匹配）复用既有通用码 `CLOUD_DISK_INVALID_ARG`/`CLOUD_DISK_SYNC_FOLDER_*`，不新增。启用前需同步：NDK 枚举新增 `OH_CLOUD_DISK_DEHYDRATE_DENIED = 34400029`；**内部枚举**新增 `E_DEHYDRATE_DENIED = 34400029`（末尾）；NDK↔inner 映射表 `innerToNErrTable` 增 1 行。完整连续链：v3（27）→v4（28）→v5（29），**v3/v4 必须先或同期落地**以保持连续。

各接口还可能返回通用返回码：`CLOUD_DISK_OK`、`CLOUD_DISK_INVALID_ARG`、`CLOUD_DISK_PERMISSION_DENIED`、`CLOUD_DISK_NOT_SUPPORTED`、`CLOUD_DISK_IPC_FAILED`、`CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED`、`CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED`、`CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST`、`CLOUD_DISK_TRY_AGAIN`（均 `@since 21`，见 `cloud_disk_error_code.h`）。Create/Update/Get 的 customInfo 校验失败均走 `CLOUD_DISK_INVALID_ARG`。Mark/Unmark/Dehydrate 的 sync folder 校验失败走 `CLOUD_DISK_SYNC_FOLDER_*` 系列。

## 4. 占位符文件接口

参数 `syncFolderPath` 为已注册的同步文件夹路径，未注册或调用方 `bundleName` 与注册方不一致时拒绝；`relativePathInfo`/`path` 为相对同步文件夹的相对路径，不以 `/` 开头或结尾，不含非法路径段，须落在同步文件夹范围内。

> v3 变化：v2 下"下列各接口对外签名与 v1 一致"。v3 **打破**此约定：`OH_CloudDisk_CreatePlaceholder`/`OH_CloudDisk_UpdatePlaceholder` 新增 `customInfo` 入参（第 4 参数）；新增 `OH_CloudDisk_GetPlaceholderCustomInfo`（§4.5）。因 26.1.0 未商用，属 clean break。

> v4 变化：新增 `OH_CloudDisk_MarkFileAsPlaceholder`（§4.6）、`OH_CloudDisk_UnmarkPlaceholderFile`（§4.7）。既有 Create/Is/Convert/Update/Get 签名与行为不变。因 26.1.0 未商用，属 clean break。

> v5 变化：§4 各接口签名与行为不变；脱水（`OH_CloudDisk_DehydrateFile`）见 §5.3。因 26.1.0 未商用，属 clean break。

### 4.1 `OH_CloudDisk_CreatePlaceholder`

```c
CloudDisk_ErrorCode OH_CloudDisk_CreatePlaceholder(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo,
    const OH_CloudDisk_PlaceholderInfo placeholderInfo,
    const OH_CloudDisk_PlaceholderCustomInfo *customInfo);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `relativePathInfo` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |
| `placeholderInfo` | `OH_CloudDisk_PlaceholderInfo` | 占位符元数据 |
| `customInfo` | `const OH_CloudDisk_PlaceholderCustomInfo *` | 可选自定义信息；NULL 或 `dataLength==0` 表示不提供 |

- 行为：在已注册同步文件夹内的 `relativePathInfo` 处创建占位符文件；按 `logicalSize` 建稀疏空洞，写入 `atime`/`mtime`，并将 `filesyncstate` 高 3 位置 `UNHYDRATED(1)`（低 5 位保留;新文件为 IDLE(0)）。同名文件已存在则失败。
- v3 customInfo 行为：`customInfo` 非空且 `dataLength>0` 时，将原始字节写入 `user.clouddisk.custominfo` xattr；NULL 或 `dataLength==0` 时不写该 xattr。`dataLength` 与实际不符或超 4 KiB 返回 `CLOUD_DISK_INVALID_ARG`（校验先于文件创建，不产生半成品占位符）。
- v2 内部副作用：创建成功后触发祖先目录 count 增量刷新（delta=+1，模型 B），见 SDD §6/§7。customInfo 写/不写不影响该副作用。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_FILE_ALREADY_EXISTS`：目标路径已存在同名文件
  - `OH_CLOUD_DISK_NO_SPACE_LEFT`：磁盘可用空间不足
  - `OH_CLOUD_DISK_NOT_A_DIRECTORY`：目标路径的父路径不是目录
  - `OH_CLOUD_DISK_NAME_TOO_LONG`：文件名或路径过长
  - `OH_CLOUD_DISK_FILE_TOO_LARGE`：文件过大
  - 通用返回码（见第 3 节；customInfo 校验失败走 `CLOUD_DISK_INVALID_ARG`）

### 4.2 `OH_CloudDisk_IsPlaceholderFile`

```c
CloudDisk_ErrorCode OH_CloudDisk_IsPlaceholderFile(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo path,
    bool *isPlaceholder);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `path` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |
| `isPlaceholder` | `bool *` | 出参，是否为占位符；仅在返回 `CLOUD_DISK_OK` 时有效，错误时置 `false` |

- 行为：读取 `path` 的 `filesyncstate` 高 3 位，`!= 0`（`UNHYDRATED`/`PARTIALLY_HYDRATED`/`FULLY_HYDRATED`）返回 `true`，高 3 位为 0（`NONE`，含无 xattr 的普通文件）返回 `false`；目录路径拒绝。对外行为与 v1/v2/v3/v4/v5 一致（仍返回 bool，不区分子态）。不读写 customInfo。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_FILE_NOT_EXIST`：目标路径不存在
  - 通用返回码（见第 3 节）

### 4.3 `OH_CloudDisk_ConvertPlaceholderToFile`

```c
CloudDisk_ErrorCode OH_CloudDisk_ConvertPlaceholderToFile(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `relativePathInfo` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |

- 行为：将占位符转换为 0 字节普通文件：`ftruncate(0)` 后将 `filesyncstate` 高 3 位置 `NONE(0)`（低 5 位保留）。前置：高 3 位 `!= 0`（任意占位态均可转换）。**数据破坏型**（清空本地内容）。
- v3 customInfo 行为：convert **不读写** `user.clouddisk.custominfo` xattr（customInfo 与 convert 正交；若需在转换时清理 customInfo，由调用方显式 Update 处理）。
- v2 变化：v1 中值为 `'2'`（hydrating-in-progress）时不可转换并返回 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`；v2/v3/v4/v5 的 `'2'` 已重定义为 `PARTIALLY_HYDRATED`（稳定态），可被转换，该阻塞分支移除。故 `Convert` 不再产生 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（错误码保留，见第 3 节）。
- v2 内部副作用：转换成功后触发祖先目录 count 增量刷新（delta=−1，模型 B），见 SDD §6/§7。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`：目标路径不是占位符文件
  - `OH_CLOUD_DISK_NOT_A_DIRECTORY`：目标路径的父路径不是目录
  - `OH_CLOUD_DISK_NAME_TOO_LONG`：文件名或路径过长
  - 通用返回码（见第 3 节）

> v4 注：本接口为**数据破坏型**（`ftruncate(0)`）。若需"保留本地数据"的占位符→普通转换，使用 §4.7 `OH_CloudDisk_UnmarkPlaceholderFile`（前置 `state==3`，无 `ftruncate`）。二者并存，应用按"丢数据 vs 留数据"自选。

### 4.4 `OH_CloudDisk_UpdatePlaceholder`

```c
CloudDisk_ErrorCode OH_CloudDisk_UpdatePlaceholder(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo,
    const OH_CloudDisk_PlaceholderInfo placeholderInfo,
    const OH_CloudDisk_PlaceholderCustomInfo *customInfo);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `relativePathInfo` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |
| `placeholderInfo` | `OH_CloudDisk_PlaceholderInfo` | 占位符元数据 |
| `customInfo` | `const OH_CloudDisk_PlaceholderCustomInfo *` | 可选自定义信息；NULL 或 `dataLength==0` 表示保留旧 customInfo |

- 行为：更新 `relativePathInfo` 的元数据，支持占位符与普通文件：先 `ftruncate(0)`，再按 `logicalSize` 重设大小、重写 `atime`/`mtime` 并将 `filesyncstate` 高 3 位置 `UNHYDRATED(1)`（低 5 位保留）。
- v3 customInfo 行为：`customInfo` 非空且 `dataLength>0` 时，覆写 `user.clouddisk.custominfo` xattr（原值被替换）；NULL 或 `dataLength==0` 时**保留**既有 customInfo xattr 不动（不写不删）。不提供"显式清空"能力。`dataLength` 与实际不符或超 4 KiB 返回 `CLOUD_DISK_INVALID_ARG`。
- v2 内部副作用：按 before/after 占位态计算 delta（`1 − IsPlaceholder(old)`），delta≠0 时触发祖先目录 count 刷新，见 SDD §7。customInfo 写/不写/保留均不影响该 delta。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_NO_SPACE_LEFT`：磁盘可用空间不足
  - `OH_CLOUD_DISK_NOT_A_DIRECTORY`：目标路径的父路径不是目录
  - `OH_CLOUD_DISK_NAME_TOO_LONG`：文件名或路径过长
  - `OH_CLOUD_DISK_FILE_TOO_LARGE`：文件过大
  - 通用返回码（见第 3 节；customInfo 校验失败走 `CLOUD_DISK_INVALID_ARG`）

### 4.5 `OH_CloudDisk_GetPlaceholderCustomInfo`（v3 新增）

```c
CloudDisk_ErrorCode OH_CloudDisk_GetPlaceholderCustomInfo(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo,
    uint8_t *dataBuf,
    size_t *inOutDataLength);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `relativePathInfo` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |
| `dataBuf` | `uint8_t *` | 出参缓冲，调用方分配（按 4 KiB 上限） |
| `inOutDataLength` | `size_t *` | 入参为 `dataBuf` 容量；出参为实际 customInfo 长度 |

- 行为：读取 `relativePathInfo` 的 `user.clouddisk.custominfo` xattr；访问校验（`syncFolderPath` 已注册、`bundleName` 匹配、路径落在范围内）与 Create/Update 一致。出参走 caller 预分配：NDK shim 把 inner 返回的字节按容量拷进 `dataBuf`，回填实际长度。
- 命中：返回 `CLOUD_DISK_OK`，`dataBuf` 前 actual 字节为 customInfo 原始字节，`*inOutDataLength` = actual。
- 缓冲不足（入参容量 < actual）：返回 `CLOUD_DISK_INVALID_ARG`，`*inOutDataLength` 回填 actual 供调用方重分配，不部分拷贝。
- 未命中：占位符存在但无 `user.clouddisk.custominfo` xattr → `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`（`*inOutDataLength` = 0）；非占位符文件 → `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`；文件不存在 → `OH_CLOUD_DISK_FILE_NOT_EXIST`。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND`：占位符存在但无 customInfo
  - `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`：目标路径不是占位符文件
  - `OH_CLOUD_DISK_FILE_NOT_EXIST`：目标路径不存在
  - 通用返回码（见第 3 节；缓冲不足走 `CLOUD_DISK_INVALID_ARG`）

> 出参采用 caller 预分配（Model 3）：不复用 `OH_CloudDisk_PlaceholderCustomInfo` 作出参，该结构体仅用于 Create/Update 入参（保持 `const`）。不新增 destroy 函数、无隐藏 malloc、单次 IPC。IDL/inner 仍返回 `PlaceholderCustomInfo`/vector（`[out]` vs 返回 sequenceable 待实现期确认，见 `docs/sdd/placeholder-custom-info-sdd.md` §13），由 NDK shim 拷贝进调用方缓冲。

### 4.6 `OH_CloudDisk_MarkFileAsPlaceholder`（v4 新增）

```c
CloudDisk_ErrorCode OH_CloudDisk_MarkFileAsPlaceholder(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `relativePathInfo` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |

- 行为：将普通文件标记为占位符，**只翻状态、不动数据**：将 `filesyncstate` 高 3 位置 `FULLY_HYDRATED(3)`（低 5 位保留）。**不 `ftruncate`、不写 `atime`/`mtime`、不写 `user.clouddisk.custominfo` xattr**。文件当前 size 即占位符 `logicalSize`（`FULLY_HYDRATED` 下数据全本地，二者相等）。前置：高 3 位 `== 0`（必须是普通文件）；已是占位符（高 3 位 `!= 0`）返回 `OH_CLOUD_DISK_IS_A_PLACEHOLDER`。
- v4 语义：本接口为 `FULLY_HYDRATED(3)` 态的**首个写入路径**——经"标记既有本地文件"写入 3，而非 v3 设想的下载回调路径（见 §2.2）。语义自洽：数据全本地 ⇔ `FULLY_HYDRATED`。与 §5.3 水合路径（`1→2→3` 推进）在 `0↔{1,2,3}` 计数契约上等价，不冲突。
- customInfo 行为：**不读写** `user.clouddisk.custominfo` xattr（与 customInfo 正交）；调用方需 customInfo 另行调既有 `UpdatePlaceholder`。
- 内部副作用：`SetHighBits(file, FULLY_HYDRATED(3))` 返回 old（应 = 0）→ `delta = IsPlaceholder(3) − IsPlaceholder(0) = +1` → 调祖先刷新 helper（复用 SDD §5/§6/§7 Model B）→ 同步 `DentrySetPlaceholder(dentry, 3)`（SDD §8.1 IPC 路径要求）。
- 同步、单次 IPC、无 callback、无 `TaskStateManager`；权限走 bundleName 归属（`CheckSyncFolderBundleName`），不走 `ACCESS_CLOUD_DISK_INFO`。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_IS_A_PLACEHOLDER`：目标路径已是占位符文件（v4 启用该码，v3 标"保留，当前无接口产生"）
  - `OH_CLOUD_DISK_NOT_A_DIRECTORY`：目标路径的父路径不是目录
  - `OH_CLOUD_DISK_NAME_TOO_LONG`：文件名或路径过长
  - 通用返回码（见第 3 节；`syncFolder` 未注册/`bundleName` 不匹配走 `CLOUD_DISK_SYNC_FOLDER_*`）

> 入参传值（与 §4.1–4.4 Convert/Update 同族一致），不带 `PlaceholderInfo`、不带 `OH_CloudDisk_PlaceholderCustomInfo`。无新 Parcelable、无新结构体。各层接线（NDK shim / inner manager 虚函数 / IDL `MarkFileAsPlaceholderInner` / SA override / mock / 测试）见 `docs/sdd/placeholder-state-only-conversion-sdd.md` §5/§11。

### 4.7 `OH_CloudDisk_UnmarkPlaceholderFile`（v4 新增）

```c
CloudDisk_ErrorCode OH_CloudDisk_UnmarkPlaceholderFile(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `relativePathInfo` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |

- 行为：将占位符标记为普通文件，**只翻状态、保留本地数据**：将 `filesyncstate` 高 3 位置 `NONE(0)`（低 5 位保留）。**不 `ftruncate`**——与 §4.3 `OH_CloudDisk_ConvertPlaceholderToFile`（数据破坏型，`ftruncate(0)`）的关键差异。二者并存，应用按"丢数据 vs 留数据"自选。
- 前置：高 3 位 `== 3`（必须是 `FULLY_HYDRATED` 占位符）。
  - 高 3 位 `== 0`（普通文件）→ `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`（34400017）。
  - 高 3 位 `∈ {1,2}`（`UNHYDRATED`/`PARTIALLY_HYDRATED`）→ `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028，v4 新增）。该闸门避免 `UNHYDRATED`（纯 stub 稀疏空洞）/`PARTIALLY_HYDRATED`（部分空洞）转"普通"产生"带空洞读零"的伪普通文件。
  - 高 3 位 `== 3` → 放行。
- customInfo 行为：**不读写** `user.clouddisk.custominfo` xattr（与 customInfo 正交）。
- 内部副作用：`SetHighBits(file, NONE(0))` 返回 old（= 3）→ `delta = IsPlaceholder(0) − IsPlaceholder(3) = −1` → 调祖先刷新 helper → 同步 `DentrySetPlaceholder(dentry, 0)`。
- 同步、单次 IPC、无 callback、无 `TaskStateManager`；权限走 bundleName 归属，不走 `ACCESS_CLOUD_DISK_INFO`。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`：目标路径不是占位符文件（state==0）
  - `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`：占位符存在但未完全水合（state∈{1,2}），v4 新增
  - `OH_CLOUD_DISK_NOT_A_DIRECTORY`：目标路径的父路径不是目录
  - `OH_CLOUD_DISK_NAME_TOO_LONG`：文件名或路径过长
  - 通用返回码（见第 3 节；`syncFolder` 未注册/`bundleName` 不匹配走 `CLOUD_DISK_SYNC_FOLDER_*`）

> 入参传值，不带 `PlaceholderInfo`/`PlaceholderCustomInfo`。各层接线见 state-only-conversion SDD §5/§11。

## 5. 水合与脱水接口

### 5.1 `OH_CloudDisk_RegisterCallbackTable` / `OH_CloudDisk_UnregisterCallbackTable`

```c
CloudDisk_ErrorCode OH_CloudDisk_RegisterCallbackTable(
    const CloudDisk_SyncFolderPath syncFolderPath,
    void (*callback)(const CloudDisk_CallbackReqHead reqHead, CloudDisk_CallbackContext reqContext));

CloudDisk_ErrorCode OH_CloudDisk_UnregisterCallbackTable(const CloudDisk_SyncFolderPath syncFolderPath);
```

`OH_CloudDisk_RegisterCallbackTable`：

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `callback` | `void (*)(const CloudDisk_CallbackReqHead, CloudDisk_CallbackContext)` | 回调函数，接收水合数据请求与脱水授权请求 |

`OH_CloudDisk_UnregisterCallbackTable`：

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |

- 行为：为已注册同步文件夹注册/反注册回调表，用于接收水合数据请求（`FETCH_DATA`/`CANCEL_FETCH_DATA`/`FETCH_RANGE_DATA`）与脱水授权请求（`DEHYDRATE`，v5）。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED`：回调表已注册（`Register` 重复注册）
  - `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`：回调表未注册（`Unregister` 未注册即反注册）
  - 通用返回码（见第 3 节）

### 5.2 `OH_CloudDisk_Execute`

```c
CloudDisk_ErrorCode OH_CloudDisk_Execute(
    const CloudDisk_CallbackReqHead reqHead,
    CloudDisk_CallbackContext reqContext,
    CloudDisk_CallbackResponse rsp);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `reqHead` | `CloudDisk_CallbackReqHead` | 回调请求头 |
| `reqContext` | `CloudDisk_CallbackContext` | 回调上下文，按 `callbackType` 取对应成员 |
| `rsp` | `CloudDisk_CallbackResponse` | 回调响应，填充 `fetchData` 返回数据 |

- 行为：在回调中同步响应请求；按 `reqHead.callbackType` 从 `reqContext` 取入参，填充 `rsp.fetchData` 返回数据。`FETCH_DATA` 时 `rsp.fetchData` 有效。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - 通用返回码（见第 3 节）

> v5 注：`OH_CloudDisk_Execute`/`CloudDisk_CallbackResponse` 仅用于 `FETCH_DATA` 数据回写。脱水（`DEHYDRATE`）的授权决定（`bool allow`）**不**经 `Execute`，而在 `OnCallback` 同步回执包中回写（应用在 `OnCallback` 中填充 `dehydrateData->allow`，由 stub 经 `WriteCallbackReply` 写入回执、服务经 `ReadCallbackReply` 读取），与 `FETCH_RANGE_DATA` 的 range data 回执同机制。

### 5.3 `OH_CloudDisk_HydratePlaceholder` / `OH_CloudDisk_DehydrateFile`

```c
CloudDisk_ErrorCode OH_CloudDisk_HydratePlaceholder(
    const CloudDisk_SyncFolderPath *syncFolderPath,
    const CloudDisk_PathInfo *filePath,
    CloudDisk_CallbackType type);

CloudDisk_ErrorCode OH_CloudDisk_DehydrateFile(
    const CloudDisk_SyncFolderPath *syncFolderPath,
    const CloudDisk_PathInfo *filePath);
```

`OH_CloudDisk_HydratePlaceholder`：

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `const CloudDisk_SyncFolderPath *` | 已注册的同步文件夹路径（指针） |
| `filePath` | `const CloudDisk_PathInfo *` | 相对同步文件夹的相对路径（指针） |
| `type` | `CloudDisk_CallbackType` | 请求类型：水合或取消水合 |

`OH_CloudDisk_DehydrateFile`（v5 落地）：

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `const CloudDisk_SyncFolderPath *` | 已注册的同步文件夹路径（指针） |
| `filePath` | `const CloudDisk_PathInfo *` | 相对同步文件夹的相对路径（指针） |

- 行为：主动发起水合/脱水。`HydratePlaceholder` 按 `type` 水合或取消水合；`DehydrateFile` 将完全水合占位符回退为未水合占位符（丢本地数据、保 `logicalSize`）。入参 `syncFolderPath`/`filePath` 为指针（与 §4.1~§4.7 的 `syncFolderPath`/`relativePathInfo` 值传递不同）。
- **`DehydrateFile` 完整行为（v5 落地）**：
  - 前置（读 `filesyncstate` 高 3 位）：
    - 高 3 位 `== 3`（`FULLY_HYDRATED`）→ 放行。
    - 高 3 位 `== 1`（`UNHYDRATED`，已未水合）→ 幂等返回 `CLOUD_DISK_OK`（不操作）。
    - 高 3 位 `== 0`（`NONE`，普通文件）→ `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`（34400017）。
    - 高 3 位 `== 2`（`PARTIALLY_HYDRATED`）→ `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`（34400028）。
  - **强制回调授权**：服务对已注册 callback table 派发 `DEHYDRATE` 回调（`callbackType = CLOUD_DISK_CALLBACK_TYPE_DEHYDRATE`，`dehydrateData->filePath` = 目标文件相对路径）；已注册 callback table 为硬前置，未注册 → `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`（34400021）。应用在 `OnCallback` 同步回执中填充 `dehydrateData->allow`：
    - `allow == false` → 不脱水、返回 `OH_CLOUD_DISK_DEHYDRATE_DENIED`（34400029），态/数据不变。
    - `allow == true` → 继续。
  - 物理操作（整文件）：`ftruncate(fd, 0)` → `ftruncate(fd, logicalSize)`（重延伸失败重试）→ `SetHighBits(file, UNHYDRATED(1))`（末步翻态 `3→1`）→ `fsync`。`logicalSize` = 当前文件 size（`state==3` ⇒ 数据全本地，二者相等）。数据先丢、state 末步 `3→1`；无瞬态 dehydrating。
  - 不读写 `user.clouddisk.custominfo` xattr（v3 §2.6 正交）；`3→1` 子态切换不改目录 count、不祖先刷新（§2.2）。
  - 并发：per-`(syncFolder,filePath)` 串行化锁防并发脱水交错；在途水合护栏（查 `PlaceholderTaskManager` 同文件 task → `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`）随水合落地补，v5 不前置实现。
  - 读安全窗口（已知，v5 维持现状）：`ftruncate(0)` 与 `SetHighBits(1)` 之间 state 仍 `==3`（声称完全水合、本地有数据）但数据已被打空；此间 FUSE/hmdfs 读按"本地有数据"直读稀疏空洞 → 读到全零。per-文件串行锁只串行化 `DehydrateFile`、不挡 FUSE 读路径；随水合落地与 FUSE 可见性一并处理（见 `docs/sdd/placeholder-dehydrate-sdd.md` §13）。
- v2/v3/v4/v5 关联：水合推进时高 3 位经 `UNHYDRATED(1)`→`PARTIALLY_HYDRATED(2)`→`FULLY_HYDRATED(3)`；脱水回退为 `UNHYDRATED(1)`。这些子态切换占位语义不变，不触发目录 count 变化。"水合进行中"的检测/阻塞（对应 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`）延后到水合落地实现时定（见 SDD §3.2）。水合/脱水不读写 customInfo xattr。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`：目标路径不是占位符文件（state==0）
  - `OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED`：占位符存在但未完全水合（state==2）
  - `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED`：回调表未注册（脱水强制授权前置）
  - `OH_CLOUD_DISK_DEHYDRATE_DENIED`：应用回调授权拒绝脱水（allow=false），v5 新增
  - 通用返回码（见第 3 节；`syncFolder` 未注册/`bundleName` 不匹配走 `CLOUD_DISK_SYNC_FOLDER_*`；`ftruncate` 重延伸耗尽走 `CLOUD_DISK_TRY_AGAIN`）

> v4 注：本节水合/脱水仍为设计态、未落地。v4 的 §4.6 `MarkFileAsPlaceholder` 直接置 `FULLY_HYDRATED(3)`，与本节水合路径（`1→2→3` 推进）在 `0↔{1,2,3}` 计数契约上等价；水合落地时复核二者协调（见 state-only-conversion SDD §13）。

> v5 注：**脱水（`DehydrateFile`）落地**；水合（`HydratePlaceholder`）仍为设计态、未落地。脱水经回调表强制授权（应用 `allow` 出参决定是否执行），区别于"直接脱水"。脱水各层接线（NDK shim / inner manager 虚函数 / IDL `DehydrateInner` / SA override / `PlaceholderCallbackManager::DispatchDehydrate` / `WriteCallbackParcel`/`WriteCallbackReply`/`ReadCallbackReply` 扩 `DEHYDRATE` 分支 / mock / 测试）见 `docs/sdd/placeholder-dehydrate-sdd.md`。测试仅落 `test/unittests/clouddiskservice/`（无下划线那棵）。

## 6. 目录占位聚合（内部）

目录级占位聚合为内部能力，**不新增对外 IPC/NDK 接口**。

- 数据：`user.clouddisk.phcount`（目录直接占位子项数，`uint32` 小端）+ 目录 `filesyncstate` 高 3 位（`HAS_PLACEHOLDER`/`NONE`）。
- 维护：
  - 文件占位态变化（create +1 / convert −1 / update 按 before-after / Mark +1 / Unmark −1（v4））沿祖先链增量更新（模型 B：count RMW 逐级到 syncRoot，仅跨 0 边界翻目录 high3 并上溯）。
  - 观察事件删除/重命名经 dentry 占位字段 best-effort 算 delta。
  - recount 兜底：对子树后序重算 count/目录 high3/dentry 字段，修正 best-effort 漂移；触发时机另行设计。
- 重启鲁棒：count/high3 在 xattr、dentry 字段在 metafile，全盘持久化。
- 算法/锁/util 落点详见 SDD §5/§6/§8/§9/§14。

> 目录占位聚合与 NDK `GetFileSyncStates`/`SetFileSyncStates` 的对外语义解耦：NDK 对目录 Get 返 `NO_SYNC_STATE`、对目录 Set 走 RMW 保高位，目录 high3 的维护不经 NDK 路径。customInfo xattr 不参与本聚合（见 §2.4、§2.6）。v4 的 Mark/Unmark 经 `placeholder_helper` 复用本机制，不引入新刷新逻辑（见 state-only-conversion SDD §4.1）。**v5 的脱水 `3→1` 子态切换不触发本机制**（`3` 与 `1` 均 counted，delta=0，见 §2.2）。
