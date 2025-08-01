/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FILEMANAGEMENT_DFS_SERVICE_CONTROL_CMD_PARSER_H
#define FILEMANAGEMENT_DFS_SERVICE_CONTROL_CMD_PARSER_H

#include <atomic>
#include <functional>
#include <string>

namespace OHOS {
namespace Storage {
namespace DistributedFile {

static const char DETERMINE_DEVICE_TYPE_KEY[] = "persist.distributed_scene.sys_settings_data_sync";

enum ControlCmdType {
    CMD_UNKNOWN = 0,
    CMD_CHECK_ALLOW_CONNECT = 1,
    CMD_MSG_RESPONSE = 2,
    CMD_PUBLISH_NOTIFICATION = 3,
    CMD_CANCEL_NOTIFICATION = 4,
    CMD_ACTIVE_DISCONNECT = 5,
};

struct ControlCmd {
    static std::atomic<int32_t> nextMsgId;

    uint16_t version = 0;
    int32_t msgId = 0;
    int32_t msgType = 0;
    std::string msgBody = "";
    std::string networkId = "";

    ControlCmd() : msgId(nextMsgId.fetch_add(1, std::memory_order_relaxed)) {}
};

inline std::atomic<int32_t> ControlCmd::nextMsgId;

class ControlCmdParser {
public:
    static bool ParseFromJson(const std::string &jsonStr, ControlCmd &outCmd);
    static bool SerializeToJson(const ControlCmd &cmd, std::string &outJsonStr);

    static bool HandleRequest(const ControlCmd &inCmd, ControlCmd &outCmd);

    static bool CheckAllowConnect(const ControlCmd &inCmd, ControlCmd &outCmd);
    static bool PublishNotification(const ControlCmd &inCmd, ControlCmd &outCmd);
    static bool CancelNotification(const ControlCmd &inCmd, ControlCmd &outCmd);
    static bool DisconnectByRemote(const ControlCmd &inCmd, ControlCmd &outCmd);

    static void RegisterDisconnectCallback(std::function<void(std::string)> cb);

private:
    static std::function<void(std::string)> disconnectCallback_;
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_CONTROL_CMD_PARSER_H