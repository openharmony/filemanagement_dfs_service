# CloudDisk 占位符（Placeholder）接口规格 v2

本文件是 v1（`clouddisk-placeholder-v1.md`）的修订版，反映占位符 xattr 重构：占位符状态并入 `user.clouddisk.filesyncstate` 高 3 位，移除独立 `user.clouddisk.placeholder` xattr；新增目录级占位计数与目录占位布尔。**对外 NDK 接口签名不变**；行为层面 `IsPlaceholder` 仍返回 bool、`Convert` 错误集微调、`GetFileSyncStates` 对占位符由 `NO_SYNC_STATE` 改返 `IDLE`（见第 2.5、3、4 节）。内部维护机制（Model B 祖先刷新、recount 兜底、dentry 字段、util/全局锁）见 `docs/sdd/placeholder-marking-refactor-sdd.md`，本规格不复述。v1 保留以记录重构前契约。

本规格来自 `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h` 与 `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h`。下列占位符特性结构体、错误码、接口均为 `@since 26.1.0`、`@kit CoreFileKit`、`@syscap SystemCapability.FileManagement.CloudDiskManager`、`@library libohclouddiskmanager.so`。

> 水合（hydrate）将占位符填充为完整内容，脱水（dehydrate）将完整文件回退为占位符。水合脱水经回调表驱动：应用注册回调接收数据请求，再同步响应；亦可显式触发。v2 下水合进度经 `UNHYDRATED`→`PARTIALLY_HYDRATED`→`FULLY_HYDRATED`（1→2→3）推进（高 3 位子态），子态切换不改变占位计数。

## 1. 结构体

### 1.1 `OH_CloudDisk_PlaceholderInfo`

占位符元数据，用于创建与更新占位符文件。

```c
typedef struct OH_CloudDisk_PlaceholderInfo {
    uint64_t logicalSize; // 云端文件的逻辑大小（字节）。占位符按此大小建为稀疏空洞，本地不下载实际内容
    uint64_t atimeMs;      // 占位符访问时间视图（毫秒），写入文件 atime
    uint64_t mtimeMs;      // 云端文件修改时间视图（毫秒），写入文件 mtime
} OH_CloudDisk_PlaceholderInfo;
```

### 1.2 水合脱水数据契约

以下类型承载水合脱水的回调数据契约，引用 `@since 21` 的既有类型 `CloudDisk_PathInfo` 与 `CloudDisk_SyncFolderPath`（后者为前者的别名）。

```c
typedef struct CloudDisk_DataBuf {
    uint8_t *data;     // 数据缓冲区
    uint64_t dataSize; // 数据长度
} CloudDisk_DataBuf;

typedef enum CloudDisk_CallbackType {
    CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA = 0,       // 获取云端文件数据（水合）
    CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA = 1, // 取消获取云端文件数据（取消水合）
    CLOUD_DISK_CALLBACK_TYPE_FETCH_RANGE_DATA = 2, // 获取云端文件指定范围数据（流读）
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

typedef union CloudDisk_CallbackContext {
    CloudDisk_PathInfo *fetchData;         // callbackType 为 FETCH_DATA 时的文件路径
    CloudDisk_PathInfo *cancelFetchData;  // callbackType 为 CANCEL_FETCH_DATA 时的文件路径
    CloudDisk_RangeInfo *fetchRangeData; // callbackType 为 FETCH_RANGE_DATA 时的范围信息
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

## 2. 存储实现

占位符是 hmdfs 挂载路径上的稀疏文件。**v2 起，占位符状态不再使用独立 `user.clouddisk.placeholder` xattr，而是并入 `user.clouddisk.filesyncstate` 的高 3 位**（低 5 位仍为 `SyncState` 0–5）。v1 的独立 xattr 契约废弃。

### 2.1 filesyncstate 字节布局

```
bit7 bit6 bit5 | bit4 bit3 bit2 bit1 bit0
<── 高 3 位 ──> <────── 低 5 位 ──────>
 PlaceholderState       SyncState
```

- **文件**：高 3 位 = `PlaceholderState`；低 5 位 = `SyncState`（0–5，bit3-4 保留 0）。
- **目录**：高 3 位 = `(count>0) ? 1 : 0`（`HAS_PLACEHOLDER`）；低 5 位 恒 0，不语义化。
- “是否占位”统一以高 3 位 `!= 0` 判（文件与目录一致）。
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

### 2.3 文件属性与创建

- 文件权限：`S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP`（`0660`）。
- 创建标志：`O_CREAT | O_EXCL | O_RDWR | O_NOFOLLOW | O_CLOEXEC`，同名已存在得到 `EEXIST` -> `OH_CLOUD_DISK_FILE_ALREADY_EXISTS`。
- 逻辑大小：通过 `ftruncate(fd, logicalSize)` 建稀疏空洞，不写入实际内容。
- 时间：`atime`/`mtime` 由 `futimens` 写入，毫秒转 `timespec`。
- 占位符创建时高 3 位置 `UNHYDRATED(1)`；convert 置 `NONE(0)`；update 重置为 `UNHYDRATED(1)`。

### 2.4 目录级占位聚合（v2 新增）

- `user.clouddisk.phcount`（`uint32_t` 小端，4 字节）：目录“直接占位子项”数（直系占位符文件 + 直系占位子目录）；缺失视为 0；仅 count≥1 的目录写该 xattr。
- 目录 `filesyncstate` 高 3 位 = `(count>0) ? HAS_PLACEHOLDER(1) : NONE(0)`；低 5 位 恒 0。
- 目录占位聚合**不对外暴露**（NDK 对目录 Get 返 `NO_SYNC_STATE`）；其维护为内部增量 + recount 兜底，算法见 SDD §6/§8/§9。

### 2.5 兼容性

- clean break：无真实用户、无迁移、无读回退分支。
- 已有普通文件 high 3 位恒 0 → 读作 `NONE`，天然兼容，无需迁移。
- v1 的独立 `user.clouddisk.placeholder` xattr 及其常量移除。
- 行为变化：v1 下 `CreatePlaceholder` 不写 filesyncstate，NDK `GetFileSyncStates` 对占位符返 `NO_SYNC_STATE`；v2 下 `CreatePlaceholder` 写 filesyncstate（high3=`UNHYDRATED`、low5=`IDLE`），`GetFileSyncStates` 对占位符返 `IDLE(0)`。此为占位态并入 filesyncstate 字节的固有结果。

> v2 的 filesyncstate 高 3 位 + `phcount` + 目录 high3 取值集合是新的跨版本持久化契约；dentry 占位字段为内部缓存，不构成对外契约。

## 3. 错误码

占位符特性随 `@since 26.1.0` 新增的错误码（`34400016`~`34400026`）。“产生接口”列使用简称，对应第 4、5 节接口。

| 枚举名 | 数值 | 语义 | 产生接口 |
| --- | --- | --- | --- |
| `OH_CLOUD_DISK_FILE_ALREADY_EXISTS` | 34400016 | 目标路径已存在同名文件 | Create |
| `OH_CLOUD_DISK_NOT_A_PLACEHOLDER` | 34400017 | 目标路径不是占位符文件 | Convert |
| `OH_CLOUD_DISK_IS_A_PLACEHOLDER` | 34400018 | 目标路径是占位符文件 | 保留，当前无接口产生 |
| `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS` | 34400019 | 文件正在水合中 | 保留，当前无接口产生 |
| `OH_CLOUD_DISK_NO_SPACE_LEFT` | 34400020 | 磁盘可用空间不足 | Create、Update |
| `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED` | 34400021 | 回调表未注册 | UnregisterCallbackTable |
| `OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED` | 34400022 | 回调表已注册 | RegisterCallbackTable |
| `OH_CLOUD_DISK_NOT_A_DIRECTORY` | 34400023 | 目标路径的父路径不是目录 | Create、Convert、Update |
| `OH_CLOUD_DISK_FILE_NOT_EXIST` | 34400024 | 目标路径不存在 | Is |
| `OH_CLOUD_DISK_NAME_TOO_LONG` | 34400025 | 文件名或路径过长 | Create、Convert、Update |
| `OH_CLOUD_DISK_FILE_TOO_LARGE` | 34400026 | 文件过大 | Create、Update |

> v2 变化：`OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（34400019）由“Convert 产生”改为“保留，当前无接口产生”。原因：v1 的 `'2'`=hydrating-in-progress 阻塞分支随 `'2'` 重定义为 `PARTIALLY_HYDRATED`（稳定态、可被 convert）而移除；错误枚举值保留，待水合真正落地后由 convert 重新产生（“如何判断正在水合中”延后到水合实现时定，见 SDD §3.2）。`34400018` 对外已声明、当前无接口产生，维持不变。启用前需同步内部错误枚举与映射。

各接口还可能返回通用返回码：`CLOUD_DISK_OK`、`CLOUD_DISK_INVALID_ARG`、`CLOUD_DISK_PERMISSION_DENIED`、`CLOUD_DISK_NOT_SUPPORTED`、`CLOUD_DISK_IPC_FAILED`、`CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED`、`CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED`、`CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST`、`CLOUD_DISK_TRY_AGAIN`（均 `@since 21`，见 `cloud_disk_error_code.h`）。

## 4. 占位符文件接口

参数 `syncFolderPath` 为已注册的同步文件夹路径，未注册或调用方 `bundleName` 与注册方不一致时拒绝；`relativePathInfo`/`path` 为相对同步文件夹的相对路径，不以 `/` 开头或结尾，不含非法路径段，须落在同步文件夹范围内。下列各接口的对外签名与 v1 一致，仅行为描述按 v2 存储模型更新。

### 4.1 `OH_CloudDisk_CreatePlaceholder`

```c
CloudDisk_ErrorCode OH_CloudDisk_CreatePlaceholder(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo,
    const OH_CloudDisk_PlaceholderInfo placeholderInfo);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `relativePathInfo` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |
| `placeholderInfo` | `OH_CloudDisk_PlaceholderInfo` | 占位符元数据 |

- 行为：在已注册同步文件夹内的 `relativePathInfo` 处创建占位符文件；按 `logicalSize` 建稀疏空洞，写入 `atime`/`mtime`，并将 `filesyncstate` 高 3 位置 `UNHYDRATED(1)`（低 5 位保留;新文件为 IDLE(0)）。同名文件已存在则失败。
- v2 内部副作用：创建成功后触发祖先目录 count 增量刷新（delta=+1，模型 B），见 SDD §6/§7。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_FILE_ALREADY_EXISTS`：目标路径已存在同名文件
  - `OH_CLOUD_DISK_NO_SPACE_LEFT`：磁盘可用空间不足
  - `OH_CLOUD_DISK_NOT_A_DIRECTORY`：目标路径的父路径不是目录
  - `OH_CLOUD_DISK_NAME_TOO_LONG`：文件名或路径过长
  - `OH_CLOUD_DISK_FILE_TOO_LARGE`：文件过大
  - 通用返回码（见第 3 节）

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

- 行为：读取 `path` 的 `filesyncstate` 高 3 位，`!= 0`（`UNHYDRATED`/`PARTIALLY_HYDRATED`/`FULLY_HYDRATED`）返回 `true`，高 3 位为 0（`NONE`，含无 xattr 的普通文件）返回 `false`；目录路径拒绝。对外行为与 v1 一致（仍返回 bool，不区分子态）。
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

- 行为：将占位符转换为 0 字节普通文件：`ftruncate(0)` 后将 `filesyncstate` 高 3 位置 `NONE(0)`（低 5 位保留）。前置：高 3 位 `!= 0`（任意占位态均可转换）。
- v2 变化：v1 中值为 `'2'`（hydrating-in-progress）时不可转换并返回 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`；v2 的 `'2'` 已重定义为 `PARTIALLY_HYDRATED`（稳定态），可被转换，该阻塞分支移除。故 `Convert` 不再产生 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`（错误码保留，见第 3 节）。
- v2 内部副作用：转换成功后触发祖先目录 count 增量刷新（delta=−1，模型 B），见 SDD §6/§7。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`：目标路径不是占位符文件
  - `OH_CLOUD_DISK_NOT_A_DIRECTORY`：目标路径的父路径不是目录
  - `OH_CLOUD_DISK_NAME_TOO_LONG`：文件名或路径过长
  - 通用返回码（见第 3 节）

### 4.4 `OH_CloudDisk_UpdatePlaceholder`

```c
CloudDisk_ErrorCode OH_CloudDisk_UpdatePlaceholder(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo,
    const OH_CloudDisk_PlaceholderInfo placeholderInfo);
```

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |
| `relativePathInfo` | `CloudDisk_PathInfo` | 相对同步文件夹的相对路径 |
| `placeholderInfo` | `OH_CloudDisk_PlaceholderInfo` | 占位符元数据 |

- 行为：更新 `relativePathInfo` 的元数据，支持占位符与普通文件：先 `ftruncate(0)`，再按 `logicalSize` 重设大小、重写 `atime`/`mtime` 并将 `filesyncstate` 高 3 位置 `UNHYDRATED(1)`（低 5 位保留）。
- v2 内部副作用：按 before/after 占位态计算 delta（`1 − IsPlaceholder(old)`），delta≠0 时触发祖先目录 count 刷新，见 SDD §7。该 delta 对“update 是否收窄为只更新已有占位符”的未来语义鲁棒（见 SDD §7.1）。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_NO_SPACE_LEFT`：磁盘可用空间不足
  - `OH_CLOUD_DISK_NOT_A_DIRECTORY`：目标路径的父路径不是目录
  - `OH_CLOUD_DISK_NAME_TOO_LONG`：文件名或路径过长
  - `OH_CLOUD_DISK_FILE_TOO_LARGE`：文件过大
  - 通用返回码（见第 3 节）

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
| `callback` | `void (*)(const CloudDisk_CallbackReqHead, CloudDisk_CallbackContext)` | 回调函数，接收水合数据请求 |

`OH_CloudDisk_UnregisterCallbackTable`：

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `CloudDisk_SyncFolderPath` | 已注册的同步文件夹路径 |

- 行为：为已注册同步文件夹注册/反注册回调表，用于接收水合数据请求。
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
| `reqContext` | `CloudDisk_CallbackContext` | 回调请求上下文，按 `callbackType` 取对应成员 |
| `rsp` | `CloudDisk_CallbackResponse` | 回调响应，填充 `fetchData` 返回数据 |

- 行为：在回调中同步响应请求；按 `reqHead.callbackType` 从 `reqContext` 取入参，填充 `rsp.fetchData` 返回数据。`FETCH_DATA` 时 `rsp.fetchData` 有效。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - 通用返回码（见第 3 节）

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

`OH_CloudDisk_DehydrateFile`：

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| `syncFolderPath` | `const CloudDisk_SyncFolderPath *` | 已注册的同步文件夹路径（指针） |
| `filePath` | `const CloudDisk_PathInfo *` | 相对同步文件夹的相对路径（指针） |

- 行为：主动发起水合/脱水。`HydratePlaceholder` 按 `type` 水合或取消水合；`DehydrateFile` 将文件回退为占位符。入参为指针（与 4.1~4.4 的值传递不同）。
- v2 关联：水合推进时高 3 位经 `UNHYDRATED(1)`→`PARTIALLY_HYDRATED(2)`→`FULLY_HYDRATED(3)`；脱水回退为 `UNHYDRATED(1)`。这些子态切换占位语义不变，不触发目录 count 变化。“水合进行中”的检测/阻塞（对应 `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`）延后到水合落地实现时定（见 SDD §3.2）。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - 通用返回码（见第 3 节）

## 6. 目录占位聚合（v2 新增，内部）

目录级占位聚合为内部能力，**不新增对外 IPC/NDK 接口**。

- 数据：`user.clouddisk.phcount`（目录直接占位子项数，`uint32` 小端）+ 目录 `filesyncstate` 高 3 位（`HAS_PLACEHOLDER`/`NONE`）。
- 维护：
  - 文件占位态变化（create +1 / convert −1 / update 按 before-after）沿祖先链增量更新（模型 B：count RMW 逐级到 syncRoot，仅跨 0 边界翻目录 high3 并上溯）。
  - 观察事件删除/重命名经 dentry 占位字段 best-effort 算 delta。
  - recount 兜底：对子树后序重算 count/目录 high3/dentry 字段，修正 best-effort 漂移；触发时机另行设计。
- 重启鲁棒：count/high3 在 xattr、dentry 字段在 metafile，全盘持久化。
- 算法/锁/util 落点详见 SDD §5/§6/§8/§9/§14。

> 目录占位聚合与 NDK `GetFileSyncStates`/`SetFileSyncStates` 的对外语义解耦：NDK 对目录 Get 返 `NO_SYNC_STATE`、对目录 Set 走 RMW 保高位，目录 high3 的维护不经 NDK 路径。
