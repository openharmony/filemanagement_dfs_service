# dfs_utils Agent Notes

`dfs_utils/` 是分布式文件相关的较底层 DFS 公共抽象，主要服务于线程、actor、singleton、mount argument 和 radar 等基础能力。当前知识库不深挖，默认不作为改动范围。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `include/block_object.h` | 阻塞对象/同步基础。 |
| `include/dfsu_actor.h` | actor 抽象。 |
| `include/dfsu_thread*.h` | 线程和线程安全队列。 |
| `include/dfsu_singleton.h` | singleton 辅助。 |
| `include/dfsu_mount_argument_descriptors.h` | mount 参数描述。 |
| `include/dfs_radar.h`、`src/dfs_radar.cpp` | radar/dfx 上报辅助。 |

## 修改约束

- 这里的类型通常被多个服务复用；除非任务明确要求或当前改动必须触及，否则不要新增或重构这里的 API。
- 并发基础设施改动必须关注死锁、线程退出、析构时阻塞和异常传播。
- 不要在这里引入具体服务业务依赖。
- 新增源文件同步 `dfs_utils/BUILD.gn`，并补 `test/unittests/utils/` 或对应调用方测试。
