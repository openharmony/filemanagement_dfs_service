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

#ifndef OHOS_STORAGE_I_DAEMON_H
#define OHOS_STORAGE_I_DAEMON_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"
#include "ipc/i_daemon.h"
#include "iremote_broker.h"
#include "iremote_stub.h"

namespace OHOS::Storage::DistributedFile {
class DaemonServiceMock : public IRemoteStub<IDaemon> {
public:
    int code_;
    DaemonServiceMock() : code_(0) {}
    virtual ~DaemonServiceMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    MOCK_METHOD1(OpenP2PConnection, int32_t(const DistributedHardware::DmDeviceInfo &deviceInfo));
    MOCK_METHOD1(CloseP2PConnection, int32_t(const DistributedHardware::DmDeviceInfo &deviceInfo));
    MOCK_METHOD2(OpenP2PConnectionEx, int32_t(const std::string &networkId,
        sptr<IFileDfsListener> remoteReverseObj));
    MOCK_METHOD1(CloseP2PConnectionEx, int32_t(const std::string &networkId));
    MOCK_METHOD5(PrepareSession, int32_t(const std::string &srcUri, const std::string &dstUri,
        const std::string &srcDeviceId, const sptr<IRemoteObject> &listener, HmdfsInfo &fileInfo));
    MOCK_METHOD4(RequestSendFile, int32_t(const std::string &, const std::string &,
        const std::string &, const std::string &));
    MOCK_METHOD3(GetRemoteCopyInfo, int32_t(const std::string &srcUri, bool &isFile, bool &isDir));
    MOCK_METHOD1(CancelCopyTask, int32_t(const std::string &sessionName));
    MOCK_METHOD3(PushAsset, int32_t(int32_t userId, const sptr<AssetObj> &assetObj,
        const sptr<IAssetSendCallback> &sendCallback));
    MOCK_METHOD1(RegisterAssetCallback, int32_t(const sptr<IAssetRecvCallback> &recvCallback));
    MOCK_METHOD1(UnRegisterAssetCallback, int32_t(const sptr<IAssetRecvCallback> &recvCallback));
};
} // namespace OHOS::Storage::DistributedFile

#endif // OHOS_STORAGE_I_DAEMON_H