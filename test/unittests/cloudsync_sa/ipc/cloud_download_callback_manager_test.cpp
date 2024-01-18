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
#include <memory>

#include "cloud_download_callback_manager.h"
#include "data_handler_mock.h"
#include "dfs_error.h"
#include "iservice_registry.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudDownloadCallbackManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class MockICloudDownloadCallback : public ICloudDownloadCallback {
public:
    void OnDownloadProcess(const DownloadProgressObj& progress) {return;}
    sptr<IRemoteObject> AsObject() {return nullptr;}
};

void CloudDownloadCallbackManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDownloadCallbackManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDownloadCallbackManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudDownloadCallbackManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: StartDonwloadTest
 * @tc.desc: Verify the StartDonwloadTest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackManagerTest, StartDonwloadTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDonwloadTest Start";
    try {
        auto cloudDownloadCallbackManager =std::make_shared<CloudDownloadCallbackManager>();
        std::string path = "data/";
        int32_t userId = 1;
        cloudDownloadCallbackManager->StartDonwload(path, userId, 0);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartDonwloadTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartDonwloadTest End";
}

/**
 * @tc.name: StopDonwloadTest
 * @tc.desc: Verify the StopDonwloadTest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackManagerTest, StopDonwloadTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDonwloadTest Start";
    try {
        CloudDownloadCallbackManager cloudDownloadCallbackManager;
        std::string path = "data/";
        int32_t userId = 1;
        DownloadProgressObj downloadId;
        cloudDownloadCallbackManager.StopDonwload(path, userId, downloadId);

        DownloadProgressObj downloadProgressObj;
        cloudDownloadCallbackManager.downloads_.insert({path, downloadProgressObj});
        cloudDownloadCallbackManager.StopDonwload(path, userId, downloadId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopDonwloadTest FAILED";
    }
    GTEST_LOG_(INFO) << "StopDonwloadTest End";
}

/**
 * @tc.name: RegisterCallbackTest
 * @tc.desc: Verify the RegisterCallbackTest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackManagerTest, RegisterCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackTest Start";
    try {
        CloudDownloadCallbackManager cloudDownloadCallbackManager;
        int32_t userId = 1;
        sptr<MockICloudDownloadCallback> callback = sptr(new MockICloudDownloadCallback());
        cloudDownloadCallbackManager.RegisterCallback(userId, callback);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackTest End";
}

/**
 * @tc.name: UnregisterCallbackTest
 * @tc.desc: Verify the UnregisterCallbackTest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackManagerTest, UnregisterCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterCallbackTest Start";
    try {
        CloudDownloadCallbackManager cloudDownloadCallbackManager;
        int32_t userId = 1;
        cloudDownloadCallbackManager.UnregisterCallback(userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " UnregisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "UnregisterCallbackTest End";
}

/**
 * @tc.name: OnDownloadedResultTest
 * @tc.desc: Verify the OnDownloadedResultTest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackManagerTest, OnDownloadedResultTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadedResultTest Start";
    try {
        CloudDownloadCallbackManager cloudDownloadCallbackManager;

        DriveKit::DKAppBundleName bundleName = "bundleName";
        DriveKit::DKContainerName containerName = "containerName";
        int32_t userId = 0;
        auto driveKit = std::make_shared<DriveKit::DriveKitNative>(userId);
        auto container = std::make_shared<DriveKit::DKContainer>(bundleName, containerName, driveKit);
        DriveKit::DKDatabaseScope scope;

        std::string path = "data/";
        std::string table = "data";
        std::vector<DriveKit::DKDownloadAsset> assetsToDownload;
        auto handler = std::make_shared<DataHandlerMock>(userId, bundleName, table);
        auto context = std::make_shared<DriveKit::DKContext>();
        auto database = std::make_shared<DriveKit::DKDatabase>(container, scope);
        std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> results;
        DriveKit::DKError err;
        DownloadProgressObj downloadProgressObj;
        cloudDownloadCallbackManager.downloads_.insert({path, downloadProgressObj});
        cloudDownloadCallbackManager.OnDownloadedResult(path, assetsToDownload, handler, context, database, results,
                                                        err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDownloadedResultTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnDownloadedResultTest End";
}

/**
 * @tc.name: OnDownloadProcessTest
 * @tc.desc: Verify the OnDownloadProcessTest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackManagerTest, OnDownloadProcessTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadProcessTest Start";
    try {
        CloudDownloadCallbackManager cloudDownloadCallbackManager;
        std::string path = "data";
        auto context = std::make_shared<DriveKit::DKContext>();
        DriveKit::DKDownloadAsset asset;
        DriveKit::TotalSize totalSize = 1024;
        DriveKit::DownloadSize downloadSize = 1024;
        DownloadProgressObj downloadProgressObj;
        cloudDownloadCallbackManager.downloads_.insert({path, downloadProgressObj});
        cloudDownloadCallbackManager.OnDownloadProcess(path, context, asset, totalSize, downloadSize);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDownloadProcessTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnDownloadProcessTest End";
}
} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
