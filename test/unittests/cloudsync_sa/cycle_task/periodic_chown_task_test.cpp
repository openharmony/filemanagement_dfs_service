/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "battery_status.h"
#include "cloud_file_fault_event.h"
#include "cloud_pref_impl_mock.h"
#include "cycle_task_runner.h"
#include "dfs_error.h"
#include "periodic_chown_task.h"
#include "screen_status.h"
#include "screen_status_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

static const std::string SPACE_OCCUPY_FILE_PATH = "spaceOccupy.xml";

class PeriodicChownTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<CloudFile::DataSyncManager> dataSyncManagerPtr_ = nullptr;
    static inline std::shared_ptr<PeriodicChownTask> periodicChownTask_ = nullptr;
    static inline std::shared_ptr<CloudPrefImplMock> cloudPrefImplMock_ = nullptr;
    static inline std::shared_ptr<ScreenStatusMock> screenStatusMock_ = nullptr;
};

void PeriodicChownTaskTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dataSyncManagerPtr_ = std::make_shared<CloudFile::DataSyncManager>();
    periodicChownTask_ = std::make_shared<PeriodicChownTask>(dataSyncManagerPtr_);
    cloudPrefImplMock_ = std::make_shared<CloudPrefImplMock>();
    CloudPrefImplMock::proxy_ = cloudPrefImplMock_;
    screenStatusMock_ = std::make_shared<ScreenStatusMock>();
    ScreenStatusMock::proxy_ = screenStatusMock_;
}

void PeriodicChownTaskTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataSyncManagerPtr_ = nullptr;
    periodicChownTask_ = nullptr;
    cloudPrefImplMock_ = nullptr;
    CloudPrefImplMock::proxy_ = nullptr;
    screenStatusMock_ = nullptr;
    ScreenStatusMock::proxy_ = nullptr;
}

void PeriodicChownTaskTest::SetUp(void)
{
    std::system("rm -rf /data/test_tdd");
    periodicChownTask_->periodicChownConfig_ = nullptr;
    periodicChownTask_->directoryTemplates_.clear();
    periodicChownTask_->directoryKeys_.clear();
    periodicChownTask_->fileCount_ = 0;
    periodicChownTask_->totalSize_ = 0;
}

void PeriodicChownTaskTest::TearDown(void)
{
    std::system("rm -rf /data/test_tdd");
}

/*
 * @tc.name: CheckChownConditionTest001
 * @tc.desc: Verify that CheckChownCondition returns false when screen is on.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, CheckChownConditionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckChownConditionTest001 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(true);

        bool result = periodicChownTask_->CheckChownCondition();
        EXPECT_FALSE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckChownConditionTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckChownConditionTest001 End";
}

/*
 * @tc.name: CheckChownConditionTest002
 * @tc.desc: Verify that CheckChownCondition returns false when not charging.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, CheckChownConditionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckChownConditionTest002 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(false);

        bool result = periodicChownTask_->CheckChownCondition();
        EXPECT_FALSE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckChownConditionTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckChownConditionTest002 End";
}

/*
 * @tc.name: CheckChownConditionTest003
 * @tc.desc: Verify that CheckChownCondition returns true when screen is off and charging.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, CheckChownConditionTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckChownConditionTest003 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        bool result = periodicChownTask_->CheckChownCondition();
        EXPECT_TRUE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckChownConditionTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckChownConditionTest003 End";
}

/*
 * @tc.name: GetTargetDirectoryTest001
 * @tc.desc: Verify that GetTargetDirectory returns correct path for valid index 0.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GetTargetDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest001 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {
            "/data/service/el2/{userId}/hmdfs/account/files/Photo",
            "/data/service/el2/{userId}/hmdfs/account/files/.thumbs",
            "/data/service/el2/{userId}/hmdfs/account/files/.editData"
        };

        int32_t userId = 100;
        int32_t index = 0;
        std::string result = periodicChownTask_->GetTargetDirectory(userId, index);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/account/files/Photo");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetTargetDirectoryTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest001 End";
}

/*
 * @tc.name: GetTargetDirectoryTest002
 * @tc.desc: Verify that GetTargetDirectory returns correct path for valid index 1.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GetTargetDirectoryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest002 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {
            "/data/service/el2/{userId}/hmdfs/account/files/Photo",
            "/data/service/el2/{userId}/hmdfs/account/files/.thumbs",
            "/data/service/el2/{userId}/hmdfs/account/files/.editData"
        };

        int32_t userId = 100;
        int32_t index = 1;
        std::string result = periodicChownTask_->GetTargetDirectory(userId, index);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/account/files/.thumbs");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetTargetDirectoryTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest002 End";
}

/*
 * @tc.name: GetTargetDirectoryTest003
 * @tc.desc: Verify that GetTargetDirectory returns correct path for valid index 2.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GetTargetDirectoryTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest003 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {
            "/data/service/el2/{userId}/hmdfs/account/files/Photo",
            "/data/service/el2/{userId}/hmdfs/account/files/.thumbs",
            "/data/service/el2/{userId}/hmdfs/account/files/.editData"
        };

        int32_t userId = 100;
        int32_t index = 2;
        std::string result = periodicChownTask_->GetTargetDirectory(userId, index);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/account/files/.editData");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetTargetDirectoryTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest003 End";
}

/*
 * @tc.name: GetTargetDirectoryTest004
 * @tc.desc: Verify that GetTargetDirectory returns empty string for negative index.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GetTargetDirectoryTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest004 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {
            "/data/service/el2/{userId}/hmdfs/account/files/Photo"
        };

        int32_t userId = 100;
        int32_t index = -1;
        std::string result = periodicChownTask_->GetTargetDirectory(userId, index);
        EXPECT_TRUE(result.empty());
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetTargetDirectoryTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest004 End";
}

/*
 * @tc.name: GetTargetDirectoryTest005
 * @tc.desc: Verify that GetTargetDirectory returns empty string for out of range index.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GetTargetDirectoryTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest005 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {
            "/data/service/el2/{userId}/hmdfs/account/files/Photo"
        };

        int32_t userId = 100;
        int32_t index = 1;
        std::string result = periodicChownTask_->GetTargetDirectory(userId, index);
        EXPECT_TRUE(result.empty());
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetTargetDirectoryTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "GetTargetDirectoryTest005 End";
}

/*
 * @tc.name: GenerateDoneKeyTest001
 * @tc.desc: Verify that GenerateDoneKey returns correct key for valid path.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateDoneKeyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateDoneKeyTest001 Start";
    try {
        std::string dirPath = "/data/service/el2/100/hmdfs/account/files/Photo";
        std::string result = periodicChownTask_->GenerateDoneKey(dirPath);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "account_files_Photo_done");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateDoneKeyTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateDoneKeyTest001 End";
}

/*
 * @tc.name: GenerateDoneKeyTest002
 * @tc.desc: Verify that GenerateDoneKey returns correct key for path with dot prefix.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateDoneKeyTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateDoneKeyTest002 Start";
    try {
        std::string dirPath = "/data/service/el2/100/hmdfs/account/files/.thumbs";
        std::string result = periodicChownTask_->GenerateDoneKey(dirPath);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "account_files_thumbs_done");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateDoneKeyTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateDoneKeyTest002 End";
}

/*
 * @tc.name: GenerateDoneKeyTest003
 * @tc.desc: Verify that GenerateDoneKey returns empty string for path with single slash.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateDoneKeyTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateDoneKeyTest003 Start";
    try {
        std::string dirPath = "/data";
        std::string result = periodicChownTask_->GenerateDoneKey(dirPath);
        EXPECT_TRUE(result.empty());
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateDoneKeyTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateDoneKeyTest003 End";
}

/*
 * @tc.name: GenerateTimestampKeyTest001
 * @tc.desc: Verify that GenerateTimestampKey returns correct key for valid path.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateTimestampKeyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateTimestampKeyTest001 Start";
    try {
        std::string dirPath = "/data/service/el2/100/hmdfs/account/files/Photo";
        std::string result = periodicChownTask_->GenerateTimestampKey(dirPath);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "account_files_Photo_timestamp");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateTimestampKeyTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateTimestampKeyTest001 End";
}

/*
 * @tc.name: GenerateTimestampKeyTest002
 * @tc.desc: Verify that GenerateTimestampKey returns correct key for path with dot prefix.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateTimestampKeyTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateTimestampKeyTest002 Start";
    try {
        std::string dirPath = "/data/service/el2/100/hmdfs/account/files/.editData";
        std::string result = periodicChownTask_->GenerateTimestampKey(dirPath);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "account_files_editData_timestamp");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateTimestampKeyTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateTimestampKeyTest002 End";
}

/*
 * @tc.name: GenerateKeyWithSuffixTest001
 * @tc.desc: Verify that GenerateKeyWithSuffix returns correct key for valid path and suffix.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateKeyWithSuffixTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest001 Start";
    try {
        std::string dirPath = "/data/service/el2/100/hmdfs/account/files/Photo";
        std::string suffix = "_custom";
        std::string result = periodicChownTask_->GenerateKeyWithSuffix(dirPath, suffix);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "account_files_Photo_custom");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateKeyWithSuffixTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest001 End";
}

/*
 * @tc.name: GenerateKeyWithSuffixTest002
 * @tc.desc: Verify that GenerateKeyWithSuffix returns key without suffix when suffix is empty.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateKeyWithSuffixTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest002 Start";
    try {
        std::string dirPath = "/data/service/el2/100/hmdfs/account/files/Photo";
        std::string suffix = "";
        std::string result = periodicChownTask_->GenerateKeyWithSuffix(dirPath, suffix);
        EXPECT_FALSE(result.empty());
        EXPECT_EQ(result, "account_files_Photo");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateKeyWithSuffixTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest002 End";
}

/*
 * @tc.name: GenerateKeyWithSuffixTest003
 * @tc.desc: Verify that GenerateKeyWithSuffix returns empty string for path with only one slash.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateKeyWithSuffixTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest003 Start";
    try {
        std::string dirPath = "/data";
        std::string suffix = "_done";
        std::string result = periodicChownTask_->GenerateKeyWithSuffix(dirPath, suffix);
        EXPECT_TRUE(result.empty());
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateKeyWithSuffixTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest003 End";
}

/*
 * @tc.name: GenerateKeyWithSuffixTest004
 * @tc.desc: Verify that GenerateKeyWithSuffix returns empty string for path with two slashes and begin with slash.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateKeyWithSuffixTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest004 Start";
    try {
        std::string dirPath = "/data/service";
        std::string suffix = "_done";
        std::string result = periodicChownTask_->GenerateKeyWithSuffix(dirPath, suffix);
        EXPECT_TRUE(result.empty());
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateKeyWithSuffixTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest004 End";
}

/*
 * @tc.name: GenerateKeyWithSuffixTest005
 * @tc.desc: Verify that GenerateKeyWithSuffix returns empty string for path with only two slashes.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, GenerateKeyWithSuffixTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest005 Start";
    try {
        std::string dirPath = "test/data/service";
        std::string suffix = "_done";
        std::string result = periodicChownTask_->GenerateKeyWithSuffix(dirPath, suffix);
        EXPECT_TRUE(result.empty());
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GenerateKeyWithSuffixTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "GenerateKeyWithSuffixTest005 End";
}

/*
 * @tc.name: ShouldChownFileTest001
 * @tc.desc: Verify that ShouldChownFile returns true for regular file with DFS_UID.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ShouldChownFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShouldChownFileTest001 Start";
    try {
        struct stat st {};
        st.st_mode = S_IFREG | 0644;
        st.st_uid = 1009;
        st.st_mtime = 1000;
        int64_t dirTimestamp = 2000;

        bool result = periodicChownTask_->ShouldChownFile(st, dirTimestamp);
        EXPECT_TRUE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ShouldChownFileTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ShouldChownFileTest001 End";
}

/*
 * @tc.name: ShouldChownFileTest002
 * @tc.desc: Verify that ShouldChownFile returns false for directory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ShouldChownFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShouldChownFileTest002 Start";
    try {
        struct stat st {};
        st.st_mode = S_IFDIR | 0755;
        st.st_uid = 1009;
        st.st_mtime = 1000;
        int64_t dirTimestamp = 2000;

        bool result = periodicChownTask_->ShouldChownFile(st, dirTimestamp);
        EXPECT_FALSE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ShouldChownFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ShouldChownFileTest002 End";
}

/*
 * @tc.name: ShouldChownFileTest003
 * @tc.desc: Verify that ShouldChownFile returns false for file with different UID.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ShouldChownFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShouldChownFileTest003 Start";
    try {
        struct stat st {};
        st.st_mode = S_IFREG | 0644;
        st.st_uid = 1000;
        st.st_mtime = 1000;
        int64_t dirTimestamp = 2000;

        bool result = periodicChownTask_->ShouldChownFile(st, dirTimestamp);
        EXPECT_FALSE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ShouldChownFileTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ShouldChownFileTest003 End";
}

/*
 * @tc.name: ShouldChownFileTest004
 * @tc.desc: Verify that ShouldChownFile returns false for file modified after timestamp.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ShouldChownFileTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShouldChownFileTest004 Start";
    try {
        struct stat st {};
        st.st_mode = S_IFREG | 0644;
        st.st_uid = 1009;
        st.st_mtime = 2000;
        int64_t dirTimestamp = 1000;

        bool result = periodicChownTask_->ShouldChownFile(st, dirTimestamp);
        EXPECT_FALSE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ShouldChownFileTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "ShouldChownFileTest004 End";
}

/*
 * @tc.name: ShouldChownFileTest005
 * @tc.desc: Verify that ShouldChownFile returns true when no timestamp is set.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ShouldChownFileTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShouldChownFileTest005 Start";
    try {
        struct stat st {};
        st.st_mode = S_IFREG | 0644;
        st.st_uid = 1009;
        st.st_mtime = 1000;
        int64_t dirTimestamp = 0;

        bool result = periodicChownTask_->ShouldChownFile(st, dirTimestamp);
        EXPECT_TRUE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ShouldChownFileTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "ShouldChownFileTest005 End";
}

/*
 * @tc.name: ValidateAndChownDirectoryTest001
 * @tc.desc: Verify that ValidateAndChownDirectory returns E_PATH for non-existent directory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ValidateAndChownDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ValidateAndChownDirectoryTest001 Start";
    try {
        std::string path = "/data/test_tdd/nonexistent_dir";
        int32_t ret = periodicChownTask_->ValidateAndChownDirectory(path);
        EXPECT_EQ(ret, E_PATH);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ValidateAndChownDirectoryTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ValidateAndChownDirectoryTest001 End";
}

/*
 * @tc.name: ValidateAndChownDirectoryTest002
 * @tc.desc: Verify that ValidateAndChownDirectory returns E_OK for existing directory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ValidateAndChownDirectoryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ValidateAndChownDirectoryTest002 Start";
    try {
        std::system("mkdir -p /data/test_tdd/test_dir");
        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ValidateAndChownDirectory(path);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ValidateAndChownDirectoryTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ValidateAndChownDirectoryTest002 End";
}

/*
 * @tc.name: ValidateAndChownDirectoryTest003
 * @tc.desc: Verify that ValidateAndChownDirectory returns E_OK for directory with DFS_UID_UID.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ValidateAndChownDirectoryTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ValidateAndChownDirectoryTest003 Start";
    try {
        std::system("mkdir -p /data/test_tdd/test_dir");
        std::string path = "/data/test_tdd/test_dir";
        std::system("chown 1009:1009 /data/test_tdd/test_dir");
        int32_t ret = periodicChownTask_->ValidateAndChownDirectory(path);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ValidateAndChownDirectoryTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ValidateAndChownDirectoryTest003 End";
}

/*
 * @tc.name: ValidateAndChownDirectoryTest004
 * @tc.desc: Verify that ValidateAndChownDirectory returns E_PATH when stat fails.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ValidateAndChownDirectoryTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ValidateAndChownDirectoryTest004 Start";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("ln -s /nonexistent_path /data/test_tdd/broken_link");
        std::string path = "/data/test_tdd/broken_link";
        int32_t ret = periodicChownTask_->ValidateAndChownDirectory(path);
        EXPECT_EQ(ret, E_PATH);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ValidateAndChownDirectoryTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "ValidateAndChownDirectoryTest004 End";
}

/*
 * @tc.name: CheckAllDirectoriesDoneTest001
 * @tc.desc: Verify that CheckAllDirectoriesDone returns true when all directories are done.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, CheckAllDirectoriesDoneTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAllDirectoriesDoneTest001 Start";
    try {
        periodicChownTask_->directoryKeys_ = {"key1", "key2", "key3"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(true)));

        bool result = periodicChownTask_->CheckAllDirectoriesDone();
        EXPECT_TRUE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckAllDirectoriesDoneTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckAllDirectoriesDoneTest001 End";
}

/*
 * @tc.name: CheckAllDirectoriesDoneTest002
 * @tc.desc: Verify that CheckAllDirectoriesDone returns false when not all directories are done.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, CheckAllDirectoriesDoneTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAllDirectoriesDoneTest002 Start";
    try {
        periodicChownTask_->directoryKeys_ = {"key1", "key2", "key3"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(true)))
            .WillOnce(DoAll(SetArgReferee<1>(false)));

        bool result = periodicChownTask_->CheckAllDirectoriesDone();
        EXPECT_FALSE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckAllDirectoriesDoneTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckAllDirectoriesDoneTest002 End";
}

/*
 * @tc.name: CheckAllDirectoriesDoneTest003
 * @tc.desc: Verify that CheckAllDirectoriesDone returns true when directory keys are empty.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, CheckAllDirectoriesDoneTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAllDirectoriesDoneTest003 Start";
    try {
        periodicChownTask_->directoryKeys_.clear();
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        bool result = periodicChownTask_->CheckAllDirectoriesDone();
        EXPECT_TRUE(result);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckAllDirectoriesDoneTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckAllDirectoriesDoneTest003 End";
}

/*
 * @tc.name: RunTaskForBundleTest001
 * @tc.desc: Verify that RunTaskForBundle returns E_STOP when screen is on.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, RunTaskForBundleTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        std::string bundleName = "com.photos";
        int32_t ret = periodicChownTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 End";
}

/*
 * @tc.name: RunTaskForBundleTest002
 * @tc.desc: Verify that RunTaskForBundle returns E_STOP when not charging.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, RunTaskForBundleTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(false);

        int32_t userId = 100;
        std::string bundleName = "com.photos";
        int32_t ret = periodicChownTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 End";
}

/*
 * @tc.name: RunTaskForBundleTest003
 * @tc.desc: Verify that RunTaskForBundle returns E_OK when all directories are done.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, RunTaskForBundleTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        periodicChownTask_->directoryKeys_ = {"key1"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(std::string())));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(true)));
        EXPECT_CALL(*cloudPrefImplMock_, SetBool(_, _))
            .Times(AnyNumber());

        int32_t userId = 100;
        std::string bundleName = "com.photos";
        int32_t ret = periodicChownTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 End";
}

/*
 * @tc.name: RunTaskForBundleTest004
 * @tc.desc: Verify that RunTaskForBundle when uid not correct.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, RunTaskForBundleTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        periodicChownTask_->directoryKeys_ = {"key1"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(std::string())));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(true)));
        EXPECT_CALL(*cloudPrefImplMock_, SetBool(_, _))
            .Times(AnyNumber());

        int32_t userId = 101;
        std::string bundleName = "com.photos";
        int32_t ret = periodicChownTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 End";
}

/*
 * @tc.name: RunTaskForBundleTest005
 * @tc.desc: Verify that RunTaskForBundle when uid not correct.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, RunTaskForBundleTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        periodicChownTask_->directoryKeys_ = {"key1"};
        periodicChownTask_->periodicChownConfig_ = nullptr;

        EXPECT_CALL(*cloudPrefImplMock_, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(std::string())));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(true)));
        EXPECT_CALL(*cloudPrefImplMock_, SetBool(_, _))
            .Times(AnyNumber());

        int32_t userId = 100;
        std::string bundleName = "com.photos";
        int32_t ret = periodicChownTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 End";
}

/*
 * @tc.name: RunTaskForBundleTest006
 * @tc.desc: Verify that RunTaskForBundle when uid not correct.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, RunTaskForBundleTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        periodicChownTask_->directoryKeys_ = {"key1"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(std::string())));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*cloudPrefImplMock_, SetBool(_, _))
            .Times(AnyNumber());

        int32_t userId = 100;
        std::string bundleName = "com.photos";
        int32_t ret = periodicChownTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 End";
}

/*
 * @tc.name: InitializeDirectoryTemplatesTest001
 * @tc.desc: Verify that InitializeDirectoryTemplates sets default templates when config is empty.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, InitializeDirectoryTemplatesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitializeDirectoryTemplatesTest001 Start";
    try {
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(std::string())));

        periodicChownTask_->InitializeDirectoryTemplates();

        EXPECT_EQ(periodicChownTask_->directoryTemplates_.size(), 3);
        EXPECT_EQ(periodicChownTask_->directoryKeys_.size(), 3);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "InitializeDirectoryTemplatesTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "InitializeDirectoryTemplatesTest001 End";
}

/*
 * @tc.name: InitializeDirectoryTemplatesTest002
 * @tc.desc: Verify that InitializeDirectoryTemplates parses existing config correctly.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, InitializeDirectoryTemplatesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitializeDirectoryTemplatesTest002 Start";
    try {
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);
        std::string configString
            = "/data/service/el2/100/hmdfs/account/files/Photo;/data/service/el2/100/hmdfs/account/files/.thumbs";

        EXPECT_CALL(*cloudPrefImplMock_, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(configString)));
        EXPECT_CALL(*cloudPrefImplMock_, SetString(_, _))
            .Times(0);

        periodicChownTask_->InitializeDirectoryTemplates();

        EXPECT_EQ(periodicChownTask_->directoryTemplates_.size(), 2);
        EXPECT_EQ(periodicChownTask_->directoryKeys_.size(), 2);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "InitializeDirectoryTemplatesTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "InitializeDirectoryTemplatesTest002 End";
}

/*
 * @tc.name: InitializeDirectoryTemplatesTest003
 * @tc.desc: Verify that InitializeDirectoryTemplates parses config with multiple separators correctly.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, InitializeDirectoryTemplatesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitializeDirectoryTemplatesTest003 Start";
    try {
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);
        std::string configString = "dir1;;dir2;dir3";

        EXPECT_CALL(*cloudPrefImplMock_, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(configString)));
        EXPECT_CALL(*cloudPrefImplMock_, SetString(_, _))
            .Times(0);

        periodicChownTask_->InitializeDirectoryTemplates();

        EXPECT_EQ(periodicChownTask_->directoryTemplates_.size(), 3);
        EXPECT_EQ(periodicChownTask_->directoryTemplates_[0], "dir1");
        EXPECT_EQ(periodicChownTask_->directoryTemplates_[1], "dir2");
        EXPECT_EQ(periodicChownTask_->directoryTemplates_[2], "dir3");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "InitializeDirectoryTemplatesTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "InitializeDirectoryTemplatesTest003 End";
}

/*
 * @tc.name: ProcessSingleDirectoryTest001
 * @tc.desc: Verify that ProcessSingleDirectory returns E_PATH for non-existent directory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ProcessSingleDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSingleDirectoryTest001 Start";
    try {
        std::string path = "/data/test_tdd/nonexistent_dir";
        int32_t ret = periodicChownTask_->ProcessSingleDirectory(path, 0);
        EXPECT_EQ(ret, E_PATH);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ProcessSingleDirectoryTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ProcessSingleDirectoryTest001 End";
}

/*
 * @tc.name: ProcessSingleDirectoryTest002
 * @tc.desc: Verify that ProcessSingleDirectory returns E_OK for empty directory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ProcessSingleDirectoryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSingleDirectoryTest002 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));

        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ProcessSingleDirectory(path, 0);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ProcessSingleDirectoryTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ProcessSingleDirectoryTest002 End";
}

/*
 * @tc.name: ProcessSingleDirectoryTest003
 * @tc.desc: Verify that ProcessSingleDirectory returns E_OK for directory with subdirectories.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ProcessSingleDirectoryTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSingleDirectoryTest003 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir/subdir1/subdir2");

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(true)));
        EXPECT_CALL(*cloudPrefImplMock_, SetBool(_, _))
            .Times(AnyNumber());

        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ProcessSingleDirectory(path, 0);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ProcessSingleDirectoryTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ProcessSingleDirectoryTest003 End";
}

/*
 * @tc.name: ChownFilesTest001
 * @tc.desc: Verify that ChownFiles returns E_OK for non-existent directory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownFilesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownFilesTest001 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));

        std::string path = "/data/test_tdd/nonexistent_dir";
        int32_t ret = periodicChownTask_->ChownFiles(path, 0);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownFilesTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownFilesTest001 End";
}

/*
 * @tc.name: ChownFilesTest002
 * @tc.desc: Verify that ChownFiles returns E_OK for empty directory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownFilesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownFilesTest002 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));

        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ChownFiles(path, 0);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownFilesTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownFilesTest002 End";
}

/*
 * @tc.name: ChownFilesTest003
 * @tc.desc: Verify that ChownFiles returns E_OK for directory with files.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownFilesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownFilesTest003 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");
        std::system("touch /data/test_tdd/test_dir/file1.txt");
        std::system("touch /data/test_tdd/test_dir/file2.txt");

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));

        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ChownFiles(path, 0);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownFilesTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownFilesTest003 End";
}

/*
 * @tc.name: ChownFilesTest004
 * @tc.desc: Verify that ChownFiles returns E_OK for directory with nested files.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownFilesTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownFilesTest004 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir/subdir");
        std::system("touch /data/test_tdd/test_dir/file1.txt");
        std::system("touch /data/test_tdd/test_dir/subdir/file2.txt");

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));

        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ChownFiles(path, 0);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownFilesTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownFilesTest004 End";
}

/*
 * @tc.name: ChownAllDirectoriesTest001
 * @tc.desc: Verify that ChownAllDirectories returns E_OK for empty templates.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownAllDirectoriesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest001 Start";
    try {
        periodicChownTask_->directoryTemplates_.clear();
        int32_t ret = periodicChownTask_->ChownAllDirectories(100);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownAllDirectoriesTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest001 End";
}

/*
 * @tc.name: ChownAllDirectoriesTest002
 * @tc.desc: Verify that ChownAllDirectories skips directory when already done.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownAllDirectoriesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest002 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/service/el2/100/hmdfs/account/files/Photo"};
        periodicChownTask_->directoryKeys_ = {"account_files_photo_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(true)));

        int32_t ret = periodicChownTask_->ChownAllDirectories(100);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownAllDirectoriesTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest002 End";
}

/*
 * @tc.name: ChownAllDirectoriesTest003
 * @tc.desc: Verify that ChownAllDirectories skips directory with timestamp.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownAllDirectoriesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest003 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/service/el2/100/hmdfs/account/files/Photo"};
        periodicChownTask_->directoryKeys_ = {"account_files_photo_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(static_cast<int64_t>(1000))));

        int32_t ret = periodicChownTask_->ChownAllDirectories(100);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownAllDirectoriesTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest003 End";
}

/*
 * @tc.name: ChownAllDirectoriesTest004
 * @tc.desc: Verify that ChownAllDirectories handles empty dirPath from GetTargetDirectory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownAllDirectoriesTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest004 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/service/el2/100/hmdfs/account/files/Photo"};
        periodicChownTask_->directoryKeys_ = {"account_files_photo_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(1000))));

        int32_t ret = periodicChownTask_->ChownAllDirectories(100);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownAllDirectoriesTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest004 End";
}

/*
 * @tc.name: ChownAllDirectoriesTest005
 * @tc.desc: Verify that ChownAllDirectories handles empty timestampKey.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownAllDirectoriesTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest005 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data"};
        periodicChownTask_->directoryKeys_ = {"data_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(false)));

        int32_t ret = periodicChownTask_->ChownAllDirectories(100);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownAllDirectoriesTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest005 End";
}

/*
 * @tc.name: ExecuteChownTaskTest001
 * @tc.desc: Verify that ExecuteChownTask returns E_OK for empty templates.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ExecuteChownTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest001 Start";
    try {
        periodicChownTask_->directoryTemplates_.clear();
        int32_t ret = periodicChownTask_->ExecuteChownTask(100);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ExecuteChownTaskTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest001 End";
}

/*
 * @tc.name: ExecuteChownTaskTest002
 * @tc.desc: Verify that ExecuteChownTask returns E_OK for valid user ID.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ExecuteChownTaskTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest002 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->directoryKeys_ = {"chown_test_dir_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*cloudPrefImplMock_, SetBool(_, _))
            .Times(1);

        int32_t ret = periodicChownTask_->ExecuteChownTask(100);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ExecuteChownTaskTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest002 End";
}

/*
 * @tc.name: ExecuteChownTaskTest003
 * @tc.desc: Verify that ExecuteChownTask skips directory when already done.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ExecuteChownTaskTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest003 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->directoryKeys_ = {"chown_test_dir_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(true)));

        int32_t ret = periodicChownTask_->ExecuteChownTask(100);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ExecuteChownTaskTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest003 End";
}

/*
 * @tc.name: ExecuteChownTaskTest004
 * @tc.desc: Verify that ExecuteChownTask handles empty dirPath from GetTargetDirectory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ExecuteChownTaskTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest004 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/service/el2/100/hmdfs/account/files/Photo"};
        periodicChownTask_->directoryKeys_ = {"account_files_photo_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(false)));
        EXPECT_CALL(*cloudPrefImplMock_, SetBool(_, _))
            .Times(AnyNumber());

        int32_t ret = periodicChownTask_->ExecuteChownTask(200);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ExecuteChownTaskTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest004 End";
}

/*
 * @tc.name: ChangeUidToMediaTest001
 * @tc.desc: Verify that ChangeUidToMedia does not crash for non-existent file.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChangeUidToMediaTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidToMediaTest001 Start";
    try {
        std::string path = "/data/test_tdd/nonexistent_file.txt";
        struct stat st {};
        st.st_mode = S_IFREG | 0644;
        st.st_size = 1024;

        periodicChownTask_->ChangeUidToMedia(path, 0660, st);
        EXPECT_TRUE(true);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChangeUidToMediaTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ChangeUidToMediaTest001 End";
}

/*
 * @tc.name: ChangeUidToMediaTest002
 * @tc.desc: Verify that ChangeUidToMedia updates counters for existing file.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChangeUidToMediaTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidToMediaTest002 Start";
    try {
        std::system("mkdir /data/test_tdd");
        std::system("touch /data/test_tdd/test_file.txt");
        std::system("chmod 0660 /data/test_tdd/test_file.txt");
        std::string path = "/data/test_tdd/test_file.txt";
        struct stat st {};
        st.st_mode = S_IFREG | 0660;
        st.st_size = 1024;
        periodicChownTask_->ChangeUidToMedia(path, 0660, st);

        EXPECT_EQ(periodicChownTask_->fileCount_, 1);
        EXPECT_EQ(periodicChownTask_->totalSize_, 1024);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChangeUidToMediaTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ChangeUidToMediaTest002 End";
}

/*
 * @tc.name: ChangeUidToMediaTest003
 * @tc.desc: Verify that ChangeUidToMedia does not update counters for directory.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChangeUidToMediaTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidToMediaTest003 Start";
    try {
        std::system("mkdir -p /data/test_tdd/test_dir");
        std::string path = "/data/test_tdd/test_dir";
        struct stat st {};
        st.st_mode = S_IFDIR | 0755;
        st.st_size = 4096;

        periodicChownTask_->fileCount_ = 0;
        periodicChownTask_->totalSize_ = 0;
        periodicChownTask_->ChangeUidToMedia(path, 0771, st);

        EXPECT_EQ(periodicChownTask_->fileCount_, 0);
        EXPECT_EQ(periodicChownTask_->totalSize_, 0);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChangeUidToMediaTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ChangeUidToMediaTest003 End";
}

/*
 * @tc.name: ReportChownFilesResultTest001
 * @tc.desc: Verify that ReportChownFilesResult logs success for E_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ReportChownFilesResultTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportChownFilesResultTest001 Start";
    try {
        periodicChownTask_->fileCount_ = 10;
        periodicChownTask_->totalSize_ = 10240;
        std::string path = "/data/test_tdd/test_dir";

        periodicChownTask_->ReportChownFilesResult(path, E_OK);
        EXPECT_TRUE(true);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ReportChownFilesResultTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ReportChownFilesResultTest001 End";
}

/*
 * @tc.name: ReportChownFilesResultTest002
 * @tc.desc: Verify that ReportChownFilesResult logs interruption for E_STOP.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ReportChownFilesResultTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportChownFilesResultTest002 Start";
    try {
        periodicChownTask_->fileCount_ = 5;
        periodicChownTask_->totalSize_ = 5120;
        std::string path = "/data/test_tdd/test_dir";

        periodicChownTask_->ReportChownFilesResult(path, E_STOP);
        EXPECT_TRUE(true);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ReportChownFilesResultTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ReportChownFilesResultTest002 End";
}

/*
 * @tc.name: ProcessSingleDirectoryTest004
 * @tc.desc: Verify that ProcessSingleDirectory returns E_STOP when CheckChownCondition fails during batch processing.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ProcessSingleDirectoryTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSingleDirectoryTest004 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");
        for (int i = 0; i < 105; i++) {
            std::string cmd = "mkdir -p /data/test_tdd/test_dir/subdir_" + std::to_string(i);
            std::system(cmd.c_str());
        }
        std::system("chown -R 1009:1009 /data/test_tdd/test_dir");

        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(true));

        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ProcessSingleDirectory(path, 0);
        EXPECT_EQ(ret, E_STOP);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ProcessSingleDirectoryTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "ProcessSingleDirectoryTest004 End";
}

/*
 * @tc.name: ChownFilesTest005
 * @tc.desc: Verify that ChownFiles returns E_STOP when CheckChownCondition fails during batch processing.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownFilesTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownFilesTest005 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/test_dir"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");
        for (int i = 0; i < 105; i++) {
            std::string cmd = "touch /data/test_tdd/test_dir/file_" + std::to_string(i) + ".txt";
            std::system(cmd.c_str());
        }
        std::system("chown -R 1009:1009 /data/test_tdd/test_dir");

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(true));

        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ChownFiles(path, 0);
        EXPECT_EQ(ret, E_STOP);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownFilesTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownFilesTest005 End";
}

/*
 * @tc.name: ChownFilesTest006
 * @tc.desc: Verify that ChownFiles handles empty timestampKey.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownFilesTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownFilesTest006 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");

        std::string path = "/data/test_tdd/test_dir";
        int32_t ret = periodicChownTask_->ChownFiles(path, 0);
        EXPECT_EQ(ret, E_PATH);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownFilesTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownFilesTest006 End";
}

/*
 * @tc.name: ChownAllDirectoriesTest006
 * @tc.desc: Verify that ChownAllDirectories returns error when ProcessSingleDirectory fails.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ChownAllDirectoriesTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest006 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/nonexistent_dir"};
        periodicChownTask_->directoryKeys_ = {"chown_nonexistent_dir_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(false)));

        int32_t ret = periodicChownTask_->ChownAllDirectories(100);
        EXPECT_EQ(ret, E_PATH);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownAllDirectoriesTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownAllDirectoriesTest006 End";
}

/*
 * @tc.name: ExecuteChownTaskTest005
 * @tc.desc: Verify that ExecuteChownTask returns error when ChownFiles fails.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ExecuteChownTaskTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest005 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data"};
        periodicChownTask_->directoryKeys_ = {"data_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        std::system("mkdir -p /data/test_tdd/test_dir");

        EXPECT_CALL(*cloudPrefImplMock_, GetLong(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int64_t>(0))));
        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(false)));

        int32_t ret = periodicChownTask_->ExecuteChownTask(100);
        EXPECT_EQ(ret, E_PATH);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ExecuteChownTaskTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest005 End";
}

/*
 * @tc.name: ExecuteChownTaskTest006
 * @tc.desc: Verify that ExecuteChownTask returns error when ChownAllDirectories fails.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicChownTaskTest, ExecuteChownTaskTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest006 Start";
    try {
        periodicChownTask_->directoryTemplates_ = {"/data/test_tdd/nonexistentexistent_dir"};
        periodicChownTask_->directoryKeys_ = {"nonexistent_dir_done"};
        periodicChownTask_->periodicChownConfig_ = std::make_unique<CloudPrefImpl>(100, "", SPACE_OCCUPY_FILE_PATH);

        EXPECT_CALL(*cloudPrefImplMock_, GetBool(_, _))
            .WillRepeatedly(DoAll(SetArgReferee<1>(false)));

        int32_t ret = periodicChownTask_->ExecuteChownTask(100);
        EXPECT_EQ(ret, E_PATH);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ExecuteChownTaskTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "ExecuteChownTaskTest006 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test
