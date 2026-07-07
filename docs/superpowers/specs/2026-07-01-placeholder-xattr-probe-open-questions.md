# Placeholder Xattr Probe 待明确问题

## 背景

`codex/placeholder-xattr-probe` 分支已经证明：

- `OH_CloudDisk_IsPlaceholderFile` 可以在穿刺分支内临时绕过 CloudDiskService，直接在 NDK 层执行 `lstat` / `getxattr`。
- RK3568 上曾通过临时 probe 目标打印 path、key、`lstat`、`getxattr`、`lgetxattr`、errno、value hex 等诊断信息；该 probe 只用于穿刺观察，不作为正式 UT 保留。
- 当前镜像下 `HMDFS_IOC_CREATE_PLACEHOLDER` / `HMDFS_IOC_GET_PLACEHOLDER` 返回 `ENOTTY`，尚不能构造真实 placeholder 文件。
- 当前正式化代码已固定读取 `user.clouddisk.placeholder`，并在 NDK 层先做 path 基础校验、`ACCESS_CLOUD_DISK_INFO` 权限校验、按服务侧 `PathToMntPathBySandboxPath` 语义将沙箱路径转换为 HMDFS mount 真实路径、同步根路径边界校验、`lstat` 和普通文件类型校验，再执行 `getxattr`。
- xattr syscall mock 已从 NDK 接口 UT 中拆到独立测试辅助文件；正式 UT 只验证接口行为，mock 文件只负责提供 `lstat` / `getxattr` 的可控结果。

因此，当前穿刺只能证明“不走 CloudDiskService 的接口调用面可执行”，不能证明“客户端 `getxattr` 能正确识别真实 placeholder”。以下问题需要在继续推进或正式化前明确。

## 1. 真实 placeholder 文件怎么构造

为什么：

当前板端 `HMDFS_IOC_CREATE_PLACEHOLDER` / `GET_PLACEHOLDER` 返回 `ENOTTY`，临时 probe 没有能力在测试内创建真实 placeholder。默认路径中的 `placeholder.txt` 只是候选路径，不是真实 placeholder。没有真实 placeholder，就无法证明 xattr 方案能区分 placeholder 和普通文件。

需要明确：

- 是否刷支持 placeholder ioctl 的镜像。
- 或由 HMDFS / 文件系统同学提供一个已存在的真实 placeholder 路径。
- 真实 placeholder 路径是否能通过临时验证工具或手工命令注入。

## 2. placeholder xattr key 的 ABI 承诺

为什么：

当前实现已按 review 意见固定使用 `user.clouddisk.placeholder`。如果真实 HMDFS 使用其他 key，NDK 会一直得到 `ENODATA` 错误。当前代码按错误处理，不把缺少 xattr 解释为普通文件成功返回；该行为参考现有 `GetFileSyncState` 对缺失 xattr 的处理方式。

需要明确：

- HMDFS 是否愿意承诺 `user.clouddisk.placeholder` 作为公开只读 xattr key。
- key 是否属于 `user.*`、`trusted.*`、`security.*` 或其他 namespace。
- key 是否仅供 NDK 内部封装使用，还是允许三方应用直接读取。

## 3. xattr value 格式是什么

为什么：

当前正式化代码只按 `getxattr` 返回长度判断：返回长度为 `1` 时认为是占位符，其他正长度返回 `CLOUD_DISK_OK + false`，读取失败则按 errno 返回错误。正式方案若使用结构体、bitmask 或版本化字段，解析方式会不同。

当前代码已将 `lstat` / `getxattr` 查询封装到 NDK util 中，避免把底层细节散在公开接口实现流程里；但 value 规则本身仍不能视为正式 ABI。

需要明确：

- value 是字符串、整数、空值即存在，还是结构化二进制。
- 是否有版本字段或保留字段。
- 异常 value 应返回 `false`，还是返回错误。

## 4. 普通应用或 NDK 进程是否有权限读取该 xattr

为什么：

方案目标是不走 CloudDiskService。如果普通应用进程读不到 xattr，只能系统服务读到，则客户端 `getxattr` 方案不成立。当前 probe 是测试进程身份，不一定等同三方应用运行身份。

需要明确：

- 目标调用身份是普通三方应用、系统应用，还是仅 NDK 测试进程。
- SELinux / sandbox 是否允许目标身份读取该 xattr。
- 不同 userId、bundle、同步根归属下是否有权限差异。
- 当前代码已经在 NDK 侧复用 `DfsuAccessTokenHelper` 执行 `ACCESS_CLOUD_DISK_INFO` 和 system app 校验，并因此将 `ohclouddiskmanager` 依赖从 `libdistributedfileutils_lite` 切到 `libdistributedfileutils`。后续仍需确认这个依赖体量是否可接受，或是否需要更轻量的 NDK 专用权限 helper。

## 5. symlink 策略

为什么：

当前实现会先按服务侧 `PathToMntPathBySandboxPath` 语义把沙箱路径转换为 HMDFS mount 路径，并在转换过程中执行 `realpath`，再用规范后的 mount 路径做同步根边界校验和 `getxattr`。这能防止 `..`、重复路径段等绕过字符串边界，但也意味着最终 symlink 会按目标文件处理；如果产品要求拒绝 symlink，需要额外在原始 `path` 上做 `lstat` 或改为 `open(..., O_NOFOLLOW) + fgetxattr`。

需要明确：

- 正式方案使用 `getxattr`、`lgetxattr`，还是 `open(..., O_NOFOLLOW) + fgetxattr`。
- 传入 symlink 时返回 `CLOUD_DISK_INVALID_ARG`，还是按目标文件处理。
- 如何防止最终路径逃逸同步根。

## 6. 路径边界和同步根归属是否仍要校验

为什么：

绕过 CloudDiskService 后，也绕过了 bundleName、同步根注册、同步根归属校验。当前代码已经根据 userId 将沙箱路径转换为 HMDFS mount 真实路径，并检查 `path` 是否位于 `syncFolderPath` 下，但这只证明调用参数自洽，并不能证明该 `syncFolderPath` 已由当前应用注册。

需要明确：

- `OH_CloudDisk_IsPlaceholderFile` 是否仍要求目标路径属于已注册同步根。
- 如果要求，客户端侧如何获取或校验同步根归属。
- review 中提到的 `callbackIndexMap` 属于 CloudDiskService 进程内变化回调状态，用于变化通知 callback，不应为了 NDK xattr 查询改变其原有数据结构或把它作为正式同步根注册数据源，除非最终方案明确同进程或共享状态。
- 当前代码已在沙箱到 mount 路径转换和 `path` 位于 `syncFolderPath` 内的边界校验之后、`lstat` / `getxattr` 之前预留接入点；后续拿到可信数据源后，应在该位置补充“同步根已注册且归属于当前调用方”的校验。
- 如果需要不走 CloudDiskService IPC 又校验注册同步根，候选数据源更可能是客户端可读的同步根注册查询接口，例如 `CloudDiskSyncFolderManager::GetSyncFolders`，但这是否符合“不走 IPC”的目标仍需确认。
- 如果不要求，接口语义是否改为任意路径的 placeholder xattr 查询。

## 7. 目录、非普通文件、不可访问路径的语义

为什么：

早期 xattr 穿刺中，目录没有目标 xattr 时会返回 `CLOUD_DISK_OK + false`。当前正式化代码已改为先 `lstat` 并检查 `S_ISREG`，目录等非普通文件返回 `CLOUD_DISK_INVALID_ARG`，避免与原 CloudDiskService helper 方案明显偏离。

需要明确：

- 除目录外，socket、fifo、设备文件分别返回什么错误码；当前实现统一按非普通文件返回 `CLOUD_DISK_INVALID_ARG`。
- symlink、无权限路径、路径不存在分别返回什么错误码。
- symlink 最终策略仍需结合第 5 节确认；当前路径转换中的 `realpath` 会跟随 symlink 到目标文件，再对目标路径做 `lstat` / `getxattr`。

## 8. 错误码映射是否与 02 正式方案保持一致

为什么：

当前 xattr 方案将 `ENODATA` 当作错误并折叠为 `CLOUD_DISK_TRY_AGAIN`，`ENOENT` 映射路径不存在，`ENOTSUP` 映射不支持。ioctl 方案还涉及 `ENOTTY`、`EISDIR`、`EINVAL` 等。两套行为如果不一致，后续切换方案会让应用观察到差异。

需要明确：

- xattr 方案最终 errno 到 `CloudDisk_ErrorCode` 的完整映射表。
- 与 02 ioctl 方案不一致的场景是否允许。
- raw errno 是否需要继续打日志，便于设备侧定位。

## 9. xattr ABI 是否允许长期公开

为什么：

如果 HMDFS 把 xattr 当内部实现细节，NDK 直接依赖会绑定底层格式，后续 HMDFS 想替换存储方式会破坏 NDK 行为。这是方案能否正式化的核心风险。

需要明确：

- HMDFS / 文件系统 / 安全评审是否接受公开只读 xattr ABI。
- 若不接受公开 ABI，是否接受 NDK 内部封装私有依赖。
- 如果两者都不接受，02 是否继续保留 CloudDiskService ioctl 方案。

## 10. 穿刺分支是否继续保留 NDK 改造

为什么：

当前分支临时改了 `OH_CloudDisk_IsPlaceholderFile`，该改动不能直接进入 02 正式 PR。它是实验代码。后续要么继续用它验证真实 placeholder，要么在结论明确后回退。

需要明确：

- 穿刺分支下一步是继续验证真实 placeholder，还是已经足够证明当前条件下不成立。
- 是否需要保留临时 NDK 实现用于下一轮设备验证。
- 若回到 02 ioctl 方案，如何避免穿刺改动混入正式 PR。

## 当前建议

当前证据更适合得出以下结论：

- NDK 侧 `getxattr` 调用面可跑通。
- 错误码基本折叠链路可跑通。
- 真实 placeholder 识别能力尚未被证明。

因此，02 正式方案暂不应直接切换到客户端 `getxattr`。若要继续推进 xattr 方案，应优先解决“真实 placeholder fixture”和“HMDFS xattr ABI 承诺”两个问题。
