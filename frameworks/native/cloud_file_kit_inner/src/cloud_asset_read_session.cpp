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

#include "cloud_asset_read_session.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;

CloudAssetReadSession::CloudAssetReadSession(std::string recordType,
                                             std::string recordId,
                                             std::string assetKey,
                                             std::string assetPath)
{
}

CloudError CloudAssetReadSession::InitSession()
{
    return CloudError::CK_NO_ERROR;
}

int64_t CloudAssetReadSession::PRead(int64_t offset, int64_t size, char *buffer, CloudError &error)
{
    return E_OK;
}

bool CloudAssetReadSession::Close(bool needRemain)
{
    return true;
}

bool CloudAssetReadSession::HasCache(int64_t offset, int64_t readSize)
{
    return true;
}

void CloudAssetReadSession::SentPrepareTraceId(std::string prepareTraceId)
{
    return;
}

void CloudAssetReadSession::SetPrepareTraceId(std::string prepareTraceId)
{
    traceId_ = prepareTraceId;
    SentPrepareTraceId(traceId_);
}

std::string CloudAssetReadSession::GetPrepareTraceId()
{
    return traceId_;
}
} // namespace OHOS::FileManagement::CloudFile