# adapter Agent Notes

`adapter/` 放云能力适配示例。当前主要目录是 `cloud_adapter_example/`。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `cloud_adapter_example/BUILD.gn` | 构建示例 `cloud_adapter` shared library。 |
| `cloud_adapter_example/include/cloud_file_kit_impl.h` | `CloudFileKit` 示例实现入口。 |
| `cloud_adapter_example/include/data_sync_manager_impl.h` | DataSyncManager 示例实现。 |
| `cloud_adapter_example/include/cloud_database_impl.h` | CloudDatabase 示例实现。 |
| `cloud_adapter_example/include/cloud_assets_downloader_impl.h` | 资产下载示例实现。 |
| `cloud_adapter_example/include/cloud_sync_helper_impl.h` | CloudSyncHelper 示例实现。 |

## 修改约束

- adapter 实现依赖 `interfaces/inner_api/native/cloud_file_kit_inner/` 的抽象，不要反向依赖服务内部实现。
- 新增 CloudFileKit virtual API 时，同步补示例实现，避免构建断裂或默认行为不明确。
- 示例实现可以简单，但返回值、空指针和错误路径要可测试。
- `dfs_service_feature_enable_cloud_adapter` 默认关闭；涉及构建时确认特性开关。

## 测试路由

- 适配抽象测试：`test/unittests/cloud_file_kit_inner/`。
- DataSyncManager 使用方：`test/unittests/data_sync_manager/`。
