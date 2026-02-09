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

#include "disk_utils.h"

#include <cstring>
#include <dirent.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/fanotify.h>

#include "assistant.h"
#include "securec.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DiskUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AssistantMock> assistantMock_;
};

void DiskUtilsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}
void DiskUtilsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DiskUtilsTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    assistantMock_ = std::make_shared<AssistantMock>();
    Assistant::ins = assistantMock_;
    assistantMock_->EnableMock();
}

void DiskUtilsTest::TearDown()
{
    assistantMock_->DisableMock();
    Assistant::ins = nullptr;
    assistantMock_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ExtractFileNameTest001
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, ExtractFileNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileNameTest001 start";
    try {
        string fileName;
        EXPECT_TRUE(DiskUtils::ExtractFileName(FAN_EVENT_INFO_TYPE_DFID, fileName, nullptr));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileNameTest001 failed";
    }
    GTEST_LOG_(INFO) << "ExtractFileNameTest001 end";
}

/**
 * @tc.name: ExtractFileNameTest002
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, ExtractFileNameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileNameTest002 start";
    try {
        string fileName;
        struct file_handle fh;
        EXPECT_FALSE(DiskUtils::ExtractFileName(0, fileName, &fh));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileNameTest002 failed";
    }
    GTEST_LOG_(INFO) << "ExtractFileNameTest002 end";
}

/**
 * @tc.name: ExtractFileNameTest003
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, ExtractFileNameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileNameTest003 start";
    try {
        string fileName = "previous_value";
        struct file_handle fh;
        EXPECT_TRUE(DiskUtils::ExtractFileName(FAN_EVENT_INFO_TYPE_FID, fileName, &fh));
        EXPECT_TRUE(fileName.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileNameTest003 failed";
    }
    GTEST_LOG_(INFO) << "ExtractFileNameTest003 end";
}

/**
 * @tc.name: ExtractFileNameTest004
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, ExtractFileNameTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileNameTest004 start";
    try {
        string fileName = "previous_value";
        struct file_handle fh;
        EXPECT_TRUE(DiskUtils::ExtractFileName(FAN_EVENT_INFO_TYPE_DFID, fileName, &fh));
        EXPECT_TRUE(fileName.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileNameTest004 failed";
    }
    GTEST_LOG_(INFO) << "ExtractFileNameTest004 end";
}

/**
 * @tc.name: ExtractFileNameTest005
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, ExtractFileNameTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileNameTest005 start";
    try {
        string fileName;
        char buf[64] = {};
        auto *fh = reinterpret_cast<struct file_handle *>(buf);
        fh->handle_bytes = 4;
        const char *name = "doc.txt";
        size_t nameOffset = 8 + fh->handle_bytes;
        if (nameOffset < sizeof(buf)) {
            (void)strncpy_s(buf + nameOffset, sizeof(buf) - nameOffset, name, sizeof(buf) - nameOffset - 1);
        }
        EXPECT_TRUE(DiskUtils::ExtractFileName(FAN_EVENT_INFO_TYPE_DFID_NAME, fileName, fh));
        EXPECT_EQ(fileName, name);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileNameTest005 failed";
    }
    GTEST_LOG_(INFO) << "ExtractFileNameTest005 end";
}

/**
 * @tc.name: GetFilePathTest001
 * @tc.desc: Verify the GetFilePath function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest001 start";
    try {
        EXPECT_CALL(*assistantMock_, readlink(_, _, _)).WillOnce(Return(-1));
        EXPECT_EQ(DiskUtils::GetFilePath(123, "file"), "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest001 end";
}

/**
 * @tc.name: GetFilePathTest002
 * @tc.desc: Verify the GetFilePath function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest002 start";
    try {
        EXPECT_CALL(*assistantMock_, readlink(_, _, _)).WillOnce(Return(PATH_MAX + 1));
        EXPECT_EQ(DiskUtils::GetFilePath(123, "file"), "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest002 end";
}

/**
 * @tc.name: GetFilePathTest003
 * @tc.desc: Verify the GetFilePath function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest003 start";
    try {
        const char *dirPath = "/data/test";
        EXPECT_CALL(*assistantMock_, readlink(_, _, _))
            .WillOnce(DoAll(SetArrayArgument<1>(dirPath, dirPath + strlen(dirPath)), Return(strlen(dirPath))));

        EXPECT_EQ(DiskUtils::GetFilePath(123, ""), "/data/test");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest003 end";
}

/**
 * @tc.name: GetFilePathTest004
 * @tc.desc: Verify the GetFilePath function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest004 start";
    try {
        const char *dirPath = "/data/test";
        EXPECT_CALL(*assistantMock_, readlink(_, _, _))
            .WillOnce(DoAll(SetArrayArgument<1>(dirPath, dirPath + strlen(dirPath)), Return(strlen(dirPath))));

        EXPECT_EQ(DiskUtils::GetFilePath(123, "."), "/data/test");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest004 end";
}

/**
 * @tc.name: GetFilePathTest005
 * @tc.desc: Verify the GetFilePath function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest005 start";
    try {
        const char *dirPath = "/data/test";
        EXPECT_CALL(*assistantMock_, readlink(_, _, _))
            .WillOnce(DoAll(SetArrayArgument<1>(dirPath, dirPath + strlen(dirPath)), Return(strlen(dirPath))));
        EXPECT_EQ(DiskUtils::GetFilePath(123, "file.txt"), "/data/test/file.txt");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest005 failed";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest005 end";
}

/**
 * @tc.name: CloseFdTest001
 * @tc.desc: Verify the CloseFd function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, CloseFdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseFdTest001 start";
    try {
        int fd = -1;
        DiskUtils::CloseFd(fd);
        EXPECT_EQ(fd, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseFdTest001 failed";
    }
    GTEST_LOG_(INFO) << "CloseFdTest001 end";
}

/**
 * @tc.name: CloseFdTest002
 * @tc.desc: Verify the CloseFd function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, CloseFdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseFdTest002 start";
    try {
        int fd = 10;
        DiskUtils::CloseFd(fd);
        EXPECT_EQ(fd, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseFdTest002 failed";
    }
    GTEST_LOG_(INFO) << "CloseFdTest002 end";
}

/**
 * @tc.name: CloseDirTest001
 * @tc.desc: Verify the CloseDir function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, CloseDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseDirTest001 Begin";
    try {
        DIR *dir = opendir("/data");
        DiskUtils::CloseDir(dir);
        EXPECT_EQ(dir, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseDirTest001 Error";
    }
    GTEST_LOG_(INFO) << "CloseDirTest001 End";
}

/**
 * @tc.name: CloseDirTest002
 * @tc.desc: Verify the CloseDir function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, CloseDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseDirTest002 Begin";
    try {
        DIR *dir = nullptr;
        DiskUtils::CloseDir(dir);
        EXPECT_EQ(dir, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseDirTest002 Error";
    }
    GTEST_LOG_(INFO) << "CloseDirTest002 End";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test