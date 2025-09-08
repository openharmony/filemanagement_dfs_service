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

#include "disk_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assistant.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DiskUtilsTest : public testing::Test {
#define FAN_EVENT_INFO_TYPE_FID 1
#define FAN_EVENT_INFO_TYPE_DFID_NAME 2
#define FAN_EVENT_INFO_TYPE_DFID 3
#define PATH_MAX 4096
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DiskUtils> diskUtils_;
    static inline shared_ptr<AssistantMock> insMock_;
};

void DiskUtilsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    diskUtils_ = make_shared<DiskUtils>();
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
}

void DiskUtilsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    diskUtils_ = nullptr;
    Assistant::ins = nullptr;
    insMock_ = nullptr;
}

void DiskUtilsTest::SetUp(void) {}

void DiskUtilsTest::TearDown(void) {}

/**
 * @tc.name: IExtractFileNameTest001
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, IExtractFileNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IExtractFileNameTest001 Begin";
    try {
        int type = FAN_EVENT_INFO_TYPE_FID;
        string fileName = "fileName";
        struct file_handle file_handle_test;
        bool res = diskUtils_->ExtractFileName(type, fileName, &file_handle_test);
        EXPECT_TRUE(res);
        EXPECT_EQ(fileName, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IExtractFileNameTest001 Error";
    }
    GTEST_LOG_(INFO) << "IExtractFileNameTest001 End";
}

/**
 * @tc.name: IExtractFileNameTest002
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, IExtractFileNameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IExtractFileNameTest002 Begin";
    try {
        int type = FAN_EVENT_INFO_TYPE_DFID_NAME;
        string fileName = "fileName";
        struct file_handle file_handle_test;
        file_handle_test.handle_bytes = 1;
        bool res = diskUtils_->ExtractFileName(type, fileName, &file_handle_test);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IExtractFileNameTest002 Error";
    }
    GTEST_LOG_(INFO) << "IExtractFileNameTest002 End";
}

/**
 * @tc.name: IExtractFileNameTest003
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, IExtractFileNameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IExtractFileNameTest003 Begin";
    try {
        int type = 4;
        string fileName = "fileName";
        struct file_handle file_handle_test;
        bool res = diskUtils_->ExtractFileName(type, fileName, &file_handle_test);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IExtractFileNameTest003 Error";
    }
    GTEST_LOG_(INFO) << "IExtractFileNameTest003 End";
}

/**
 * @tc.name: IExtractFileNameTest004
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, IExtractFileNameTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IExtractFileNameTest004 Begin";
    try {
        int type = FAN_EVENT_INFO_TYPE_DFID;
        string fileName = "fileName";
        struct file_handle file_handle_test;
        bool res = diskUtils_->ExtractFileName(type, fileName, &file_handle_test);
        EXPECT_TRUE(res);
        EXPECT_EQ(fileName, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IExtractFileNameTest004 Error";
    }
    GTEST_LOG_(INFO) << "IExtractFileNameTest004 End";
}

/**
 * @tc.name: CloseFdTest001
 * @tc.desc: Verify the CloseFd function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, CloseFdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseFdTest001 Begin";
    try {
        int fd = 0;
        diskUtils_->CloseFd(fd);
        EXPECT_EQ(fd, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseFdTest001 Error";
    }
    GTEST_LOG_(INFO) << "CloseFdTest001 End";
}

/**
 * @tc.name: CloseFdTest002
 * @tc.desc: Verify the CloseFd function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, CloseFdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseFdTest002 Begin";
    try {
        int fd = -1;
        diskUtils_->CloseFd(fd);
        EXPECT_EQ(fd, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseFdTest002 Error";
    }
    GTEST_LOG_(INFO) << "CloseFdTest002 End";
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
        diskUtils_->CloseDir(dir);
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
        diskUtils_->CloseDir(dir);
        EXPECT_EQ(dir, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseDirTest002 Error";
    }
    GTEST_LOG_(INFO) << "CloseDirTest002 End";
}

/**
 * @tc.name: GetFilePathTest001
 * @tc.desc: Verify the CloseDir function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest001 Begin";
    try {
        int32_t eventFd = 1;
        string fileName = "fileName";
        EXPECT_CALL(*insMock_, readlink(_, _, _)).WillOnce(Return(-1));
        string res = diskUtils_->GetFilePath(eventFd, fileName);
        EXPECT_EQ(res, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest001 Error";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest001 End";
}

/**
 * @tc.name: GetFilePathTest002
 * @tc.desc: Verify the CloseDir function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest002 Begin";
    try {
        int32_t eventFd = 1;
        string fileName = "fileName";
        EXPECT_CALL(*insMock_, readlink(_, _, _)).WillOnce(Return(PATH_MAX + 1));
        string res = diskUtils_->GetFilePath(eventFd, fileName);
        EXPECT_EQ(res, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest002 Error";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest002 End";
}

/**
 * @tc.name: GetFilePathTest003
 * @tc.desc: Verify the CloseDir function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest003 Begin";
    try {
        int32_t eventFd = 1;
        string fileName = "fileName";
        char path[] = "/path";
        EXPECT_CALL(*insMock_, readlink(_, _, _))
            .WillOnce(DoAll(SetArrayArgument<1>(path, path + sizeof(path) - 1), Return(5)));
        string res = diskUtils_->GetFilePath(eventFd, fileName);
        EXPECT_EQ(res, "/path/fileName");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest003 Error";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest003 End";
}

/**
 * @tc.name: GetFilePathTest004
 * @tc.desc: Verify the CloseDir function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest004 Begin";
    try {
        int32_t eventFd = 1;
        string fileName = "";
        char path[] = "/path";
        EXPECT_CALL(*insMock_, readlink(_, _, _))
            .WillOnce(DoAll(SetArrayArgument<1>(path, path + sizeof(path) - 1), Return(5)));
        string res = diskUtils_->GetFilePath(eventFd, fileName);
        EXPECT_EQ(res, "/path");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest004 Error";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest004 End";
}

/**
 * @tc.name: GetFilePathTest005
 * @tc.desc: Verify the CloseDir function
 * @tc.type: FUNC
 */
HWTEST_F(DiskUtilsTest, GetFilePathTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest005 Begin";
    try {
        int32_t eventFd = 1;
        string fileName = ".";
        char path[] = "/path";
        EXPECT_CALL(*insMock_, readlink(_, _, _))
            .WillOnce(DoAll(SetArrayArgument<1>(path, path + sizeof(path) - 1), Return(5)));
        string res = diskUtils_->GetFilePath(eventFd, fileName);
        EXPECT_EQ(res, "/path");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFilePathTest005 Error";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest005 End";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test