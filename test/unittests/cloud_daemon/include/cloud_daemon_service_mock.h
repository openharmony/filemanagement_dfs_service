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

#ifndef MOCK_I_CLOUD_DAEMON_H
#define MOCK_I_CLOUD_DAEMON_H

#include <gmock/gmock.h>

#include "i_cloud_daemon.h"
#include "iremote_stub.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;
class CloudDaemonServiceMock : public IRemoteStub<ICloudDaemon> {
public:
    int code_;
    CloudDaemonServiceMock() : code_(0) {}
    virtual ~ CloudDaemonServiceMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));

    int32_t InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return E_OK;
    }

    int32_t StartFuse(int32_t userId, int32_t devFd, const string &path)
    {
        return E_OK;
    }
};
} // OHOS::FileManagement::CloudFile

#endif // MOCK_I_CLOUD_DAEMON_H