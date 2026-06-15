# DFS Service Agent Links

本文件是 OpenHarmony `filemanagement/dfs_service` 根仓级 Agent 规则，适用于整个仓库。进入子目录工作时，先读取该路径下更具体的 `AGENTS.md`；子级规则是根规则的补充，若有冲突，以更具体路径的规则为准。

这个仓是分布式/云文件服务实现，主要提供云文件同步、三方网盘、CloudFile FUSE daemon、云盘数据库和内部 native kit 能力。上层通过 `interfaces/inner_api/native/` 和 `frameworks/native/` 调用 `CloudSync`、`CloudFileKit`、`CloudDaemon`、`CloudDiskService` 等接口；底层由 `services/cloudsyncservice/`、`services/cloudfiledaemon/`、`services/clouddiskservice/` 和 `services/clouddisk_database/` 承担同步调度、与 `/dev/fuse` 建链、FUSE 文件访问、三方网盘文件变更监听、元数据/RDB 存储、文件类资产上传下载等核心流程。

# 代码地图

| 路径 | 作用 |
| --- | --- |
| `CODEOWNERS` | IPC interface code 变更评审规则。 |
| `interfaces/AGENTS.md`、`interfaces/inner_api/native/AGENTS.md` | 接口层局部规则。改 inner API、IPC broker、callback、Parcelable、接口码前先读。 |
| `interfaces/inner_api/native/` | 对内 C++ API、IPC broker 接口、回调、Parcelable 数据结构和接口码。 |
| `frameworks/native/AGENTS.md` | 客户端 framework 局部规则。改 manager/proxy/callback client 前先读。 |
| `frameworks/native/` | inner API 的客户端实现、SA proxy、callback stub/client、基础实现。 |
| `services/AGENTS.md` | 服务层通用规则。改任一 System Ability、daemon、RDB 服务前先读。 |
| `services/cloudsyncservice/AGENTS.md`、`services/cloudsyncservice/` | 【云同步】CloudSync System Ability，负责云同步调度、同步规则、周期任务、文件类资产下载和 SoftBus 端端辅助同步。 |
| `services/cloudfiledaemon/AGENTS.md`、`services/cloudfiledaemon/` | 【云盘/云图】CloudFile daemon/FUSE 服务，负责与 `/dev/fuse` 驱动建链、FUSE 操作、云/本地文件操作、账号和应用状态监听；其中 `cloud_disk` 对应云盘，`fuse_manager` 对应云图。 |
| `services/clouddiskservice/AGENTS.md`、`services/clouddiskservice/` | 【三方网盘】CloudDiskService SA，负责三方网盘文件变更监听、变更日志、文件同步状态和磁盘事件处理；不对外承担同步文件夹注册/同步根管理职责。 |
| `services/clouddisk_database/AGENTS.md`、`services/clouddisk_database/` | 【云盘数据库】云盘 RDB、通知、事务、同步辅助和历史兼容迁移；迁移属于一次性兼容处理，不作为常态设计主线。 |
| `services/distributedfiledaemon/` | 分布式文件 daemon，当前知识库不深挖。除非任务明确要求或当前服务改动必须触及，否则不要默认进入。 |
| `adapter/AGENTS.md`、`adapter/cloud_adapter_example/` | cloud adapter 示例实现，连接 `CloudFileKit` 抽象和外部云能力。 |
| `utils/AGENTS.md`、`utils/` | 共享日志、错误码、系统工具、dentry/metafile、插件加载、异步工作、FFRT timer 等。 |
| `dfs_utils/AGENTS.md`、`dfs_utils/` | 分布式文件相关底层 DFS 工具抽象，如 actor、thread、singleton、mount argument、radar；当前知识库不深挖，默认不改动。 |
| `test/AGENTS.md`、`test/` | 单元测试、模块测试和 mock。测试目标从 `test/unittests/BUILD.gn` 聚合。 |

## Where to look

| 任务 | 先看 |
| --- | --- |
| 理解全仓架构 | `docs/agent-knowledge/logical-view.md`，再读 `interfaces/AGENTS.md`、`frameworks/native/AGENTS.md`、`services/AGENTS.md`。 |
| 理解业务流程 | `docs/agent-knowledge/scenario-view.md`。 |
| 找编译目标和测试入口 | `docs/agent-knowledge/build-and-test.md`。 |
| 改 inner API 或 IPC | `interfaces/inner_api/native/AGENTS.md`、对应 `.idl`/接口头、framework proxy/manager、service stub/实现、mock 和测试。 |
| 改云同步调度、同步规则、文件类资产下载 | `services/cloudsyncservice/AGENTS.md`、`services/cloudsyncservice/`、相关 `frameworks/native/cloudsync_*` 和测试。 |
| 改 FUSE、`/dev/fuse` 建链、云盘/云图文件操作 | `services/cloudfiledaemon/AGENTS.md`、`services/cloudfiledaemon/`、`services/clouddisk_database/`、相关权限和 uid/gid 逻辑。 |
| 改三方网盘事件、变更日志、文件同步状态 | `services/clouddiskservice/AGENTS.md`、`services/clouddiskservice/`、相关数据库和回调测试。 |
| 改 RDB 表、通知、事务或迁移 | `services/clouddisk_database/AGENTS.md`、现有 schema/migration/compat 代码和数据库单测。 |
| 改日志、错误码、权限工具、异步/timer 公共能力 | `utils/AGENTS.md`、`utils/` 下同类实现和测试。 |

# 知识路由

动手前按任务类型、路径和术语选择要读的上下文。计划或首条实现说明里应能说清：任务类别、已读文档/局部 `AGENTS.md`、关键边界、准备验证的目标。

## Task-based routing

| 任务类型 | 风险提示 | 必读 |
| --- | --- | --- |
| 架构理解或跨模块改动 | 容易绕过接口层或反向依赖服务实现。 | `docs/agent-knowledge/logical-view.md`、相关子目录 `AGENTS.md`。 |
| 业务流程改动 | 同步、下载、FUSE、数据库通知之间有顺序和生命周期关系。 | `docs/agent-knowledge/scenario-view.md`、对应服务目录 `AGENTS.md`。 |
| API/IPC/Parcelable/接口码变更 | 影响客户端、服务 stub、mock、兼容性和 CODEOWNERS 评审。 | `interfaces/inner_api/native/AGENTS.md`、`CODEOWNERS`、相关 framework/service/test。 |
| 权限、token、bundleName、userId、uid/gid 变更 | 不能绕过 access token、多用户隔离和本地资源归属。 | 同目录现有 `*access_token*`、`dfsu_access_token_helper`、相关服务目录规则。 |
| 异步、FFRT、timer、callback map 变更 | 要处理取消、重复注册、死亡通知、多用户隔离和 SA 退出。 | 同目录既有任务管理、`TaskStateManager` 或 callback 生命周期实现。 |
| 数据库/RDB/schema/迁移变更 | 持久化格式和历史兼容不可破坏，迁移不是常态设计主线。 | `services/clouddisk_database/AGENTS.md`、现有 migration/schema/transaction 测试。 |
| 构建、依赖或特性开关变更 | `distributedfile.gni`、BUILD 条件和 so 边界必须一致。 | `docs/agent-knowledge/build-and-test.md`、相关 `BUILD.gn`、`distributedfile.gni`。 |
| 文档或 AGENTS.md 变更 | 要保持链接可达、路由可执行，不把知识全文塞进根文件。 | 本文件、被引用的 docs/子级 `AGENTS.md`。 |

## Path-based routing

| 修改路径 | 同时读取 |
| --- | --- |
| `interfaces/**` | `interfaces/AGENTS.md`；若在 `interfaces/inner_api/native/**`，再读 `interfaces/inner_api/native/AGENTS.md`。 |
| `frameworks/native/**` | `frameworks/native/AGENTS.md` 和对应接口层定义。 |
| `services/**` | `services/AGENTS.md` 和具体服务目录 `AGENTS.md`。 |
| `services/cloudsyncservice/**` | `services/cloudsyncservice/AGENTS.md`、同步场景文档、相关 cloudsync framework/test。 |
| `services/cloudfiledaemon/**` | `services/cloudfiledaemon/AGENTS.md`、FUSE/云盘数据库相关实现、权限和 uid/gid 逻辑。 |
| `services/clouddiskservice/**` | `services/clouddiskservice/AGENTS.md`、变更监听、回调和文件同步状态测试。 |
| `services/clouddisk_database/**` | `services/clouddisk_database/AGENTS.md`、schema/migration/transaction/notify 测试。 |
| `utils/**` | `utils/AGENTS.md` 和同类工具实现。 |
| `adapter/**` | `adapter/AGENTS.md` 和 `CloudFileKit` 抽象契约。 |
| `test/**` | `test/AGENTS.md`、被测模块的局部 `AGENTS.md`、`test/unittests/BUILD.gn`。 |
| `dfs_utils/**`、`services/distributedfiledaemon/**` | 仅在任务明确要求或必须触及时读取对应规则并改动。 |

## Vocabulary-based routing

| 术语 | 风险提示 | 先读 |
| --- | --- | --- |
| `SoftBus`、端端辅助同步 | 这里只是云同步辅助能力，不要泛化成通用通信层。 | `services/cloudsyncservice/AGENTS.md`、`docs/agent-knowledge/logical-view.md`。 |
| `FUSE`、`/dev/fuse`、`cloud_disk`、`fuse_manager` | 牵涉驱动建链、文件访问、账号/应用状态和本地权限。 | `services/cloudfiledaemon/AGENTS.md`。 |
| `CloudDiskService`、三方网盘、变更日志 | 负责变更监听和同步状态，不承担同步根/同步文件夹注册职责。 | `services/clouddiskservice/AGENTS.md`。 |
| `RDB`、`NativeRdb`、schema、migration、notify | 影响持久化兼容和通知事务边界。 | `services/clouddisk_database/AGENTS.md`。 |
| `SA`、`System Ability`、`OnStart`、`OnStop` | 生命周期、发布、监听注册和资源释放要清晰分离。 | `services/AGENTS.md` 和对应服务实现。 |
| `Parcelable`、`proxy`、`stub`、`interface code` | 需要同步客户端、服务端、mock 和测试，读写必须逐项检查返回值。 | `interfaces/inner_api/native/AGENTS.md`、`CODEOWNERS`。 |
| `AccessToken`、`bundleName`、`userId`、`token` | 权限和多用户边界不可绕过。 | 同目录现有 `*access_token*`、`dfsu_access_token_helper`。 |
| `TaskStateManager`、FFRT、timer、callback、death recipient | 要考虑取消、失败、重复注册、死亡通知和非常驻 SA 退出。 | 同目录任务管理和 callback 生命周期实现。 |
| `dfsu_fd_guard`、`dfsu_memory_guard` | C++ 资源要 RAII，不让裸 fd/裸内存跨函数漂移。 | `utils/AGENTS.md`、现有 guard 用法。 |
| `user_data_rw`、uid/gid、chown | 服务内本地落盘资源归属不能混淆。 | `services/cloudfiledaemon/AGENTS.md` 和现有权限处理。 |

# 约束边界

## 架构约束

- 不要让上层直接依赖服务实现。新能力通常先补 `interfaces/inner_api/native/`，再补 `frameworks/native/`，最后落到 `services/`。
- 不要手改 IDL 生成产物。需要变更 IPC 时，改 `.idl` 或显式维护的接口码/接口头，并同步 proxy/stub/manager、服务实现和测试。
- `services/distributedfiledaemon/` 和 `dfs_utils/` 不作为默认改动范围。除非任务明确要求或当前服务改动必须触及，否则不要为新知识库或新功能深入整理/重构它们。
- 特性开关集中在 `distributedfile.gni`。受开关保护的功能要在 BUILD 和代码里保持条件一致。
- 服务层要保持 SA 生命周期清晰：`OnStart`、`OnStop`、发布 SA、注册监听、释放资源不要互相穿插成隐式副作用。

## Clean Code 规范

- 变更范围要小：接口、实现、BUILD、测试成组修改，不做顺手大重构。
- 头文件只暴露稳定契约；实现细节放到 `.cpp` 或服务内部 include。
- 保持既有命名空间：常见为 `OHOS::FileManagement::CloudSync`、`OHOS::FileManagement::CloudFile`、`OHOS::FileManagement::CloudDiskService`。
- C++ 资源使用 RAII。优先使用仓内已有 guard，如 `dfsu_fd_guard`、`dfsu_memory_guard`，不要裸 fd/裸内存跨函数漂移。
- IPC/Parcelable 读写必须逐项检查返回值，失败要返回明确错误码并打日志。
- 日志使用仓内宏和 `LOG_DOMAIN`/`LOG_TAG`。敏感数据用 `%{private}`，可公开的状态和错误码才用 `%{public}`；热路径日志必须考虑 buffer 限流和条件保护，避免日志打印超限。
- 错误码优先复用 `utils/log/include/dfs_error.h`、`utils/clouddiskservice/include/cloud_disk_service_error.h` 等已有定义。
- 路径、bundleName、userId、token、权限检查不要绕过；涉及 access token 的逻辑要读同目录现有 `*access_token*` 或 `dfsu_access_token_helper`。
- 异步、FFRT、timer、callback map 变更必须考虑生命周期、取消、重复注册、死亡通知和多用户隔离。
- 新增长时任务要纳入 `TaskStateManager` 或同目录既有任务管理，并成对处理开始、完成、失败和取消，避免非常驻 SA 优雅退出打断任务。
- 服务内本地落盘资源要明确 uid/gid 归属：云盘文件类资产归属各应用，云图媒体类资产归属 `user_data_rw`，改动前先核对现有目录权限和 chown 逻辑。
- 对外接口若能在客户端执行且无安全风险，优先考虑落到客户端侧，避免服务侧承担不必要逻辑。
- 新增依赖、缓存或常驻数据结构必须评估内存影响；BUILD 变更要避免重复编译同一 `.cpp`，so 构建边界保持独立。
- 非常驻服务若引入系统级强制停止设计，必须同步设计系统级自动恢复路径。
- 新增文件必须加入对应 `BUILD.gn`，测试目标也要同步。
- 格式遵守根目录 `.clang-format`。不要混入无关格式化。

## Do not

- 不要为通过测试删除或削弱日志、错误码、事件、诊断信息和权限检查。
- 不要把三方网盘职责扩展成同步文件夹注册或同步根管理，除非任务明确要求并同步接口设计。
- 不要把历史兼容迁移当作常态设计主线；新增持久化逻辑应优先走当前 schema/事务/通知模型。
- 不要在未核对 `BUILD.gn` 和特性开关的情况下新增文件、依赖或编译单元。
- 不要默认修改 `dfs_utils/` 或 `services/distributedfiledaemon/` 来支撑云同步/云盘需求。

## Ask before

以下动作必须先向维护者确认设计或评审路径：

- 修改 public/inner API 签名、错误码语义、生命周期语义或兼容行为。
- 新增、删除或重排 CODEOWNERS 覆盖的接口码。
- 改动权限模型、token 校验、多用户隔离、uid/gid 归属或本地目录权限。
- 改动 RDB schema、持久化格式、历史数据迁移策略或跨版本兼容路径。
- 引入新的第三方依赖、常驻缓存、系统级强制停止机制或自动恢复机制。
- 执行会影响真实设备、真实用户数据或系统挂载状态的破坏性调试操作。

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

# 验证闭环

本仓是 OpenHarmony 子仓，通常需要在 OpenHarmony 源码根目录执行构建。开始验证前先读 `docs/agent-knowledge/build-and-test.md`，确认产品名、GN target 和本次改动范围。

## Minimum checks

| 改动类型 | 最小验证 |
| --- | --- |
| 文档或 AGENTS.md | `git diff --check`；确认新增链接和子级 `AGENTS.md` 路径存在。 |
| BUILD 或特性开关 | `git diff --check`；检查 `distributedfile.gni`、相关 `BUILD.gn` 条件和 target 边界一致。 |
| C++ 实现 | `git diff --check`；在 OpenHarmony 根目录执行对应 `./build.sh --product-name <product> --build-target <target>`。 |
| 测试改动 | 运行被改测试所属 target，并确认 mock 与接口/服务实现同步。 |

常用构建目标：

```bash
./build.sh --product-name <product> --build-target services_target
./build.sh --product-name <product> --build-target cloudsync_test_module
./build.sh --product-name <product> --build-target distributedfile_test_module
```

## Task-specific checks

| 任务 | 验证重点 |
| --- | --- |
| API/IPC/Parcelable 变更 | 构建对应 inner kit target；运行相关 proxy/stub/Parcelable/mock 单测；检查读写失败路径返回码和日志。 |
| `services/cloudsyncservice/` 变更 | 构建 `services_target`；优先运行 `cloudsync_test_module` 或 `test/unittests/BUILD.gn` 中 cloudsync 相关测试。 |
| `services/cloudfiledaemon/` 变更 | 构建 `services_target`；覆盖 FUSE、云盘/云图、账号/应用状态、uid/gid 权限相关测试或说明无法本地验证的设备条件。 |
| `services/clouddiskservice/` 变更 | 构建 `services_target`；覆盖三方网盘变更监听、文件同步状态、callback/death recipient 测试。 |
| `services/clouddisk_database/` 变更 | 运行数据库相关单测；覆盖 schema、事务、通知、迁移兼容和异常回滚路径。 |
| 权限/AccessToken 变更 | 覆盖允许、拒绝、多用户、bundleName/userId/token 不匹配路径。 |
| 异步/timer/callback 变更 | 覆盖开始、完成、失败、取消、重复注册、死亡通知和 SA 退出路径。 |

## Done definition

只有同时满足以下条件，才能把任务报告为完成：

- 已按知识路由读取对应 docs 和局部 `AGENTS.md`，并遵守发现的边界。
- 代码、BUILD、测试和文档的改动范围一致，没有遗漏新增文件或测试目标。
- 已运行与改动风险匹配的最小验证；若因环境缺失无法运行，说明缺失条件、未验证风险和建议命令。
- 最终回复列出改动文件、验证命令和结果；不要只说“已完成”。
