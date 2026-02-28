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

namespace {
    const string EXPECTED_LOG_PATH = "/data/service/el2/1/hmdfs/cache/account_cache/dentry_cache/"
                                     "clouddisk_service_cache/0000000000000001/history.log";
    const uint64_t BUCKET_ADDR = 8192;
    const uint64_t EXPECTED_CURRENT_LINE = 5u;
}

class CloudDiskServiceLogFileStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<AssistantMock> insMock_;
};

void CloudDiskServiceLogFileStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskServiceLogFileStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskServiceLogFileStaticTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    insMock_->EnableMock();
}

void CloudDiskServiceLogFileStaticTest::TearDown()
{
    insMock_->DisableMock();
    insMock_ = nullptr;
    Assistant::ins = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetLogFileByPathTest001
 * @tc.desc: Verify the GetLogFileByPath function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetLogFileByPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLogFileByPathTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        string res = GetLogFileByPath(userId, syncFolderIndex);
        EXPECT_EQ(res, EXPECTED_LOG_PATH);
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
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetBucketAndOffsetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketAndOffsetTest001 start";
    try {
        uint64_t line = 1;
        uint32_t bucket = 1;
        uint32_t offset = 0;
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
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetBucketaddrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketaddrTest001 start";
    try {
        uint32_t bucket = 1;
        auto res = GetBucketaddr(bucket);
        EXPECT_EQ(res, BUCKET_ADDR);
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
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, LoadCurrentPageTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadCurrentPageTest001 start";
    try {
        int fd = 1;
        uint32_t bucket = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(0));
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
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, LoadCurrentPageTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadCurrentPageTest002 start";
    try {
        int fd = 1;
        uint32_t bucket = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(LOGGROUP_SIZE));
        auto res = LoadCurrentPage(fd, bucket);
        EXPECT_NE(res, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadCurrentPageTest002 failed";
    }
    GTEST_LOG_(INFO) << "LoadCurrentPageTest002 end";
}

/**
 * @tc.name: SyncCurrentPageTest001
 * @tc.desc: Verify SyncCurrentPage function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, SyncCurrentPageTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncCurrentPageTest001 start";
    try {
        LogGroup logGroup;
        int fd = 1;
        uint32_t bucket = 1;
        EXPECT_CALL(*insMock_, WriteFile(_, _, _, _)).WillOnce(Return(0));
        auto res = SyncCurrentPage(logGroup, fd, bucket);
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
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, SyncCurrentPageTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncCurrentPageTest002 start";
    try {
        LogGroup logGroup;
        int fd = 1;
        uint32_t bucket = 1;
        EXPECT_CALL(*insMock_, WriteFile(_, _, _, _)).WillOnce(Return(LOGGROUP_SIZE));
        auto res = SyncCurrentPage(logGroup, fd, bucket);
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
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, FillLogGroupTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillLogGroupTest001 start";
    try {
        LogGroup logGroup = {0};
        LogBlock logBlock = {0};
        uint32_t offset = 0;
        logGroup.nsl[0].timestamp = 0;
        auto res = FillLogGroup(logGroup, logBlock, offset);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(logGroup.logBlockCnt, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillLogGroupTest001 failed";
    }
    GTEST_LOG_(INFO) << "FillLogGroupTest001 end";
}

/**
 * @tc.name: FillLogGroupTest002
 * @tc.desc: Verify the FillLogGroup function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, FillLogGroupTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillLogGroupTest002 start";
    try {
        LogGroup logGroup = {0};
        LogBlock logBlock = {0};
        uint32_t offset = 0;
        logGroup.nsl[0].timestamp = 1;
        auto res = FillLogGroup(logGroup, logBlock, offset);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(logGroup.logBlockCnt, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillLogGroupTest002 failed";
    }
    GTEST_LOG_(INFO) << "FillLogGroupTest002 end";
}

/**
 * @tc.name: GetCurrentLineTest001
 * @tc.desc: Verify the GetCurrentLine function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetCurrentLineTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentLineTest001 start";
    try {
        int fd = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(0));
        EXPECT_EQ(GetCurrentLine(fd), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCurrentLineTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetCurrentLineTest001 end";
}

/**
 * @tc.name: GetCurrentLineTest002
 * @tc.desc: Verify the GetCurrentLine function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetCurrentLineTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentLineTest002 start";
    try {
        int fd = 1;
        uint32_t callCount = 0;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _))
            .WillRepeatedly(Invoke([&callCount](int, off_t, size_t size, void *buf) {
                auto group = static_cast<LogGroup*>(buf);
                (void)memset_s(group, LOGGROUP_SIZE, 0, LOGGROUP_SIZE);
                if (callCount == 0) {
                    group->nsl[0].line = 0;
                    group->logBlockCnt = 2;
                } else if (callCount == 1) {
                    group->nsl[0].line = 2;
                    group->nsl[0].timestamp = 1;
                    group->logBlockCnt = 3;
                } else {
                    group->nsl[0].timestamp = 0;
                }
                callCount++;
                return static_cast<ssize_t>(size);
            }));
        uint64_t res = GetCurrentLine(fd);
        EXPECT_EQ(res, EXPECTED_CURRENT_LINE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCurrentLineTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetCurrentLineTest002 end";
}

/**
 * @tc.name: GetCurrentLineTest003
 * @tc.desc: Verify the GetCurrentLine function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetCurrentLineTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentLineTest003 start";
    try {
        int fd = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _))
            .WillOnce(Return(LOGGROUP_SIZE))
            .WillOnce(Return(0));
        EXPECT_EQ(GetCurrentLine(fd), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCurrentLineTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetCurrentLineTest003 end";
}

/**
 * @tc.name: GetCurrentLineTest004
 * @tc.desc: Verify the GetCurrentLine function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetCurrentLineTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCurrentLineTest004 start";
    try {
        int fd = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _))
            .WillOnce(Return(LOGGROUP_SIZE))
            .WillOnce(Return(LOGGROUP_SIZE));
        EXPECT_EQ(GetCurrentLine(fd), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCurrentLineTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetCurrentLineTest004 end";
}

/**
 * @tc.name: GetReversalTest001
 * @tc.desc: Verify the GetReversal function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetReversalTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetReversalTest001 start";
    try {
        int fd = 1;
        EXPECT_TRUE(GetReversal(fd, LOG_COUNT_MAX + 1));
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
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetReversalTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetReversalTest002 start";
    try {
        int fd = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(0));
        EXPECT_FALSE(GetReversal(fd, 0));
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
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetReversalTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetReversalTest003 start";
    try {
        int fd = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _))
            .WillOnce(Invoke([](int fd, off_t offset, size_t size, void *buf) {
                auto group = static_cast<LogGroup*>(buf);
                group->nsl[LOGBLOCK_PER_GROUP - 1].timestamp = 1;
                return LOGGROUP_SIZE;
            }));
        EXPECT_TRUE(GetReversal(fd, 0));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetReversalTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetReversalTest003 end";
}

/**
 * @tc.name: GetReversalTest004
 * @tc.desc: Verify the GetReversal function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileStaticTest, GetReversalTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetReversalTest004 start";
    try {
        int fd = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _))
            .WillOnce(Invoke([](int fd, off_t offset, size_t size, void *buf) {
                auto group = static_cast<LogGroup*>(buf);
                group->nsl[LOGBLOCK_PER_GROUP - 1].timestamp = 0;
                return LOGGROUP_SIZE;
            }));
        EXPECT_FALSE(GetReversal(fd, 0));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetReversalTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetReversalTest004 end";
}

} // namespace OHOS::FileManagement::CloudDiskService::Test