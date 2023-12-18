/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_MESSAGE_HANDLER_H
#define OHOS_FILEMGMT_MESSAGE_HANDLER_H

#include <string>

namespace OHOS::FileManagement::CloudSync {
constexpr int32_t NETWORK_ID_SIZE_MAX = 65;

template<typename T>
constexpr T GET_ALIGNED_SIZE(T x, int alignWidth)
{
    return (x + (alignWidth - 1)) & ~(alignWidth - 1);
}

#pragma pack(1)
struct MessageHeader {
    uint8_t magic;
    uint8_t version;
    uint16_t msgType;
    uint32_t dataLen;
    int32_t errorCode;
};

struct SessionDeviceInfo {
    char sourceNetworkId[NETWORK_ID_SIZE_MAX];
    char tartgeNetworkId[NETWORK_ID_SIZE_MAX];
};

struct UserData {
    uint64_t taskId;
    uint32_t userId;
    uint32_t uriLen;
    char uri[0];
};
#pragma pack()

enum MsgType : uint16_t {
    MSG_DOWNLOAD_FILE_REQ = 0,
    MSG_DOWNLOAD_FILE_RSP,
    MSG_FINISH_FILE_RECV,
};

struct MessageInputInfo {
    std::string srcNetworkId;
    std::string dstNetworkId;
    std::string uri;
    uint16_t msgType;
    int32_t errorCode;
    int32_t userId;
    uint64_t taskId;
};

class MessageHandler {
public:
    MessageHandler() = default;
    MessageHandler(MessageInputInfo &info);

    bool PackData(uint8_t *data, uint32_t totalLen);
    bool UnPackData(uint8_t *data, uint32_t totalLen);
    void GetMessageHeader(MessageHeader &header);
    uint32_t GetDataSize();
    uint64_t GetTaskId();
    uint16_t GetMsgType();
    int32_t GetUserId();
    int32_t GetErrorCode();

    std::string GetUri();
    std::string GetSrcNetworkId();
    std::string GetDstNetworkId();

private:
    MessageHeader msgHdr_{0};
    std::string srcNetworkId_;
    std::string dstNetworkId_;
    std::string uri_;
    int32_t userId_{0};
    uint64_t taskId_{0};
    uint32_t dataSize_{0};
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_MESSAGE_HANDLER_H