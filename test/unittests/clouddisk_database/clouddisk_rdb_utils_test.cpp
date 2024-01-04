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
 * @tc.name: ResultSetToFileInfoTest001
 * @tc.desc: Verify the ResultSetToFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfo Start";
    try {
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSet> resultSet = nullptr;
        int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(resultSet, info);
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
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), info);
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
        CloudDiskFileInfo info;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetRow(_)).WillOnce(Return(E_RDB));
        int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), info);
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
    CloudDiskFileInfo info;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetRow(_)).WillOnce(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(std::move(resultSet), info);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "ResultSetToFileInfo End";
}

/**
 * @tc.name: ResultSetToFileInfosTest001
 * @tc.desc: Verify the ResultSetToFileInfos function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfosTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfos Start";
    try {
        vector<CloudDiskFileInfo> infos;
        std::shared_ptr<ResultSet> resultSet = nullptr;
        int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfos(resultSet, infos);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ResultSetToFileInfos ERROR";
    }
    GTEST_LOG_(INFO) << "ResultSetToFileInfos End";
}

/**
 * @tc.name: ResultSetToFileInfosTest002
 * @tc.desc: Verify the ResultSetToFileInfos function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfosTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfos Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_RDB));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfos(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "ResultSetToFileInfos End";
}

/**
 * @tc.name: ResultSetToFileInfosTest003
 * @tc.desc: Verify the ResultSetToFileInfos function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbUtilsTest, ResultSetToFileInfosTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToFileInfos Start";
    vector<CloudDiskFileInfo> infos;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GoToNextRow())
        .Times(2)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(2).WillRepeatedly(Return(E_OK));
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfos(std::move(resultSet), infos);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "ResultSetToFileInfos End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
