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

#include "cloud_database_impl.h"

#include "cloud_asset_read_session_impl.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;

CloudDatabaseImpl::CloudDatabaseImpl(const int32_t userId, const string &bundleName)
    : CloudDatabase(userId, bundleName)
{
}

std::shared_ptr<CloudAssetReadSession> CloudDatabaseImpl::NewAssetReadSession(std::string recordType,
                                                                              std::string recordId,
                                                                              std::string assetKey,
                                                                              std::string assetPath)
{
    return std::make_shared<CloudAssetReadSessionImpl>(recordType, recordId, assetKey, assetPath);
}
} // namespace OHOS::FileManagement::CloudFile