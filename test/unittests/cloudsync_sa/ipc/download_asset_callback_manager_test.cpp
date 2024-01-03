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

#include "download_asset_callback_manager.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

class DownloadAssetCallbackManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DownloadAssetCallbackManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void DownloadAssetCallbackManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void DownloadAssetCallbackManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void DownloadAssetCallbackManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DownloadAssetCallbackManagerTest, GetInstanceTest, TestSize.Level1)
{
    try {
        DownloadAssetCallbackManager::GetInstance();
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
