# services Agent Notes

本目录包含服务实现和服务配置。进入具体服务前继续读取子目录 `AGENTS.md`。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `cloudsyncservice/` | 【云同步】：CloudSync SA，同步控制、周期任务、同步规则、资产下载、SoftBus 传输。 |
| `cloudfiledaemon/` | 【云盘/云图】：CloudFile daemon，FUSE 挂载、云盘文件操作、账号/应用状态监听；其中 `cloud_disk` 对应云盘，`fuse_manager` 对应云图。 |
| `clouddiskservice/` | 【三方网盘】：CloudDiskService SA，同步文件夹、变更日志、磁盘事件、文件同步状态。 |
| `clouddisk_database/` | 【云盘数据库】：云盘 RDB、事务、通知、迁移。 |
| `distributedfiledaemon/` | 分布式文件 daemon，当前低优先级。 |
| `*.cfg`、`*.para` | SA 配置和参数文件。 |

## 路由

- 云同步能力：读 `cloudsyncservice/AGENTS.md`。
- 云盘/云图 daemon：读 `cloudfiledaemon/AGENTS.md`。
- 三方网盘同步文件夹和磁盘事件：读 `clouddiskservice/AGENTS.md`。
- 云盘数据库 RDB/迁移/通知：读 `clouddisk_database/AGENTS.md`。
- 分布式文件 daemon：只有任务明确提到时再进入 `distributedfiledaemon/`。

## 服务层约束

- 服务实现不要绕过 `interfaces/inner_api/native/` 和 `frameworks/native/` 的契约。
- IPC 入参要校验 token、userId、bundleName、path 和 callback 空指针。
- `OnStart`/`OnStop` 中注册的 observer、listener、timer、callback 必须有对应释放或幂等处理。
- 新增源文件同步更新当前服务的 `BUILD.gn`，并补对应 `test/unittests/*`。
- 日志遵守根文档的 `%{public}`/`%{private}` 约束。
