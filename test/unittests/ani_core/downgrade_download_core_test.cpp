/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "downgrade_download_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager_mock.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;

class DowngradeDownloadCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DowngradeDownloadCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DowngradeDownloadCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DowngradeDownloadCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DowngradeDownloadCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the DowngradeDownloadCore::Constructor function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(DowngradeDownloadCoreTest, ConstructorTest1, TestSize.Level1)
{
    string bundleName = "com.example.test";
    auto data = DowngradeDownloadCore::Constructor(bundleName);
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the DowngradeDownloadCore::Constructor function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(DowngradeDownloadCoreTest, ConstructorTest2, TestSize.Level1)
{
    string bundleName = "";
    auto data = DowngradeDownloadCore::Constructor(bundleName);
    EXPECT_FALSE(data.IsSuccess());
}

/**
 * @tc.name: GetBundleName
 * @tc.desc: Verify the DowngradeDownloadCore::GetBundleName function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(DowngradeDownloadCoreTest, GetBundleNameTest1, TestSize.Level1)
{
    string bundleName = "com.example.test";
    DowngradeDownloadCore *downgradeDlSync =
        DowngradeDownloadCore::Constructor(bundleName).GetData().value();
    string ret = downgradeDlSync->GetBundleName();
    EXPECT_EQ(ret, bundleName);
}

/**
 * @tc.name: DoDowngradeDlGetCloudFileInfo
 * @tc.desc: Verify the DowngradeDownloadCore::DoDowngradeDlGetCloudFileInfo function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(DowngradeDownloadCoreTest, DoDowngradeDlGetCloudFileInfoTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "DowngradeDownloadCore::DoDowngradeDlGetCloudFileInfoTest1 begin.";
        string bundleName = "com.example.test";
        DowngradeDownloadCore *downgradeDlCore =
            DowngradeDownloadCore::Constructor(bundleName).GetData().value();
        auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
        EXPECT_CALL(cloudMock, GetCloudFileInfo(_, _)).WillOnce(Return(E_OK));
        auto data = downgradeDlCore->DoDowngradeDlGetCloudFileInfo();
        EXPECT_TRUE(data.IsSuccess());

        EXPECT_CALL(cloudMock, GetCloudFileInfo(_, _)).WillOnce(Return(OHOS::FileManagement::E_INNER_FAILED));
        auto ret = downgradeDlCore->DoDowngradeDlGetCloudFileInfo();
        EXPECT_FALSE(ret.IsSuccess());
        const auto &err = ret.GetError();
        int errorCode = err.GetErrNo();
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_INNER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DowngradeDownloadCore::DoDowngradeDlGetCloudFileInfoTest1 ERROR.";
    }
}

/**
 * @tc.name: DoDowngradeDlStartDownload
 * @tc.desc: Verify the DowngradeDownloadCore::DoDowngradeDlStartDownload function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(DowngradeDownloadCoreTest, DoDowngradeDlStartDownloadTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "DowngradeDownloadCore::DoDowngradeDlStartDownloadTest1 begin.";
        string bundleName = "com.example.test";
        DowngradeDownloadCore *downgradeDlCore =
            DowngradeDownloadCore::Constructor(bundleName).GetData().value();
        auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
        auto callback = make_shared<DowngradeCallbackAniImpl>(nullptr, nullptr);
        EXPECT_CALL(cloudMock, StartDowngrade(_, _)).WillOnce(Return(E_OK));
        auto data = downgradeDlCore->DoDowngradeDlStartDownload(callback);
        EXPECT_TRUE(data.IsSuccess());

        EXPECT_CALL(cloudMock, StartDowngrade(_, _)).WillOnce(Return(OHOS::FileManagement::E_INNER_FAILED));
        auto ret = downgradeDlCore->DoDowngradeDlStartDownload(callback);
        EXPECT_FALSE(ret.IsSuccess());
        const auto &err = ret.GetError();
        int errorCode = err.GetErrNo();
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_INNER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DowngradeDownloadCore::DoDowngradeDlStartDownloadTest1 ERROR.";
    }
}

/**
 * @tc.name: DoDowngradeDlStopDownload
 * @tc.desc: Verify the DowngradeDownloadCore::DoDowngradeDlStopDownload function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(DowngradeDownloadCoreTest, DoDowngradeDlStopDownloadTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "DowngradeDownloadCore::DoDowngradeDlStopDownloadTest1 begin.";
        string bundleName = "com.example.test";
        DowngradeDownloadCore *downgradeDlCore =
            DowngradeDownloadCore::Constructor(bundleName).GetData().value();
        auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
        EXPECT_CALL(cloudMock, StopDowngrade(_)).WillOnce(Return(E_OK));
        auto data = downgradeDlCore->DoDowngradeDlStopDownload();
        EXPECT_TRUE(data.IsSuccess());

        EXPECT_CALL(cloudMock, StopDowngrade(_)).WillOnce(Return(OHOS::FileManagement::E_INNER_FAILED));
        auto ret = downgradeDlCore->DoDowngradeDlStopDownload();
        EXPECT_FALSE(ret.IsSuccess());
        const auto &err = ret.GetError();
        int errorCode = err.GetErrNo();
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_INNER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DowngradeDownloadCore::DoDowngradeDlStopDownloadTest1 ERROR.";
    }
}
} // namespace OHOS::FileManagement::CloudDisk::Test