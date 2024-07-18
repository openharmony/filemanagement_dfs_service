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
#include "daemonstub_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "daemon_stub.h"
#include "distributed_file_daemon_ipc_interface_code.h"
#include "message_option.h"
#include "message_parcel.h"
#include "securec.h"

namespace OHOS {

constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;

using namespace OHOS::Storage::DistributedFile;

class DaemonStubImpl : public DaemonStub {
public:
    DaemonStubImpl() = default;
    ~DaemonStubImpl() override {}
    int32_t OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) override
    {
        return 0;
    }

    int32_t CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) override
    {
        return 0;
    }

    int32_t OpenP2PConnectionEx(const std::string &networkId, sptr<IFileDfsListener> remoteReverseObj) override
    {
        return 0;
    }

    int32_t CloseP2PConnectionEx(const std::string &networkId) override
    {
        return 0;
    }

    int32_t PrepareSession(const std::string &srcUri,
                           const std::string &dstUri,
                           const std::string &srcDeviceId,
                           const sptr<IRemoteObject> &listener,
                           HmdfsInfo &info) override
    {
        return 0;
    }

    int32_t CancelCopyTask(const std::string &sessionName) override
    {
        return 0;
    }

    int32_t RequestSendFile(const std::string &srcUri,
                            const std::string &dstPath,
                            const std::string &remoteDeviceId,
                            const std::string &sessionName) override
    {
        return 0;
    }

    int32_t GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir) override
    {
        return 0;
    }

    int32_t PushAsset(int32_t userId,
                      const sptr<AssetObj> &assetObj,
                      const sptr<IAssetSendCallback> &sendCallback) override
    {
        return 0;
    }

    int32_t RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override
    {
        return 0;
    }

    int32_t UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override
    {
        return 0;
    }
};

void HandleOpenP2PConnectionFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                     const uint8_t *data,
                                     size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandleCloseP2PConnectionFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                      const uint8_t *data,
                                      size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandleOpenP2PConnectionExFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                       const uint8_t *data,
                                       size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION_EX);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandleCloseP2PConnectionExFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                        const uint8_t *data,
                                        size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION_EX);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandlePrepareSessionFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr, const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PREPARE_SESSION);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandleCancelCopyTaskFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr, const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CANCEL_COPY_TASK);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandleRequestSendFileFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                   const uint8_t *data,
                                   size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REQUEST_SEND_FILE);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandleGetRemoteCopyInfoFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                     const uint8_t *data,
                                     size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandlePushAssetFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr, const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PUSH_ASSET);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandleRegisterRecvCallbackFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                        const uint8_t *data,
                                        size_t size)
{
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REGISTER_ASSET_CALLBACK);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void HandleUnRegisterRecvCallbackFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                          const uint8_t *data,
                                          size_t size)
{
    uint32_t code =
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_UN_REGISTER_ASSET_CALLBACK);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    auto daemonStubPtr = std::make_shared<OHOS::DaemonStubImpl>();
    OHOS::HandleOpenP2PConnectionFuzzTest(daemonStubPtr, data, size);
    OHOS::HandleCloseP2PConnectionFuzzTest(daemonStubPtr, data, size);
    OHOS::HandlePrepareSessionFuzzTest(daemonStubPtr, data, size);
    OHOS::HandleRequestSendFileFuzzTest(daemonStubPtr, data, size);
    OHOS::HandleOpenP2PConnectionExFuzzTest(daemonStubPtr, data, size);
    OHOS::HandleCloseP2PConnectionExFuzzTest(daemonStubPtr, data, size);
    OHOS::HandleCancelCopyTaskFuzzTest(daemonStubPtr, data, size);
    OHOS::HandleGetRemoteCopyInfoFuzzTest(daemonStubPtr, data, size);
    OHOS::HandlePushAssetFuzzTest(daemonStubPtr, data, size);
    OHOS::HandleRegisterRecvCallbackFuzzTest(daemonStubPtr, data, size);
    OHOS::HandleUnRegisterRecvCallbackFuzzTest(daemonStubPtr, data, size);
    return 0;
}
