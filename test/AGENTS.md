# test Agent Notes

`test/` 包含单元测试、模块测试和 mock。代码改动应优先补同模块已有测试。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `unittests/BUILD.gn` | 聚合 `cloudsyncunittests` 和 `distributedfileunittests`。 |
| `unittests/ani_core/` | ANI 层核心测试。 |
| `unittests/cloudsync_api/` | CloudSync API/manager 测试。 |
| `unittests/cloudsync_sa/` | CloudSync SA、transport、SoftBus 测试。 |
| `unittests/cloud_file_kit_inner/` | CloudFileKit、DataSync、状态管理测试。 |
| `unittests/cloud_daemon/` | CloudDaemon manager/proxy 测试。 |
| `unittests/services_daemon/` | cloudfiledaemon/FUSE manager 相关测试。 |
| `unittests/cloud_disk/` | FUSE file operations、database manager、meta file 等测试。 |
| `unittests/clouddisk_database/` | 云盘 RDB/迁移/通知测试。 |
| `unittests/clouddiskservice/` | CloudDiskService 旧测试入口。 |
| `unittests/clouddisk_service/` | CloudDiskService 新同步文件夹相关测试。 |
| `unittests/utils/` | 公共工具测试。 |
| `mock/` | 跨模块通用 mock。 |
| `moduletest/` | 模块测试。 |

## 测试选择

- 改 `services/cloudsyncservice/`：优先跑 `cloudsync_sa`，必要时加 `cloudsync_api`、`data_sync_manager`。
- 改 `services/cloudfiledaemon/`：优先跑 `services_daemon`、`cloud_disk`、`cloud_daemon`。
- 改 `services/clouddiskservice/`：优先跑 `clouddiskservice`、`clouddisk_service`。
- 改 `services/clouddisk_database/`：跑 `clouddisk_database`，再跑相关使用方。
- 改 `utils/`：跑 `utils`，再跑受影响服务测试。
- 改接口/Parcelable：同时跑接口层测试、服务测试和 callback/mock 相关测试。

## 修改约束

- 新测试沿用同目录 `HWTEST_F` 风格、mock 组织和 `BUILD.gn` 目标。
- mock 只覆盖外部依赖和边界，不要把被测逻辑复制到 mock 里。
- 测试名保持现有编号风格；新增失败路径测试要明确错误码和日志前置条件。
- Windows 本地只做静态文档/搜索校验；OpenHarmony 构建测试在源码根目录用 GN/build.sh 目标执行。
