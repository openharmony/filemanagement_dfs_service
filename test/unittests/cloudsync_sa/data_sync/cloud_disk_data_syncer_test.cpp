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
#include "cloud_disk_data_handler.h"
#include "cloud_disk_data_syncer.h"
#include "clouddisk_rdbstore.h"
#include "dfs_error.h"
#include "rdb_store_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

enum {
    BEGIN,
    DOWNLOADFILE,
    COMPLETEPULL,
    UPLOADFILE,
    COMPLETEPUSH,
    END
};

class CloudDiskDataSyncerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskDataSyncerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskDataSyncerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskDataSyncerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskDataSyncerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: InitTest001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, InitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Init Start";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    int result = cloudDiskDataSyncer.Init(bundleName, userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "Init End";
}

/**
 * @tc.name: CleanTest001
 * @tc.desc: Verify the Clean function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, CleanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Clean Start";
    const string bundleName = "bundleName";
    const int32_t userId = 100;
    const int action = 0;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    int result = cloudDiskDataSyncer.Clean(action);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "Clean End";
}

/**
 * @tc.name: StartDownloadFileTest001
 * @tc.desc: Verify the StartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, StartDownloadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDownloadFile Start";
    const string path = "";
    const string bundleName = "bundleName";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    int32_t result = cloudDiskDataSyncer.StartDownloadFile(path, userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StartDownloadFile End";
}

/**
 * @tc.name: StopDownloadFileTest001
 * @tc.desc: Verify the StopDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, StopDownloadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadFile Start";
    const string path = "";
    const string bundleName = "bundleName";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    int32_t result = cloudDiskDataSyncer.StopDownloadFile(path, userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StopDownloadFile End";
}

/**
 * @tc.name: CleanCacheTest001
 * @tc.desc: Verify the CleanCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, CleanCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCache Start";
    const int32_t userId = 100;
    const string bundleName = "bundleName";
    string uri = "sample_uri";
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    int32_t result = cloudDiskDataSyncer.Init(bundleName, userId);
    EXPECT_EQ(result, E_OK);
    result = cloudDiskDataSyncer.CleanCache(uri);
    EXPECT_EQ(result, 2);
    GTEST_LOG_(INFO) << "CleanCache End";
}

/**
 * @tc.name: ScheduleTest001
 * @tc.desc: Verify the Schedule function.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloudDiskDataSyncerTest, ScheduleTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Schedule Start";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    cloudDiskDataSyncer.stage_ = BEGIN;
    cloudDiskDataSyncer.Schedule();
    EXPECT_EQ(cloudDiskDataSyncer.stage_, DOWNLOADFILE);
    GTEST_LOG_(INFO) << "Schedule End";
}

/**
 * @tc.name: ScheduleTest002
 * @tc.desc: Verify the Schedule function.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloudDiskDataSyncerTest, ScheduleTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Schedule Start";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    cloudDiskDataSyncer.stage_ = DOWNLOADFILE;
    cloudDiskDataSyncer.Schedule();
    EXPECT_EQ(cloudDiskDataSyncer.stage_, COMPLETEPULL);
    GTEST_LOG_(INFO) << "Schedule End";
}

/**
 * @tc.name: ScheduleTest003
 * @tc.desc: Verify the Schedule function.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloudDiskDataSyncerTest, ScheduleTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Schedule Start";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    cloudDiskDataSyncer.stage_ = COMPLETEPULL;
    cloudDiskDataSyncer.Schedule();
    EXPECT_EQ(cloudDiskDataSyncer.stage_, UPLOADFILE);
    GTEST_LOG_(INFO) << "Schedule End";
}

/**
 * @tc.name: ScheduleTest004
 * @tc.desc: Verify the Schedule function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, ScheduleTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Schedule Start";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    cloudDiskDataSyncer.stage_ = UPLOADFILE;
    cloudDiskDataSyncer.Schedule();
    EXPECT_EQ(cloudDiskDataSyncer.stage_, COMPLETEPUSH);
    GTEST_LOG_(INFO) << "Schedule End";
}

/**
 * @tc.name: ScheduleTest005
 * @tc.desc: Verify the Schedule function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, ScheduleTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Schedule Start";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    cloudDiskDataSyncer.stage_ = COMPLETEPUSH;
    cloudDiskDataSyncer.Schedule();
    EXPECT_EQ(cloudDiskDataSyncer.stage_, END);
    GTEST_LOG_(INFO) << "Schedule End";
}

/**
 * @tc.name: ScheduleTest006
 * @tc.desc: Verify the Schedule function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, ScheduleTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Schedule Start";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    cloudDiskDataSyncer.stage_ = END;
    std::shared_ptr<SdkHelper> sdkHelper = make_shared<SdkHelper>();
    cloudDiskDataSyncer.SetSdkHelper(sdkHelper);
    cloudDiskDataSyncer.Schedule();
    EXPECT_EQ(cloudDiskDataSyncer.stage_, 6);
    GTEST_LOG_(INFO) << "Schedule End";
}

/**
 * @tc.name: ResetTest001
 * @tc.desc: Verify the Reset function.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CloudDiskDataSyncerTest, ResetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Reset Start";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    int result = cloudDiskDataSyncer.Init(bundleName, userId);
    EXPECT_EQ(result, E_OK);
    cloudDiskDataSyncer.Reset();
    EXPECT_EQ(cloudDiskDataSyncer.stage_, BEGIN);
    GTEST_LOG_(INFO) << "Reset End";
}

/**
 * @tc.name: DownloadFileTest001
 * @tc.desc: Verify the DownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, DownloadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFile Start";
    const string bundleName = "bundleName";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    int result = cloudDiskDataSyncer.DownloadFile();
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "DownloadFile End";
}

/**
 * @tc.name: UploadFileTest001
 * @tc.desc: Verify the UploadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, UploadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UploadFile Start";
    const string bundleName = "bundleName";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    int32_t result = cloudDiskDataSyncer.UploadFile();
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "UploadFile End";
}

/**
 * @tc.name: CompleteTest001
 * @tc.desc: Verify the Complete function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, CompleteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Complete Start";
    const string bundleName = "bundleName";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    std::shared_ptr<SdkHelper> sdkHelper = make_shared<SdkHelper>();
    cloudDiskDataSyncer.SetSdkHelper(sdkHelper);
    int32_t result = cloudDiskDataSyncer.Complete();
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "Complete End";
}

/**
 * @tc.name: ChangesNotifyTest001
 * @tc.desc: Verify the ChangesNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, ChangesNotifyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangesNotify Start";
    const string bundleName = "bundleName";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    std::shared_ptr<SdkHelper> sdkHelper = make_shared<SdkHelper>();
    cloudDiskDataSyncer.SetSdkHelper(sdkHelper);
    cloudDiskDataSyncer.ChangesNotify();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "ChangesNotify End";
}

/**
 * @tc.name: ScheduleByTypeTest001
 * @tc.desc: Verify the ScheduleByType function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataSyncerTest, ScheduleByTypeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScheduleByType Start";
    const string bundleName = "bundleName";
    const int32_t userId = 100;
    CloudDiskDataSyncer cloudDiskDataSyncer(bundleName, userId);
    cloudDiskDataSyncer.ScheduleByType(SyncTriggerType::APP_TRIGGER);
    GTEST_LOG_(INFO) << "ScheduleByType End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
