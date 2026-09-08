# CloudDisk 占位符（Placeholder）接口规格

本文件承载三方网盘占位符特性的对外规格，来自 `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h` 与 `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h`。下列占位符特性新增的结构体、错误码、接口均为 `@since 26.1.0`、`@kit CoreFileKit`、`@syscap SystemCapability.FileManagement.CloudDiskManager`、`@library libohclouddiskmanager.so`。

> 水合（hydrate）将占位符填充为完整内容，脱水（dehydrate）将完整文件回退为占位符。水合脱水经回调表驱动：应用注册回调接收数据请求，再同步响应；亦可显式触发。

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

占位符是 hmdfs 挂载路径上的稀疏文件，状态用扩展属性（xattr）标记。

- xattr 名：`user.clouddisk.placeholder`。
- 文件权限：`S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP`（`0660`）。
- 创建标志：`O_CREAT | O_EXCL | O_RDWR | O_NOFOLLOW | O_CLOEXEC`，同名已存在得到 `EEXIST` -> `OH_CLOUD_DISK_FILE_ALREADY_EXISTS`。
- 逻辑大小：通过 `ftruncate(fd, logicalSize)` 建稀疏空洞，不写入实际内容。
- 时间：`atime`/`mtime` 由 `futimens` 写入，毫秒转 `timespec`。

xattr 值（单字节）：

| 值 | 枚举 | 含义 |
| --- | --- | --- |
| `'1'` | `PLACEHOLDER_STATE_PLACEHOLDER` | 占位符 |
| `'2'` | `PLACEHOLDER_STATE_HYDRATING` | 水合中；查询时也视为占位符，不可转换 |
| `'0'` | —（非枚举） | 已转换为 0 字节普通文件 |

> xattr 名与取值集合是跨版本持久化契约，改名/改取值会破坏存量占位符的查询、转换与注销清理。

## 3. 错误码

占位符特性随 `@since 26.1.0` 新增的错误码（`34400016`~`34400026`）。“产生接口”列使用简称，对应第 4、5 节接口。

| 枚举名 | 数值 | 语义 | 产生接口 |
| --- | --- | --- | --- |
| `OH_CLOUD_DISK_FILE_ALREADY_EXISTS` | 34400016 | 目标路径已存在同名文件 | Create |
| `OH_CLOUD_DISK_NOT_A_PLACEHOLDER` | 34400017 | 目标路径不是占位符文件 | Convert |
| `OH_CLOUD_DISK_IS_A_PLACEHOLDER` | 34400018 | 目标路径是占位符文件 | 保留，当前无接口产生 |
| `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS` | 34400019 | 文件正在水合中 | Convert |
| `OH_CLOUD_DISK_NO_SPACE_LEFT` | 34400020 | 磁盘可用空间不足 | Create、Update |
| `OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED` | 34400021 | 回调表未注册 | UnregisterCallbackTable |
| `OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED` | 34400022 | 回调表已注册 | RegisterCallbackTable |
| `OH_CLOUD_DISK_NOT_A_DIRECTORY` | 34400023 | 目标路径的父路径不是目录 | Create、Convert、Update |
| `OH_CLOUD_DISK_FILE_NOT_EXIST` | 34400024 | 目标路径不存在 | Is |
| `OH_CLOUD_DISK_NAME_TOO_LONG` | 34400025 | 文件名或路径过长 | Create、Convert、Update |
| `OH_CLOUD_DISK_FILE_TOO_LARGE` | 34400026 | 文件过大 | Create、Update |

> 保留位 `34400018` 对外已声明，当前服务实现尚未产生；启用前需同步内部错误枚举与映射。

各接口还可能返回通用返回码：`CLOUD_DISK_OK`、`CLOUD_DISK_INVALID_ARG`、`CLOUD_DISK_PERMISSION_DENIED`、`CLOUD_DISK_NOT_SUPPORTED`、`CLOUD_DISK_IPC_FAILED`、`CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED`、`CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED`、`CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST`、`CLOUD_DISK_TRY_AGAIN`（均 `@since 21`，见 `cloud_disk_error_code.h`）。

## 4. 占位符文件接口

参数 `syncFolderPath` 为已注册的同步文件夹路径，未注册或调用方 `bundleName` 与注册方不一致时拒绝；`relativePathInfo`/`path` 为相对同步文件夹的相对路径，不以 `/` 开头或结尾，不含非法路径段，须落在同步文件夹范围内。

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

- 行为：在已注册同步文件夹内的 `relativePathInfo` 处创建占位符文件；按 `logicalSize` 建稀疏空洞，写入 `atime`/`mtime`，并标记 xattr 为 `'1'`。同名文件已存在则失败。
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

- 行为：查询 `path` 的 xattr。值为 `'1'` 或 `'2'` 返回 `true`，无 xattr 或为 `'0'` 返回 `false`；目录路径拒绝。
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

- 行为：将占位符转换为 0 字节普通文件：`ftruncate(0)` 后将 xattr 置为 `'0'`。前置：目标 xattr 必须为 `'1'`；值为 `'2'` 时不可转换。
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - `OH_CLOUD_DISK_NOT_A_PLACEHOLDER`：目标路径不是占位符文件
  - `OH_CLOUD_DISK_HYDRATE_IN_PROGRESS`：文件正在水合中
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

- 行为：更新 `relativePathInfo` 的元数据，支持占位符与普通文件：先 `ftruncate(0)`，再按 `logicalSize` 重设大小、重写 `atime`/`mtime` 并将 xattr 置为 `'1'`。
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
- 返回：成功返回 `CLOUD_DISK_OK`；失败返回下列错误码。
- 错误码：
  - 通用返回码（见第 3 节）
