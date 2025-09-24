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

#include <cstring>
#include <nlohmann/json.hpp>

#include "control_cmd_parser.h"
#include "dfs_error.h"
#include "parameter.h"
#include "system_notifier.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement;
using json = nlohmann::json;

std::function<void(std::string)> ControlCmdParser::disconnectCallback_;

bool ControlCmdParser::ParseFromJson(const std::string &jsonStr, ControlCmd &outCmd)
{
    if (jsonStr.empty() || !json::accept(jsonStr)) {
        LOGE("ParseFromJson: jsonStr is empty or invalid");
        return false;
    }

    json j = json::parse(jsonStr, nullptr, false);
    if (j.is_discarded()) {
        LOGE("ParseFromJson: jsonStr is discarded");
        return false;
    }

    // --- Required Fields Validation ---
    if (!j.contains("msgId") || !j["msgId"].is_number_integer()) {
        LOGE("ParseFromJson: msgId is missing or not an integer");
        return false;
    }
    outCmd.msgId = j["msgId"].get<int32_t>();

    if (!j.contains("msgType") || !j["msgType"].is_number_integer()) {
        LOGE("ParseFromJson: msgType is missing or not an integer");
        return false;
    }
    outCmd.msgType = j["msgType"].get<int32_t>();

    // --- Optional Fields (parse only if present and valid) ---
    if (j.contains("version") && j["version"].is_number_unsigned()) {
        outCmd.version = j["version"].get<uint16_t>();
    }

    if (j.contains("msgBody") && j["msgBody"].is_string()) {
        outCmd.msgBody = j["msgBody"].get<std::string>();
    }

    if (j.contains("networkId") && j["networkId"].is_string()) {
        outCmd.networkId = j["networkId"].get<std::string>();
    }
    return true;
}

bool ControlCmdParser::SerializeToJson(const ControlCmd &cmd, std::string &outJsonStr)
{
    json j;
    j["version"] = cmd.version;
    j["msgId"] = cmd.msgId;
    j["msgType"] = cmd.msgType;
    j["msgBody"] = cmd.msgBody;
    j["networkId"] = cmd.networkId;

    constexpr auto indent = -1;
    constexpr auto indent_char = ' ';
    constexpr auto ensure_ascii = false;
    constexpr auto error_handler = nlohmann::json::error_handler_t::replace;

    outJsonStr = j.dump(indent, indent_char, ensure_ascii, error_handler);
    return !outJsonStr.empty();
}

bool ControlCmdParser::HandleRequest(const ControlCmd &inCmd, ControlCmd &outCmd)
{
    LOGI("HandleRequest typecode = %{public}d", inCmd.msgType);
    switch (inCmd.msgType) {
        case ControlCmdType::CMD_CHECK_ALLOW_CONNECT:
            return CheckAllowConnect(inCmd, outCmd);
        case ControlCmdType::CMD_PUBLISH_NOTIFICATION:
            return PublishNotification(inCmd, outCmd);
        case ControlCmdType::CMD_CANCEL_NOTIFICATION:
            return CancelNotification(inCmd, outCmd);
        case ControlCmdType::CMD_ACTIVE_DISCONNECT:
            return DisconnectByRemote(inCmd, outCmd);
        default:
            LOGE("Cannot response request %{public}d: unknown transaction", inCmd.msgType);
            return false;
    }
}

bool ControlCmdParser::IsLocalItDevice()
{
    const char *syncType = "1";
    const int bufferLen = 10;
    char paramOutBuf[bufferLen] = {0};
    int ret = GetParameter(DETERMINE_DEVICE_TYPE_KEY, "", paramOutBuf, bufferLen);
    LOGI("paramOutBuf: %{public}s, ret: %{public}d", paramOutBuf, ret);
    if (ret > 0 && strncmp(paramOutBuf, syncType, strlen(syncType)) == 0) {
        LOGI("Determining the e2e device succeeded.");
        return true;
    }
    return false;
}

bool ControlCmdParser::CheckAllowConnect(const ControlCmd &inCmd, ControlCmd &outCmd)
{
    outCmd.msgType = ControlCmdType::CMD_MSG_RESPONSE;
    outCmd.msgId = inCmd.msgId;
    outCmd.msgBody = IsLocalItDevice() || SystemNotifier::GetInstance().GetNotificationMapSize() > 0 ? "true" : "false";
    return true;
}

bool ControlCmdParser::PublishNotification(const ControlCmd &inCmd, ControlCmd &outCmd)
{
    LOGI("publishNotification networkId = %{public}.6s", inCmd.networkId.c_str());
    auto ret = SystemNotifier::GetInstance().CreateNotification(inCmd.networkId);
    if (ret != E_OK) {
        LOGE("Failed to publish notification. ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool ControlCmdParser::CancelNotification(const ControlCmd &inCmd, ControlCmd &outCmd)
{
    LOGI("cancelNotification networkId = %{public}.6s", inCmd.networkId.c_str());
    auto ret = SystemNotifier::GetInstance().DestroyNotifyByNetworkId(inCmd.networkId);
    if (ret != E_OK) {
        LOGE("Failed to cancel notification. ret: %{public}d", ret);
        return false;
    }
    return true;
}

bool ControlCmdParser::DisconnectByRemote(const ControlCmd &inCmd, ControlCmd &outCmd)
{
    LOGI("DisconnectByRemote networkId = %{public}.6s", inCmd.networkId.c_str());
    if (disconnectCallback_ == nullptr) {
        LOGE("callback is null");
        return false;
    }
    disconnectCallback_(inCmd.networkId);
    return true;
}

void ControlCmdParser::RegisterDisconnectCallback(std::function<void(std::string)> cb)
{
    LOGI("start RegisterDisconnectCallback function.");
    disconnectCallback_ = cb;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS