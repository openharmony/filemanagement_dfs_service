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
#include "service_proxy.h"
#include "service_proxy_mock.h"
#include "dfs_error.h"
#include "utils_log.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS {
namespace FileManagement::CloudSync {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudSyncAssetManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<MockServiceProxy> proxy_ = nullptr;
    static inline sptr<CloudSyncServiceMock> serviceProxy_ = nullptr;
};

void CloudSyncAssetManagerImplTest::SetUpTestCase(void)
{
    proxy_ = std::make_shared<MockServiceProxy>();
    IserviceProxy::proxy_ = proxy_;
    serviceProxy_ = sptr(new CloudSyncServiceMock());
    CloudSyncAssetManagerImpl::GetInstance().isFirstCall_.test_and_set();
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncAssetManagerImplTest::TearDownTestCase(void)
{
    IserviceProxy::proxy_ = nullptr;
    proxy_ = nullptr;
    serviceProxy_ = nullptr;
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
 * @tc.name: SetDeathRecipientTest
 * @tc.desc: Verify the SetDeathRecipient function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, SetDeathRecipientTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipientTest Start";
    try {
        CloudSyncAssetManagerImpl::GetInstance().SetDeathRecipient(serviceProxy_->AsObject());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetDeathRecipientTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest End";
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
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));

        int32_t res = CloudSyncAssetManagerImpl::GetInstance().UploadAsset(userId, request, result);
        EXPECT_EQ(res, E_OK);
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
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));

        int32_t res = CloudSyncAssetManagerImpl::GetInstance().DownloadFile(userId, bundleName, assetInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFileTest FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFileTest End";
}

/*
 * @tc.name: DownloadFilesTest001
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, DownloadFilesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFilesTest001 Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        std::vector<AssetInfo> assetInfo;
        std::vector<bool> assetResultMap;
        int32_t connectTime = 1;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));

        int32_t res = CloudSyncAssetManagerImpl::GetInstance().DownloadFiles(userId,
                                                    bundleName, assetInfo, assetResultMap, connectTime);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFilesTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFilesTest001 End";
}

/*
 * @tc.name: DownloadFilesTest002
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncAssetManagerImplTest, DownloadFilesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFilesTest002 Start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        std::vector<AssetInfo> assetInfo;
        std::vector<bool> assetResultMap;
        int32_t connectTime = 1;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));

        int32_t res = CloudSyncAssetManagerImpl::GetInstance().DownloadFiles(userId,
                                                    bundleName, assetInfo, assetResultMap, connectTime);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFilesTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFilesTest002 End";
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
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));

        int32_t res = CloudSyncAssetManagerImpl::GetInstance().DeleteAsset(userId, uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteAssetTest FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteAssetTest End";
}

} // namespace FileManagement::CloudSync
} // namespace OHOS