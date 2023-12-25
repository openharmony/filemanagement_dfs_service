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

#include "clouddisk_rdb_utils.h"
#include "dfs_error.h"
#include "result_set_mock.h"
#include "file_column.h"

namespace OHOS {
namespace FileManagement::CloudDisk {
namespace Test {
using namespace testing::ext;
using namespace std;

class CloudDiskRdbUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskRdbUtilsTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDiskRdbUtilsTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDiskRdbUtilsTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudDiskRdbUtilsTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: GetIntTest001
 * @tc.desc: Verify the GetInt function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetIntTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInt Start";
    const string key = FileColumn::IS_DIRECTORY;
    int32_t value;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetInt(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::GetInt(key, value, resultSet);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetInt End";
}

/**
 * @tc.name: GetIntTest002
 * @tc.desc: Verify the GetInt function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetIntTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInt Start";
    try {
        const string key = "";
        int32_t value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetInt(_, _)).WillOnce(Return(E_OK));
        int32_t ret = CloudDiskRdbUtils::GetInt(key, value, std::move(resultSet));
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetInt ERROR";
    }
    GTEST_LOG_(INFO) << "GetInt End";
}

/**
 * @tc.name: GetIntTest003
 * @tc.desc: Verify the GetInt function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetIntTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInt Start";
    try {
        const string key = FileColumn::IS_DIRECTORY;
        int32_t value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::GetInt(key, value, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetInt ERROR";
    }
    GTEST_LOG_(INFO) << "GetInt End";
}

/**
 * @tc.name: GetIntTest004
 * @tc.desc: Verify the GetInt function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetIntTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInt Start";
    try {
        const string key = FileColumn::IS_DIRECTORY;
        int32_t value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetInt(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::GetInt(key, value, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetInt ERROR";
    }
    GTEST_LOG_(INFO) << "GetInt End";
}


/**
 * @tc.name: GetLongTest001
 * @tc.desc: Verify the GetLong function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetLongTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLong Start";
    const string key = FileColumn::FILE_SIZE;
    int64_t value;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetLong(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::GetLong(key, value, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetLong End";
}

/**
 * @tc.name: GetLongTest002
 * @tc.desc: Verify the GetLong function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetLongTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLong Start";
    try {
        const string key = "";
        int64_t value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetLong(_, _)).WillOnce(Return(E_OK));
        int32_t ret = CloudDiskRdbUtils::GetLong(key, value, std::move(resultSet));
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLong ERROR";
    }
    GTEST_LOG_(INFO) << "GetLong End";
}

/**
 * @tc.name: GetLongTest003
 * @tc.desc: Verify the GetLong function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetLongTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLong Start";
    try {
        const string key = FileColumn::FILE_SIZE;
        int64_t value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::GetLong(key, value, resultSet);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLong ERROR";
    }
    GTEST_LOG_(INFO) << "GetLong End";
}

/**
 * @tc.name: GetLongTest004
 * @tc.desc: Verify the GetLong function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetLongTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLong Start";
    try {
        const string key = FileColumn::FILE_SIZE;
        int64_t value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetLong(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::GetLong(key, value, resultSet);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLong ERROR";
    }
    GTEST_LOG_(INFO) << "GetLong End";
}

/**
 * @tc.name: GetStringTest001
 * @tc.desc: Verify the GetString function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetStringTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetString Start";
    const string key = FileColumn::FILE_NAME;
    string value;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::GetString(key, value, resultSet);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetString End";
}

/**
 * @tc.name: GetStringTest002
 * @tc.desc: Verify the GetString function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetStringTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetString Start";
    try {
        const string key = "";
        string value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetString(_, _)).WillOnce(Return(E_OK));
        int32_t ret = CloudDiskRdbUtils::GetString(key, value, resultSet);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetString ERROR";
    }
    GTEST_LOG_(INFO) << "GetString End";
}

/**
 * @tc.name: GetStringTest003
 * @tc.desc: Verify the GetString function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetStringTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetString Start";
    try {
        const string key = FileColumn::FILE_NAME;
        string value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::GetString(key, value, resultSet);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetString ERROR";
    }
    GTEST_LOG_(INFO) << "GetString End";
}

/**
 * @tc.name: GetStringTest004
 * @tc.desc: Verify the GetString function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, GetStringTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetString Start";
    try {
        const string key = FileColumn::FILE_NAME;
        string value;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetString(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::GetString(key, value, resultSet);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetString ERROR";
    }
    GTEST_LOG_(INFO) << "GetString End";
}


/**
 * @tc.name: FillInfoFileNameTest001
 * @tc.desc: Verify the FillInfoFileName function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileName Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoFileName(info, resultSet);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoFileName End";
}

/**
 * @tc.name: FillInfoFileNameTest002
 * @tc.desc: Verify the FillInfoFileName function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileNameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileName Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoFileName(info, resultSet);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoFileName ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoFileName End";
}

/**
 * @tc.name: FillInfoCloudIdTest001
 * @tc.desc: Verify the FillInfoCloudId function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoCloudIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoCloudId Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoCloudId(info, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoCloudId End";
}

/**
 * @tc.name: FillInfoCloudIdTest002
 * @tc.desc: Verify the FillInfoCloudId function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoCloudIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoCloudId Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoCloudId(info, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoCloudId ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoCloudId End";
}

/**
 * @tc.name: FillInfoParentIdTest001
 * @tc.desc: Verify the FillInfoParentId function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoParentIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoParentId Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoParentId(info, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoParentId End";
}

/**
 * @tc.name: FillInfoParentIdTest002
 * @tc.desc: Verify the FillInfoParentId function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoParentIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoParentId Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoParentId(info, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoParentId ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoParentId End";
}

/**
 * @tc.name: FillInfoLocationTest001
 * @tc.desc: Verify the FillInfoLocation function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoLocationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoLocation Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetInt(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoLocation(info, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoLocation End";
}

/**
 * @tc.name: FillInfoLocationTest002
 * @tc.desc: Verify the FillInfoLocation function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoLocationTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoLocation Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoLocation(info, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoLocation ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoLocation End";
}

/**
 * @tc.name: FillInfoFileSizeTest001
 * @tc.desc: Verify the FillInfoFileSize function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileSizeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileSize Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetLong(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoFileSize(info, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoFileSize End";
}

/**
 * @tc.name: FillInfoFileSizeTest002
 * @tc.desc: Verify the FillInfoFileSize function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileSizeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileSize Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoFileSize(info, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoFileSize ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoFileSize End";
}

/**
 * @tc.name: FillInfoFileATimeTest001
 * @tc.desc: Verify the FillInfoFileATime function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileATimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileATime Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetLong(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoFileATime(info, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoFileATime End";
}

/**
 * @tc.name: FillInfoFileATimeTest002
 * @tc.desc: Verify the FillInfoFileATime function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileATimeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileATime Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoFileATime(info, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoFileATime ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoFileATime End";
}

/**
 * @tc.name: FillInfoFileCTimeTest001
 * @tc.desc: Verify the FillInfoFileCTime function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileCTimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileCTime Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();

    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetLong(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoFileCTime(info, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoFileCTime End";
}

/**
 * @tc.name: FillInfoFileCTimeTest002
 * @tc.desc: Verify the FillInfoFileCTime function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileCTimeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileCTime Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoFileCTime(info, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoFileCTime ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoFileCTime End";
}

/**
 * @tc.name: FillInfoFileMTimeTest001
 * @tc.desc: Verify the FillInfoFileMTime function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileMTimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileMTime Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetLong(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoFileMTime(info, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoFileMTime End";
}

/**
 * @tc.name: FillInfoFileMTimeTest002
 * @tc.desc: Verify the FillInfoFileMTime function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileMTimeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileMTime Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoFileMTime(info, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoFileMTime ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoFileMTime End";
}

/**
 * @tc.name: FillInfoFileTypeTest001
 * @tc.desc: Verify the FillInfoFileType function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileTypeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileType Start";
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetInt(_, _)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::FillInfoFileType(info, std::move(resultSet));
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FillInfoFileType End";
}

/**
 * @tc.name: FillInfoFileTypeTest002
 * @tc.desc: Verify the FillInfoFileType function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, FillInfoFileTypeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillInfoFileType Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::FillInfoFileType(info, std::move(resultSet));
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillInfoFileType ERROR";
    }
    GTEST_LOG_(INFO) << "FillInfoFileType End";
}

/**
 * @tc.name: ResultSetToFileInfoTest001
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    try {
        vector<CloudDiskFileInfo> infos;
        std::shared_ptr<ResultSet> resultSet = nullptr;
        int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(resultSet, infos);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ResultSetToFileInfo ERROR";
    }
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest002
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    try {
        vector<CloudDiskFileInfo> infos;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ResultSetToFileInfo ERROR";
    }
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest003
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    try {
        vector<CloudDiskFileInfo> infos;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        int32_t rowCount = -1;
        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
        int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ResultSetToFileInfo ERROR";
    }
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest004
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow())
        .Times(2)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(9).WillRepeatedly(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest005
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(10)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest006
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();

    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(9)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest007
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(8)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest008
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(7)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest009
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(6)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest010
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(5)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest011
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(4)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest012
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(3)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfoTest013
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    int32_t rowCount = 1;
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(2).WillOnce(Return(E_RDB)).WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
