# cloudfiledaemon Agent Notes

`cloudfiledaemon` 是【云盘/云图】模块，负责 CloudFile daemon 与 `/dev/fuse` 驱动建链及 FUSE 相关能力；其中 `cloud_disk` 对应云盘，`fuse_manager` 对应云图。优先关注本目录；不要把 `services/distributedfiledaemon/` 的逻辑混入这里。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `BUILD.gn` | 构建 `cloudfiledaemon` shared library，依赖 cloudsync lite kit、cloudfile kit core、clouddisk_database、utils lite。 |
| `include/ipc/`、`src/ipc/` | `CloudDaemon` SA、stub 和 `StartFuse` IPC 实现。 |
| `include/fuse_manager/`、`src/fuse_manager/` | 【云图】：FUSE 管理和云图 FUSE 操作，负责和 `/dev/fuse` 驱动建链、统计、LRU RDB store。 |
| `include/cloud_disk/`、`src/cloud_disk/` | 【云盘】：云盘 FUSE 操作落地：local/cloud file operations、database manager、账号和应用状态。 |
| `include/utils/`、`src/utils/` | 设置数据监听和同步开关辅助。 |

## 主要链路

云图 `fuse_manager` 链路：

```text
CloudDaemonManager
  -> cloud daemon proxy/stub
  -> CloudDaemon::StartFuse
  -> FuseManager::StartFuse
  -> cloudMediaFuseOps
```

云盘 `cloud_disk` 链路：

```text
CloudDaemonManager
  -> cloud daemon proxy/stub
  -> CloudDaemon::StartFuse
  -> FuseManager::StartFuse
  -> cloudDiskFuseOps
  -> CloudDisk::FuseOperations
  -> FileOperationsBase
    -> FileOperationsLocal
    -> FileOperationsCloud
  -> DatabaseManager / clouddisk_database
```

## 先读文件

- IPC 入口：`include/ipc/cloud_daemon.h`、`src/ipc/cloud_daemon.cpp`、`src/ipc/cloud_daemon_stub.cpp`。
- FUSE 生命周期：`include/fuse_manager/fuse_manager.h`、`src/fuse_manager/fuse_manager.cpp`。
- FUSE 操作分派：`include/cloud_disk/fuse_operations.h`、`src/cloud_disk/fuse_operations.cpp`。
- 文件操作：`file_operations_base.*`、`file_operations_local.*`、`file_operations_cloud.*`、`file_operations_helper.*`。
- 数据依赖：`database_manager.*` 和 `services/clouddisk_database/AGENTS.md`。
- 开关/账号：`account_status.*`、`account_status_listener.*`、`setting_data_helper.*`、`appstate_observer.*`。

## 修改约束

- `StartFuse` 相关变更要同步 `interfaces/inner_api/native/cloud_daemon_kit_inner/` 和 `frameworks/native/cloud_daemon_kit_inner/`。
- FUSE 操作必须明确区分本地态、云端态、缓存态和错误态；不要把云端降级逻辑塞进 local operations。
- 文件路径、fd、mount path、userId 和 bundleName 必须校验。
- 涉及 fd 或 mmap/内存时优先使用仓内 RAII 工具。
- 对 FUSE 行为的改动优先补 `test/unittests/cloud_disk/` 或 `test/unittests/services_daemon/`。

## 测试路由

- FUSE 和文件操作：`test/unittests/cloud_disk/`。
- daemon/manager/proxy：`test/unittests/cloud_daemon/`、`test/unittests/services_daemon/`。
- 相关 mock：`test/unittests/cloud_disk/mock/`、`test/unittests/services_daemon/mock/`。
