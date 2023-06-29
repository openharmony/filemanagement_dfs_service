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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cloud_sync_asset_manager_impl.h"
#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudSyncAssetManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncAssetManagerImplTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncAssetManagerImplTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncAssetManagerImplTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncAssetManagerImplTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: UploadAssetTest
 * @tc.desc: Verify the UploadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, UploadAssetTest, TestSize.Level1)
{
    CloudSyncAssetManagerImpl assetManager;
    int32_t userId = 100;
    std::string request = "sample_request";
    std::string result;
    int32_t ret = assetManager.UploadAsset(userId, request, result);
    EXPECT_EQ(ret, E_SA_LOAD_FAILED);
}

/**
 * @tc.name: DownloadFileTest
 * @tc.desc: Verify the DownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, DownloadFileTest, TestSize.Level1)
{
    CloudSyncAssetManagerImpl assetManager;
    int32_t userId = 100;
    std::string request = "sample_request";
    AssetInfo assetInfo;
    int32_t ret = assetManager.DownloadFile(userId, request, assetInfo);
    EXPECT_EQ(ret, E_SA_LOAD_FAILED);
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
