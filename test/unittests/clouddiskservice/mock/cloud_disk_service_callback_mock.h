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
#ifndef OHOS_FILEMGMT_CLOUD_DISK_SERVICE_MOCK
#define OHOS_FILEMGMT_CLOUD_DISK_SERVICE_MOCK

#include "iremote_stub.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
class CloudDiskServiceCallbackMock : public IRemoteStub<ICloudDiskServiceCallback> {
public:
    int code_;
    CloudDiskServiceCallbackMock() : code_(0) {}
    virtual ~CloudDiskServiceCallbackMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));

    void OnChangeData(const string& sandboxPath, const vector<ChangeData> &changeData) override {}
};
}
#endif // OHOS_FILEMGMT_CLOUD_DISK_SERVICE_MOCK