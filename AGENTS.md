# DFS Service Agent Links


这个仓是 OpenHarmony filemanagement/dfs_service 的分布式/云文件服务实现，主要提供云文件同步、三方网盘、CloudFile FUSE daemon、云盘数据库和内部 native kit 能力：上层通过 interfaces/inner_api/native 和 frameworks/native 调用 CloudSync、CloudFileKit、CloudDaemon、CloudDiskService 等接口，底层由 services/cloudsyncservice、services/cloudfiledaemon、services/clouddiskservice 和 services/clouddisk_database 承担同步调度、文件挂载访问、三方网盘同步根管理及变更监听、元数据/RDB 存储、资产上传下载以及跨设备传输等核心流程。

# 代码地图

| 路径 | 作用 |
| --- | --- |
| `CODEOWNERS` | IPC interface code 变更评审规则。 |
| `interfaces/inner_api/native/` | 对内 C++ API、IPC broker 接口、回调、Parcelable 数据结构和接口码。 |
| `frameworks/native/` | inner API 的客户端实现、SA proxy、callback stub/client、基础实现。 |
| `services/cloudsyncservice/` | 【云同步】：CloudSync System Ability，负责云同步调度、同步规则、周期任务、资产下载和 SoftBus 传输。 |
| `services/cloudfiledaemon/` | 【云盘/云图】：CloudFile daemon/FUSE 服务，负责挂载、FUSE 操作、云/本地文件操作、账号和应用状态监听；其中 `cloud_disk` 对应云盘，`fuse_manager` 对应云图。 |
| `services/clouddiskservice/` | 【三方网盘】：CloudDiskService SA，负责同步文件夹注册、变更监听、文件同步状态和磁盘事件处理。 |
| `services/clouddisk_database/` | 【云盘数据库】：云盘 RDB、通知、事务、迁移和同步辅助。 |
| `services/distributedfiledaemon/` | 分布式文件 daemon，当前知识库不深挖。 |
| `adapter/cloud_adapter_example/` | cloud adapter 示例实现，连接 `CloudFileKit` 抽象和外部云能力。 |
| `utils/` | 共享日志、错误码、系统工具、dentry/metafile、插件加载、异步工作、FFRT timer 等。 |
| `dfs_utils/` | 较底层的 DFS 工具抽象，如 actor、thread、singleton、mount argument、radar。 |
| `test/` | 单元测试、模块测试和 mock。测试目标从 `test/unittests/BUILD.gn` 聚合。 |


# DFS Service 知识路由

| 遇到的问题 | 先读什么 |
| --- | --- |
| 想理解全仓架构 | `docs/agent-knowledge/logical-view.md`，再读 `interfaces/AGENTS.md`、`frameworks/AGENTS.md`、`services/AGENTS.md`。 |
| 想理解业务流程 | `docs/agent-knowledge/scenario-view.md`。 |
| 周边依赖 | `docs/agent-knowledge/peripheral_dependency.md`，找上下游依赖关系。 |
| 编译相关 | `docs/agent-knowledge/build-and-test.md`，找编译目标和构建命令。 |

# 开发约束


## 架构约束

- 不要让上层直接依赖服务实现。新能力通常先补 `interfaces/inner_api/native/`，再补 `frameworks/native/`，最后落到 `services/`。
- 不要手改 IDL 生成产物。需要变更 IPC 时，改 `.idl` 或显式维护的接口码/接口头，并同步 proxy/stub/manager、服务实现和测试。
- `services/distributedfiledaemon/` 不作为默认改动范围。除非任务明确要求，否则不要为新知识库或新功能深入整理/重构它。
- 特性开关集中在 `distributedfile.gni`。受开关保护的功能要在 BUILD 和代码里保持条件一致。
- 服务层要保持 SA 生命周期清晰：`OnStart`、`OnStop`、发布 SA、注册监听、释放资源不要互相穿插成隐式副作用。

## Clean Code 规范

- 变更范围要小：接口、实现、BUILD、测试成组修改，不做顺手大重构。
- 头文件只暴露稳定契约；实现细节放到 `.cpp` 或服务内部 include。
- 保持既有命名空间：常见为 `OHOS::FileManagement::CloudSync`、`OHOS::FileManagement::CloudFile`、`OHOS::FileManagement::CloudDiskService`。
- C++ 资源使用 RAII。优先使用仓内已有 guard，如 `dfsu_fd_guard`、`dfsu_memory_guard`，不要裸 fd/裸内存跨函数漂移。
- IPC/Parcelable 读写必须逐项检查返回值，失败要返回明确错误码并打日志。
- 日志使用仓内宏和 `LOG_DOMAIN`/`LOG_TAG`。敏感数据用 `%{private}`，可公开的状态和错误码才用 `%{public}`。
- 错误码优先复用 `utils/log/include/dfs_error.h`、`utils/clouddiskservice/include/cloud_disk_service_error.h` 等已有定义。
- 路径、bundleName、userId、token、权限检查不要绕过；涉及 access token 的逻辑要读同目录现有 `*access_token*` 或 `dfsu_access_token_helper`。
- 异步、FFRT、timer、callback map 变更必须考虑生命周期、取消、重复注册、死亡通知和多用户隔离。
- 新增文件必须加入对应 `BUILD.gn`，测试目标也要同步。
- 格式遵守根目录 `.clang-format`。不要混入无关格式化。

## IPC 和接口码约束

`CODEOWNERS` 标明以下接口码变更需要额外评审：

- `services/distributedfiledaemon/include/ipc/distributed_file_daemon_ipc_interface_code.h`
- `interfaces/inner_api/native/cloud_daemon_kit_inner/cloud_file_daemon_interface_code.h`
- `interfaces/inner_api/native/cloudsync_kit_inner/cloud_file_sync_service_interface_code.h`

改接口码时同步检查：

- 客户端 manager/proxy。
- 服务 stub/实现。
- callback 类型和死亡通知。
- 对应单元测试和 mock。
