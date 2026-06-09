# frameworks/native Agent Notes

本目录实现 native inner kit 的客户端侧逻辑。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `cloudsync_kit_inner/` | CloudSync manager impl、ServiceProxy、callback stub/client。 |
| `cloudsync_kit_inner_lite/` | lite 版本 CloudSync proxy/manager。 |
| `cloud_daemon_kit_inner/` | CloudDaemon manager impl、proxy、死亡通知。 |
| `cloud_file_kit_inner/` | CloudFileKit 默认实现、DataSyncManager、CloudDatabase、sync rule/data sync 基础实现。 |
| `clouddiskservice_kit_inner/` | CloudDiskService manager impl、ServiceProxy、callback client/stub、common parcel。 |
| `distributed_file_inner/` | 分布式文件 inner API 客户端，低优先级。 |

## 典型改动路线

- 新增/修改 CloudSync API：先改 `interfaces/inner_api/native/cloudsync_kit_inner/`，再改 `cloudsync_kit_inner/src/service_proxy.cpp` 和对应 manager impl。
- 新增/修改 CloudDaemon API：同步 `cloud_daemon_kit_inner/`。
- 新增/修改 CloudDiskService API：同步 `clouddiskservice_kit_inner/`，尤其 `cloud_disk_common.cpp` parcel 读写。
- 修改 DataSyncManager 或规则：同步 `cloud_file_kit_inner/src/data_sync_manager.cpp`、`src/data_sync/`、`src/sync_rule/`。

## 修改约束

- `ServiceProxy::GetInstance`、`LoadSystemAbility`、死亡通知逻辑要保持可重入。
- IPC 调用前检查 `remote`、interface token 写入、parcel 写入和 `SendRequest` 返回值。
- callback 相关类要避免持有悬空引用，优先用已有 client manager 模式。
- lite 和 full 版本行为差异要明确；改 full 不代表 lite 自动同步。

## 测试路由

- API manager：`test/unittests/cloudsync_api/`、`test/unittests/cloud_daemon/`、`test/unittests/clouddiskservice/`。
- CloudFileKit/DataSync：`test/unittests/cloud_file_kit_inner/`、`test/unittests/data_sync_manager/`。
- ANI core 入口：`test/unittests/ani_core/`。
