# Placeholder Xattr Probe 待明确问题

## 背景

`codex/placeholder-xattr-probe` 分支已经证明：

- `OH_CloudDisk_IsPlaceholderFile` 可以在穿刺分支内临时绕过 CloudDiskService，直接在 NDK 层执行 `lstat` / `getxattr`。
- RK3568 上曾通过临时 probe 目标打印 path、key、`lstat`、`getxattr`、`lgetxattr`、errno、value hex 等诊断信息；该 probe 只用于穿刺观察，不作为正式 UT 保留。
- 当前镜像下 `HMDFS_IOC_CREATE_PLACEHOLDER` / `HMDFS_IOC_GET_PLACEHOLDER` 返回 `ENOTTY`，尚不能构造真实 placeholder 文件。

因此，当前穿刺只能证明“不走 CloudDiskService 的接口调用面可执行”，不能证明“客户端 `getxattr` 能正确识别真实 placeholder”。以下问题需要在继续推进或正式化前明确。

## 1. 真实 placeholder 文件怎么构造

为什么：

当前板端 `HMDFS_IOC_CREATE_PLACEHOLDER` / `GET_PLACEHOLDER` 返回 `ENOTTY`，临时 probe 没有能力在测试内创建真实 placeholder。默认路径中的 `placeholder.txt` 只是候选路径，不是真实 placeholder。没有真实 placeholder，就无法证明 xattr 方案能区分 placeholder 和普通文件。

需要明确：

- 是否刷支持 placeholder ioctl 的镜像。
- 或由 HMDFS / 文件系统同学提供一个已存在的真实 placeholder 路径。
- 真实 placeholder 路径是否能通过临时验证工具或手工命令注入。

## 2. placeholder xattr 的正式 key 是什么

为什么：

当前临时实现使用 `user.hmdfs.placeholder`。若真实 HMDFS 使用其他 key，当前 NDK 实现会一直得到 `ENODATA=false`。

需要明确：

- HMDFS 是否愿意承诺一个公开只读 xattr key。
- key 是否属于 `user.*`、`trusted.*`、`security.*` 或其他 namespace。
- key 是否仅供 NDK 内部封装使用，还是允许三方应用直接读取。

## 3. xattr value 格式是什么

为什么：

当前穿刺规则是临时猜测：空 value、`1`、`true`、非零首字节视为 true，`0`、`false` 视为 false。正式方案若使用结构体、bitmask 或版本化字段，解析方式会不同。

当前代码已将临时解析逻辑封装到 NDK util 中，避免把解析细节散在公开接口实现流程里；但解析规则本身仍不能视为正式 ABI。

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

## 5. symlink 策略

为什么：

当前临时实现用 `lstat` 打印类型，但实际 `getxattr` 会跟随最终 symlink。若正式接口允许 symlink，可能绕过路径边界或查到目标文件属性，而不是 symlink 本身。

需要明确：

- 正式方案使用 `getxattr`、`lgetxattr`，还是 `open(..., O_NOFOLLOW) + fgetxattr`。
- 传入 symlink 时返回 `CLOUD_DISK_INVALID_ARG`，还是按目标文件处理。
- 如何防止最终路径逃逸同步根。

## 6. 路径边界和同步根归属是否仍要校验

为什么：

绕过 CloudDiskService 后，也绕过了 userId、bundleName、同步根注册、路径归属校验。对只读查询也许可以接受，但这会改变 02 原设计的安全边界。

需要明确：

- `OH_CloudDisk_IsPlaceholderFile` 是否仍要求目标路径属于已注册同步根。
- 如果要求，客户端侧如何获取或校验同步根归属。
- 如果不要求，接口语义是否改为任意路径的 placeholder xattr 查询。

## 7. 目录、非普通文件、不可访问路径的语义

为什么：

当前 xattr 穿刺中，目录没有目标 xattr 时会返回 `CLOUD_DISK_OK + false`。而原 CloudDiskService helper 方案会检查普通文件类型，目录通常应返回参数非法。两种方案行为不一致会影响 API 兼容。

需要明确：

- 目录、socket、fifo、设备文件分别返回什么错误码。
- symlink、无权限路径、路径不存在分别返回什么错误码。
- 是否需要先检查 `S_ISREG`，再读取 xattr。

## 8. 错误码映射是否与 02 正式方案保持一致

为什么：

当前 xattr 方案将 `ENODATA` 映射为 `CLOUD_DISK_OK + false`，`ENOENT` 映射路径不存在，`ENOTSUP` 映射不支持。ioctl 方案还涉及 `ENOTTY`、`EISDIR`、`EINVAL` 等。两套行为如果不一致，后续切换方案会让应用观察到差异。

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
