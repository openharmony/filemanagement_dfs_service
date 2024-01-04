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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "download_asset_callback_client.h"
#include "asset_callback_mock.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

class DownloadAssetCallbackClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DownloadAssetCallbackClientTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void DownloadAssetCallbackClientTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void DownloadAssetCallbackClientTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void DownloadAssetCallbackClientTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
