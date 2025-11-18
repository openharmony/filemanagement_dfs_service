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

#ifndef MOCK_CLOUD_FILE_KIT_H
#define MOCK_CLOUD_FILE_KIT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "cloud_file_kit.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudFile {

class CloudFilekitImplMock : public CloudFileKit {
public:
    MOCK_METHOD3(GetAppConfigParams, int32_t(const int32_t userId,
        const std::string &bundleName, std::map<std::string, std::string> &param));
};

} // OHOS::FileManagement::CloudFile
#endif // MOCK_CLOUD_FILE_KIT_H