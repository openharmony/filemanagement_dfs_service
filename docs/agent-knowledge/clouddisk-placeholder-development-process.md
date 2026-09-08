# CloudDisk 占位符 v2～v8 增量开发流程

> 状态：流程已确认。下一步按本文档执行 callback table 基线核对、实现、验证和首次推送。

## 1. 目的与范围

本文档规定 CloudDisk 占位符 v2～v8 的开发、验证、提交和推送流程。它不替代功能规格或 SDD：

- 功能语义以 `docs/agent-knowledge/clouddisk-placeholder-spec.md` 为最终依据。
- 每个版本的实现范围以该版本对应的 SDD 为依据。
- 维护者提供的历史 PR/提交只作为实现参考；与最终 spec 不一致时，以 spec 为准。
- NDK 实现优先参考 interface_sdk_c PR #6149；callback table 基线 PR #4353 中的 NDK 命名不作为依据。
- 本文档负责约束实施顺序、质量门禁、提交粒度和远端协作方式。
- 当前 spec 与各版本 SDD 已完成维护者设计 sign-off，可作为正式实现依据。

## 2. 总体原则

1. 先导入 callback table 注册/解注册实现，形成开发基线，再开始 v2～v8。
2. 严格按版本顺序串行开发，不并行堆叠多个未验证版本。
3. 一个 version 对应一个功能增量和一个主 SDD；每个版本验证通过后 amend 到同一个开发提交。
4. 开发优先保证功能正确，不强制采用 TDD。
5. 如需新增测试，只允许放在 `test/unittests/clouddiskservice/` 下。
6. 禁止在 `test/unittests/clouddisk_service/` 下新增测试用例。
7. 每个版本必须通过四组固定编译验证，才能提交和推送。
8. 每次只暂存当前版本涉及的文件，不夹带无关修改或未跟踪文档。
9. dfs_service 最终只创建一个 PR；各阶段使用 `commit --amend` 更新同一个开发提交。
10. amend 后如需更新远端，只使用 `push --force-with-lease`，禁止使用裸 `force push`。
11. 流程核对完成前，不读取参考 PR 代码、不导入基线、不修改业务代码。
12. v8 分两个检查点执行：Execute 验证并提交后，才开始 ArkTS API；两部分不得一次性混合开发。
13. 本轮 ArkTS API 只修改 dfs_service 与 user_file_service，不修改 interface_sdk-js 仓。
14. 最终提交只包含功能实现所需的代码和构建修改；流程、spec、版本和 SDD 文档均不纳入 PR。
15. spec/SDD 已明确的 API、错误码、持久化、权限和生命周期方案不再逐项暂停请示。
16. dfs_service 唯一提交从基线阶段起使用覆盖整个需求的固定总标题，后续 amend 不更换标题。
17. 每个提交/amend 检查点都在 `docs/diff/` 保存增量代码 diff，并更新功能点索引；该目录不纳入 PR。

## 3. 依据与冲突优先级

发生实现差异时，按以下顺序裁决：

1. 最新的 `clouddisk-placeholder-spec.md`。
2. 当前版本对应的 SDD；若 SDD 与 spec 冲突，以 spec 为准并记录差异。
3. 本文档中经维护者确认的流程和范围说明。
4. NDK 部分参考 interface_sdk_c PR #6149。
5. 维护者提供的其他参考 PR/提交。
6. 当前本地代码和历史实现。

发现以下情况时，暂停当前版本开发并先完成核对：

- spec 与 SDD 存在无法通过最终语义消解的冲突。
- 参考提交修改了 spec 未授权的 API、错误码、权限或持久化行为。
- 当前代码基线与 SDD 的前置依赖不一致。
- 需要跨仓修改，但对应仓库、分支或推送目标尚未准备好。

仓库 `AGENTS.md` 中的 ask-before 项，如已被当前 spec/SDD 明确覆盖，视为本需求已取得维护者确认。
只有设计冲突、设计缺失或实现必须超出 spec/SDD 时才重新暂停请示。

## 4. 阶段 0：建立 callback table 基线

### 4.1 输入

- callback table 注册/解注册基线 PR：
  `https://gitcode.com/openharmony/filemanagement_dfs_service/pull/4353`。
- NDK 实现参考 PR：
  `https://gitcode.com/openharmony/interface_sdk_c/pull/6149`。
- PR 的准确提交或 patch 范围。
- 各版本可供参考的历史 PR/提交链接。

基线采用规则：

- PR #4353 只要求保留 callback table 的完整功能链路。
- PR #4353 中的测试可根据当前开发需要删减，不要求原样带入。
- PR #4353 存在 NDK 命名问题，其 NDK 命名和签名不得直接照搬。
- NDK 层参考 PR #6149 的代码形态，但最终名称、结构体和签名仍以当前 spec 为准。

已知参考 PR：

- placeholder state 重构参考：
  `https://gitcode.com/openharmony/filemanagement_dfs_service/pull/4354`。
- ArkTS API 的 user_file_service 参考：
  `https://gitcode.com/openharmony/filemanagement_user_file_service/pull/1227`。
- ArkTS API 的 dfs_service 参考：
  `https://gitcode.com/openharmony/filemanagement_dfs_service/pull/4358`。
- 水合、脱水和 callback table 框架参考：
  `https://gitcode.com/openharmony/filemanagement_dfs_service/pull/4245`、
  `https://gitcode.com/openharmony/filemanagement_dfs_service/pull/4217`。

参考限制：

- PR #4245 与 #4217 重复度高，且设计相对当前 spec 滞后，只参考分层、IPC、callback 和任务框架。
- 不从 PR #4245/#4217 照搬状态机、取消语义、Execute 形状、优先级或其他已被当前 spec 修订的行为。
- 上述 PR 不覆盖全部版本；缺少参考 PR 不构成阻塞，直接按最终 spec、对应 SDD 和本地代码模式实现。

### 4.2 只读核对

1. 流程文档确认后，先只检查基线 PR #4353 和 NDK 参考 PR #6149 的提交、文件范围和依赖。
2. 核对 callback table 的 NDK、inner API、framework、IDL、service、死亡通知和测试接线。
3. 单独核对 NDK：以 PR #6149 为实现参考，以最终 spec 为命名和接口契约依据。
4. 对照最终 spec，列出可直接复用、需要修正和不能带入的内容。
5. 对 PR #4353 的测试逐项判断保留、改写或删除；测试删减不得影响功能代码完整性判断。
6. 记录基线起点 commit、callback table 基线 commit 和最终开发分支。
7. 其余参考 PR 不在基线阶段提前读取，进入对应 version 的规格核对步骤时再读取。

### 4.3 导入与基线验证

1. 以当前本地 `HEAD` 为起点创建开发分支 `codex/clouddisk-placeholder-v2-v8`，解决当前 detached HEAD。
2. 以核对后的方式导入 callback table 修改，不盲目合入 PR 中的无关内容。
3. 执行第 7 节四组固定编译命令。
4. 四组均通过后，使用 `git commit --signoff` 创建本需求的第一个提交。
5. 将 callback table 基线提交推送到维护者 fork 的开发分支，并记录 commit hash。
6. 首次推送成功后立即暂停任务并通知维护者，不由开发代理创建 PR。
7. 维护者在 GitCode 创建从个人 fork 到 OpenHarmony 官方仓的 PR，并回传 PR 链接。
8. 收到 PR 链接和继续指令前不开始 v2。
9. v2～v8 每个版本验证通过后，继续 amend 这一个提交并更新同一远端分支，持续更新该 PR。

## 5. 版本与 SDD 路由

| 阶段 | 功能增量 | 主 SDD | 备注 |
| --- | --- | --- | --- |
| 基线 | callback table 注册/解注册 | dfs_service PR #4353；NDK 参考 interface_sdk_c PR #6149 | 必须先于 v2～v8 |
| v2 | placeholder 状态位重构与目录聚合 | `docs/sdd/placeholder-marking-refactor-sdd.md` | 涉及持久化格式和 dentry reserved 字节评审 |
| v3 | customInfo | `docs/sdd/placeholder-custom-info-sdd.md` | spec 高于历史参考提交 |
| v4 | Mark/Unmark state-only 转换 | `docs/sdd/placeholder-state-only-conversion-sdd.md` | 一个版本内完成完整功能 |
| v5 | Dehydrate | `docs/sdd/placeholder-dehydrate-sdd.md` | 依赖 callback table 基线和 v2～v4 |
| v6 | Hydrate priority | `docs/sdd/placeholder-hydrate-priority-sdd.md` | 必须保证该增量独立可编译 |
| v7 | Hydrate 主体 | `docs/sdd/placeholder-hydrate-main-sdd.md` | 异步任务、FFRT 队列、取消与护栏 |
| v8-A | Execute 最终实现 | `docs/sdd/placeholder-execute-sdd.md` | 编译通过并 amend/push 后才进入 v8-B |
| v8-B | 文件管理器 ArkTS API | `docs/sdd/placeholder-arkts-api-sdd.md` | 涉及 dfs 与 user_file_service，分仓提交 |

v8 作为明确例外，按两个功能增量和两份 SDD 串行执行：

1. 先实现 Execute，完成编译验证后 amend dfs_service 的唯一开发提交并推送。
2. 再实现 ArkTS API，完成编译验证后分别更新 dfs_service 和 user_file_service 的开发提交并推送。
3. Execute 未成功编译、提交和推送前，不开始 ArkTS API。
4. interface_sdk-js 中的 ArkTS 类型定义视为外部既有前置，本轮不对该仓创建分支、提交或 PR。
5. user_file_service 在本轮只有一笔 ArkTS API 改动；创建带 `--signoff` 的提交并推送后，该仓流程结束。

### 5.1 参考 PR 路由

| 阶段 | 参考 PR | 使用边界 |
| --- | --- | --- |
| 基线 | dfs_service #4353；interface_sdk_c #6149 | #4353 取 callback table 功能；NDK 命名与实现优先参考 #6149，最终服从 spec |
| v2 | dfs_service #4354 | 参考 placeholder state 重构；最终存储和聚合语义服从 spec/SDD |
| v3～v6 | 无完整参考 PR | 不等待补充，按 spec/SDD 实现；#4245/#4217 仅可借鉴框架 |
| v7～v8-A | dfs_service #4245、#4217 | 只参考水合/脱水/callback table 框架，不继承滞后设计 |
| v8-B | dfs_service #4358；user_file_service #1227 | 参考 ArkTS 跨仓接线，最终 API 与进度语义服从 spec/SDD |

## 6. 单版本执行循环

每个版本都完整执行以下步骤，前一版本未完成不得进入后一版本。

### 6.1 规格核对

1. 完整读取最终 spec 中该功能相关章节。
2. 完整读取该版本主 SDD。
3. 核对 changelog 中该版本的前置依赖和相对前版变化。
4. 检查该路径下适用的 `AGENTS.md`。
5. 只阅读当前 version 对应的参考提交，输出与 spec 的差异清单；不提前读取后续版本 PR。

NDK 层额外执行以下规则：

- 不沿用 PR #4353 中与 spec 不一致的 NDK 命名。
- 参考 PR #6149 的 NDK 代码组织、参数校验和桥接方式。
- PR #6149 与当前 spec 不一致时，仍以当前 spec 为准。

### 6.2 代码事实基线

按 NDK → inner API → framework → IDL → service → BUILD → test 顺序核对：

- 已存在能力。
- 缺失接线。
- 需要新增或修改的文件。
- API/ABI、错误码、Parcelable、权限、持久化和生命周期影响。
- 本版本可独立编译的边界。

核对完成后形成版本实施清单。spec/SDD 已覆盖的 ask-before 项直接按已确认设计执行；仅对未覆盖、
冲突或超范围事项重新取得维护者确认。

### 6.3 功能实现

1. 只修改当前版本范围内的代码。
2. 不提前实现后一版本语义。
3. 不为了复用参考提交而保留与最终 spec 冲突的旧行为。
4. 不做无关重构和全仓格式化。
5. 新增文件时同步维护对应 `BUILD.gn`。

### 6.4 测试策略

- 不要求先写测试再实现。
- 优先完成并自审功能代码。
- 根据风险补充必要测试，重点覆盖接口接线、状态转换、错误码和失败路径。
- 所有新增测试必须位于 `test/unittests/clouddiskservice/`。
- 不得在 `test/unittests/clouddisk_service/` 新增测试用例。
- callback table 基线 PR #4353 的既有测试允许删减；不以原样保留其测试作为基线完成条件。

### 6.5 提交前检查

1. 运行 `git diff --check`。
2. 检查 `git status --short`，确认没有夹带其他版本或用户文件。
3. 审核完整 diff，检查 spec/SDD 对齐、日志脱敏、错误码、资源释放和 BUILD 接线。
4. 执行第 7 节全部编译验证。
5. 任一命令失败时，只修复当前版本；修复后重新执行本阶段要求的全部编译命令。v8-B 阶段除 DFS 四组命令外，还必须重新执行 `user_file_service` target。

### 6.6 提交与报告

1. 使用显式文件列表暂存，只包含当前版本功能。
   允许范围包括源码、头文件、IDL、BUILD 及必要测试代码，不包括本地需求/流程文档。
2. 检查 staged diff。
3. 首个阶段使用 `git commit --signoff` 创建开发提交；后续版本均使用带 `--signoff` 的
   `git commit --amend` 更新该提交。
   commit message 从首次提交起使用整个 CloudDisk 占位符需求的总标题，后续 amend 保持不变。
4. commit/amend 后，按第 9 节生成当前检查点相对上一检查点的增量代码 diff，并更新索引。
5. 记录新的 commit hash、文件列表、四组编译结果和已知限制。
6. 通过 SSH 更新维护者 fork 的同一目标开发分支；远端已有旧提交时使用 `--force-with-lease`。
7. 推送成功后自行进入下一版本，不再逐版本暂停等待维护者确认。

v8 额外执行规则：

- v8-A Execute 作为独立检查点完整执行上述循环。
- v8-B ArkTS API 在 v8-A 推送成功后再开始，并对每个涉及的仓分别检查 diff、编译、提交和推送。
- dfs_service 继续 amend 当前唯一开发提交；user_file_service 使用其自身唯一开发提交。
- user_file_service 不执行版本式反复 amend；其唯一 ArkTS API 提交推送成功后即完成，不设置等待 PR 的暂停点。

## 7. 固定编译验证

所有命令均从 OpenHarmony 源码根目录执行，产品固定为 `rk3568`。每个版本必须依次通过：

```bash
./build.sh --product-name rk3568 --build-target dfs_service
./build.sh --product-name rk3568 --build-target cloudsyncunittests
./build.sh --product-name rk3568 --build-target dfs_service \
    --gn-args dfs_service_feature_enable_cloud_disk=true
./build.sh --product-name rk3568 --build-target cloudsyncunittests \
    --gn-args dfs_service_feature_enable_cloud_disk=true
```

验证规则：

- 默认配置和开启 `dfs_service_feature_enable_cloud_disk` 的配置都必须通过。
- 功能编译与 `cloudsyncunittests` 都必须通过。
- 不以单个源文件编译或静态检查替代上述四组命令。
- 编译环境问题和代码问题分开记录；未得到四组成功结果，不提交当前版本。

v8-B ArkTS API 完成后，在上述四组命令之外额外执行：

```bash
./build.sh --product-name rk3568 --build-target user_file_service
```

`user_file_service` target 不要求增加带 `dfs_service_feature_enable_cloud_disk=true` 的重复构建。

## 8. Git 分支、提交与 SSH 推送

dfs_service 目标仓库：`https://gitcode.com/wangyanyue/filemanagement_dfs_service`。

dfs_service 开发分支：`codex/clouddisk-placeholder-v2-v8`。

正式执行前需要配置并验证以下 SSH remote：

```text
remote name: fork
remote URL:  git@gitcode.com:wangyanyue/filemanagement_dfs_service.git
```

ArkTS API 的 user_file_service 仓：

```text
local path:  code/foundation/filemanagement/user_file_service
fork URL:    https://gitcode.com/wangyanyue/filemanagement_user_file_service
remote URL:  git@gitcode.com:wangyanyue/filemanagement_user_file_service.git
transport:   SSH
branch:      codex/clouddisk-placeholder-v2-v8
remote name: fork
branch base: current local HEAD
```

推送规则：

- 不把官方 HTTPS `gitcode` 远端误作个人 fork 推送目标。
- callback table 基线与 v2～v8 最终汇总为一个开发提交和一个 PR。
- 第一次推送的提交内容只包含 callback table 基线功能；它是后续所有 version amend 的起点。
- 唯一提交从首次推送起使用覆盖 callback table 与 v2～v8 的需求总标题，后续 amend 不修改标题。
- callback table 基线首次推送后必须暂停，由维护者手动创建到 OpenHarmony 官方仓的 PR。
- 开发代理不得代替维护者创建首次 PR；收到 PR 链接和继续指令后才能进入 v2。
- 上述暂停是 dfs_service 增量开发中的唯一强制暂停点；v2～v8 后续 amend/push 成功后自动继续。
- 每个 version 验证通过后 amend 同一个提交，不新增版本提交。
- 首次提交和后续 amend 都必须带 `--signoff`，提交前检查 `Signed-off-by`，确保通过 DCO。
- 首次使用普通 push；amend 后更新远端使用 `--force-with-lease`，禁止使用裸 `--force`。
- 所有推送都指向 `fork/codex/clouddisk-placeholder-v2-v8`。
- 推送前确认本地分支不是 detached HEAD，并核对 upstream/目标 ref。
- 跨仓内容无法放进同一个 Git 提交；dfs_service 与 user_file_service 分别维护各自的提交，
  并按 v8-A → v8-B 的统一流程串行推进。
- user_file_service 只创建并推送一个带 `--signoff` 的 ArkTS API 提交；推送成功后该仓工作结束。
- user_file_service 从其当前本地 `HEAD` 创建 `codex/clouddisk-placeholder-v2-v8` 分支。

## 9. 本地 diff 归档

归档目录固定为 `docs/diff/`，索引固定为 `docs/diff/README.md`。该目录只用于本地追踪开发过程，
不得暂存、提交或推送到功能 PR。

归档规则：

1. 保存当前检查点相对上一个已提交检查点的增量 diff，不保存每次从初始 HEAD 开始的累计 diff。
2. diff 包含源码、头文件、IDL、BUILD 和测试代码差异，排除 `docs/**`。
3. 使用 `git diff --binary --full-index` 保留完整对象信息。
4. 生成后更新 `docs/diff/README.md`，登记仓库、功能点、SDD、base/head commit 和编译结果。
5. diff 与索引都不进入功能提交。
6. v8-B 分别保存 dfs_service 和 user_file_service 的增量 diff，统一登记在 dfs_service 的
   `docs/diff/README.md`。

计划文件名：

| 检查点 | diff 文件 |
| --- | --- |
| callback table 基线 | `00-callback-table-baseline.diff` |
| v2 | `02-placeholder-marking-refactor.diff` |
| v3 | `03-placeholder-custom-info.diff` |
| v4 | `04-placeholder-state-only-conversion.diff` |
| v5 | `05-placeholder-dehydrate.diff` |
| v6 | `06-placeholder-hydrate-priority.diff` |
| v7 | `07-placeholder-hydrate-main.diff` |
| v8-A | `08a-placeholder-execute.diff` |
| v8-B dfs_service | `08b-placeholder-arkts-dfs.diff` |
| v8-B user_file_service | `08b-placeholder-arkts-ufs.diff` |

基线 diff 使用“初始本地 HEAD → callback table 基线提交”。后续 dfs_service diff 使用“amend 前的
检查点 commit → amend 后的新 commit”。user_file_service diff 使用“该仓初始本地 HEAD → ArkTS API 提交”。

## 10. 单版本完成标准

一个版本只有同时满足以下条件才算完成：

- 最终 spec、对应 SDD、参考提交和代码事实基线已核对。
- 当前版本全部功能已实现，没有混入后一版本行为。
- 必要测试只新增在允许目录。
- `git diff --check` 通过。
- 第 7 节 DFS 四组编译命令全部通过；v8-B 还必须通过 `user_file_service` target。
- 当前版本已 amend 到唯一开发提交，且提交包含 `Signed-off-by`。
- amend 后的新 commit 已通过 SSH 更新到指定远端分支。
- 已向维护者报告 commit hash、变更文件和验证结果。

callback table 基线阶段额外要求：首次推送完成后暂停，等待维护者创建 PR 并回传链接；此时不得开始 v2。

## 11. 当前已知状态

- 本地实现仍处于独立 `user.clouddisk.placeholder` xattr 的 v1 模型。
- callback table 注册/解注册接口尚不在本地代码中，基线 PR #4353 已提供但尚未读取代码。
- NDK 参考 PR #6149 已提供但尚未读取代码。
- 规格、版本文档和 SDD 当前为未跟踪文件，默认不夹带到功能提交。
- 本流程文档同样只作为本地执行依据，不纳入最终提交和 PR。
- `docs/diff/` 同样只保留在本地，不纳入最终提交和 PR。
- 当前仓库处于 detached HEAD。
- 当前 `gitcode` remote 指向 OpenHarmony 官方 HTTPS 仓库；正式执行时另配 `fork` SSH remote，不覆盖
  官方 remote。
- interface_sdk-js 不在本轮修改范围内。
- 当前 spec 与全部版本 SDD 已由维护者确认为可执行设计依据。

## 12. 执行时核对项

流程层面已无待维护者确认项。开始执行后首先只读核对 PR #4353 与 #6149，确定 PR #4353 中需要采用的
准确 commit/patch 范围；这是基线实施步骤，不是新的流程决策。
