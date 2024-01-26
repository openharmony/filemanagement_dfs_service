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
#include <memory>

#include "directory_ex.h"
#include "file_utils.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FileUtilsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileUtilsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileUtilsTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileUtilsTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ReadFileTest001
 * @tc.desc: Verify the ReadFile function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(FileUtilsTest, ReadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFileTest001 Begin";
    try {
        int fd = -1;
        off_t offset = 0;
        size_t size = 1024;
        void *data = nullptr;
        int ret = FileUtils::ReadFile(fd, offset, size, data);
        EXPECT_EQ(ret, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFileTest001 ERROR";
    }

    GTEST_LOG_(INFO) << "ReadFileTest001 End";
}

/**
 * @tc.name: ReadFileTest002
 * @tc.desc: Verify the ReadFile function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(FileUtilsTest, ReadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFileTest002 Begin";
    try {
        int fd = 1;
        off_t offset = 1;
        size_t size = 5;
        void *data = new int8_t;
        int ret = FileUtils::ReadFile(fd, offset, size, data);
        EXPECT_EQ(ret, -29);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReadFileTest002 ERROR";
    }

    GTEST_LOG_(INFO) << "ReadFileTest002 End";
}

/**
 * @tc.name: ReadFileTest003
 * @tc.desc: Verify the ReadFile function
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(FileUtilsTest, ReadFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadFileTest003 Start";
    try {
        auto data = make_shared<char>(1);
        int64_t ret = FileUtils::ReadFile(-1, 0, 0, data.get());
        EXPECT_EQ(ret, -1);
        ret = FileUtils::ReadFile(0, -1, 0, data.get());
        EXPECT_EQ(ret, -1);
        ret = FileUtils::ReadFile(0, 0, 0, nullptr);
        EXPECT_EQ(ret, -1);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " ReadFileTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "ReadFileTest003 End";
}

/**
 * @tc.name: WriteFileTest001
 * @tc.desc: Verify the WriteFile function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(FileUtilsTest, WriteFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteFileTest001 Begin";
    try {
        int fd = -1;
        void *data = nullptr;
        off_t offset = 0;
        size_t size = 1024;
        int ret = FileUtils::WriteFile(fd, data, offset, size);
        EXPECT_EQ(ret, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " WriteFileTest001 ERROR";
    }

    GTEST_LOG_(INFO) << "WriteFileTest001 End";
}

/**
 * @tc.name: WriteFileTest002
 * @tc.desc: Verify the ReadFile function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(FileUtilsTest, WriteFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteFileTest002 Begin";
    try {
        int fd = 0;
        const void* data = "data";
        off_t offset = 0;
        size_t size = 5;
        int ret = FileUtils::WriteFile(fd, data, offset, size);
        EXPECT_EQ(ret, -29);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " WriteFileTest002 ERROR";
    }

    GTEST_LOG_(INFO) << "WriteFileTest002 End";
}

/**
 * @tc.name: WriteFileTest003
 * @tc.desc: Verify the WriteFile function
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(FileUtilsTest, WriteFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteFileTest003 Start";
    try {
        auto data = make_shared<char>(1);
        int64_t ret = FileUtils::WriteFile(-1, data.get(), 0, 1);
        EXPECT_EQ(ret, -1);
        ret = FileUtils::WriteFile(0, nullptr, 0, 0);
        EXPECT_EQ(ret, -1);
        ret = FileUtils::WriteFile(0, data.get(), -1, 1);
        EXPECT_EQ(ret, -1);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " WriteFileTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "WriteFileTest003 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test
