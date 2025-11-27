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
#include "handlegetremotecopyinfo_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "copy/ipc_wrapper.h"
#include "daemon_stub.h"
#include "distributed_file_daemon_ipc_interface_code.h"
#include "ipc_skeleton.h"
#include "message_option.h"
#include "message_parcel.h"
#include "securec.h"
#include "accesstoken_kit.h"
#include "utils_log.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
constexpr pid_t DAEMON_UID = 1009;
pid_t g_uid = 1009;
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
pid_t IPCSkeleton::GetCallingUid()
{
    return g_uid;
}
}

namespace OHOS {
using namespace OHOS::Storage::DistributedFile;

class DaemonStubImpl : public DaemonStub {
public:
    DaemonStubImpl() = default;
    ~DaemonStubImpl() override {}
    int32_t ConnectDfs(const std::string &networkId) override
    {
        return 0;
    }

    int32_t DisconnectDfs(const std::string &networkId) override
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

    int32_t CancelCopyTask(const std::string &srcUri, const std::string &dstUri) override
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

    int32_t RequestSendFileACL(const std::string &srcUri,
                               const std::string &dstPath,
                               const std::string &remoteDeviceId,
                               const std::string &sessionName,
                               const AccountInfo &callerAccountInfo) override
    {
        return 0;
    }

    int32_t GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir) override
    {
        return 0;
    }

    int32_t GetRemoteCopyInfoACL(const std::string &srcUri,
                                 bool &isFile,
                                 bool &isDir,
                                 const AccountInfo &callerAccountInfo) override
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

    int32_t GetDfsUrisDirFromLocal(const std::vector<std::string> &uriList,
                                   const int32_t userId,
                                   std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo>
                                   &uriToDfsUriMaps) override
    {
        return 0;
    }

    int32_t GetDfsSwitchStatus(const std::string &networkId, int32_t &switchStatus) override
    {
        return 0;
    }

    int32_t UpdateDfsSwitchStatus(int32_t switchStatus) override
    {
        return 0;
    }

    int32_t GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList) override
    {
        return 0;
    }

    int32_t RegisterFileDfsListener(const std::string &instanceId, const sptr<IFileDfsListener> &listener) override
    {
        return 0;
    }

    int32_t UnregisterFileDfsListener(const std::string &instanceId) override
    {
        return 0;
    }

    int32_t IsSameAccountDevice(const std::string &networkId, bool &isSameAccount) override
    {
        return 0;
    }
};
void HandleGetRemoteCopyInfoFuzzTest(std::shared_ptr<DaemonStub> daemonStubPtr,
                                     const uint8_t *data,
                                     size_t size)
{
    OHOS::g_uid = DAEMON_UID;
    uint32_t code = static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO);
    MessageParcel datas;
    datas.WriteInterfaceToken(DaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    daemonStubPtr->OnRemoteRequest(code, datas, reply, option);
}

void SetAccessTokenPermission()
{
    uint64_t tokenId;
    const char *perms[1];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "distributedFileDaemonstubFuzzer",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    if (tokenId == 0) {
        LOGE("Get Acess Token Id Failed");
        return;
    }
    int ret = SetSelfTokenID(tokenId);
    if (ret != 0) {
        LOGE("Set Acess Token Id Failed");
        return;
    }
    ret = Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    if (ret < 0) {
        LOGE("Reload Native Token Info Failed");
        return;
    }
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::SetAccessTokenPermission();
    auto daemonStubPtr = std::make_shared<OHOS::DaemonStubImpl>();
    OHOS::HandleGetRemoteCopyInfoFuzzTest(daemonStubPtr, data, size);
    return 0;
}
