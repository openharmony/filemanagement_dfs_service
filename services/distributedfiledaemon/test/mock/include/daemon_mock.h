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

#ifndef FILEMANAGEMENT_DFS_SERVICE_DAEMON_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_DAEMON_MOCK_H

#include <gmock/gmock.h>
#include "daemon.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DaemonMock : public DaemonStub {
public:
    MOCK_METHOD1(OpenP2PConnection, int32_t(const DmDeviceInfoExt &deviceInfo));
    MOCK_METHOD1(CloseP2PConnection, int32_t(const DmDeviceInfoExt &deviceInfo));
    MOCK_METHOD2(OpenP2PConnectionEx,
                 int32_t(const std::string &networkId, const sptr<IFileDfsListener> &remoteReverseObj));
    MOCK_METHOD1(CloseP2PConnectionEx, int32_t(const std::string &networkId));
    MOCK_METHOD4(RequestSendFile,
                 int32_t(const std::string &srcUri,
                         const std::string &dstPath,
                         const std::string &remoteDeviceId,
                         const std::string &sessionName));
    MOCK_METHOD5(PrepareSession,
                 int32_t(const std::string &srcUri,
                         const std::string &dstUri,
                         const std::string &srcDeviceId,
                         const sptr<IRemoteObject> &listener,
                         HmdfsInfoExt &fileInfo));
    MOCK_METHOD1(CancelCopyTask, int32_t(const std::string &sessionName));
    MOCK_METHOD3(GetRemoteCopyInfo, int32_t(const std::string &srcUri, bool &isFile, bool &isDir));

    MOCK_METHOD3(PushAsset,
                 int32_t(int32_t userId,
                         const AssetObj &assetObj,
                         const sptr<IAssetSendCallback> &sendCallback));
    MOCK_METHOD1(RegisterAssetCallback, int32_t(const sptr<IAssetRecvCallback> &recvCallback));
    MOCK_METHOD1(UnRegisterAssetCallback, int32_t(const sptr<IAssetRecvCallback> &recvCallback));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_DAEMON_MOCK_H
