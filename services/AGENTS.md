# services Agent Notes

本目录包含服务实现和服务配置。进入具体服务前继续读取子目录 `AGENTS.md`。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `cloudsyncservice/` | 【云同步】：CloudSync SA，同步控制、周期任务、同步规则、文件类资产下载、SoftBus 端端辅助同步。 |
| `cloudfiledaemon/` | 【云盘/云图】：CloudFile daemon，负责与 `/dev/fuse` 驱动建链、FUSE 操作、云盘文件操作、账号/应用状态监听；其中 `cloud_disk` 对应云盘，`fuse_manager` 对应云图。 |
| `clouddiskservice/` | 【三方网盘】：CloudDiskService SA，负责文件变更监听、变更日志、磁盘事件和文件同步状态；不对外承担同步文件夹注册/同步根管理职责。 |
| `clouddisk_database/` | 【云盘数据库】：云盘 RDB、事务、通知、同步辅助和历史兼容迁移；迁移属于一次性兼容处理。 |
| `distributedfiledaemon/` | 分布式文件 daemon，当前知识库不深挖。 |
| `*.cfg`、`*.para` | SA 配置和参数文件。 |

## 路由

- 云同步能力：读 `cloudsyncservice/AGENTS.md`。
- 云盘/云图 daemon：读 `cloudfiledaemon/AGENTS.md`。
- 三方网盘变更监听、磁盘事件和文件同步状态：读 `clouddiskservice/AGENTS.md`。
- 云盘数据库 RDB/通知/同步辅助：读 `clouddisk_database/AGENTS.md`；历史迁移只作为兼容背景处理。
- 分布式文件 daemon：只有任务明确提到时再进入 `distributedfiledaemon/`。

## 服务层约束

- 服务实现不要绕过 `interfaces/inner_api/native/` 和 `frameworks/native/` 的契约。
- IPC 入参要校验 token、userId、bundleName、path 和 callback 空指针。
- `OnStart`/`OnStop` 中注册的 observer、listener、timer、callback 必须有对应释放或幂等处理。
- 新增源文件同步更新当前服务的 `BUILD.gn`，并补对应 `test/unittests/*`。
- 日志遵守根文档的 `%{public}`/`%{private}` 和热路径 buffer 限流约束。
