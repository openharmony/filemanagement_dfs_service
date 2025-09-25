/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "cloud_disk_service_logfile.cpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assistant.h"
#include "cloud_disk_common.h"
#include "file_utils.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskServiceLogFileStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AssistantMock> insMock_;
};

void CloudDiskServiceLogFileStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
}

void CloudDiskServiceLogFileStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    insMock_ = nullptr;
    Assistant::ins = nullptr;
}

void CloudDiskServiceLogFileStaticTest::SetUp()
{
}

void CloudDiskServiceLogFileStaticTest::TearDown()
{
}

/**
 * @tc.name: GetLogFileByPathTest001
 * @tc.desc: Verify the GetLogFileByPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetLogFileByPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLogFileByPathTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        string res = GetLogFileByPath(userId, syncFolderIndex);
        string str =
            "/data/service/el2/1/hmdfs/cache/account_cache/dentry_cache/"
            "clouddisk_service_cache/0000000000000001/history.log";
        EXPECT_EQ(res, str);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLogFileByPathTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetLogFileByPathTest001 end";
}

/**
 * @tc.name: GetBucketAndOffsetTest001
 * @tc.desc: Verify the GetBucketAndOffset function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetBucketAndOffsetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketAndOffsetTest001 start";
    try {
        uint64_t line = 1;
        uint32_t bucket = 1;
        uint32_t offset = 1;
        GetBucketAndOffset(line, bucket, offset);
        EXPECT_EQ(bucket, 0);
        EXPECT_EQ(offset, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketAndOffsetTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetBucketAndOffsetTest001 end";
}

/**
 * @tc.name: GetBucketaddrTest001
 * @tc.desc: Verify the GetBucketaddr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetBucketaddrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketaddrTest001 start";
    try {
        uint32_t bucket = 1;
        auto res = GetBucketaddr(bucket);
        EXPECT_EQ(res, 8192);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketaddrTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetBucketaddrTest001 end";
}

/**
 * @tc.name: LoadCurrentPageTest001
 * @tc.desc: Verify the LoadCurrentPage function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, LoadCurrentPageTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadCurrentPageTest001 start";
    try {
        int fd = 1;
        uint32_t bucket = 1;
        auto res = LoadCurrentPage(fd, bucket);
        EXPECT_EQ(res, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadCurrentPageTest001 failed";
    }
    GTEST_LOG_(INFO) << "LoadCurrentPageTest001 end";
}

/**
 * @tc.name: LoadCurrentPageTest002
 * @tc.desc: Verify the LoadCurrentPage function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, LoadCurrentPageTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadCurrentPageTest002 start";
    try {
        int fd = 1;
        uint32_t bucket = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(1));
        auto res = LoadCurrentPage(fd, bucket);
        EXPECT_EQ(res, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadCurrentPageTest002 failed";
    }
    GTEST_LOG_(INFO) << "LoadCurrentPageTest002 end";
}

/**
 * @tc.name: LoadCurrentPageTest003
 * @tc.desc: Verify the LoadCurrentPage function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, LoadCurrentPageTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadCurrentPageTest003 start";
    try {
        int fd = 1;
        uint32_t bucket = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(LOGGROUP_SIZE));
        auto res = LoadCurrentPage(fd, bucket);
        EXPECT_NE(res, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadCurrentPageTest003 failed";
    }
    GTEST_LOG_(INFO) << "LoadCurrentPageTest003 end";
}

/**
 * @tc.name: SyncCurrentPageTest001
 * @tc.desc: Verify the SyncCurrentPage function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, SyncCurrentPageTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncCurrentPageTest001 start";
    try {
        LogGroup logGroup;
        int fd = 1;
        uint32_t line = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(1));
        auto res = SyncCurrentPage(logGroup, fd, line);
        EXPECT_EQ(res, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncCurrentPageTest001 failed";
    }
    GTEST_LOG_(INFO) << "SyncCurrentPageTest001 end";
}

/**
 * @tc.name: SyncCurrentPageTest002
 * @tc.desc: Verify the SyncCurrentPage function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, SyncCurrentPageTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncCurrentPageTest002 start";
    try {
        LogGroup logGroup;
        int fd = 1;
        uint32_t line = 1;
        EXPECT_CALL(*insMock_, WriteFile(_, _, _, _)).WillOnce(Return(LOGGROUP_SIZE));
        auto res = SyncCurrentPage(logGroup, fd, line);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncCurrentPageTest002 failed";
    }
    GTEST_LOG_(INFO) << "SyncCurrentPageTest002 end";
}

/**
 * @tc.name: FillLogGroupTest001
 * @tc.desc: Verify the FillLogGroup function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, FillLogGroupTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillLogGroupTest001 start";
    try {
        LogGroup logGroup;
        LogBlock logBlock;
        uint32_t offset = 0;
        logGroup.nsl[0].timestamp = 1;
        auto res = FillLogGroup(logGroup, logBlock, offset);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillLogGroupTest001 failed";
    }
    GTEST_LOG_(INFO) << "FillLogGroupTest001 end";
}

/**
 * @tc.name: GetCurrentLineTest001
 * @tc.desc: Verify the GetCurrentLine function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetCurrentLineTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentLineTest001 start";
    try {
        int fd = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(4096));
        GetCurrentLine(fd);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCurrentLineTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetCurrentLineTest001 end";
}

/**
 * @tc.name: GetReversalTest001
 * @tc.desc: Verify the GetReversal function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetReversalTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetReversalTest001 start";
    try {
        int fd = 1;
        uint64_t line = 0;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(4096));
        bool res = GetReversal(fd, line);
        EXPECT_EQ(res, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetReversalTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetReversalTest001 end";
}

/**
 * @tc.name: GetReversalTest002
 * @tc.desc: Verify the GetReversal function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetReversalTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetReversalTest002 start";
    try {
        int fd = 1;
        uint64_t line = 0;
        bool res = GetReversal(fd, line);
        EXPECT_EQ(res, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetReversalTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetReversalTest002 end";
}

/**
 * @tc.name: GetReversalTest003
 * @tc.desc: Verify the GetReversal function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetReversalTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetReversalTest003 start";
    try {
        int fd = 1;
        uint64_t line = LOG_COUNT_MAX + 1;
        bool res = GetReversal(fd, line);
        EXPECT_EQ(res, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetReversalTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetReversalTest003 end";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
