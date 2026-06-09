# clouddisk_database Agent Notes

本目录是【云盘数据库】模块，封装云盘 RDB、事务、通知、迁移和同步辅助。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `BUILD.gn` | 构建 `clouddisk_database` shared library。 |
| `include/clouddisk_rdbstore.h`、`src/clouddisk_rdbstore.cpp` | RDB store 主封装。 |
| `include/clouddisk_rdb_transaction.h`、`src/clouddisk_rdb_transaction.cpp` | 事务封装。 |
| `include/clouddisk_rdb_utils.h`、`src/clouddisk_rdb_utils.cpp` | RDB 工具。 |
| `include/file_column.h`、`src/file_column.cpp` | 文件表列定义和相关逻辑。 |
| `include/migration_manager.h`、`src/migration_manager.cpp` | 数据库迁移。 |
| `include/clouddisk_notify*.h`、`src/clouddisk_notify*.cpp` | 变更通知。 |
| `include/clouddisk_sync_helper.h`、`src/clouddisk_sync_helper.cpp` | 同步辅助。 |

## 修改约束

- 表结构、列名、索引和迁移必须一起考虑；不能只改查询不改迁移。
- 事务边界要清楚，失败路径要回滚并返回明确错误码。
- NativeRdb `ResultSet` 使用前后要检查空指针和返回码。
- 通知逻辑要避免重复通知和跨用户污染。
- 所有 schema/迁移/查询改动都要补 `test/unittests/clouddisk_database/`。

## 测试路由

- 主测试目录：`test/unittests/clouddisk_database/`。
- 使用方测试：`test/unittests/cloud_disk/`、`test/unittests/services_daemon/`。
