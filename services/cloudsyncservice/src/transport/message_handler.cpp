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

#include "message_handler.h"

#include <endian.h>
#include <securec.h>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
constexpr uint8_t MSG_MAGIC = 0xF8;
constexpr uint8_t VERSION = 0x1;
constexpr int8_t ALIGN_WIDH = 8;
MessageHandler::MessageHandler(MessageInputInfo &info)
    : srcNetworkId_(info.srcNetworkId),
      dstNetworkId_(info.dstNetworkId),
      uri_(info.uri),
      userId_(info.userId),
      taskId_(info.taskId)
{
    msgHdr_.msgType = info.msgType;
    msgHdr_.errorCode = info.errorCode;
    auto totalSize = sizeof(MessageHeader) + sizeof(SessionDeviceInfo) + sizeof(UserData) + uri_.size();
    dataSize_ = GET_ALIGNED_SIZE(totalSize, ALIGN_WIDH);
}

bool MessageHandler::PackData(uint8_t *data, uint32_t totalLen)
{
    if (!data || totalLen < dataSize_) {
        LOGE("Invalid param, totalLen:%{public}d, dataSize:%{public}d", totalLen, dataSize_);
        return false;
    }

    uint8_t *ptr = data;
    MessageHeader *head = reinterpret_cast<MessageHeader *>(ptr);
    head->magic = MSG_MAGIC;
    head->version = VERSION;
    head->msgType = htole16(msgHdr_.msgType);
    head->dataLen = htole32(totalLen - sizeof(MessageHeader));
    head->errorCode = static_cast<int32_t>(htole32(msgHdr_.errorCode));

    ptr += sizeof(MessageHeader);
    SessionDeviceInfo *deviceInfo = reinterpret_cast<SessionDeviceInfo *>(ptr);
    auto ret = strcpy_s(deviceInfo->sourceNetworkId, NETWORK_ID_SIZE_MAX, srcNetworkId_.c_str());
    if (ret != 0) {
        LOGE("strcpy for source device id failed, ret is %{public}d", ret);
        return false;
    }

    ret = strcpy_s(deviceInfo->tartgeNetworkId, NETWORK_ID_SIZE_MAX, dstNetworkId_.c_str());
    if (ret != 0) {
        LOGE("strcpy for source device id failed, ret is %{public}d", ret);
        return false;
    }

    ptr += sizeof(SessionDeviceInfo);
    UserData *userData = reinterpret_cast<UserData *>(ptr);
    userData->taskId = htole64(taskId_);
    userData->userId = htole32(userId_);
    userData->uriLen = htole32(uri_.size()); // not include '\0'
    ptr += sizeof(UserData);

    uint8_t *end = data + totalLen;
    ret = memcpy_s(userData->uri, end - ptr, uri_.c_str(), uri_.size());
    if (ret != 0) {
        LOGE("memcpy failed, error:%{public}d", errno);
        return false;
    }
    return true;
}

bool MessageHandler::UnPackData(uint8_t *data, uint32_t totalLen)
{
    if (!data || totalLen < sizeof(MessageHeader)) {
        LOGE("invalid input data, totalLen:%{public}d", totalLen);
        return false;
    }

    uint8_t *ptr = data;
    uint32_t leftSize = totalLen;
    MessageHeader *head = reinterpret_cast<MessageHeader *>(ptr);
    msgHdr_.magic = head->magic;
    msgHdr_.version = head->version;
    msgHdr_.msgType = le16toh(head->msgType);
    msgHdr_.dataLen = le32toh(head->dataLen);
    msgHdr_.errorCode = static_cast<int32_t>(le32toh(head->errorCode));
    if (msgHdr_.magic != MSG_MAGIC) {
        LOGE("not valid data");
        return false;
    }

    leftSize -= sizeof(MessageHeader);
    if (leftSize < sizeof(SessionDeviceInfo)) {
        LOGE("failed to parse sessionDeviceInfo, leftsize:%{public}d", leftSize);
        return false;
    }
    ptr += sizeof(MessageHeader);
    SessionDeviceInfo *deviceInfo = reinterpret_cast<SessionDeviceInfo *>(ptr);
    srcNetworkId_ = string(deviceInfo->sourceNetworkId, strnlen(deviceInfo->sourceNetworkId, NETWORK_ID_SIZE_MAX));
    dstNetworkId_ = string(deviceInfo->tartgeNetworkId, strnlen(deviceInfo->tartgeNetworkId, NETWORK_ID_SIZE_MAX));

    leftSize -= sizeof(SessionDeviceInfo);
    if (leftSize < sizeof(UserData)) {
        LOGE("failed to parse UserData, leftsize:%{public}d", leftSize);
        return false;
    }
    ptr += sizeof(SessionDeviceInfo);
    UserData *userData = reinterpret_cast<UserData *>(ptr);
    taskId_ = le64toh(userData->taskId);
    userId_ = static_cast<int32_t>(le32toh(userData->userId));
    auto uriLen = userData->uriLen;
    if (uriLen > PATH_MAX) {
        LOGE("exception uriLen:%{public}d", uriLen);
        return false;
    }
    uri_ = string(userData->uri, uriLen);
    return true;
}

uint32_t MessageHandler::GetDataSize()
{
    return dataSize_;
}

void MessageHandler::GetMessageHeader(MessageHeader &header)
{
    header = msgHdr_;
}
uint64_t MessageHandler::GetTaskId()
{
    return taskId_;
}

int32_t MessageHandler::GetUserId()
{
    return userId_;
}

std::string MessageHandler::GetUri()
{
    return uri_;
}

std::string MessageHandler::GetSrcNetworkId()
{
    return srcNetworkId_;
}

std::string MessageHandler::GetDstNetworkId()
{
    return dstNetworkId_;
}

uint16_t MessageHandler::GetMsgType()
{
    return msgHdr_.msgType;
}

int32_t MessageHandler::GetErrorCode()
{
    return msgHdr_.errorCode;
}
} // namespace OHOS::FileManagement::CloudSync