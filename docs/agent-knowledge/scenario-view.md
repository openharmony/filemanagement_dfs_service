# DFS Service 场景视图

## 云同步控制

典型链路：

```text
调用方
  -> CloudSyncManager / CloudSyncAssetManager
  -> frameworks/native/cloudsync_kit_inner ServiceProxy
  -> services/cloudsyncservice/ICloudSyncService.idl 生成 proxy/stub
  -> CloudSyncService
  -> CloudFile::DataSyncManager / CloudFileKit
  -> callback, RDB, sync rule, cycle task
```

关键文件：

| 环节 | 文件 |
| --- | --- |
| API 和接口码 | `interfaces/inner_api/native/cloudsync_kit_inner/cloud_sync_manager.h`、`cloud_sync_asset_manager.h`、`cloud_file_sync_service_interface_code.h` |
| 客户端 proxy | `frameworks/native/cloudsync_kit_inner/src/service_proxy.cpp`、`cloud_sync_manager_impl.cpp` |
| SA 入口 | `services/cloudsyncservice/include/ipc/cloud_sync_service.h`、`src/ipc/cloud_sync_service.cpp` |
| 同步状态和规则 | `interfaces/inner_api/native/cloud_file_kit_inner/sync_state_manager.h`、`frameworks/native/cloud_file_kit_inner/src/data_sync/`、`src/sync_rule/` |
| 周期任务 | `services/cloudsyncservice/src/cycle_task/` |
| 测试 | `test/unittests/cloudsync_sa/`、`test/unittests/cloudsync_api/`、`test/unittests/data_sync_manager/` |

## 资产上传、下载和跨设备传输

典型链路：

```text
CloudSyncAssetManager
  -> cloudsync service IPC
  -> download/upload callback manager
  -> CloudFileKit / CloudAssetsDownloader / CloudDatabase
  -> optional SoftBus transport for remote file transfer
```

关键文件：

| 环节 | 文件 |
| --- | --- |
| 资产 API | `interfaces/inner_api/native/cloudsync_kit_inner/cloud_sync_asset_manager.h` |
| 下载回调 | `interfaces/inner_api/native/cloudsync_kit_inner/download_asset_callback.h`、`i_download_asset_callback.h` |
| 回调管理 | `services/cloudsyncservice/src/ipc/download_asset_callback_manager.cpp` |
| 传输 | `services/cloudsyncservice/src/transport/file_transfer_manager.cpp`、`message_handler.cpp`、`transport/softbus/` |
| 云适配 | `interfaces/inner_api/native/cloud_file_kit_inner/cloud_assets_downloader.h`、`adapter/cloud_adapter_example/` |

## CloudFile daemon 和 FUSE

典型链路：

```text
CloudDaemonManager
  -> cloud daemon proxy/stub
  -> CloudDaemon::StartFuse
  -> FuseManager
  -> fuse_operations
  -> FileOperationsLocal / FileOperationsCloud / DatabaseManager
  -> clouddisk_database and dentry/metafile utils
```

关键文件：

| 环节 | 文件 |
| --- | --- |
| daemon API | `interfaces/inner_api/native/cloud_daemon_kit_inner/cloud_daemon_manager.h`、`i_cloud_daemon.h` |
| 客户端 | `frameworks/native/cloud_daemon_kit_inner/` |
| IPC 和 SA | `services/cloudfiledaemon/include/ipc/`、`src/ipc/` |
| FUSE 管理 | `services/cloudfiledaemon/include/fuse_manager/fuse_manager.h`、`src/fuse_manager/fuse_manager.cpp` |
| 文件操作 | `services/cloudfiledaemon/include/cloud_disk/`、`src/cloud_disk/` |
| 元数据和工具 | `utils/dentry/`、`utils/cloud_disk/`、`services/clouddisk_database/` |
| 测试 | `test/unittests/services_daemon/`、`test/unittests/cloud_disk/`、`test/unittests/cloud_daemon/` |

## CloudDiskService 同步文件夹和变更日志

典型链路：

```text
CloudDiskServiceManager
  -> clouddiskservice ServiceProxy
  -> CloudDiskService SA
  -> RegisterSyncFolder / RegisterSyncFolderChanges
  -> DiskMonitor fanotify
  -> CloudDiskServiceLogFile + CloudDiskServiceMetaFile
  -> ChangesResult callback/query
```

关键文件：

| 环节 | 文件 |
| --- | --- |
| API 和数据结构 | `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_common.h`、`cloud_disk_service_manager.h` |
| 客户端 | `frameworks/native/clouddiskservice_kit_inner/` |
| SA 入口 | `services/clouddiskservice/ipc/include/cloud_disk_service.h`、`ipc/src/cloud_disk_service.cpp` |
| 同步文件夹状态 | `services/clouddiskservice/ipc/include/cloud_disk_sync_folder.h` |
| 变更监听 | `services/clouddiskservice/monitor/` |
| 日志和 meta 文件 | `services/clouddiskservice/sync_folder/` |
| 测试 | `test/unittests/clouddiskservice/`、`test/unittests/clouddisk_service/` |

## 云盘数据库

典型链路：

```text
服务层
  -> ClouddiskRdbStore / ClouddiskRdbTransaction
  -> NativeRdb
  -> Notify / Migration / SyncHelper
```

关键文件：

| 环节 | 文件 |
| --- | --- |
| 表结构和常量 | `services/clouddisk_database/include/file_column.h`、`clouddisk_db_const.h`、`clouddisk_type_const.h` |
| RDB 封装 | `clouddisk_rdbstore.*`、`clouddisk_rdb_transaction.*`、`clouddisk_rdb_utils.*` |
| 通知 | `clouddisk_notify.*`、`clouddisk_notify_utils.*` |
| 迁移 | `migration_manager.*` |
| 测试 | `test/unittests/clouddisk_database/` |
