# clouddisk_database Agent Notes

本目录是【云盘数据库】模块，封装云盘 RDB、事务、通知和同步辅助；迁移只作为历史一次性兼容处理，不作为常态业务主线。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `BUILD.gn` | 构建 `clouddisk_database` shared library。 |
| `include/clouddisk_rdbstore.h`、`src/clouddisk_rdbstore.cpp` | RDB store 主封装。 |
| `include/clouddisk_rdb_transaction.h`、`src/clouddisk_rdb_transaction.cpp` | 事务封装。 |
| `include/clouddisk_rdb_utils.h`、`src/clouddisk_rdb_utils.cpp` | RDB 工具。 |
| `include/file_column.h`、`src/file_column.cpp` | 文件表列定义和相关逻辑。 |
| `include/migration_manager.h`、`src/migration_manager.cpp` | 历史数据库兼容迁移，属于一次性兼容产物。 |
| `include/clouddisk_notify*.h`、`src/clouddisk_notify*.cpp` | 变更通知。 |
| `include/clouddisk_sync_helper.h`、`src/clouddisk_sync_helper.cpp` | 同步辅助。 |

## 修改约束

- 表结构、列名、索引要按迁移后的数据库模型考虑；只有触及历史兼容路径时才同步检查一次性迁移逻辑，不能只改查询不改兼容处理。
- 事务边界要清楚，失败路径要回滚并返回明确错误码。
- NativeRdb `ResultSet` 使用前后要检查空指针和返回码。
- 通知逻辑要避免重复通知和跨用户污染。
- 所有 schema/查询改动都要补 `test/unittests/clouddisk_database/`；触及历史迁移时再补迁移兼容测试。

## 测试路由

- 主测试目录：`test/unittests/clouddisk_database/`。
- 使用方测试：`test/unittests/cloud_disk/`、`test/unittests/services_daemon/`。
