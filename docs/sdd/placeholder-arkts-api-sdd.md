> 标题:clouddiskservice 文件管理器 ArkTS API 设计
> 状态:设计已签核并实现；本文已与异步水合 callback/Execute 生命周期对齐。**前置依赖** 水合主体 SDD(`docs/sdd/placeholder-hydrate-main-sdd.md`)、Execute SDD(`docs/sdd/placeholder-execute-sdd.md`)与回调表基础设施(PR #4353)。接口定义 PR #35509(`interface_sdk-js`,`@ohos.file.cloudDiskManager.d.ts`)提供 .d.ts 参考。NAPI 入口在 ufs 仓(`filemanagement_user_file_service/interfaces/kits/native/clouddiskmanager`),SA 实现在 dfs 仓。

# Placeholder ArkTS API SDD

## 1. 背景

水合主体 SDD 和 Execute SDD 定了 CAPI 侧(`OH_CloudDisk_HydratePlaceholder`/`OH_CloudDisk_Execute`/`OH_CloudDisk_DehydrateFile`)的完整设计。但文件管理器(ArkTS 应用)不直接调 CAPI,而是通过 NAPI 模块 `file.cloudDiskManager` 访问 SA 能力。PR #35509 定义了 `CloudDiskSystemAccessor` 类(无参构造)和 4 个接口:`hydratePlaceholder`、`dehydrateFile`、`on('hydrateProgress')`、`off('hydrateProgress')`。

现有 NAPI 入口(`cloud_disk_access_n_exporter.cpp` 在 ufs 仓)已实现 `CloudDiskAccessor(syncFolderPath)` 类 + `isPlaceholderFile(relativePath)` 方法,范式为:NAPI → `CloudDiskPlaceholderManager`(ufs inner API)→ IPC → dfs SA。本设计沿用此范式,新建 `CloudDiskSystemAccessor` NAPI 类 + `CloudDiskSystemManager` inner API + SA 侧进度回调基础设施。

CAPI D10 明确"跨进程通知后续单独设计"——本 SDD 正是该设计的落地:file manager(进程 A)调用 `hydratePlaceholder` 触发水合,cloud disk app(进程 B)注册的 callback table 接收 one-way `OnCallback(FETCH_DATA)`,深拷贝请求后异步下载并通过独立 Execute 回写,SA 推送进度给 file manager。

## 2. 目标

1. NAPI 新增 `CloudDiskSystemAccessor` 类(无参构造),4 个方法:`hydratePlaceholder(filePath, callbackType, priority)`、`dehydrateFile(filePath)`、`on('hydrateProgress', callback)`、`off('hydrateProgress', callback?)`。
2. PR #35509 变更:加 `HydratePriority` enum(LOW/NORMAL/HIGH)、`hydratePlaceholder` 加 `priority` 参、`HydrateProgressState` 改为 PENDING/IN_PROGRESS/COMPLETED/CANCELLED(删 FAILED)。
3. ufs inner API 新建 `CloudDiskSystemManager` 类:`HydratePlaceholder(filePath, callbackType, priority)`、`DehydrateFile(filePath)`、`RegisterProgressCallback(callback)`、`UnregisterProgressCallback(callback?)`。
4. SA 侧新增 IDL:`StartHydrationByPathInner(absolutePath, type, priority)`、`DehydrateFileByPathInner(absolutePath)`、`RegisterProgressCallbackInner(ICloudDiskProgressCallback)`、`UnregisterProgressCallbackInner()`。
5. SA 侧新建 `PlaceholderProgressManager`:持有 progress callback proxy 列表 + death recipient,task 状态变化/成功 Execute 导致 cachedSize 增长时经 `ICloudDiskProgressCallback::OnProgress(HydrateProgress)` 推送。时间节流 500ms,末次(COMPLETED/CANCELLED)必推。
6. `PlaceholderTaskRecord` 加 `cachedSize`(A5 修正)+ `absolutePath`(A12),TaskManager 在进度变化点调 `ProgressManager::OnTaskProgress(reqKey)`。
7. 推送当前用户范围内的所有 task 进度(含 CAPI 发起的),file manager 再按 `filePath` 自行过滤(A15);不跨用户推送。
8. 测试落 ufs 仓 NAPI 测试 + dfs 仓 SA 测试。

## 3. 总体决策

| 项 | 决策 |
| --- | --- |
| NAPI 类 | 新建 `CloudDiskSystemAccessor`(无参构造),与既有 `CloudDiskAccessor(syncFolderPath)` 平行 |
| inner API | 新建 `CloudDiskSystemManager`(ufs `cloud_disk_manager_js_kit`),不依赖 `CloudDiskPlaceholderManager` |
| filePath 语义 | `/storage/Users/currentUser/` 下的完整绝对路径(沙箱路径),SA 内部 `ResolveSystemAccessorPath` 按当前用户解析已注册目录;不要求文件管理器与 provider 的 bundleName 相同 |
| 路径解析 | SA 侧解析(新 IDL `StartHydrationByPathInner`/`DehydrateFileByPathInner` 传绝对路径),一次 IPC |
| priority | ArkTS API 需显式传入(PR #35509 加 `HydratePriority` enum + `hydratePlaceholder` 加参) |
| HydrateProgressState | PENDING(0)/IN_PROGRESS(1)/COMPLETED(2)/CANCELLED(3),删 FAILED,对齐 CAPI TaskState |
| Hydrate Promise | task 创建并入队后 resolve；不表示 provider 已收到 FETCH 或水合完成 |
| provider callback | FETCH/CANCEL 为 one-way；provider 深拷贝请求后异步 Execute，callback 返回不结束 task |
| task 生命周期 | 最终 Execute、主动取消、5分钟滑动空闲超时、callback 死亡、注销、用户切换或 SA 停止时结束 |
| task 限额 | 按目标同步目录的 provider bundleName 计数：每应用5个、全局10个；满额返回 34400034 |
| 进度回调机制 | SA 侧新 broker `ICloudDiskProgressCallback`(`OnProgress`)+ `PlaceholderProgressManager` + `RegisterProgressCallbackInner`/`UnregisterProgressCallbackInner` |
| 推送模型 | TaskManager push(TaskManager→ProgressManager::OnTaskProgress on state change + cachedSize growth) |
| 推送频率 | 时间节流 500ms,末次(COMPLETED/CANCELLED)必推 |
| 推送范围 | 与订阅者 userId 相同的所有 task(含 CAPI 发起的),file manager 再按 filePath 过滤 |
| 订阅模式 | 当前用户范围内订阅(`on` 一次收该用户所有文件进度,`filePath` 区分),不跨用户 |
| on/off 范式 | 标准 OpenHarmony `napi_ref` 管理:`on` 存 ref,`off` 删 ref(无 callback 参删全部) |
| death recipient | progress callback 挂 `SvcDeathRecipient`(同 E5 范式),file manager 崩溃→清理 proxy |
| cachedSize | 加到 `PlaceholderTaskRecord`,本次 Execute 全部必要步骤成功后按 size 有界累加 |
| absolutePath | 加到 `PlaceholderTaskRecord`,推送时用原始绝对路径 |
| totalSize 来源 | 首次 fstat logicalSize,Execute 回填后用 app totalSize |
| 权限 | 客户端和 SA 均先检查 `ohos.permission.ACCESS_CLOUD_DISK_INFO`,再检查系统应用身份;HAP 须为系统应用,native token 仍须具有权限(§6) |
| 特性开关 | `dfs_service_feature_enable_cloud_disk`/`SUPPORT_CLOUD_DISK_SERVICE` |
| 前置依赖 | 水合主体 SDD + Execute SDD + 回调表基础设施 + v8 规格 |

## 4. 数据结构

### 4.1 ArkTS 类型(PR #35509 变更后)

```typescript
// 新增
enum HydratePriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
}

// 修改:删 FAILED,加 PENDING + CANCELLED
enum HydrateProgressState {
    PENDING = 0,      // 新增:task 已创建,FFRT worker 尚未 dispatch
    IN_PROGRESS = 1,  // one-way FETCH 已投递,等待异步 Execute
    COMPLETED = 2,
    CANCELLED = 3,    // 主动取消/空闲超时/provider death/注销/用户切换/SA停止/不可恢复内部终止
}

// 不变
interface HydrateProgress {
    filePath: string;         // 原始绝对路径(与 hydratePlaceholder 传入的一致)
    state: HydrateProgressState;
    processedSize: number;    // 已下载字节数(IN_PROGRESS 时有效)
    totalSize: number;        // 文件总大小
}

// 修改:hydratePlaceholder 加 priority 参
class CloudDiskSystemAccessor {
    constructor();
    on(type: 'hydrateProgress', callback: Callback<HydrateProgress>): void;
    off(type: 'hydrateProgress', callback?: Callback<HydrateProgress>): void;
    hydratePlaceholder(filePath: string, callbackType: CallbackType, priority: HydratePriority): Promise<void>;
    dehydrateFile(filePath: string): Promise<void>;
}
```

### 4.2 SA 侧 HydrateProgress Parcelable(新增)

```cpp
// cloud_disk_common.h (dfs inner API)
struct HydrateProgress final : public Parcelable {
    std::string filePath;         // 原始绝对路径
    int32_t state = 0;           // HydrateProgressState
    uint64_t processedSize = 0;  // cachedSize
    uint64_t totalSize = 0;      // logicalSize 或 app 回填

    bool Marshalling(Parcel &parcel) const override;
    static HydrateProgress *Unmarshalling(Parcel &parcel);
};
```

Marshalling 顺序:filePath → state → processedSize → totalSize。

### 4.3 PlaceholderTaskRecord 扩展(修正 hydrate-main SDD §5.2)

```cpp
struct PlaceholderTaskRecord {
    // ... 既有字段 ...
    uint64_t cachedSize = 0;      // 整次成功 Execute 的 size 有界累加
    std::string absolutePath;     // 新增(A12):推送进度时用原始绝对路径
};
```

### 4.4 ICloudDiskProgressCallback broker(新增)

```cpp
// i_cloud_disk_progress_callback.h (dfs inner API)
class ICloudDiskProgressCallback : public IRemoteBroker {
public:
    enum {
        SERVICE_CMD_ON_PROGRESS = 0,
    };
    virtual void OnProgress(const HydrateProgress &progress) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Filemanagement.Dfs.ICloudDiskProgressCallback")
};
```

## 5. 行为语义

### 5.1 NAPI 层(ufs 仓 `cloud_disk_system_access_n_exporter.cpp`)

**`CloudDiskSystemAccessor` 构造**:
```
new CloudDiskSystemAccessor()
  → NClass::SetEntityFor<CloudDiskSystemManager>(env, this, make_unique<CloudDiskSystemManager>())
```

**`hydratePlaceholder(filePath, callbackType, priority)`**:
```
NAPI: ParseStringArg(filePath) + ParseEnumArg(callbackType) + ParseEnumArg(priority)
  → CloudDiskSystemManager::HydratePlaceholder(filePath, callbackType, priority)
  → serviceProxy->StartHydrationByPathInner(filePath, callbackType, priority)
  → NAsyncWorkPromise (Promise<void>)
```

**`dehydrateFile(filePath)`**:
```
NAPI: ParseStringArg(filePath)
  → CloudDiskSystemManager::DehydrateFile(filePath)
  → serviceProxy->DehydrateFileByPathInner(filePath)
  → NAsyncWorkPromise (Promise<void>)
```

**`on('hydrateProgress', callback)`**:
```
NAPI: ParseCallbackArg(callback) → napi_ref
  → ProgressCallbackStub(stub, holds napi_ref)
  → CloudDiskSystemManager::RegisterProgressCallback(stub)
  → serviceProxy->RegisterProgressCallbackInner(stub)
  → SA PlaceholderProgressManager::Register(proxy)
  → 挂 SvcDeathRecipient
```

**`off('hydrateProgress', callback?)`**:
```
NAPI: 若有 callback 参 → 找对应 napi_ref 删除
      若无 callback 参 → 删除所有 napi_ref
  → CloudDiskSystemManager::UnregisterProgressCallback(callback?)
  → serviceProxy->UnregisterProgressCallbackInner()
  → SA PlaceholderProgressManager::Unregister(proxy or all)
  → 移除 death recipient
```

### 5.2 SA 侧 StartHydrationByPathInner(cloud_disk_service.cpp)

```
StartHydrationByPathInner(absolutePath, type, priority)
  → CheckSystemAccessorPermission()
    缺少 ACCESS_CLOUD_DISK_INFO → E_PERMISSION_DENIED(201)
    有权限但不满足系统应用身份 → E_PERMISSION_SYSTEM(202)
  → 校验 type / priority 枚举;非法 → E_INVALID_ARG(34400001)
  → ResolveSystemAccessorPath(absolutePath, context, relativePath)
    路径格式非法 → E_INVALID_ARG(34400001)
    按调用者 userId 映射物理路径,限定在该用户的 Docs 根内
    选择包含目标路径的最长已注册 syncFolder → 拆出 (syncFolder, relativePath)
    context.bundleName 使用目录登记的 provider,不与文件管理器 bundleName 比较
    用户、目录或路径映射解析失败 → E_CALLBACK_NOT_REGISTERED(安全脱敏)
  → IsCallbackRegistered(provider bundleName, syncFolderIndex)
    未注册 → E_CALLBACK_NOT_REGISTERED
  → CANCEL_FETCH_DATA:PlaceholderTaskManager::CancelTask(...)
    FETCH_DATA:CreateHydrationTask(context, relativePath, priority)
    task 保存 absolutePath,后续由 provider 以自身身份 Execute 回写
```

FETCH_DATA 分支在 callback、路径/state、per-file 护栏和容量校验通过并创建 task 后立即完成 Promise。
每应用5个或全局10个 active task 已满时返回
`E_HYDRATION_TASK_LIMIT_REACHED(34400034)`。Promise resolve 仅表示 task 已入队；provider callback
是否送达及最终水合结果通过进度回调观察。

`DehydrateFileByPathInner(absolutePath)` 同理:先 `CheckSystemAccessorPermission`,再 `ResolveSystemAccessorPath`,最后调用 `DehydratePlaceholderFile`;仍保留 provider 的 `DEHYDRATE` 回调授权。不能直接委托网盘侧 `DehydrateInner`,否则会错误地把文件管理器当作目录所有者。

### 5.3 PlaceholderProgressManager(新建)

```
PlaceholderProgressManager(singleton):
  - subscribers_: 按 {callingFullTokenId, callingPid} 保存 userId、callback、death recipient、active
  - lastProgress_: 按 reqKey 保存最近一次推送的 state 和时间

  Register(key, userId, proxy):
    校验注册参数 → 挂 death recipient → 保存当前调用者的订阅

  Unregister(key):
    仅注销对应调用者的订阅,标记不再接收后续进度并移除 death recipient

  OnRemoteDied(remoteObject):
    清理对应订阅,不影响其他调用者

  OnTaskProgress(reqKey, userId, progress):  ← TaskManager 调,携带 task 所属用户
    节流判断:
      if progress.state == COMPLETED || progress.state == CANCELLED → 必推并清理该 reqKey 的节流记录
      else if 状态相同且距上次推送不足 500ms → 跳过
      else → 推送并更新 lastProgress_
    仅选取订阅 userId == task userId 的有效 subscriber:
      proxy->OnProgress(progress)  ← IPC 到同用户的 file manager
```

订阅按 `{callingFullTokenId, callingPid}` 隔离,保存订阅者 `userId`;注销只影响该调用者的订阅。上面的流程仅示意推送行为,实际通过串行队列派发。

### 5.4 TaskManager 推送调用点

| 事件 | 调用位置 | 推送内容 |
| --- | --- | --- |
| task 创建(PENDING) | `CreateHydrateTask` 末尾 | `{absolutePath, PENDING, 0, logicalSize}` |
| one-way FETCH 投递成功(IN_PROGRESS) | callback proxy 接受 FETCH 后 | `{absolutePath, IN_PROGRESS, 0, totalSize}` |
| Execute 成功(cachedSize 增长) | 非最终请求全部步骤成功后 | `{absolutePath, IN_PROGRESS, cachedSize, totalSize}` |
| task 完成(COMPLETED) | 最终 `pwrite→fsync→非空文件确保state=2→state=3` 均成功后 | `{absolutePath, COMPLETED, cachedSize, totalSize}` |
| task 取消(CANCELLED) | 主动取消/timeout/death/internal terminal | `{absolutePath, CANCELLED, cachedSize, totalSize}` |

注意:COMPLETED/CANCELLED 推送在 task erase **之前**(读 record 后推,再 erase)。

### 5.5 进度回调生命周期

```
file manager 启动:
  accessor = new CloudDiskSystemAccessor()
  accessor.on('hydrateProgress', cb)  → 注册 progress callback

用户点开文件:
  accessor.hydratePlaceholder('/abs/path', FETCH_DATA, HIGH)
  → SA 创建 task → 推送 PENDING → Promise resolve
  → one-way FETCH 投递成功 → 推送 IN_PROGRESS
  → provider callback 深拷贝后返回,异步下载并调用 Execute
  → Execute 成功 → 推送 IN_PROGRESS (节流)
  → 最终 Execute 完成 → 推送 COMPLETED

file manager 退出:
  accessor.off('hydrateProgress')  → 注销
  → SA 移除 progress callback proxy
```

## 6. 安全约束

1. **双侧鉴权**:`hydratePlaceholder`(含取消)、`dehydrateFile`、`on`、`off` 在 NAPI 入口调用 UFS `CloudDiskSystemManager::CheckAccess`;DFS 的四个 IPC 入口再调用 `CheckSystemAccessorPermission`,不可绕过客户端直调 IPC 获得能力。两侧顺序一致:先检查 `ohos.permission.ACCESS_CLOUD_DISK_INFO`(失败 201),再检查系统应用身份(失败 202)。构造函数只创建访问器,不替代方法调用时的鉴权。
2. **路径解析**:`StartHydrationByPathInner` / `DehydrateFileByPathInner` 在 SA 侧用 `ResolveSystemAccessorPath` 解析绝对路径,限定当前用户的 Docs 根并匹配已注册同步目录。使用目录登记的 provider bundleName,不要求与文件管理器匹配。路径格式非法返回 `E_INVALID_ARG`(34400001);用户解析或目录/路径映射失败返回 `E_CALLBACK_NOT_REGISTERED`(34400021);解析后的文件状态、I/O 等错误按具体操作返回。
3. **progress callback 不可伪造**:`RegisterProgressCallbackInner` 的 callback proxy 由 SA 持有,只有 SA 能调 `OnProgress`。file manager 无法伪造进度推送。
4. **@systemapi 身份判定**:HAP token 必须通过 `IsSystemAppByFullTokenID`;native token 可通过身份检查但仍须具有上述权限;shell token 不放行。`@systemapi` 标注有客户端和 SA 的运行时检查支撑。
5. **与网盘接口分离**:本节系统接口的权限与身份要求不变。网盘侧 `StartHydrationInner` / `CancelHydrationInner` / `ExecuteInner` / `DehydrateInner` 不要求上述系统权限或系统应用身份,但仍校验调用者对目录/任务的归属,见 [spec §5](../agent-knowledge/clouddisk-placeholder-spec.md)。
6. **进度隔离**:订阅按调用者 full token ID + pid 管理,只接收相同 userId 的 task 进度;全文件订阅不等于跨用户订阅。

## 7. ABI 兼容

- `@since 26 dynamic`,`@systemapi`,clean break。
- NAPI 新增 `CloudDiskSystemAccessor` 类(新增 NAPI 属性,不改既有 `CloudDiskAccessor`)。
- SA 新增 4 个 IDL 方法(`StartHydrationByPathInner`/`DehydrateFileByPathInner`/`RegisterProgressCallbackInner`/`UnregisterProgressCallbackInner`),新增接口码(IDL 自动生成)。
- 新增 `ICloudDiskProgressCallback` broker + `HydrateProgress` Parcelable,新增导出符号。
- `PlaceholderTaskRecord` 加 `cachedSize` + `absolutePath` 字段(内部结构,不影响 ABI)。

## 8. 实现落点

### 8.1 NAPI 层(ufs 仓)

| 文件 | 改动 |
| --- | --- |
| `interfaces/kits/native/clouddiskmanager/include/cloud_disk_system_access_n_exporter.h` | **新建**:`CloudDiskSystemNExporter` 类 |
| `interfaces/kits/native/clouddiskmanager/src/cloud_disk_system_access_n_exporter.cpp` | **新建**:`CloudDiskSystemAccessor` 类 + `Constructor`/`HydratePlaceholder`/`DehydrateFile`/`On`/`Off` NAPI 实现 |
| `interfaces/kits/native/clouddiskmanager/include/progress_callback_stub.h` | **新建**:`ProgressCallbackStub`(持 `napi_ref`,收到 `OnProgress` 调 JS callback) |
| `interfaces/kits/native/clouddiskmanager/src/progress_callback_stub.cpp` | **新建** |
| `interfaces/kits/native/clouddiskmanager/module.cpp | 注册 `CloudDiskSystemNExporter` |
| `interfaces/kits/native/clouddiskmanager/BUILD.gn | 新增源文件 |

### 8.2 inner API 层(ufs 仓)

| 文件 | 改动 |
| --- | --- |
| `interfaces/inner_api/cloud_disk_kit_inner/include/cloud_disk_system_manager.h` | **新建**:`CloudDiskSystemManager` 类(`HydratePlaceholder`/`DehydrateFile`/`RegisterProgressCallback`/`UnregisterProgressCallback`) |
| `interfaces/inner_api/cloud_disk_kit_inner/src/cloud_disk_system_manager.cpp` | **新建**:走 serviceProxy 调 SA |
| `interfaces/inner_api/cloud_disk_kit_inner/BUILD.gn` | 新增源文件 |

### 8.3 dfs inner API 层

| 文件 | 改动 |
| --- | --- |
| `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_common.h` | 新增 `HydrateProgress` Parcelable + `ICloudDiskProgressCallback` broker |
| `interfaces/inner_api/native/clouddiskservice_kit_inner/i_cloud_disk_progress_callback.h` | **新建**:broker 接口 |
| `interfaces/inner_api/native/clouddiskservice_kit_inner/cloud_disk_service_manager.h` | 新增 `RegisterProgressCallback`/`UnregisterProgressCallback`/`StartHydrationByPath`/`DehydrateFileByPath` 虚函数 |
| `interfaces/inner_api/native/clouddiskservice_kit_inner/clouddiskservice_kit_inner.map` | 新增 `*HydrateProgress*`/`*ICloudDiskProgressCallback*` 导出符号 |
| `interfaces/inner_api/native/clouddiskservice_kit_inner/BUILD.gn` | 新增 header |

### 8.4 framework 层(dfs)

| 文件 | 改动 |
| --- | --- |
| `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_common.cpp` | `HydrateProgress::Marshalling`/`Unmarshalling` |
| `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_service_manager_impl.cpp` | `RegisterProgressCallback`/`UnregisterProgressCallback`/`StartHydrationByPath`/`DehydrateFileByPath` impl |
| `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_progress_callback_proxy.h` | **新建**:`CloudDiskProgressCallbackProxy`(`OnProgress` → IPC `SendRequest`) |
| `frameworks/native/clouddiskservice_kit_inner/src/cloud_disk_progress_callback_proxy.cpp` | **新建** |

### 8.5 IDL 层(dfs)

| 文件 | 改动 |
| --- | --- |
| `services/clouddiskservice/ICloudDiskService.idl` | 新增 `StartHydrationByPathInner([in] String absolutePath, [in] int32 type, [in] int32 priority)`、`DehydrateFileByPathInner([in] String absolutePath)`、`RegisterProgressCallbackInner([in] ICloudDiskProgressCallback callback)`、`UnregisterProgressCallbackInner()` |

### 8.6 service 层(dfs)

| 文件 | 改动 |
| --- | --- |
| `services/clouddiskservice/ipc/include/cloud_disk_service.h` | 新增 4 个 override 声明 |
| `services/clouddiskservice/ipc/src/cloud_disk_service.cpp` | 4 个 override 均先 `CheckSystemAccessorPermission`;按路径操作由文件内静态 helper `ResolveSystemAccessorPath` 解析用户和 provider,再进入任务/脱水逻辑 |
| `services/clouddiskservice/ipc/include/placeholder_progress_manager.h` | **新建**:`PlaceholderProgressManager` |
| `services/clouddiskservice/ipc/src/placeholder_progress_manager.cpp` | **新建**:Register/Unregister/OnTaskProgress/OnRemoteDied/节流 |
| `services/clouddiskservice/ipc/include/placeholder_task_manager.h` | `PlaceholderTaskRecord` 加 `cachedSize`/`absolutePath`;新增 `GetProgressInfo(reqKey, progress)` 供 ProgressManager 读 |
| `services/clouddiskservice/ipc/src/placeholder_task_manager.cpp` | 成功 Execute 后有界累加 `cachedSize`;`CreateHydrateTask` 存 `absolutePath`;状态变化点调 `ProgressManager::OnTaskProgress(reqKey)` |
| `services/clouddiskservice/BUILD.gn` | 新增 `placeholder_progress_manager.cpp` |

### 8.7 测试层

| 文件 | 改动 |
| --- | --- |
| `test/unittests/clouddiskservice/ipc/PlaceholderProgressTest.cpp` | 扩展:Register/Unregister/OnTaskProgress/节流/OnRemoteDied |
| `test/unittests/clouddiskservice/ipc/PlaceholderTaskManagerTest.cpp` | 扩展:cachedSize 累加/absolutePath 存储/OnTaskProgress 调用验证 |
| `test/unittests/clouddiskservice/ipc/cloud_disk_service_static_test.cpp` | SystemAccessorPermission/SystemAccessorPath 等用例:系统权限、系统应用身份、路径合法性及已注册目录解析 |
| `test/unittests/clouddiskservice/ipc/HydrateProgressParcelTest.cpp` | 扩展:HydrateProgress Marshalling/Unmarshalling |
| ufs 仓 NAPI 测试 | `CloudDiskSystemAccessor` 构造/hydratePlaceholder/dehydrateFile/on/off |

## 9. 测试要求

| 场景 | 覆盖点 |
| --- | --- |
| hydratePlaceholder 正常 | Promise resolve、progress callback 收到 PENDING→IN_PROGRESS→COMPLETED |
| Promise 完成语义 | task 入队即 resolve；provider 尚未收到 FETCH 或尚未完成时不得延迟 Promise |
| hydratePlaceholder 取消 | Promise resolve、progress callback 收到 CANCELLED |
| 空闲超时 | 5分钟无成功非空 Execute 后收到 CANCELLED；成功非空 Execute 刷新期限 |
| task 限额 | 每应用第6个或全局第11个任务返回 34400034；完成/取消后释放名额 |
| dehydrateFile 正常 | Promise resolve |
| on/off 基本流程 | on 注册→收进度→off 注销→不再收 |
| off 无 callback 参 | 注销全部 callback |
| off 指定 callback | 只注销指定 callback |
| 进度推送节流 | 500ms 内多次 cachedSize 增长只推一次;末次必推 |
| 推送当前用户所有 task | 同用户 CAPI 发起的 task 也推送进度,其他用户的 task 不推送 |
| filePath 回传 | progress.filePath == hydratePlaceholder 传入的绝对路径 |
| file manager 崩溃 | death recipient 触发→progress callback proxy 清理 |
| 绝对路径解析 | StartHydrationByPathInner 在当前用户 Docs 范围内选择最长匹配的已注册 syncFolder |
| 文件管理器与 provider bundleName 不同 | 系统权限与身份满足、当前用户目录已注册时允许解析;后续仍检查回调/文件/任务状态 |
| 路径格式非法 | `E_INVALID_ARG`(34400001),包括非规定沙箱前缀及 `..` / `.` 路径段 |
| 用户/目录/路径映射解析失败 | `E_CALLBACK_NOT_REGISTERED`(安全脱敏) |
| 缺少系统权限 | 201,且不继续系统应用身份检查 |
| 有权限但非系统 HAP | 202 |
| native / shell token | native 仍检查权限;shell 拒绝 |
| 多 subscriber | 两个 on 注册→都收到进度 |
| HydrateProgress parcel 往返 | Marshalling→Unmarshalling 一致 |

## 10. 实现和评审注意

- **PR #35509 变更**:加 `HydratePriority` enum、`hydratePlaceholder` 加 priority 参、`HydrateProgressState` 删 FAILED 加 PENDING+CANCELLED；走 interface_sdk-js 评审。
- **新增 SA IDL 接口码**:`StartHydrationByPathInner`/`DehydrateFileByPathInner`/`RegisterProgressCallbackInner`/`UnregisterProgressCallbackInner`；走 CODEOWNERS 评审。
- **新增 `ICloudDiskProgressCallback` broker**:与 provider callback table 保持接口和生命周期隔离。
- **节流**:固定 500ms，COMPLETED/CANCELLED 不节流。
- **跨仓改动**:ufs 仓(NAPI + inner API)和 dfs 仓(SA)的接口、实现、测试必须成组同步。

## 11. 后续扩展

1. **per-file 订阅**:当前在用户范围内全文件订阅,后续可加 `on('hydrateProgress', filePath, callback)` 按 filePath 过滤,不改变用户隔离。
2. **进度查询接口**:file manager 未 `on` 时主动查 `GetHydrateStatus(filePath)`。
3. **FAILED 态**:当前无 FAILED；pwrite/fsync/xattr 的可重试失败保持 IN_PROGRESS。后续如定义不可重试业务失败,可再评估拆分 FAILED。
4. **dehydrate 进度**:当前 progress 只推送水合进度,脱水是同步单次 IPC 无进度。后续脱水批量化时可加脱水进度。
5. **DFX 打点**:进度推送次数/延迟/file manager 响应时间→纳入 `clouddiskservice-dfx-sdd`。

## 12. 与既有 SDD 的关系

| 既有 SDD | 本 SDD 的关系 |
| --- | --- |
| hydrate-main SDD §5.2 `PlaceholderTaskRecord` | 本 SDD 加 `cachedSize` + `absolutePath` 字段(修正 E3"v1 不加"→"v1 加") |
| Execute SDD E3 "fd 合并到 PlaceholderTaskRecord" | 本 SDD 确认不变,在 record 上叠加 `cachedSize`/`absolutePath` |
| Execute SDD E5 "death recipient 恢复" | 本 SDD 的 progress callback death recipient 同范式 |
| Execute SDD E6 "安全脱敏" | `ResolveSystemAccessorPath` 的用户/目录/路径映射失败同样脱敏;路径格式错误仍返回 `E_INVALID_ARG` |
| hydrate-main SDD D10 "跨进程通知后续单独设计" | **本 SDD 即是该设计的落地** |
| v8 §5.2 Execute | 本 SDD 不改 Execute 规格,新增 ArkTS API 规格补充到 v8 |
