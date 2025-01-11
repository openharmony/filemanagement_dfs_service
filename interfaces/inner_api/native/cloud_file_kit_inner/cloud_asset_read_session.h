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

#ifndef OHOS_CLOUD_FILE_CLOUD_ASSET_READ_SESSION_H
#define OHOS_CLOUD_FILE_CLOUD_ASSET_READ_SESSION_H

#include <memory>
#include <string>

#include "cloud_info.h"

namespace OHOS::FileManagement::CloudFile {
class CloudAssetReadSession {
public:
    CloudAssetReadSession(std::string recordType, std::string recordId, std::string assetKey, std::string assetPath);
    virtual ~CloudAssetReadSession() = default;

    virtual CloudError InitSession();
    virtual int64_t PRead(int64_t offset, int64_t size, char *buffer, CloudError &error);
    virtual bool Close(bool needRemain = false);
    virtual bool HasCache(int64_t offset, int64_t readSize);
    void SetPrepareTraceId(std::string prepareTraceId);
    std::string GetPrepareTraceId();

private:
    std::string prepareTraceId_;
};
} // namespace OHOS::FileManagement::CloudFile

#endif // OHOS_CLOUD_FILE_CLOUD_ASSET_READ_SESSION_H