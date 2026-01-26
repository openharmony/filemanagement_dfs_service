/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_FILE_CLOUD_ASSET_READ_SESSION_MOCK_H
#define OHOS_CLOUD_FILE_CLOUD_ASSET_READ_SESSION_MOCK_H

#include <gmock/gmock.h>
#include <memory>
#include <shared_mutex>
#include <string>

#include "cloud_asset_read_session.h"
#include "cloud_info.h"

namespace OHOS::FileManagement::CloudFile {
class CloudAssetReadSessionMock : public CloudAssetReadSession {
public:
    CloudAssetReadSessionMock(const int32_t userId, std::string recordType,
        std::string recordId, std::string assetKey, std::string assetPath)
        : CloudAssetReadSession(userId, recordType, recordId, assetKey, assetPath)
    {
    };
    virtual ~CloudAssetReadSessionMock() = default;

    MOCK_METHOD0(InitSession, CloudError());
    MOCK_METHOD4(PRead, int64_t(int64_t offset, int64_t size, CloudError &error, const std::string appId));
    MOCK_METHOD0(CancelSession, void());
    MOCK_METHOD1(Close, bool(bool needRemain));
    MOCK_METHOD1(SentPrepareTraceId, void(std::string prepareTraceId));
    MOCK_METHOD2(Catch, bool(CloudError &error, uint32_t catchTimeOutPara));
    MOCK_METHOD2(ChownUidForSyncDisk, bool(bool needChown, const std::string &dirPath));
};
} // namespace OHOS::FileManagement::CloudFile

#endif // OHOS_CLOUD_FILE_CLOUD_ASSET_READ_SESSION_MOCK_H