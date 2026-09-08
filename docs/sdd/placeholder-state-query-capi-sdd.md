# Placeholder 状态查询 CAPI SDD

> 状态：设计已确认，尚未实现。设计结论来自 2026-09-07 的 grill-me 评审；本阶段只更新文档，不修改代码、不编译。

## 1. 背景与目标

现有 `OH_CloudDisk_IsPlaceholderFile` 只能返回“是否为占位符”，会把 `UNHYDRATED`、`PARTIALLY_HYDRATED`、`FULLY_HYDRATED` 折叠为 `true`。`OH_CloudDisk_GetFileSyncStates` 只返回 `filesyncstate` 低 5 位，`OH_CloudDisk_GetPlaceholderCustomInfo` 只返回 customInfo，均不能获得占位符子状态。

本设计新增 provider 侧 CAPI `OH_CloudDisk_GetPlaceholderState`，同步返回文件在查询时刻的占位符四态快照。它不等待水合或脱水完成，也不返回任务状态。

## 2. 已确认决策

| 项目 | 决策 |
| --- | --- |
| 调用方 | 网盘 provider，不是文件管理器系统访问器 |
| 路径模型 | 已注册 `syncFolderPath` + 文件相对路径 `relativePathInfo` |
| 对象范围 | 仅文件；目录返回 `CLOUD_DISK_INVALID_ARG` |
| 状态范围 | `NONE / UNHYDRATED / PARTIALLY_HYDRATED / FULLY_HYDRATED` |
| 普通文件 | 返回 `CLOUD_DISK_OK` + `NONE` |
| 查询一致性 | 单次读取 `filesyncstate` 高 3 位的瞬时快照 |
| 旧接口 | 保留 `OH_CloudDisk_IsPlaceholderFile` 的签名和既有行为 |
| 保留值 | 高 3 位为 4～7 时，新接口返回新增错误码 `34400033` |
| 权限 | 不要求 `ACCESS_CLOUD_DISK_INFO` 或系统应用身份；保留 provider 归属校验 |
| 版本 | `@since 26.1.0` |
| ABI | 只追加 enum、函数、错误码、IDL 方法和 virtual 方法；不修改既有结构体或 Parcelable |

## 3. CAPI 契约

### 3.1 状态枚举

```c
typedef enum OH_CloudDisk_PlaceholderState {
    OH_CLOUD_DISK_PLACEHOLDER_STATE_NONE = 0,
    OH_CLOUD_DISK_PLACEHOLDER_STATE_UNHYDRATED = 1,
    OH_CLOUD_DISK_PLACEHOLDER_STATE_PARTIALLY_HYDRATED = 2,
    OH_CLOUD_DISK_PLACEHOLDER_STATE_FULLY_HYDRATED = 3,
} OH_CloudDisk_PlaceholderState;
```

枚举值与持久化状态一一对应：

| 值 | 含义 |
| --- | --- |
| `NONE(0)` | 普通文件，不具有占位符语义 |
| `UNHYDRATED(1)` | 未水合占位符，本地数据为空洞 |
| `PARTIALLY_HYDRATED(2)` | 部分水合，占位符仅有部分本地数据 |
| `FULLY_HYDRATED(3)` | 完全水合，占位符数据已完整落地 |

4～7 是持久化保留值，不加入公开状态枚举。

### 3.2 查询函数

```c
CloudDisk_ErrorCode OH_CloudDisk_GetPlaceholderState(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo,
    OH_CloudDisk_PlaceholderState *state);
```

- `syncFolderPath`：调用者已经注册的同步目录沙箱路径。
- `relativePathInfo`：同步目录内的文件相对路径；不得为空、为绝对路径、以 `/` 结尾或包含 `.` / `..` 非法路径段。
- `state`：必填出参。入口先写入 `OH_CLOUD_DISK_PLACEHOLDER_STATE_NONE`；仅函数返回 `CLOUD_DISK_OK` 时表示有效查询结果。
- 函数同步返回，不注册 callback，不创建任务，不修改文件状态、同步状态、目录计数或 customInfo。

调用示例：

```c
OH_CloudDisk_PlaceholderState state = OH_CLOUD_DISK_PLACEHOLDER_STATE_NONE;
CloudDisk_ErrorCode ret = OH_CloudDisk_GetPlaceholderState(syncFolderPath, relativePathInfo, &state);
if (ret == CLOUD_DISK_OK && state == OH_CLOUD_DISK_PLACEHOLDER_STATE_PARTIALLY_HYDRATED) {
    // 文件当前只完成了部分水合。
}
```

## 4. 新增错误码

```c
OH_CLOUD_DISK_INVALID_PLACEHOLDER_STATE = 34400033
```

内部错误码：

```cpp
E_INVALID_PLACEHOLDER_STATE = 34400033
```

该错误只表示持久化 `filesyncstate` 高 3 位为保留值 4～7。此时 NDK 出参保持 `NONE`，服务记录错误日志，不把未知状态伪装成普通文件。

## 5. 返回语义

| 场景 | 返回值 | NDK `state` |
| --- | --- | --- |
| 普通文件或 `filesyncstate` xattr 不存在 | `CLOUD_DISK_OK` | `NONE` |
| 未水合占位符 | `CLOUD_DISK_OK` | `UNHYDRATED` |
| 部分水合占位符 | `CLOUD_DISK_OK` | `PARTIALLY_HYDRATED` |
| 完全水合占位符 | `CLOUD_DISK_OK` | `FULLY_HYDRATED` |
| 高 3 位为 4～7 | `OH_CLOUD_DISK_INVALID_PLACEHOLDER_STATE` | `NONE` |
| `state == NULL`、路径格式非法或目标为目录 | `CLOUD_DISK_INVALID_ARG` | NULL 或 `NONE` |
| 文件不存在 | `OH_CLOUD_DISK_FILE_NOT_EXIST` | `NONE` |
| 同步目录未注册或登记 bundleName 与调用者不匹配 | `CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED` | `NONE` |
| SA/IPC/临时错误 | 沿用现有错误码 | `NONE` |
| 特性关闭 | `CLOUD_DISK_NOT_SUPPORTED` | `NONE` |

`OH_CloudDisk_IsPlaceholderFile` 的兼容行为不变：若底层意外读到 4～7，旧接口仍按 `rawState != NONE` 返回成功和 `true`；只有新接口执行严格的 0～3 合法性检查。

## 6. 状态来源与并发

- 状态来自 `user.clouddisk.filesyncstate` 的高 3 位；低 5 位同步状态不参与本接口返回，也不得被修改。
- 继续复用 `GetFilePlaceholderState` 的单次 xattr 读取和现有互斥保护。
- xattr 缺失按零处理，因此普通文件返回 `NONE`，不是 `NOT_A_PLACEHOLDER`。
- 返回值是读取时刻的快照。函数返回后，水合、Execute、脱水、Mark 或 Unmark 均可能立即改变状态。
- 本接口不返回 PENDING/IN_PROGRESS 等任务状态；调用方不能用它判断任务是否正在调度。

## 7. helper 设计

当前没有一个现成 helper 能在不改变错误语义的前提下完整承载新接口，但已有两层底层能力可以直接复用：

- `GetFilePlaceholderState`：读取 `filesyncstate` 并解析高 3 位。
- `IsValidPlaceholderState`：判断状态是否处于 0～3。
- `GetRegisteredMntSyncFolder`：实现 `IsPlaceholderFileInner` 当前的 userId、bundleName、同步目录注册和挂载路径语义。

新增两个 `cloud_disk_service.cpp` 文件内静态 helper，不新增类或源文件：

```cpp
static int32_t ResolvePlaceholderQueryPath(const std::string &syncFolder,
                                           const std::string &relativePath,
                                           std::string &queryPath);

static int32_t QueryPlaceholderStateByXattr(const std::string &queryPath,
                                            uint8_t &state);
```

### 7.1 ResolvePlaceholderQueryPath

从现有 `IsPlaceholderFileInner` 机械抽取以下步骤，保持顺序和返回码不变：

1. 校验相对路径。
2. 获取调用者 userId；userId 为 0 时沿用 accountId fallback。
3. 调用 `GetRegisteredMntSyncFolder`，校验注册信息和 bundleName 归属。
4. 拼接 mount 路径并执行同步目录边界检查。
5. 拒绝目录路径。

不能改用 `ResolvePlaceholderStatePath`：它对 bundleName 不匹配返回 `E_SYNC_FOLDER_PATH_UNAUTHORIZED`，对不存在路径返回 `E_SYNC_FOLDER_PATH_NOT_EXIST`，且会获取查询不需要的 syncRoot 上下文，会改变旧查询接口的错误码和校验顺序。

不能复用完整的 `GetPlaceholderCustomInfoInner`：它要求目标已经是占位符，并带有 customInfo 专属错误语义，与普通文件返回 `NONE` 冲突。

### 7.2 QueryPlaceholderStateByXattr

该 helper 调用 `GetFilePlaceholderState(queryPath, state)`，并复用现有 `ConvertPlaceholderXattrErrno` 映射。它返回原始高 3 位，不在共享层校验 4～7。

```text
IsPlaceholderFileInner
  -> ResolvePlaceholderQueryPath
  -> QueryPlaceholderStateByXattr
  -> isPlaceholder = IsPlaceholderState(rawState)

GetPlaceholderStateInner
  -> ResolvePlaceholderQueryPath
  -> QueryPlaceholderStateByXattr
  -> IsValidPlaceholderState(rawState)
       false -> E_INVALID_PLACEHOLDER_STATE
       true  -> state = rawState
```

这保证两接口共享路径与 xattr 查询逻辑，同时保持旧接口对保留值的行为。

## 8. 分层接线

```text
OH_CloudDisk_GetPlaceholderState
  -> CloudDiskServiceManager::GetPlaceholderState
  -> CloudDiskServiceManagerImpl::GetPlaceholderState
  -> ICloudDiskService::GetPlaceholderStateInner
  -> CloudDiskService::GetPlaceholderStateInner
  -> ResolvePlaceholderQueryPath
  -> QueryPlaceholderStateByXattr
  -> GetFilePlaceholderState
```

IDL 使用整数返回值承载状态，无需新增 Parcelable：

```idl
int GetPlaceholderStateInner([in] String syncFolder, [in] String relativePath);
```

对应 C++ override/manager 通过 `int32_t &state` 接收 IDL 返回值。新 IDL 方法必须追加到接口末尾；新 virtual 方法必须追加到 `CloudDiskServiceManager` 末尾。

## 9. 预计修改范围

| 层级 | 文件 | 设计改动 |
| --- | --- | --- |
| CAPI | `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_manager.h` | 新增状态 enum 和查询声明 |
| CAPI | `interfaces/kits/ndk/clouddiskmanager/include/cloud_disk_error_code.h` | 末尾追加错误码 34400033 |
| CAPI | `interfaces/kits/ndk/clouddiskmanager/include/oh_cloud_disk_utils.h` | 增加内部错误码到 CAPI 错误码映射 |
| CAPI | `interfaces/kits/ndk/clouddiskmanager/src/oh_cloud_disk_manager.cpp` | 参数校验、出参初始化和 manager 调用 |
| 导出 | `interfaces/kits/ndk/clouddiskmanager/liboh_cloud_disk_manager.ndk.json` | 追加新 C 符号 |
| inner API | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_service_manager.h` | 末尾追加 `GetPlaceholderState` virtual |
| framework | `frameworks/native/clouddiskservice_kit_inner/include/cloud_disk_service_manager_impl.h` | 新增 override |
| framework | `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_service_manager_impl.cpp` | 新增 proxy 转发 |
| IDL | `services/clouddiskservice/ICloudDiskService.idl` | 接口末尾追加 `GetPlaceholderStateInner` |
| service | `services/clouddiskservice/ipc/include/cloud_disk_service.h` | 新增 override |
| service | `services/clouddiskservice/ipc/src/cloud_disk_service.cpp` | 抽取 helper，实现四态查询和严格校验 |
| 错误码 | `utils/clouddiskservice/include/cloud_disk_service_error.h` | 末尾追加 `E_INVALID_PLACEHOLDER_STATE` |
| mock/test | `test/unittests/clouddiskservice/` | 同步 manager/IDL mock，并补 NDK、framework、service 测试 |

不新增源文件，不修改 BUILD，不修改 `OH_CloudDisk_PlaceholderInfo`、inner `PlaceholderInfo` 或任何 Parcelable。

## 10. 兼容性与安全

- 新 C 函数和 enum 是追加式 CAPI；现有符号和结构体布局不变。
- 新错误码使用下一个连续值 `34400033`，不重排已有错误码。
- 新 IDL 方法追加到末尾，避免重排现有 transaction ordinal。
- 新 virtual 方法追加到末尾，避免移动已有 vtable slot；所有派生实现和 mock 同步新增。
- inner kit map 已通配 `CloudDiskServiceManager` 符号，无需单独增加 map 条目；NDK JSON 必须登记新 C 符号。
- 不调用 `CheckPermissions(PERM_CLOUD_DISK_SERVICE, true)`，不要求系统应用身份。
- 继续使用现有查询接口的 userId、accountId fallback、bundleName 和注册目录校验；目录未注册与 bundleName 不匹配均返回 `E_SYNC_FOLDER_NOT_REGISTERED`，不泄露其他 provider 信息。
- 不改变现有查询路径的符号链接和竞态行为；相关安全加固若需要，应作为独立变更评审，不能夹带在本接口中。

## 11. 实现阶段测试计划

本阶段不实现、不编译。后续取得实现授权后，至少覆盖：

1. NDK：NULL `state`、非法 `syncFolderPath` / `relativePathInfo`、manager 错误映射、四态成功回填、失败保持 `NONE`。
2. Service：四个合法状态、xattr 缺失、文件不存在、目录、非法相对路径、越界路径、userId=0 fallback、bundleName 获取失败、目录未注册和 bundleName 不匹配。
3. 保留值：4～7 均返回 `E_INVALID_PLACEHOLDER_STATE`；NDK 映射为 `OH_CLOUD_DISK_INVALID_PLACEHOLDER_STATE`。
4. 兼容回归：旧 `IsPlaceholderFileInner` 对 4～7 仍返回成功和 `true`，其已有错误码不变。
5. Framework/IPC：proxy 为空、IDL 状态回传、mock 接口同步。
6. 特性关闭：返回 `E_NOT_SUPPORTED` / `CLOUD_DISK_NOT_SUPPORTED`。
7. 实现完成后执行 `git diff --check`，并按仓库规则执行 `rk3568` 下 `dfs_service`、`cloudsyncunittests` 的默认及 CloudDisk 特性开启四组编译。

## 12. 本阶段完成边界

本 SDD、canonical spec 和 changelog 对齐后，设计阶段结束。源码、测试、BUILD、提交及远端分支均不在本阶段修改范围内。
