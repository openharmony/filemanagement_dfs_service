/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "filetransfermanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "cloud_fuzzer_helper.h"
#include "file_transfer_manager.h"
#include "task_state_manager.h"

namespace OHOS {
constexpr size_t U16_AT_SIZE = 2;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t U64_AT_SIZE = 8;

using namespace std;
using namespace OHOS::FileManagement::CloudSync;

bool DownloadFileFromRemoteDeviceFuzzTest(shared_ptr<FileTransferManager> fileTransferMgrPtr,
                                          FuzzData &fuzzData,
                                          size_t size)
{
    fuzzData.ResetData(size);
    int32_t userId = fuzzData.GetData<int32_t>();
    uint64_t taskId = fuzzData.GetData<uint64_t>();
    std::string networkId = fuzzData.GetStringFromData(NETWORK_ID_SIZE_MAX);
    int len = fuzzData.GetRemainSize();
    std::string uri = fuzzData.GetStringFromData(len);
    fileTransferMgrPtr->DownloadFileFromRemoteDevice(networkId, userId, taskId, uri);
    return true;
}

bool HandleDownloadFileRequestFuzzTest(shared_ptr<FileTransferManager> fileTransferMgrPtr,
                                       FuzzData &fuzzData,
                                       size_t size)
{
    fuzzData.ResetData(size);
    int32_t userId = fuzzData.GetData<int32_t>();
    uint64_t taskId = fuzzData.GetData<uint64_t>();
    int receiverSessionId = fuzzData.GetData<int>();
    std::string senderNetworkId = fuzzData.GetStringFromData(NETWORK_ID_SIZE_MAX);
    int len = fuzzData.GetRemainSize();
    std::string uri = fuzzData.GetStringFromData(len);
    MessageInputInfo msgInputInfo;
    msgInputInfo.userId = userId;
    msgInputInfo.taskId = taskId;
    msgInputInfo.uri = uri;
    MessageHandler msgHandle(msgInputInfo);
    fileTransferMgrPtr->HandleDownloadFileRequest(msgHandle, senderNetworkId, receiverSessionId);
    return true;
}

bool HandleDownloadFileResponseFuzzTest(shared_ptr<FileTransferManager> fileTransferMgrPtr,
                                        FuzzData &fuzzData,
                                        size_t size)
{
    fuzzData.ResetData(size);
    int32_t errorCode = fuzzData.GetData<int32_t>();
    uint64_t taskId = fuzzData.GetData<uint64_t>();
    int len = fuzzData.GetRemainSize();
    std::string uri = fuzzData.GetStringFromData(len);
    MessageInputInfo msgInputInfo;
    msgInputInfo.errorCode = errorCode;
    msgInputInfo.taskId = taskId;
    msgInputInfo.uri = uri;
    MessageHandler msgHandle(msgInputInfo);
    fileTransferMgrPtr->HandleDownloadFileResponse(msgHandle);
    return true;
}

bool OnMessageHandleFuzzTest(shared_ptr<FileTransferManager> fileTransferMgrPtr, FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::string senderNetworkId = fuzzData.GetStringFromData(NETWORK_ID_SIZE_MAX);
    int receiverSessionId = fuzzData.GetData<int>();
    int32_t errorCode = fuzzData.GetData<int32_t>();
    int32_t userId = fuzzData.GetData<int32_t>();
    uint64_t taskId = fuzzData.GetData<uint64_t>();
    uint16_t msgType =
        fuzzData.GetData<uint16_t>() % (MsgType::MSG_FINISH_FILE_RECV + static_cast<int16_t>(U16_AT_SIZE));
    int len = static_cast<int>(fuzzData.GetRemainSize());
    std::string uri = fuzzData.GetStringFromData(len);
    MessageInputInfo msgInputInfo;
    msgInputInfo.errorCode = errorCode;
    msgInputInfo.taskId = taskId;
    msgInputInfo.uri = uri;
    msgInputInfo.userId = userId;
    msgInputInfo.msgType = msgType;
    MessageHandler msgHandle(msgInputInfo);
    auto dataSubPtr = std::make_unique<uint8_t[]>(size);
    msgHandle.PackData(dataSubPtr.get(), static_cast<uint32_t>(size));
    fileTransferMgrPtr->OnMessageHandle(senderNetworkId, receiverSessionId, dataSubPtr.get(), msgHandle.GetDataSize());
    return true;
}

bool OnFileRecvHandleFuzzTest(shared_ptr<FileTransferManager> fileTransferMgrPtr, FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::string senderNetworkId = fuzzData.GetStringFromData(NETWORK_ID_SIZE_MAX);
    int result = fuzzData.GetData<int>();
    int len = static_cast<int>(fuzzData.GetRemainSize());
    std::string filePath = fuzzData.GetStringFromData(len);
    fileTransferMgrPtr->OnFileRecvHandle(senderNetworkId, filePath.c_str(), result);
    fileTransferMgrPtr->HandleRecvFileFinished();
    fileTransferMgrPtr->OnSessionClosed();
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= (OHOS::U16_AT_SIZE + OHOS::U64_AT_SIZE + (OHOS::U32_AT_SIZE << 1) +
                                    (OHOS::FileManagement::CloudSync::NETWORK_ID_SIZE_MAX << 1))) {
        return 0;
    }

    OHOS::FuzzData fuzzData(data, size);
    auto sessionMgrPtr = std::make_shared<OHOS::FileManagement::CloudSync::SessionManager>();
    std::shared_ptr<OHOS::FileManagement::CloudSync::FileTransferManager> fileTransferMgrPtr =
        std::make_shared<OHOS::FileManagement::CloudSync::FileTransferManager>(sessionMgrPtr);
    fileTransferMgrPtr->Init();

    OHOS::DownloadFileFromRemoteDeviceFuzzTest(fileTransferMgrPtr, fuzzData, size);
    OHOS::HandleDownloadFileRequestFuzzTest(fileTransferMgrPtr, fuzzData, size);
    OHOS::HandleDownloadFileResponseFuzzTest(fileTransferMgrPtr, fuzzData, size);
    OHOS::OnMessageHandleFuzzTest(fileTransferMgrPtr, fuzzData, size);
    OHOS::OnFileRecvHandleFuzzTest(fileTransferMgrPtr, fuzzData, size);

    if (OHOS::FileManagement::CloudSync::TaskStateManager::GetInstance().unloadTaskHandle_ != nullptr) {
        OHOS::FileManagement::CloudSync::TaskStateManager::GetInstance().queue_.wait(
            OHOS::FileManagement::CloudSync::TaskStateManager::GetInstance().unloadTaskHandle_);
    }
    return 0;
}
