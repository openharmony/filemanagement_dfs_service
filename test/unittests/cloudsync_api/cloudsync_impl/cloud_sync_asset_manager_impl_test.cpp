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

#include "cloud_sync_asset_manager_impl.h"
#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
#include "utils_log.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

/*
 * @tc.name: UploadAssetTest
 * @tc.desc: Verify the UploadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, UploadAssetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UploadAssetTest Start";
    try {
        int32_t userId = 100;
        std::string request = "";
        std::string result = "";
        int32_t res = CloudSyncAssetManagerImpl::GetInstance().UploadAsset(userId, request, result);
        EXPECT_EQ(res, E_BROKEN_IPC);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UploadAsseteTest FAILED";
    }
    GTEST_LOG_(INFO) << "UploadAssetTest End";
}

/*
 * @tc.name: DownloadFileTest001
 * @tc.desc: Verify the DownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, DownloadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFileTest Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        AssetInfo assetInfo;
        int32_t res = CloudSyncAssetManagerImpl::GetInstance().DownloadFile(userId, bundleName, assetInfo);
        EXPECT_EQ(res, E_BROKEN_IPC);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFileTest FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFileTest End";
}

/*
 * @tc.name: DownloadFileTest002
 * @tc.desc: Verify the DownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, DownloadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFileTest Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        std::string networkId = "";
        AssetInfo assetInfo;
        CloudSyncAssetManager::ResultCallback resultCallback;
        int32_t res = CloudSyncAssetManagerImpl::GetInstance().DownloadFile(userId,
                                            bundleName, networkId, assetInfo, resultCallback);
        EXPECT_EQ(res, E_BROKEN_IPC);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFileTest FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFileTest End";
}

/*
 * @tc.name: DownloadFilesTest
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, DownloadFilesTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFilesTest Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        std::vector<AssetInfo> assetInfo;
        std::vector<bool> assetResultMap;
        int32_t res = CloudSyncAssetManagerImpl::GetInstance().DownloadFiles(userId,
                                                    bundleName, assetInfo, assetResultMap);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFilesTest FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFilesTest End";
}

/*
 * @tc.name: DeleteAssetTest
 * @tc.desc: Verify the DeleteAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, DeleteAssetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAssetTest Start";
    try {
        int32_t userId = 100;
        std::string uri = "uri";
        int32_t res = CloudSyncAssetManagerImpl::GetInstance().DeleteAsset(userId, uri);
        EXPECT_EQ(res, E_BROKEN_IPC);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteAssetTest FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteAssetTest End";
}

/*
 * @tc.name: SetDeathRecipientTest
 * @tc.desc: Verify the SetDeathRecipient function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, SetDeathRecipientTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
        CloudSyncAssetManagerImpl::GetInstance().SetDeathRecipient(CloudSyncServiceProxy->AsObject());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetDeathRecipientTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS