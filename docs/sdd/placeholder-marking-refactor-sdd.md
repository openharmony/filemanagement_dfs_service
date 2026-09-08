> **opencode session**:`ses_fbcce9d8affeJxW32yVC3Gkq9i`
> slug:`swift-planet` | 标题:项目需求讨论与澄清 | 目录:`D:\code\dfs\filemanagement_dfs_service`
> 续接:`opencode resume ses_fbcce9d8affeJxW32yVC3Gkq9i`(或在 TUI session 列表选 `swift-planet`)

# Placeholder Marking Refactor SDD

## 1. 背景

`CloudDiskService`(三方网盘 SA)当前在文件上维护两个独立 xattr:

- `user.clouddisk.filesyncstate`:单字节 `uint8`,值为 `SyncState` 枚举(0–5)。
- `user.clouddisk.placeholder`:单字节 char 值 `'1'`(占位符)/`'0'`(普通,convert 写)/`'2'`(hydrating,仅定义、无生产写入者)。

目录级别没有占位聚合状态。上游 PR #4236 曾提出目录级 `user.clouddisk.placeholdercount` 与祖先链增量更新,但未合入、本仓不存在其代码,本设计不依赖它,仅借鉴其祖先链思路并完全重新实现。

本设计把文件级两个 xattr 合并为单个 `uint8` `filesyncstate`,并在目录级维护"子树占位符计数"与"目录是否占位"聚合状态,作为占位符标记重构的统一基础。

## 2. 目标

1. 文件占位符状态并入 `filesyncstate` 高 3 位,移除独立 `placeholder` xattr。
2. 目录维护 count(`user.clouddisk.phcount`)与目录占位布尔(`filesyncstate` 高 3 位)。
3. 文件占位态变化时,沿祖先链增量更新目录 count 与目录占位布尔(模型 B)。
4. NDK Get/Set filesyncstate **操作语义**不变(低 5 位 mask / RMW 保高位 / `>5` 校验),对外仍只暴露低 5 位 `SyncState`。**例外(合并固有,仅占位符)**:`CreatePlaceholder` 现写 filesyncstate(占位态在高 3 位)→ 占位符带上 filesyncstate → `GetFileSyncStates` 对占位符由 v1 的 `NO_SYNC_STATE` 改返 `IDLE(0)`;**普通文件不受影响**(仍不自动带 filesyncstate)。
5. 高位编码不写死,以 `!= 0` 为"是否占位符"锚,为未来水合子态预留。
6. 删除/重命名等观察事件无法精确算 delta 时,接受 best-effort + recount 兜底。
7. 不新增 IPC / IDL / 接口码 / Parcelable。

## 3. 数据模型

### 3.1 filesyncstate 字节布局

复用 xattr 名 `user.clouddisk.filesyncstate`,值仍为单个 `uint8`:

```
bit7 bit6 bit5 | bit4 bit3 bit2 bit1 bit0
<── 高 3 位 ──> <────── 低 5 位 ──────>
   占位符态           SyncState
```

- **文件**:高 3 位 = `PlaceholderState`;低 5 位 = `SyncState`(0–5,bit3-4 保留 0)。
- **目录**:高 3 位 = (count>0)?非0:0(取值 1 表示"子树含占位符");低 5 位 恒 0,不语义化。
- "是否占位"统一以高 3 位 `!= 0` 判(文件与目录一致)。

### 3.2 PlaceholderState 编码(4 态,前瞻)

| 值 | 常量 | 含义 | counted |
| --- | --- | --- | --- |
| 0 | `NONE` | 普通文件(与已有普通文件兼容:旧 filesyncstate 高位恒 0 → 自然读作 `NONE`,无需迁移) | 否 |
| 1 | `UNHYDRATED` | 未水合占位符(稀疏文件,纯云 stub) | 是 |
| 2 | `PARTIALLY_HYDRATED` | 部分水合占位符(数据部分下载本地) | 是 |
| 3 | `FULLY_HYDRATED` | 完全水合占位符(数据全下载本地,对外仍是占位符/仍受云管) | 是 |
| 4–7 | reserved | 预留 | — |

关键谓词:`IsPlaceholder(s) = (s != NONE)`,即 `(!= 0)`;1/2/3 均计为占位符。目录 high3 仅取 {0,1}:0=无、`1=HAS_PLACEHOLDER`(有占位);不复用文件 2/3,1 与文件 `UNHYDRATED(1)` 数值同但语义异(节点类型区分),count 只看 `!=0`。

- count 与 delta 全部基于该谓词,不基于具体码值。
- 水合进度切换(1↔2↔3):`IsPlaceholder` 不变 → delta = 0 → 不触发 count 变化,无需改 count/delta/目录逻辑。
- 只有 `0 ↔ {1,2,3}`(普通↔占位)才 ±1。
- 旧 `'2'`(hydrating-in-progress,`CheckPlaceHolderXattr` 返 `E_HYDRATE_IN_PROGRESS`,无生产写入者)删除;`'2'` 重定义为 `PARTIALLY_HYDRATED`(稳定态,非进行中)。clean break、无真实用户,盘上无 `'2'` 文件,重定义无冲突。
- 已有普通文件高位恒 0 → 读作 `NONE`;clean break 下需兼容的唯一对象即普通文件,天然兼容,无迁移。
- 当前写入者只产生 `1`(create)/`0`(convert);`2/3` 为未来水合预留(clouddiskservice 现无水合路径),编码先行固定,未来水合接入不改 count/delta/目录逻辑。
- **水合进行中:内部处理,不暴露**。应用对占位符只有 `IsPlaceholder`(bool,`!=0` 即 true,不区分子态)与 convert 失败错误码两个触点;`1/2/3`(及未来任何"下载中"码)对应用全为内部。暴露水合进度需新增/改 NDK 接口,违反"不新增 IPC/IDL"约束,故不做。
- **`E_HYDRATE_IN_PROGRESS`**:错误枚举值**保留**(既有 convert 失败契约,可能已有应用依赖,勿删枚举项);但 `CheckPlaceHolderXattr` 产生它的旧 `'2'` 分支**现删**,简化为 `(!=0 ? E_OK : E_NOT_A_PLACEHOLDER)`。水合落地时再在 convert 加检查重新产生该错误。
- **"如何判断正在水合中"延后**:检测机制等水合真正实现时再定——取决于哪个组件做水合(若 clouddiskservice 自做,内存态即可、重启丢弃中断下载;若跨组件,可能需 on-disk 码)。`4–7` 暂留 reserved,不预绑 `4=downloading`,避免过早承诺存储。

### 3.3 count xattr

- 名:`user.clouddisk.phcount`(共享常量,放共享内部头,供 `cloud_disk_service.cpp` 与 `cloud_disk_sync_folder.cpp` 共用,避免重复定义)。
- 格式:`uint32_t` 小端,4 字节定长。
- 缺失视为 0;仅 count≥1 的目录写该 xattr,count=0 目录不写。
- 选择 `uint32` 而非字符串:xattr 值(1–4B)相对名与条目开销是噪声,"省空间"在值层面不构成区分;`uint32` RMW 仅 2 次系统调用、无解析、无损坏态、无溢出,正确性与简单性全面占优。

### 3.4 dentry 占位字段(metafile 层)

- 借 `CloudDiskServiceDentry.reserved[3]`(`cloud_disk_service_metafile.h:57`,注释 "reserved bytes for long term extend")中 1 字节做 `placeholder` 字段。
- **不改尺寸**:`CloudDiskServiceDentry` 仍 52B,`CloudDiskServiceDentryGroup` 仍 4096(`static_assert` :67 不破)→ **二进制布局兼容、无迁移**。
- 按 AGENTS.md 补读写 + 升级测试;老记录 `reserved` 可能是脏值,读时非合法 `PlaceholderState` 值按 0(`NONE`)处理(当前合法 {0,1},水合后扩为 {0,1,2,3})。
- 字段缓存该节点(文件或目录)的高 3 位,作为观察删除/重命名时的 before-state 来源。

### 3.5 移除

移除 `user.clouddisk.placeholder` xattr 及其常量(`cloud_disk_service.cpp:53`、`cloud_disk_sync_folder.cpp:32`)。clean break,无迁移、无读回退分支。

## 4. NDK 语义保持

- `GetFileSyncStates`:读字节 → `byte & 0x1F` 取低 5 位 → 校验 `> SYNC_CONFLICTED(5)` 拒绝 → 返回 `SyncState`。
- `SetFileSyncStates`:RMW `(byte & 0xE0) | (syncState & 0x1F)`,保留高 3 位。
- **目录 Get**:加目录守卫,返 `NO_SYNC_STATE`(保持现状:目录原先无该 xattr → `ENODATA` → `NO_SYNC_STATE`)。
- **目录 Set**:不加守卫(保持现状"成功"),但走 RMW 保高位,不整字节覆盖。
- IPC wire / Parcelable 顺序 / 错误码 / NDK 枚举整数(0–5)全部不变;`CloudDisk_SyncState` 与 inner `SyncState` 仍 `static_cast` 对齐。
- **占位符行为变化(合并固有,仅占位符)**:`CreatePlaceholder` 写 filesyncstate → 占位符带上 xattr → `GetFileSyncStates` 对占位符返 `IDLE(0)`(v1 因不写 filesyncstate 返 `NO_SYNC_STATE`)。**普通文件不受影响**(未调过 Set 的普通文件仍无 filesyncstate → `NO_SYNC_STATE`;调过 Set 的仍返其 `SyncState`)。详见 v2 规格 §2.5。

## 5. 工具函数

集中 mask / RMW / 锁,业务调用点只调 util。

### 5.1 filesyncstate util

| util | 行为 |
| --- | --- |
| `GetHighBits(path/fd)` | 读字节,返回高 3 位 `PlaceholderState` |
| `SetHighBits(path/fd, newState)` | 一把锁内:读旧 state → RMW 写新高位(保低 5 位)→ **返回旧 state** |
| `GetLowBits(path/fd)` | 读字节,返回低 5 位 `SyncState` |
| `SetLowBits(path/fd, syncState)` | 一把锁内:RMW 写低 5 位(保高 3 位) |

`SetHighBits` 返回旧 state,使调用方在同一次原子操作里拿到 before-state,无 TOCTOU。

### 5.2 dentry util(metafile 层)

| util | 行为 |
| --- | --- |
| `DentryGetPlaceholder(dentry*)` | 读 `reserved` 借用字节 |
| `DentrySetPlaceholder(dentry*, state)` | 写 `reserved` 借用字节 |

### 5.3 全局锁

所有 RMW(filesyncstate 高/低位、dentry 字段、count、祖先刷新临界区)共用同一把全局 `std::mutex`(函数内 static),正确性优先。同一 inode 上并发 RMW 会丢更新,必须串行;全局锁收口。per-syncRoot 锁留作压测后优化。

## 6. Model B 祖先刷新算法

count 定义:**直接占位符子项**数 = 本目录直系占位符文件(`state != 0`)+ 直系子目录中高 3 位 `!= 0` 的数量。目录高 3 位 = `(count > 0) ? 非0 : 0`。

文件占位态变化(delta `d ∈ {−1, 0, +1}`)时,从文件**父目录**起逐级到 **syncRoot(含)**:

```
cur = parent(file);  propagated = d
while cur 在 syncRoot 内(含 syncRoot):
    oldCount = get count(cur)
    newCount = oldCount + propagated
    set count(cur) = newCount                 # RMW, 全局锁
    if (oldCount>0) != (newCount>0):          # count>0 布尔变 = 跨 0 边界
        newDirState = newCount>0 ? HAS_PLACEHOLDER : NONE
        oldDirState = GetHighBits(cur)
        if oldDirState == 0 and newDirState != 0:   # 0 -> 非0
            SetHighBits(cur, newDirState); propagate up
        elif oldDirState != 0 and newDirState == 0:  # 非0 -> 0
            SetHighBits(cur, newDirState); propagate up
        else: break                           # 已一致
        cur = parent(cur);  # propagated 保持同号 d
    else:
        break                                # 占位目录身份未变 -> 停
```

要点:
- count RMW **无条件**逐级到 syncRoot;**只有**该级 `(count>0)` 布尔变才翻高 3 位并继续上溯。
- 上溯传播的 delta 始终同号(文件变占位符→各级"新增一个占位目录"→+1;反之 −1)。
- high 3 位始终保持 = (count>0)。

## 7. 接线(delta = IsPlaceholder(new) − IsPlaceholder(old))

`old` 来自 `SetHighBits` 返回值;`IsPlaceholder(s) = (s != 0)`。

| 变迁点 | 入口 | old → new | delta | 接线 |
| --- | --- | --- | --- | --- |
| create 占位符 | `CreatePlaceholderFileInner` | 0 → 1 | +1 | `SetHighBits(file, UNHYDRATED)` 成功后调 helper(+1) |
| convert→normal | `ConvertPlaceholderToEmptyFile` | 1 → 0 | −1 | `SetHighBits(file, NONE)` 成功后调 helper(−1) |
| update | `UpdatePlaceholderAttr` | old → 1 | `1 − IsPlaceholder(old)` | `SetHighBits(file, UNHYDRATED)` 后按 delta 调 helper(可能 0) |
| delete(观察) | `DoRemove` | 读 dentry 字段 | best-effort −1 | `DentryGetPlaceholder`==非0 → helper(−1) |
| rename(观察) | `DoRenameOld/New` | 读 dentry 字段 | best-effort ±1 | 旧父 −1、新父 +1;字段随 dentry 迁移 |
| 注销清理 | `UnregisterSyncFolderInner`/`UnregisterForSaInner` | — | — | 已有 `RemoveXattr(filesyncstate)` + 新增 `RemoveXattr(phcount)` |

### 7.1 update 的鲁棒性(留意)

当前 `UpdatePlaceholderAttr` 无前置占位符检查、可对普通文件脱水(0→1)。但同步侧其他设计可能把 update 收窄为"只更新已有占位符"(加 `CheckPlaceHolderXattr` 类前置 → before 恒 1)。本设计 delta 用**实际读到的 old** 算,不硬编码 +1:

- 收窄前:old 可能 0 → delta +1(脱水)→ hook 触发。
- 收窄后:old 恒 1 → delta 0 → hook 自然 no-op,无需回改。

两套设计的耦合点在"update 前置条件",review 时对齐。**禁止把 update 的 delta 写死 +1**。

## 8. dentry 字段同步与 create-race

### 8.1 写入点

- `ProduceCreateLog`(FAN_CREATE 观察,logfile 层):**dentry 字段主入口**。建 dentry 时 `getxattr` 文件 `filesyncstate` 取高 3 位,**无条件** `DentrySetPlaceholder(dentry, GetHighBits(file))`(占位→`UNHYDRATED`、非占位→`NONE`)。这是"建占位符→直接删除"常见路径 before-state 的唯一来源,不可省。**best-effort / create-race**:`FAN_CREATE` 在 `openat` 创建时触发,早于 IPC 的 `fsetxattr(高位)` → `getxattr` 可能读 0(应 1)→ 字段记 0;accepted,recount 兜底。若将来要消 race,改在 `FAN_CLOSE_WRITE`(创建进程 close 后高位必已写)回写。
- `convert`/`update`(IPC,`cloud_disk_service.cpp`):`SetHighBits` 后同步 `DentrySetPlaceholder`(此时 dentry 已由先前 FAN_CREATE 建好)→ **准确**。
- 分层:`ProduceCreateLog`(metafile 层)需调 `GetHighBits`(xattr util)——`GetHighBits` 须为**共享 util**,ipc 与 sync_folder 层都能访问。

### 8.2 读取点

- `DoRemove`(delete):`DentryGetPlaceholder` → 非0 则 helper(−1)。字段在 `revalidate=INVALIDATE` 之前读。
- `DoRenameOld/New`(rename):`DentryGetPlaceholder` → 旧父 −1、新父 +1;字段随 dentry 迁移。

### 8.3 接受的不准确性(均 recount 可修)

仅 **create-race**(create 时 dentry 尚未存在、且时序竞态)。convert/update 已同步。create-race 若将来要彻底修,可改在 `FAN_CLOSE_WRITE`(创建进程 close 后高位必已写)回写字段;现按 recount 兜底,留作后续。

### 8.4 重启鲁棒

count/high3 在 xattr、dentry 字段在 metafile,**全盘持久化**。SA 重启后状态都在,增量维护继续,重启后的删除/重命名仍可读 dentry 字段算 delta。这规避了"内存 map"方案重启丢表的致命缺口。

## 9. recount 兜底机制

### 9.1 接口

`Recount(subPath)`:对指定子树重算并回写 count / 目录 high3 / dentry.placeholder。

### 9.2 实现

后序遍历:每个文件读 high3(ground truth,`!= 0` 即占位)→ 自底向上算每级目录 count(直接占位子项数)+ high3(`=count>0`)→ 回写 `phcount` + 目录 `filesyncstate` 高位 + 每个节点 dentry.placeholder。

### 9.3 锁

按项/按目录加锁,**不整树持全局锁**(避免冻 SA)。recount 与并发增量交错产生的误差由下次 recount 收敛(已接受 best-effort)。

### 9.4 触发

接口与实现就绪,触发时机另行设计(候选:周期、同步完成、按需、注销前)。

## 10. 错误处理

1. count / 目录 high3 更新失败不回滚已完成文件业务操作。
2. 单个目录 `getxattr`/`setxattr` 失败记日志,继续后续祖先。
3. count 缺失按 0;dentry 字段非合法 `PlaceholderState` 值按 0(`NONE`)。
4. 业务流程必须先确定 delta 再调 helper;helper 不读文件高位反推 delta(create/convert/update 由 `SetHighBits` 返回值得 old;delete/rename 由 dentry 字段得 old)。
5. 全局锁只覆盖 RMW 临界区,不覆盖文件创建/转换等业务主操作。

## 11. 安全与约束

1. 不绕过 `CloudDiskServiceAccessToken` 的 token / userId / bundleName 校验。
2. 不新增对外 IPC / inner API / 接口码;不改 IDL。
3. 不扫描子树计算 count(除 recount);增量维护只读当前文件与祖先目录 count。
4. 只更新 syncRoot 内目录。
5. dentry 字段借用 `reserved`,不改记录尺寸,保持 metafile 二进制布局兼容。

## 12. 性能

- 增量更新:模型 B 下,文件占位态变化通常只触达直接父目录(O(1));仅当跨 0 边界才上溯,最坏 O(depth)。
- RMW:每次写多一次 `getxattr`(create/convert/update/删除/重命名/NDK Set),非超热路径,可接受。
- 全局锁:不同 syncRoot/文件间串行,保守实现;若压测成瓶颈,改 per-syncRoot 锁(需锁表生命周期设计)。
- recount:O(subtree),非频繁触发。

## 13. 测试要求

1. `PlaceholderState`:`NONE(0)` 非占位、`UNHYDRATED(1)`/`PARTIALLY_HYDRATED(2)`/`FULLY_HYDRATED(3)` 占位;`IsPlaceholder = (!=0)`。
2. NDK Get/Set 语义:文件返回低 5 位 0–5;`Set` RMW 不破坏高位;目录 Get 返 `NO_SYNC_STATE`;目录 Set 不破坏高位。
3. `SetHighBits` 返回旧 state;delta = `IsPlaceholder(new) − IsPlaceholder(old)`。
4. create +1 / convert −1 / update `1−IsPlaceholder(old)`(覆盖 old=0 与 old=1)。
5. 未来水合进度 1↔2↔3 切换 delta=0(占位语义不变,不触发 count)。
6. Model B:count RMW 逐级到 syncRoot;只有跨 0 边界翻 high3 并上溯;父目录 count 1→2(新增兄弟占位符)不上溯。
7. dentry 字段:ProduceCreateLog 写、DoRemove 读、DoRename 迁移;脏值按 0。
8. create-race 场景 best-effort;recount 修正。
9. recount:从 high3 重算 count/high3/dentry 字段一致。
10. 并发:同 inode RMW 不丢更新(全局锁)。
11. 注销清理 `RemoveXattr(filesyncstate)` + `RemoveXattr(phcount)`。
12. metafile reserved 字段读写 + 老记录兼容(非合法 `PlaceholderState` 值按 0)。

## 14. 实现落点

| 内容 | 路径 |
| --- | --- |
| filesyncstate util + 祖先刷新 helper | `services/clouddiskservice/ipc/src/placeholder_helper.{h,cpp}`(新增,仿 #4236 位) |
| dentry util | `services/clouddiskservice/sync_folder/`(metafile 层,`cloud_disk_service_metafile.{h,cpp}` 或新增小文件) |
| 改造点 | `cloud_disk_service.cpp`(`Get/SetFileSyncState`、`IsPlaceholder`、`CheckPlaceHolderXattr`、create/convert/update、`RemoveXattr` 常量)、`cloud_disk_sync_folder.cpp`(常量、`RemoveXattr(phcount)`、`ProduceCreateLog`/`DoRemove`/`DoRename`) |
| BUILD | `services/clouddiskservice/BUILD.gn` 的 `clouddiskservice_sa` 加新源文件 |
| 测试 | `test/unittests/clouddiskservice/`,目标 `cloud_disk_service_static_test`(实现时核对确切名) |

## 15. Ask-before / 评审项

- `filesyncstate` 盘上格式重解释(高 3 位语义变更):持久化格式范畴,需维护者 sign-off。
- `CloudDiskServiceDentry.reserved` 借用字节语义变更:metafile 二进制布局范畴(虽无尺寸变化),需 sign-off + 读写/升级测试。
- NDK 邻接行为(目录 Get 守卫):需确认语义不变。
- NDK 占位符 Get 行为变化(合并固有,仅占位符):`GetFileSyncStates` 对占位符由 `NO_SYNC_STATE` 改返 `IDLE(0)`,普通文件不变;已接受为合并固有结果(见 §4、v2 §2.5),需评审知悉。
- 无 IPC/IDL/接口码改动 → `CODEOWNERS` 接口码评审大概率不触发。
- DCO / `Signed-off-by`:按上游合入标准,commit 须含 `Signed-off-by`。

## 16. 后续扩展

1. 水合子态:实现 `UNHYDRATED/PARTIALLY_HYDRATED/FULLY_HYDRATED`(1↔2↔3)切换(delta=0,不改 count/目录逻辑),补水合写入路径。
2. per-syncRoot 锁:压测后优化,设计锁表生命周期与异常清理。
3. recount 触发策略:周期 / 同步完成 / 按需 / 注销前。
4. create-race 彻底修:改在 `FAN_CLOSE_WRITE` 回写 dentry 字段。
5. 目录 high3 更细聚合(如子树是否全 `FULLY_HYDRATED`):高 3 位尚有 4–7 余量。
