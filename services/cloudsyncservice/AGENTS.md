# cloudsyncservice Agent Notes

`cloudsyncservice` 是【云同步】模块，也是 CloudSync System Ability。它连接 CloudSync inner API、CloudFileKit、同步规则、周期任务、文件类资产传输和回调；SoftBus 只作为端端辅助同步能力使用。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `ICloudSyncService.idl` | CloudSync IPC IDL，生成 proxy/stub。 |
| `BUILD.gn` | 构建 `cloudsync_sa`、proxy/stub source set 和 static 版本。 |
| `include/ipc/`、`src/ipc/` | SA 入口、账号状态、下载资产 callback manager/proxy。 |
| `include/sync_rule/`、`src/sync_rule/` | 电量、屏幕、用户状态监听。 |
| `include/cycle_task/`、`src/cycle_task/` | 周期任务和 task runner。 |
| `include/transport/`、`src/transport/` | 端端辅助 file transfer、message handler、SoftBus session 管理。 |

## 主要链路

```text
CloudSyncManager / CloudSyncAssetManager
  -> frameworks/native/cloudsync_kit_inner ServiceProxy
  -> ICloudSyncService.idl generated stub
  -> CloudSyncService
  -> CloudFile::DataSyncManager
  -> sync rules / cycle tasks / callbacks / transport
```

## 先读文件

- IPC 入口：`include/ipc/cloud_sync_service.h`、`src/ipc/cloud_sync_service.cpp`。
- IDL 契约：`ICloudSyncService.idl`。
- 客户端契约：`interfaces/inner_api/native/cloudsync_kit_inner/cloud_sync_manager.h`、`cloud_sync_asset_manager.h`。
- 数据同步核心：`interfaces/inner_api/native/cloud_file_kit_inner/data_sync_manager.h`、`frameworks/native/cloud_file_kit_inner/src/data_sync_manager.cpp`。
- 周期任务：`src/cycle_task/cycle_task_runner.cpp` 和 `src/cycle_task/tasks/*.cpp`。
- 传输：`src/transport/file_transfer_manager.cpp`、`src/transport/message_handler.cpp`、`src/transport/softbus/`。

## 修改约束

- IDL 变更要同步 frameworks proxy/manager、service 实现、mock 和测试。
- `cloud_file_sync_service_interface_code.h` 属于 CODEOWNERS 关注文件，改动前先读根 `CODEOWNERS`。
- 同步规则变更必须考虑多用户、网络切换、电量/温控、屏幕和云开关组合状态。
- 周期任务要保证幂等和可重复启动；新增任务要注册到 runner 并加入 BUILD source list。
- SoftBus 相关逻辑仅用于端端辅助同步，受 `dfs_service_feature_enable_distributed_ability` 影响，BUILD 和代码条件要一致。

## 测试路由

- SA 和 transport：`test/unittests/cloudsync_sa/`。
- API/manager：`test/unittests/cloudsync_api/`、`test/unittests/ani_core/`。
- DataSyncManager：`test/unittests/data_sync_manager/`。
- 传输细节：`test/unittests/cloudsync_sa/transport/`。
