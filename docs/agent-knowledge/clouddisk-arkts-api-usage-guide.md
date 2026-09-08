# CloudDisk ArkTS API 使用说明

本文面向文件管理器等系统应用，说明如何通过 `@ohos.file.cloudDiskManager` 使用占位文件的水合、取消水合、脱水和进度订阅能力。

> 设计基线：DFS `02cb9a095a0848e5d6e7b803cb0a853f74e5eb16` 及配套 UFS 实现。接口计划自 API 26 dynamic 提供；当前工作区的 `interface_sdk-js` 主干声明尚未包含 `CloudDiskSystemAccessor`，使用前需确认对应 SDK 接口声明已合入。

## 1. 使用条件

调用方必须同时满足：

1. 是系统应用，并申请 `ohos.permission.ACCESS_CLOUD_DISK_INFO`。
2. 设备支持 `SystemCapability.FileManagement.CloudDiskManager`，且 CloudDisk 特性已启用。
3. `filePath` 是 `/storage/Users/currentUser/` 下的文件绝对路径，且位于当前用户已注册的同步目录内。
4. 对应网盘应用已注册 callback table；真正的数据下载及脱水授权由网盘应用处理。

权限声明示例：

```json5
{
  "module": {
    "requestPermissions": [
      {
        "name": "ohos.permission.ACCESS_CLOUD_DISK_INFO"
      }
    ]
  }
}
```

权限在 UFS 客户端和 DFS 服务端都会检查。缺少权限返回 `201`；有权限但不是系统应用返回 `202`。构造 `CloudDiskSystemAccessor` 不代表已经通过鉴权，每个方法调用都会单独检查。

## 2. 接口速览

```typescript
class CloudDiskSystemAccessor {
  constructor();
  on(type: 'hydrateProgress', callback: Callback<HydrateProgress>): void;
  off(type: 'hydrateProgress', callback?: Callback<HydrateProgress>): void;
  hydratePlaceholder(
    filePath: string,
    callbackType: CallbackType,
    priority: HydratePriority
  ): Promise<void>;
  dehydrateFile(filePath: string): Promise<void>;
}
```

| 接口 | 用途 |
| --- | --- |
| `hydratePlaceholder(..., FETCH_DATA, priority)` | 发起整文件水合 |
| `hydratePlaceholder(..., CANCEL_FETCH_DATA, priority)` | 取消正在进行的水合；`priority` 仍须传入，但取消流程不使用它 |
| `dehydrateFile(filePath)` | 请求释放本地数据，保留占位文件和逻辑大小 |
| `on('hydrateProgress', callback)` | 订阅当前用户的水合进度 |
| `off('hydrateProgress', callback?)` | 注销指定回调；省略回调时注销当前访问器的全部进度回调 |

枚举值：

| 类型 | 值 |
| --- | --- |
| `CallbackType.FETCH_DATA` | `0` |
| `CallbackType.CANCEL_FETCH_DATA` | `1` |
| `HydratePriority.LOW / NORMAL / HIGH` | `0 / 1 / 2` |
| `HydrateProgressState.PENDING` | `0`，任务已创建、尚未调度 |
| `HydrateProgressState.IN_PROGRESS` | `1`，service 已投递网盘请求，等待异步下载/回写 |
| `HydrateProgressState.COMPLETED` | `2`，水合完成 |
| `HydrateProgressState.CANCELLED` | `3`，主动取消、空闲超时、网盘应用退出或不可恢复异常终止 |

`HydrateProgress` 包含：

```typescript
interface HydrateProgress {
  filePath: string;
  state: HydrateProgressState;
  processedSize: number;
  totalSize: number;
}
```

## 3. 完整示例

```typescript
import cloudDiskManager from '@ohos.file.cloudDiskManager';
import { BusinessError } from '@kit.BasicServicesKit';

const accessor = new cloudDiskManager.CloudDiskSystemAccessor();
const targetPath = '/storage/Users/currentUser/sync/report.pdf';

const progressCallback = (progress: cloudDiskManager.HydrateProgress): void => {
  // 订阅会收到当前用户所有水合任务的进度，调用方需要按路径过滤。
  if (progress.filePath !== targetPath) {
    return;
  }
  console.info(`state=${progress.state}, ` +
    `size=${progress.processedSize}/${progress.totalSize}`);
};

function printError(operation: string, error: BusinessError): void {
  console.error(`${operation} failed: code=${error.code}, message=${error.message}`);
}

export async function startHydration(): Promise<void> {
  try {
    // 建议先订阅，避免遗漏任务刚创建时的进度。
    accessor.on('hydrateProgress', progressCallback);
    await accessor.hydratePlaceholder(
      targetPath,
      cloudDiskManager.CallbackType.FETCH_DATA,
      cloudDiskManager.HydratePriority.HIGH
    );
    // Promise resolve 只表示任务已受理，不表示文件已经水合完成。
  } catch (error) {
    printError('startHydration', error as BusinessError);
  }
}

export async function cancelHydration(): Promise<void> {
  try {
    await accessor.hydratePlaceholder(
      targetPath,
      cloudDiskManager.CallbackType.CANCEL_FETCH_DATA,
      cloudDiskManager.HydratePriority.NORMAL
    );
  } catch (error) {
    printError('cancelHydration', error as BusinessError);
  }
}

export async function dehydrate(): Promise<void> {
  try {
    await accessor.dehydrateFile(targetPath);
  } catch (error) {
    printError('dehydrate', error as BusinessError);
  }
}

export function stopProgressListening(): void {
  try {
    accessor.off('hydrateProgress', progressCallback);
  } catch (error) {
    printError('stopProgressListening', error as BusinessError);
  }
}
```

## 4. 关键行为

- 水合是异步任务。`hydratePlaceholder()` 的 Promise resolve 只表示任务创建成功；以 `COMPLETED` 进度判断真正完成。
- 同一文件已有水合任务时，再次发起返回 `34400019`。优先级只影响排队顺序：`HIGH > NORMAL > LOW`，不会抢占正在执行的任务。
- 网盘应用收到请求后可以立即从 callback 返回，再异步下载和回写。每次成功的非空回写会刷新5分钟空闲超时，因此持续下载没有固定总时长上限。
- 每个网盘应用最多同时保留5个水合任务，服务全局最多10个；达到上限返回 `34400034`，任务完成或取消后释放名额。
- 取消操作不会触发脱水；没有进行中的任务时返回 `34400032`。
- 脱水会先由网盘应用确认云端数据可用。网盘拒绝时返回 `34400029`；部分水合文件不能脱水，返回 `34400028`。
- 进度订阅以当前用户为边界，但会包含该用户由其他入口发起的水合任务，因此必须按 `filePath` 过滤。非终态进度约按 500 ms 节流，`COMPLETED` 和 `CANCELLED` 必定推送。
- 文件管理器只需具有系统权限和身份，不要求其 bundleName 与网盘应用相同。服务会在当前用户范围内选择包含目标文件的最长已注册同步目录，并转交给该目录登记的网盘 provider。

## 5. 常用错误码

| 错误码 | 含义 / 处理建议 |
| --- | --- |
| `201` | 缺少 `ACCESS_CLOUD_DISK_INFO` 权限 |
| `202` | 调用方不是系统应用 |
| `801` | 当前设备或构建不支持该能力 |
| `34400001` | 参数或路径格式非法；检查参数数量、枚举值和路径前缀 |
| `34400014` | 临时失败，可稍后重试 |
| `34400017` | 目标不是占位文件 |
| `34400019` | 该文件已有水合任务 |
| `34400021` | 网盘 callback 未注册，或目标路径无法解析到当前用户的已注册同步目录 |
| `34400028` | 文件仅部分水合，不能脱水 |
| `34400029` | 网盘应用拒绝脱水 |
| `34400031` | 文件已经完全水合 |
| `34400032` | 没有可取消的水合任务 |
| `34400034` | 水合任务达到应用或服务容量上限；等待已有任务完成/取消后重试 |

## 6. 最新权限边界

ArkTS `CloudDiskSystemAccessor` 的四个方法仍要求系统权限和系统应用身份，且客户端、服务端双重校验。

网盘应用侧的 `StartHydrationInner`、`CancelHydrationInner`、`ExecuteInner`、`DehydrateInner` 不要求该系统权限，但仍校验调用者 userId、bundleName、同步目录归属、路径、callback 和任务信息。同步目录的注册/注销入口仍保留系统权限检查。

详细设计见 [Placeholder ArkTS API SDD](../sdd/placeholder-arkts-api-sdd.md)，完整规格见 [CloudDisk Placeholder 接口规格](clouddisk-placeholder-spec.md)。
