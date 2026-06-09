# clouddiskservice Agent Notes

`clouddiskservice` 是【三方网盘】模块，也是 CloudDiskService SA，负责同步文件夹注册、变更日志、文件同步状态和磁盘事件监听。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `ICloudDiskService.idl` | CloudDiskService IPC IDL。 |
| `BUILD.gn` | 构建 `clouddiskservice_sa` 和 IDL 生成目标。 |
| `ipc/` | SA 入口、access token、callback manager/proxy、同步文件夹内存状态。 |
| `monitor/` | fanotify 磁盘事件监听和事件处理。 |
| `sync_folder/` | 同步文件夹日志、meta file、uuid、路径转换。 |
| `seccomp_policy/` | disk monitor seccomp filter。 |

## 主要链路

```text
CloudDiskServiceManager
  -> frameworks/native/clouddiskservice_kit_inner ServiceProxy
  -> ICloudDiskService generated stub
  -> CloudDiskService
  -> CloudDiskSyncFolder / CallbackManager
  -> DiskMonitor
  -> CloudDiskServiceLogFile / CloudDiskServiceMetaFile
  -> ChangesResult and FileSyncState
```

## 先读文件

- SA 入口：`ipc/include/cloud_disk_service.h`、`ipc/src/cloud_disk_service.cpp`。
- Access token：`ipc/include/cloud_disk_service_access_token.h`、`ipc/src/cloud_disk_service_access_token.cpp`。
- 同步文件夹状态：`ipc/include/cloud_disk_sync_folder.h`、`ipc/src/cloud_disk_sync_folder.cpp`。
- Callback：`ipc/include/cloud_disk_service_callback_manager.h`、`ipc/src/cloud_disk_service_callback_manager.cpp`。
- 事件监听：`monitor/include/disk_monitor.h`、`monitor/src/disk_monitor.cpp`。
- 变更日志和 meta：`sync_folder/include/cloud_disk_service_logfile.h`、`cloud_disk_service_metafile.h`。

## 修改约束

- `ICloudDiskService.idl` 变更要同步 `interfaces/inner_api/native/clouddiskservice_kit_inner/`、`frameworks/native/clouddiskservice_kit_inner/`、服务实现和测试。
- `CloudDiskServiceAccessToken` 不可绕过；新增 IPC 方法要明确权限策略。
- 变更日志和 meta file 要保持二进制布局兼容，修改结构体时必须补读写/升级测试。
- fanotify/seccomp 变更要检查 `seccomp_policy/BUILD.gn` 和策略文件。
- 同步文件夹路径要规范化并处理重复注册、取消注册、userId/bundleName 隔离。

## 测试路由

- 旧目录：`test/unittests/clouddiskservice/`。
- 新同步文件夹测试：`test/unittests/clouddisk_service/`。
- meta/log/uuid/convertor：`test/unittests/clouddisk_service/sync_folder/`。
