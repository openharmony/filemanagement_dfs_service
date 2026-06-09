# interfaces Agent Notes

`interfaces/` 是 API 契约层。这里的改动会向 framework 和 service 扩散，必须谨慎。

## 目录地图

| 路径 | 作用 |
| --- | --- |
| `inner_api/native/` | 对内 C++ API、IPC broker、callback、Parcelable、接口码。 |

## 修改约束

- 改接口前先确认调用方、framework proxy/manager、service stub/实现和测试是否都要同步。
- 不要在接口层引入服务实现细节或具体 adapter 依赖。
- Parcelable 字段变更要保持读写顺序一致，新增字段要考虑兼容性。
- 接口码文件改动要检查根 `CODEOWNERS`。

继续读取：`interfaces/inner_api/native/AGENTS.md`。
