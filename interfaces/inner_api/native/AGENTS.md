# inner_api/native Agent Notes

本目录定义 DFS service 的内部 native API。它是 agent 理解业务入口的第一站。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `cloudsync_kit_inner/` | CloudSync manager、asset manager、sync callback、upload/download/downgrade callback、接口码。 |
| `cloud_daemon_kit_inner/` | Cloud daemon manager、daemon IPC broker、接口码。 |
| `cloud_file_kit_inner/` | CloudFileKit 抽象、DataSyncManager、CloudDatabase、CloudAssetsDownloader、sync rule、状态管理、统计。 |
| `clouddiskservice_kit_inner/` | CloudDiskService manager、callback、Parcelable common types。 |

## 路由

- 云同步控制：`cloudsync_kit_inner/cloud_sync_manager.h`。
- 云同步资产：`cloudsync_kit_inner/cloud_sync_asset_manager.h`。
- 云 daemon/FUSE：`cloud_daemon_kit_inner/cloud_daemon_manager.h`、`i_cloud_daemon.h`。
- 云适配抽象：`cloud_file_kit_inner/cloud_file_kit.h`、`cloud_database.h`、`cloud_assets_downloader.h`、`cloud_sync_helper.h`。
- 同步状态/规则：`cloud_file_kit_inner/data_sync_manager.h`、`sync_state_manager.h`、`network_status.h`、`battery_status.h`、`system_load.h`。
- 云盘同步文件夹：`clouddiskservice_kit_inner/cloud_disk_service_manager.h`、`cloud_disk_common.h`。

## 修改约束

- 头文件是契约，不要塞服务实现逻辑。
- virtual API 新增默认实现时要评估 adapter 示例和测试 mock 是否需要覆盖。
- 新 IPC callback 要同时补 `I*Callback` broker、stub/client、manager 注册和死亡处理。
- `cloud_file_sync_service_interface_code.h` 和 `cloud_file_daemon_interface_code.h` 属于敏感接口码，改动要同步 CODEOWNERS 评审。
- 任何 API 字段变更都要补 framework 和 service 的 parcel 读写测试。
