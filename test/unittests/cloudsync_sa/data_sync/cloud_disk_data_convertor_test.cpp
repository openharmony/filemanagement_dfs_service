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

#include "cloud_disk_data_convertor.h"
#include "cloud_disk_data_handler.h"
#include "clouddisk_rdbstore.h"
#include "result_set_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
const int32_t USER_ID = 100;

class CloudDiskDataConvertorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskDataConvertorTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskDataConvertorTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskDataConvertorTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskDataConvertorTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Convert001
 * @tc.desc: Verify the Convert function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, Convert001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Convert001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        record.SetDelete(true);
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        localConvertor->SetRootId(bundName);
        int res = localConvertor->Convert(record, values);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "Convert001 ERROR";
    }

    GTEST_LOG_(INFO) << "Convert001 End";
}

/**
 * @tc.name: HandleErr001
 * @tc.desc: Verify the HandleErr function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleErr001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleErr001 Begin";
    try {
        string bundName = "com.ohos.photos";
        CloudDisk::CloudDiskRdbStore cloudDiskRdbStore(bundName, USER_ID);
        auto rdb = cloudDiskRdbStore.GetRaw();
        ASSERT_TRUE(rdb);
        auto stopFlag = make_shared<bool>(false);
        shared_ptr<CloudDiskDataHandler> cloudDiskHandler =
            make_shared<CloudDiskDataHandler>(USER_ID, bundName, rdb, stopFlag);
        shared_ptr<CloudDiskDataConvertor> createConvertor = make_shared<CloudDiskDataConvertor>(
            USER_ID, bundName, FILE_CREATE,
            std::bind(&CloudDiskDataHandler::HandleCreateConvertErr, cloudDiskHandler, std::placeholders::_1,
            std::placeholders::_2));
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        int32_t err = E_RDB;
        createConvertor->HandleErr(err, *resultSet);
        EXPECT_EQ(cloudDiskHandler->modifyFailSet_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleErr001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleErr001 End";
}

/**
 * @tc.name: HandleErr002
 * @tc.desc: Verify the HandleErr function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleErr002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleErr002 Begin";
    try {
        string bundName = "com.ohos.photos";
        CloudDisk::CloudDiskRdbStore cloudDiskRdbStore(bundName, USER_ID);
        auto rdb = cloudDiskRdbStore.GetRaw();
        ASSERT_TRUE(rdb);
        auto stopFlag = make_shared<bool>(false);
        shared_ptr<CloudDiskDataHandler> cloudDiskHandler =
            make_shared<CloudDiskDataHandler>(USER_ID, bundName, rdb, stopFlag);
        shared_ptr<CloudDiskDataConvertor> fdirtyConvertor = make_shared<CloudDiskDataConvertor>(
            USER_ID, bundName, FILE_DATA_MODIFY,
            std::bind(&CloudDiskDataHandler::HandleFdirtyConvertErr, cloudDiskHandler, std::placeholders::_1,
            std::placeholders::_2));
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        int32_t err = E_RDB;
        fdirtyConvertor->HandleErr(err, *resultSet);
        EXPECT_EQ(cloudDiskHandler->modifyFailSet_.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleErr002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleErr002 End";
}

/**
 * @tc.name: HandleErr003
 * @tc.desc: Verify the HandleErr function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleErr003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleErr003 Begin";
    try {
        string bundName = "com.ohos.photos";
        CloudDisk::CloudDiskRdbStore cloudDiskRdbStore(bundName, USER_ID);
        auto rdb = cloudDiskRdbStore.GetRaw();
        ASSERT_TRUE(rdb);
        auto stopFlag = make_shared<bool>(false);
        shared_ptr<CloudDiskDataHandler> cloudDiskHandler =
            make_shared<CloudDiskDataHandler>(USER_ID, bundName, rdb, stopFlag);
        shared_ptr<CloudDiskDataConvertor> createConvertor = make_shared<CloudDiskDataConvertor>(
            USER_ID, bundName, FILE_CREATE,
            std::bind(&CloudDiskDataHandler::HandleCreateConvertErr, cloudDiskHandler, std::placeholders::_1,
            std::placeholders::_2));
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        int32_t err = E_RDB;
        createConvertor->HandleErr(err, *resultSet);
        EXPECT_EQ(cloudDiskHandler->createFailSet_.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleErr003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleErr003 End";
}

/**
 * @tc.name: Convert101
 * @tc.desc: Verify the Convert function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, Convert101, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Convert101 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);
        int res = localConvertor->Convert(record, *resultSet);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Convert101 ERROR";
    }

    GTEST_LOG_(INFO) << "Convert101 End";
}

/**
 * @tc.name: Convert102
 * @tc.desc: Verify the Convert function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, Convert102, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Convert102 Begin";
    try {
        string bundName = "com.ohos.photos";
        CloudDisk::CloudDiskRdbStore cloudDiskRdbStore(bundName, USER_ID);
        auto rdb = cloudDiskRdbStore.GetRaw();
        ASSERT_TRUE(rdb);
        auto stopFlag = make_shared<bool>(false);
        shared_ptr<CloudDiskDataHandler> cloudDiskHandler =
            make_shared<CloudDiskDataHandler>(USER_ID, bundName, rdb, stopFlag);
        shared_ptr<CloudDiskDataConvertor> createConvertor = make_shared<CloudDiskDataConvertor>(
            USER_ID, bundName, FILE_CREATE,
            std::bind(&CloudDiskDataHandler::HandleCreateConvertErr, cloudDiskHandler, std::placeholders::_1,
            std::placeholders::_2));
        DriveKit::DKRecord record;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        int res = createConvertor->Convert(record, *resultSet);
        EXPECT_FALSE(record.isNewCreate_);
        EXPECT_EQ(cloudDiskHandler->createFailSet_.size(), 0);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Convert102 ERROR";
    }

    GTEST_LOG_(INFO) << "Convert102 End";
}

/**
 * @tc.name: FillRecordId001
 * @tc.desc: Verify the FillRecordId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, FillRecordId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillRecordId001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->FillRecordId(record, *resultSet);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillRecordId001 ERROR";
    }

    GTEST_LOG_(INFO) << "FillRecordId001 End";
}

/**
 * @tc.name: FillCreatedTime001
 * @tc.desc: Verify the FillCreatedTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, FillCreatedTime001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillCreatedTime001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->FillCreatedTime(record, *resultSet);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillCreatedTime001 ERROR";
    }

    GTEST_LOG_(INFO) << "FillCreatedTime001 End";
}

/**
 * @tc.name: FillCreatedTime002
 * @tc.desc: Verify the FillCreatedTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, FillCreatedTime002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillCreatedTime002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> deleteConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);
        int32_t res = deleteConvertor->FillCreatedTime(record, *resultSet);
        EXPECT_EQ(res, E_OK);
        GTEST_LOG_(INFO) << "zqw record create time is " << record.GetCreateTime();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillCreatedTime002 ERROR";
    }

    GTEST_LOG_(INFO) << "FillCreatedTime002 End";
}

/**
 * @tc.name: FillMetaEditedTime001
 * @tc.desc: Verify the FillMetaEditedTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, FillMetaEditedTime001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillMetaEditedTime001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->FillMetaEditedTime(record, *resultSet);
        EXPECT_EQ(res, E_OK);
        GTEST_LOG_(INFO) << "FillMetaEditedTime001, zqw record edit time is " << record.GetEditedTime();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillMetaEditedTime001 ERROR";
    }

    GTEST_LOG_(INFO) << "FillMetaEditedTime001 End";
}

/**
 * @tc.name: FillMetaEditedTime002
 * @tc.desc: Verify the FillMetaEditedTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, FillMetaEditedTime002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillMetaEditedTime002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> deleteConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);
        int32_t res = deleteConvertor->FillMetaEditedTime(record, *resultSet);
        EXPECT_EQ(res, E_OK);
        GTEST_LOG_(INFO) << "FillMetaEditedTime002, zqw record edit time is " << record.GetEditedTime();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillMetaEditedTime002 ERROR";
    }

    GTEST_LOG_(INFO) << "FillMetaEditedTime002 End";
}

/**
 * @tc.name: FillVersion001
 * @tc.desc: Verify the FillVersion function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, FillVersion001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillVersion001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->FillVersion(record, *resultSet);
        EXPECT_EQ(res, E_OK);
        GTEST_LOG_(INFO) << "FillVersion001, zqw record version is " << record.GetVersion();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillVersion001 ERROR";
    }

    GTEST_LOG_(INFO) << "FillVersion001 End";
}

/**
 * @tc.name: ExtractCompatibleValue001
 * @tc.desc: Verify the ExtractCompatibleValue function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractCompatibleValue001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractCompatibleValue001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->ExtractCompatibleValue(record, data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractCompatibleValue001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractCompatibleValue001 End";
}

/**
 * @tc.name: ExtractCompatibleValue002
 * @tc.desc: Verify the ExtractCompatibleValue function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractCompatibleValue002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractCompatibleValue002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        record.SetDelete(true);
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->ExtractCompatibleValue(record, data, values);
        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(record.GetIsDelete());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractCompatibleValue002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractCompatibleValue002 End";
}

/**
 * @tc.name: CompensateAttributes001
 * @tc.desc: Verify the CompensateAttributes function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, CompensateAttributes001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompensateAttributes001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->CompensateAttributes(data, record, values);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CompensateAttributes001 ERROR";
    }

    GTEST_LOG_(INFO) << "CompensateAttributes001 End";
}

/**
 * @tc.name: CompensateAttributes002
 * @tc.desc: Verify the CompensateAttributes function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, CompensateAttributes002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompensateAttributes002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_FILE_ATTRIBUTES, DriveKit::DKRecordField("DKRecordFieldTest"));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->CompensateAttributes(data, record, values);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CompensateAttributes002 ERROR";
    }

    GTEST_LOG_(INFO) << "CompensateAttributes002 End";
}

/**
 * @tc.name: CompensateAttributes003
 * @tc.desc: Verify the CompensateAttributes function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, CompensateAttributes003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompensateAttributes003 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_FILE_ATTRIBUTES, DriveKit::DKRecordField(1));
        data.emplace(DK_FILE_TIME_ADDED, DriveKit::DKRecordField(1));
        data.emplace(DK_FILE_TIME_EDITED, DriveKit::DKRecordField(1));
        data.emplace(DK_META_TIME_EDITED, DriveKit::DKRecordField(1));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        int32_t res = localConvertor->CompensateAttributes(data, record, values);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CompensateAttributes003 ERROR";
    }

    GTEST_LOG_(INFO) << "CompensateAttributes003 End";
}

/**
 * @tc.name: ExtractCloudId001
 * @tc.desc: Verify the ExtractCloudId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractCloudId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractCloudId001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractCloudId(record, values);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractCloudId001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractCloudId001 End";
}

/**
 * @tc.name: ExtractFileName001
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileName001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileName001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileName(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileName001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileName001 End";
}

/**
 * @tc.name: ExtractFileName002
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileName002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileName002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_FILE_NAME, DriveKit::DKRecordField("1.txt"));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileName(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileName002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileName002 End";
}

/**
 * @tc.name: ExtractFileName003
 * @tc.desc: Verify the ExtractFileName function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileName003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileName003 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_FILE_NAME, DriveKit::DKRecordField("testFile"));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileName(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileName003 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileName003 End";
}

/**
 * @tc.name: ExtractFileParentCloudId001
 * @tc.desc: Verify the ExtractFileParentCloudId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileParentCloudId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileParentCloudId001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileParentCloudId(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileParentCloudId001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileParentCloudId001 End";
}

/**
 * @tc.name: ExtractFileParentCloudId002
 * @tc.desc: Verify the ExtractFileParentCloudId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileParentCloudId002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileParentCloudId002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        data.emplace(DK_PARENT_CLOUD_ID, DriveKit::DKRecordField("parentFolder"));
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileParentCloudId(data, values);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileParentCloudId002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileParentCloudId002 End";
}

/**
 * @tc.name: ExtractFileParentCloudId003
 * @tc.desc: Verify the ExtractFileParentCloudId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileParentCloudId003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileParentCloudId003 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_PARENT_CLOUD_ID, DriveKit::DKRecordField(1));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileParentCloudId(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileParentCloudId003 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileParentCloudId003 End";
}

/**
 * @tc.name: ExtractFileSize001
 * @tc.desc: Verify the ExtractFileSize function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileSize001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileSize001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileSize(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileSize001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileSize001 End";
}

/**
 * @tc.name: ExtractFileSize002
 * @tc.desc: Verify the ExtractFileSize function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileSize002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileSize002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_FILE_SIZE, DriveKit::DKRecordField("test"));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileSize(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileSize002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileSize002 End";
}

/**
 * @tc.name: ExtractFileSize003
 * @tc.desc: Verify the ExtractFileSize function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileSize003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileSize003 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_FILE_SIZE, DriveKit::DKRecordField(1));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileSize(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileSize003 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileSize003 End";
}

/**
 * @tc.name: ExtractSha256001
 * @tc.desc: Verify the ExtractSha256 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractSha256001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractSha256001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractSha256(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractSha256001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractSha256001 End";
}

/**
 * @tc.name: ExtractSha256002
 * @tc.desc: Verify the ExtractSha256 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractSha256002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractSha256002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_FILE_SHA256, DriveKit::DKRecordField(1));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractSha256(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractSha256002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractSha256002 End";
}

/**
 * @tc.name: ExtractSha256003
 * @tc.desc: Verify the ExtractSha256 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractSha256003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractSha256003 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_FILE_SHA256, DriveKit::DKRecordField("sha256"));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractSha256(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractSha256003 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractSha256003 End";
}

/**
 * @tc.name: ExtractFileTimeRecycled001
 * @tc.desc: Verify the ExtractFileTimeRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileTimeRecycled001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileTimeRecycled001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractFileTimeRecycled(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileTimeRecycled001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileTimeRecycled001 End";
}

/**
 * @tc.name: ExtractFileTimeRecycled002
 * @tc.desc: Verify the ExtractFileTimeRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileTimeRecycled002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileTimeRecycled002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;

        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        data.emplace(DK_IS_RECYCLED, DriveKit::DKRecordField(false));
        int32_t res = localConvertor->ExtractFileTimeRecycled(data, values);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileTimeRecycled002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileTimeRecycled002 End";
}

/**
 * @tc.name: ExtractFileTimeRecycled003
 * @tc.desc: Verify the ExtractFileTimeRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileTimeRecycled003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileTimeRecycled003 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;

        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        data.emplace(DK_FILE_TIME_RECYCLED, DriveKit::DKRecordField(false));
        int32_t res = localConvertor->ExtractFileTimeRecycled(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileTimeRecycled003 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileTimeRecycled003 End";
}

/**
 * @tc.name: ExtractFileTimeRecycled004
 * @tc.desc: Verify the ExtractFileTimeRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractFileTimeRecycled004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractFileTimeRecycled004 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;

        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        data.emplace(DK_IS_RECYCLED, DriveKit::DKRecordField(true));
        data.emplace(DK_FILE_TIME_RECYCLED, DriveKit::DKRecordField(true));
        int32_t res = localConvertor->ExtractFileTimeRecycled(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractFileTimeRecycled004 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractFileTimeRecycled004 End";
}

/**
 * @tc.name: ExtractDirectlyRecycled001
 * @tc.desc: Verify the ExtractDirectlyRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractDirectlyRecycled001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractDirectlyRecycled001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractDirectlyRecycled(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractDirectlyRecycled001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractDirectlyRecycled001 End";
}

/**
 * @tc.name: ExtractDirectlyRecycled002
 * @tc.desc: Verify the ExtractDirectlyRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractDirectlyRecycled002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractDirectlyRecycled002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_DIRECTLY_RECYCLED, DriveKit::DKRecordField(false));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractDirectlyRecycled(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractDirectlyRecycled002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractDirectlyRecycled002 End";
}

/**
 * @tc.name: ExtractDirectlyRecycled003
 * @tc.desc: Verify the ExtractDirectlyRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractDirectlyRecycled003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractDirectlyRecycled003 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_DIRECTLY_RECYCLED, DriveKit::DKRecordField(true));
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractDirectlyRecycled(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractDirectlyRecycled003 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractDirectlyRecycled003 End";
}

/**
 * @tc.name: ExtractIsDirectory001
 * @tc.desc: Verify the ExtractIsDirectory function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractIsDirectory001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractIsDirectory001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractIsDirectory(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractIsDirectory001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractIsDirectory001 End";
}

/**
 * @tc.name: ExtractIsDirectory002
 * @tc.desc: Verify the ExtractIsDirectory function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractIsDirectory002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractIsDirectory002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_IS_DIRECTORY, "dir");
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractIsDirectory(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractIsDirectory002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractIsDirectory002 End";
}

/**
 * @tc.name: ExtractIsDirectory003
 * @tc.desc: Verify the ExtractIsDirectory function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractIsDirectory003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractIsDirectory003 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        data.emplace(DK_IS_DIRECTORY, "file");
        record.GetRecordData(data);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractIsDirectory(data, values);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractIsDirectory003 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractIsDirectory003 End";
}

/**
 * @tc.name: ExtractVersion001
 * @tc.desc: Verify the ExtractVersion function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractVersion001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractVersion001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractVersion(record, values);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(record.GetVersion(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractVersion001 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractVersion001 End";
}

/**
 * @tc.name: ExtractVersion002
 * @tc.desc: Verify the ExtractVersion function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, ExtractVersion002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtractVersion002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        record.SetVersion(1);
        NativeRdb::ValuesBucket values;
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->ExtractVersion(record, values);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(record.GetVersion(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtractVersion002 ERROR";
    }

    GTEST_LOG_(INFO) << "ExtractVersion002 End";
}

/**
 * @tc.name: HandleFileName001
 * @tc.desc: Verify the HandleFileName function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleFileName001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFileName001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleFileName(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleFileName001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleFileName001 End";
}

/**
 * @tc.name: HandleFileName002
 * @tc.desc: Verify the HandleFileName function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleFileName002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFileName002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);

        int32_t res = localConvertor->HandleFileName(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleFileName002 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleFileName002 End";
}

/**
 * @tc.name: HandleParentId001
 * @tc.desc: Verify the HandleParentId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleParentId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleParentId001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleParentId(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleParentId001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleParentId001 End";
}

/**
 * @tc.name: HandleParentId002
 * @tc.desc: Verify the HandleParentId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleParentId002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleParentId002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);

        int32_t res = localConvertor->HandleParentId(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleParentId002 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleParentId002 End";
}

/**
 * @tc.name: HandleDirectlyRecycled001
 * @tc.desc: Verify the HandleDirectlyRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleDirectlyRecycled001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDirectlyRecycled001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleDirectlyRecycled(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleDirectlyRecycled001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleDirectlyRecycled001 End";
}

/**
 * @tc.name: HandleDirectlyRecycled002
 * @tc.desc: Verify the HandleDirectlyRecycled function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleDirectlyRecycled002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDirectlyRecycled002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);

        int32_t res = localConvertor->HandleDirectlyRecycled(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleDirectlyRecycled002 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleDirectlyRecycled002 End";
}

/**
 * @tc.name: HandleRecycleTime001
 * @tc.desc: Verify the HandleRecycleTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleRecycleTime001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRecycleTime001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleRecycleTime(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleRecycleTime001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleRecycleTime001 End";
}

/**
 * @tc.name: HandleRecycleTime002
 * @tc.desc: Verify the HandleRecycleTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleRecycleTime002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRecycleTime002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);

        int32_t res = localConvertor->HandleRecycleTime(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleRecycleTime002 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleRecycleTime002 End";
}

/**
 * @tc.name: HandleType001
 * @tc.desc: Verify the HandleType function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleType001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleType001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleType(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleType001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleType001 End";
}

/**
 * @tc.name: HandleOperateType001
 * @tc.desc: Verify the HandleOperateType function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleOperateType001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOperateType001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleOperateType(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOperateType001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleOperateType001 End";
}

/**
 * @tc.name: HandleOperateType002
 * @tc.desc: Verify the HandleOperateType function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleOperateType002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOperateType002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);

        int32_t res = localConvertor->HandleOperateType(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOperateType002 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleOperateType002 End";
}

/**
 * @tc.name: HandleCompatibleFileds001
 * @tc.desc: Verify the HandleCompatibleFileds function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleCompatibleFileds001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCompatibleFileds001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleCompatibleFileds(data, *resultSet);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCompatibleFileds001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleCompatibleFileds001 End";
}

/**
 * @tc.name: HandleAttributes001
 * @tc.desc: Verify the HandleAttributes function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleAttributes001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleAttributes001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleAttributes(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleAttributes001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleAttributes001 End";
}

/**
 * @tc.name: HandleAttributes002
 * @tc.desc: Verify the HandleAttributes function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleAttributes002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleAttributes002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);

        int32_t res = localConvertor->HandleAttributes(data, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(data.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleAttributes002 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleAttributes002 End";
}

/**
 * @tc.name: HandleCreateTime001
 * @tc.desc: Verify the HandleCreateTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleCreateTime001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCreateTime001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecordFieldMap map;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleCreateTime(map, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(map.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCreateTime001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleCreateTime001 End";
}

/**
 * @tc.name: HandleMetaEditedTime001
 * @tc.desc: Verify the HandleMetaEditedTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleMetaEditedTime001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMetaEditedTime001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecordFieldMap map;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleMetaEditedTime(map, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(map.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMetaEditedTime001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleMetaEditedTime001 End";
}

/**
 * @tc.name: HandleEditedTime001
 * @tc.desc: Verify the HandleEditedTime function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleEditedTime001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleEditedTime001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecordFieldMap map;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        int32_t res = localConvertor->HandleEditedTime(map, *resultSet);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(map.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleEditedTime001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleEditedTime001 End";
}

/**
 * @tc.name: HandleAttachments001
 * @tc.desc: Verify the HandleAttachments function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleAttachments001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleAttachments001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        CloudDisk::CloudDiskRdbStore cloudDiskRdbStore(bundName, USER_ID);
        auto rdb = cloudDiskRdbStore.GetRaw();
        ASSERT_TRUE(rdb);
        auto stopFlag = make_shared<bool>(false);
        shared_ptr<CloudDiskDataHandler> cloudDiskHandler =
            make_shared<CloudDiskDataHandler>(USER_ID, bundName, rdb, stopFlag);
        shared_ptr<CloudDiskDataConvertor> createConvertor = make_shared<CloudDiskDataConvertor>(
            USER_ID, bundName, FILE_CREATE,
            std::bind(&CloudDiskDataHandler::HandleCreateConvertErr, cloudDiskHandler, std::placeholders::_1,
            std::placeholders::_2));

        int32_t res = createConvertor->HandleAttachments(data, *resultSet);
        EXPECT_EQ(res, E_PATH);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleAttachments001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleAttachments001 End";
}

/**
 * @tc.name: HandleAttachments002
 * @tc.desc: Verify the HandleAttachments function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleAttachments002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleAttachments002 Begin";
    try {
        string bundName = "com.ohos.photos";
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        DriveKit::DKRecord recordFile;
        DriveKit::DKRecordData dataFile;
        dataFile.emplace(DK_IS_DIRECTORY, "file");
        recordFile.GetRecordData(dataFile);
        NativeRdb::ValuesBucket values;
        int32_t res = localConvertor->ExtractIsDirectory(dataFile, values);
        EXPECT_EQ(res, E_INVAL_ARG);

        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        res = localConvertor->HandleAttachments(data, *resultSet);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleAttachments002 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleAttachments002 End";
}

/**
 * @tc.name: HandleAttachments003
 * @tc.desc: Verify the HandleAttachments function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleAttachments003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleAttachments003 Begin";
    try {
        string bundName = "com.ohos.photos";
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        DriveKit::DKRecord recordFile;
        DriveKit::DKRecordData dataFile;
        dataFile.emplace(DK_IS_DIRECTORY, "dir");
        recordFile.GetRecordData(dataFile);
        NativeRdb::ValuesBucket values;
        int32_t res = localConvertor->ExtractIsDirectory(dataFile, values);
        EXPECT_EQ(res, E_INVAL_ARG);

        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        res = localConvertor->HandleAttachments(data, *resultSet);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleAttachments003 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleAttachments003 End";
}

/**
 * @tc.name: HandleAttachments004
 * @tc.desc: Verify the HandleAttachments function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleAttachments004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleAttachments004 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecordData data;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        shared_ptr<CloudDiskDataConvertor> deleteConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DELETE);

        int32_t res = deleteConvertor->HandleAttachments(data, *resultSet);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleAttachments004 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleAttachments004 End";
}

/**
 * @tc.name: HandleContent001
 * @tc.desc: Verify the HandleContent function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleContent001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleContent001 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);
        string filePath;
        string cloudId;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        if (!localConvertor->GetString(FileColumn::CLOUD_ID, cloudId, *resultSet)) {
            filePath = CloudFileUtils::GetLocalFilePath(cloudId, bundName, USER_ID);
        }
        EXPECT_FALSE(filePath.empty());

        int32_t res = localConvertor->HandleContent(data, filePath);
        EXPECT_EQ(res, E_PATH);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleContent001 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleContent001 End";
}

/**
 * @tc.name: HandleContent002
 * @tc.desc: Verify the HandleContent function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudDiskDataConvertorTest, HandleContent002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleContent002 Begin";
    try {
        string bundName = "com.ohos.photos";
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        record.GetRecordData(data);
        shared_ptr<CloudDiskDataConvertor> localConvertor =
            make_shared<CloudDiskDataConvertor>(USER_ID, bundName, FILE_DOWNLOAD);

        DriveKit::DKRecord recordFile;
        DriveKit::DKRecordData dataFile;
        dataFile.emplace(DK_IS_DIRECTORY, "file");
        recordFile.GetRecordData(dataFile);
        NativeRdb::ValuesBucket values;
        int32_t res = localConvertor->ExtractIsDirectory(dataFile, values);
        EXPECT_EQ(res, E_INVAL_ARG);

        string filePath;
        string cloudId;
        unique_ptr<ResultSetMock> resultSet = make_unique<ResultSetMock>();
        if (!localConvertor->GetString(FileColumn::CLOUD_ID, cloudId, *resultSet)) {
            filePath = CloudFileUtils::GetLocalFilePath(cloudId, bundName, USER_ID);
        }
        EXPECT_FALSE(filePath.empty());

        res = localConvertor->HandleContent(data, filePath);
        EXPECT_EQ(res, E_PATH);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleContent002 ERROR";
    }

    GTEST_LOG_(INFO) << "HandleContent002 End";
}

} // namespace OHOS::FileManagement::CloudSync::Test