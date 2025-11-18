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
#include "dfs_error.h"

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
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudSyncAssetManagerImplTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudSyncAssetManagerImplTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudSyncAssetManagerImplTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: UploadAssetTest
 * @tc.desc: Verify the UploadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, UploadAssetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UploadAssetTest Start";
    int32_t userId = 100;
    std::string request = "sample_request";
    std::string result;
    int32_t ret = CloudSyncAssetManagerImpl::GetInstance().UploadAsset(userId, request, result);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

/**
 * @tc.name: DownloadFileTest
 * @tc.desc: Verify the DownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, DownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFileTest Start";
    int32_t userId = 100;
    std::string request = "sample_request";
    AssetInfo assetInfo;
    int32_t ret = CloudSyncAssetManagerImpl::GetInstance().DownloadFile(userId, request, assetInfo);
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
