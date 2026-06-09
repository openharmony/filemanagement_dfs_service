# utils Agent Notes

`utils/` 是共享工具库。改这里的影响面通常比单个服务更大。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `BUILD.gn` | 构建 `libdistributedfileutils`、`libdistributedfiledentry`、`libdistributedfileutils_lite`。 |
| `log/` | 日志、错误码、metrics。 |
| `system/` | 账号、目录、sys utils、access token、fd/memory guard、timer。 |
| `dentry/` | meta file、cloud disk meta file、file utils。 |
| `cloud_disk/` | cloud file utils。 |
| `clouddiskservice/` | CloudDiskService 错误定义。 |
| `work/` | async work。 |
| `ffrt/` | FFRT timer。 |
| `load/` | plugin loader。 |
| `ioctl/` | FUSE ioctl 头。 |
| `inner_api/` | 面向内部使用的轻量公共头。 |

## 修改约束

- 共享错误码和日志改动要检查所有调用方，不要只修一个服务。
- RAII guard、timer、async work 改动必须补 utils 单测。
- dentry/metafile 逻辑涉及文件系统元数据，改动要跑 cloud_disk 和 clouddisk_service 相关测试。
- `libdistributedfileutils_lite` 依赖更少，给 lite/daemon 场景使用；不要随意引入重外部依赖。
- 新增公共工具前先搜索是否已有同类实现，避免重复封装。

## 测试路由

- `test/unittests/utils/`。
- dentry/cloud disk 使用方：`test/unittests/cloud_disk/`。
- system 工具：`test/unittests/utils/system/`。
