# DFS Service 编译和测试

本仓是 OpenHarmony 子仓，通常需要在 OpenHarmony 源码根目录执行构建，而不是只在本目录运行普通 CMake/Make。

## 常用目标

常用目标来自根 `BUILD.gn`：

| 目标 | 用途 |
| --- | --- |
| `services_target` | 构建主要服务，包括 cloudsync、cloudfiledaemon、clouddisk_database，按特性开关包含 clouddiskservice 和 distributedfiledaemon。 |
| `cloudsync_kit_inner_target` | 构建 CloudSync inner kit。 |
| `cloud_daemon_kit_inner_target` | 构建 CloudDaemon inner kit。 |
| `cloud_file_kit_inner_target` | 构建 CloudFileKit。 |
| `clouddiskservice_kit_inner_target` | 构建 CloudDiskService inner kit。 |
| `cloudsync_asset_kit_inner_target` | 构建 CloudSync asset kit。 |
| `cloudsync_test_module` | 聚合 cloudsync 相关单元测试。 |
| `distributedfile_test_module` | 聚合 distributed file 相关单元测试。 |
| `dfs_test_moudule` | 历史拼写的 distributed file daemon 模块测试 group。 |

## 示例命令

示例命令需要按产品名替换：

```bash
./build.sh --product-name <product> --build-target services_target
./build.sh --product-name <product> --build-target cloudsync_test_module
./build.sh --product-name <product> --build-target distributedfile_test_module
```

## 更细测试入口

- `test/unittests/BUILD.gn` 的 `cloudsyncunittests`。
- `test/unittests/BUILD.gn` 的 `distributedfileunittests`。
- 各测试子目录自己的 `BUILD.gn`，例如 `test/unittests/cloud_disk/BUILD.gn`、`test/unittests/clouddisk_database/BUILD.gn`。

## 文档类改动校验

```powershell
git diff --check
Get-ChildItem -Recurse -Filter AGENTS.md | Select-Object FullName
```

代码类改动还要构建对应 GN target，并运行同模块测试。
