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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <securec.h>
#include <string>

#include "oh_cloud_disk_utils.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskManagerTest::SetUp(void)
{
}

void CloudDiskManagerTest::TearDown(void)
{
}

/**
 * @tc.name: AllocFieldTest001
 * @tc.desc: Verify the AllocField function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, AllocFieldTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocFieldTest001 start";
    try {
        std::string value = "123456";
        int32_t length = -2;
        char * fileId = AllocField(value.c_str(), length);
        EXPECT_EQ(fileId, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AllocFieldTest001 failed";
    }
    GTEST_LOG_(INFO) << "AllocFieldTest001 end";
}

/**
 * @tc.name: AllocFieldTest002
 * @tc.desc: Verify the AllocField function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, AllocFieldTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocFieldTest002 start";
    try {
        std::string value = "123456";
        int32_t length = 10;
        char * fileId = AllocField(nullptr, length);
        EXPECT_EQ(fileId, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AllocFieldTest002 failed";
    }
    GTEST_LOG_(INFO) << "AllocFieldTest002 end";
}

/**
 * @tc.name: AllocFieldTest003
 * @tc.desc: Verify the AllocField function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, AllocFieldTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocFieldTest003 start";
    try {
        std::string value = "123456";
        int32_t length = value.size();
        char * fileId = AllocField(value.c_str(), length);
        EXPECT_NE(fileId, nullptr);
        delete []fileId;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AllocFieldTest003 failed";
    }
    GTEST_LOG_(INFO) << "AllocFieldTest003 end";
}

/**
 * @tc.name: AllocFieldTest004
 * @tc.desc: Verify the AllocField function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, AllocFieldTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocFieldTest004 start";
    try {
        std::string value = std::string("123") + '\0' + std::string("456");
        constexpr int32_t length = 7;

        char expectResult[length + 1] = {'1', '2', '3', '\0', '4', '5', '6', '\0'};
        char * fileId = AllocField(value.c_str(), length);
        EXPECT_EQ(strncmp(expectResult, fileId, length + 1), 0);
        delete []fileId;

        char *fileId2 = new(std::nothrow) char[length + 1];
        if (fileId2 == nullptr) {
            GTEST_LOG_(ERROR) << "new failed" << std::endl;
            return;
        }
        if (strncpy_s(fileId2, length + 1, value.c_str(), length) != 0) {
            GTEST_LOG_(ERROR) << "strncpy_s failed" << std::endl;
            delete[] fileId2;
            return;
        }
        fileId2[length] = '\0';
        char expectResult2[length + 1] = {'1', '2', '3', '\0', '\0', '\0', '\0', '\0'};
        EXPECT_EQ(strncmp(expectResult2, fileId2, length + 1), 0);
        delete []fileId2;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AllocFieldTest004 failed";
    }
    GTEST_LOG_(INFO) << "AllocFieldTest004 end";
}

/**
 * @tc.name: IsValidPathInfoTest001
 * @tc.desc: Verify the IsValidPathInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsValidPathInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidPathInfoTest001 start";
    try {
        std::string value = "123456";
        int32_t length = value.size();
        bool ret = IsValidPathInfo(nullptr, length);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidPathInfoTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsValidPathInfoTest001 end";
}

/**
 * @tc.name: IsValidPathInfoTest002
 * @tc.desc: Verify the IsValidPathInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsValidPathInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidPathInfoTest002 start";
    try {
        std::string value = "123456";
        bool ret = IsValidPathInfo(value.c_str(), 0);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidPathInfoTest002 failed";
    }
    GTEST_LOG_(INFO) << "IsValidPathInfoTest002 end";
}

/**
 * @tc.name: IsValidPathInfoTest003
 * @tc.desc: Verify the IsValidPathInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsValidPathInfoTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidPathInfoTest003 start";
    try {
        std::string value = "123456";
        int32_t length = value.size();
        bool ret = IsValidPathInfo(value.c_str(), length + 10);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidPathInfoTest003 failed";
    }
    GTEST_LOG_(INFO) << "IsValidPathInfoTest003 end";
}

/**
 * @tc.name: IsValidPathInfoTest004
 * @tc.desc: Verify the IsValidPathInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsValidPathInfoTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidPathInfoTest004 start";
    try {
        std::string value = "123456";
        int32_t length = value.size();
        bool ret = IsValidPathInfo(value.c_str(), length);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidPathInfoTest004 failed";
    }
    GTEST_LOG_(INFO) << "IsValidPathInfoTest004 end";
}

/**
 * @tc.name: ConvertToErrorCodeTest001
 * @tc.desc: Verify the ConvertToErrorCode function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, ConvertToErrorCodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest001 start";
    try {
        int32_t innerErrorCode = -1;
        CloudDisk_ErrorCode ret = ConvertToErrorCode(innerErrorCode);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToErrorCodeTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest001 end";
}

/**
 * @tc.name: ConvertToErrorCodeTest002
 * @tc.desc: Verify the ConvertToErrorCode function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, ConvertToErrorCodeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest002 start";
    try {
        int32_t innerErrorCode = 0;
        CloudDisk_ErrorCode ret = ConvertToErrorCode(innerErrorCode);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToErrorCodeTest002 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest002 end";
}

/**
 * @tc.name: ConvertToErrorCodeTest003
 * @tc.desc: Verify the ConvertToErrorCode function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, ConvertToErrorCodeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest003 start";
    try {
        int32_t innerErrorCode = 34400001;
        CloudDisk_ErrorCode ret = ConvertToErrorCode(innerErrorCode);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToErrorCodeTest003 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest003 end";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test