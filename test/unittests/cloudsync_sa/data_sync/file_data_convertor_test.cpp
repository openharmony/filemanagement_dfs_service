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

#include "dfs_error.h"
#include "file_data_convertor.h"
#include "gallery_file_const.h"
#include "medialibrary_db_const.h"
#include "result_set_mock.h"
#include "thumbnail_const.h"
#include "data_sync_const.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
using namespace Media;

class FileDataConvertorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<FileDataConvertor> fileDataConvertor_;
};
void FileDataConvertorTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileDataConvertorTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileDataConvertorTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    int32_t userId = 100;
    string bundleName = "com.ohos.test";
    OperationType Type = OperationType::FILE_CREATE;
    fileDataConvertor_ = make_shared<FileDataConvertor>(userId, bundleName, Type);
}

void FileDataConvertorTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(FileDataConvertorTest, FillRecordIdTest, TestSize.Level1)
{
    DriveKit::DKRecord record;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    auto ret = fileDataConvertor_->FillRecordId(record, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, FillRecordIdTest1, TestSize.Level1)
{
    DriveKit::DKRecord record;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    auto ret = fileDataConvertor_->FillRecordId(record, resultSet);
    EXPECT_EQ(E_OK, ret);
}


HWTEST_F(FileDataConvertorTest, HandleAlbumIdTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "albumId";
    auto ret = fileDataConvertor_->HandleAlbumId(data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleHashTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "hashId";
    auto ret = fileDataConvertor_->HandleHash(data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileNameTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "fileName";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    auto ret = fileDataConvertor_->HandleFileName(data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileNameTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    string key = "fileName";
    auto ret = fileDataConvertor_->HandleFileName(data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourceTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    auto ret = fileDataConvertor_->HandleSource(data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleCreatedTimeTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "createdTime";
    auto ret = fileDataConvertor_->HandleCreatedTime(data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleCreatedTimeTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    string key = "createdTime";
    auto ret = fileDataConvertor_->HandleCreatedTime(data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFavoriteTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "favorite";
    auto ret = fileDataConvertor_->HandleFavorite(data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFavoriteTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
    string key = "favorite";
    auto ret = fileDataConvertor_->HandleFavorite(data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRecycleTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "recycled";
    auto ret = fileDataConvertor_->HandleRecycleTime(data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRecycleTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    string key = "recycled";
    auto ret = fileDataConvertor_->HandleRecycleTime(data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleDescriptionTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    string key = "description";
    auto ret = fileDataConvertor_->HandleDescription(data, resultSet);
    EXPECT_TRUE(E_OK == ret || E_RDB == ret);
}

HWTEST_F(FileDataConvertorTest, HandlePropertiesTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    auto ret = fileDataConvertor_->HandleProperties(data, resultSet);
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetString(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetLong(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetDouble(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetInt(_, _)).WillRepeatedly(Return(0));
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandlePositionTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "position";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetDouble(_, _)).WillRepeatedly(Return(0));
    auto ret = fileDataConvertor_->HandlePosition(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleHeightTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "height";
    auto ret = fileDataConvertor_->HandleHeight(map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleHeightTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
    string key = "height";
    auto ret = fileDataConvertor_->HandleHeight(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRotateTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "rotation";
    auto ret = fileDataConvertor_->HandleRotate(map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRotateTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
    string key = "rotation";
    auto ret = fileDataConvertor_->HandleRotate(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleWidthTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "width";
    auto ret = fileDataConvertor_->HandleWidth(map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleWidthTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
    string key = "width";
    auto ret = fileDataConvertor_->HandleWidth(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleDataModifiedTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "data_modified";
    auto ret = fileDataConvertor_->HandleDataModified(map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleDataModifiedTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    string key = "data_modified";
    auto ret = fileDataConvertor_->HandleDataModified(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleDetailTimeTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "detail_time";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    auto ret = fileDataConvertor_->HandleDetailTime(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileCreateTimeTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "fileCreateTime";
    auto ret = fileDataConvertor_->HandleFileCreateTime(map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileCreateTimeTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    string key = "fileCreateTime";
    auto ret = fileDataConvertor_->HandleFileCreateTime(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFirstUpdateTimeTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "first_update_time";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    auto ret = fileDataConvertor_->HandleFirstUpdateTime(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRelativeBucketIdTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "relative_bucket_id";
    auto ret = fileDataConvertor_->HandleRelativeBucketId(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourceFileNameTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "sourceFileName";
    auto ret = fileDataConvertor_->HandleSourceFileName(map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourceFileNameTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    string key = "sourceFileName";
    auto ret = fileDataConvertor_->HandleSourceFileName(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourcePathTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "sourcePath";
    auto ret = fileDataConvertor_->HandleSourcePath(map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourcePathTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    string key = "sourcePath";
    auto ret = fileDataConvertor_->HandleSourcePath(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleTimeZoneTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "time_zone";
    auto ret = fileDataConvertor_->HandleTimeZone(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleContentTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    string path;
    auto ret = fileDataConvertor_->HandleContent(data, path);
    EXPECT_EQ(E_PATH, ret);
}

HWTEST_F(FileDataConvertorTest, HandleThumbnailTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    string path;
    auto ret = fileDataConvertor_->HandleThumbnail(data, path);
    EXPECT_EQ(E_PATH, ret);
}

HWTEST_F(FileDataConvertorTest, HandleLcdTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    string path;
    auto ret = fileDataConvertor_->HandleLcd(data, path);
    EXPECT_EQ(E_PATH, ret);
}

HWTEST_F(FileDataConvertorTest, HandleGeneralTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "general";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetString(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetLong(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetDouble(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetInt(_, _)).WillRepeatedly(Return(0));
    auto ret = fileDataConvertor_->HandleGeneral(map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleAttachmentsTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "attachments";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    auto ret = fileDataConvertor_->HandleAttachments(data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleAttachments1Test, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "attachments";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    auto ret = fileDataConvertor_->HandleAttachments(data, resultSet);
    EXPECT_EQ(E_PATH, ret);
}

HWTEST_F(FileDataConvertorTest, HandleAttachments2Test, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "attachments";
    fileDataConvertor_->type_ = OperationType::FILE_METADATA_MODIFY;
    auto ret = fileDataConvertor_->HandleAttachments(data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ConvertTest, TestSize.Level1)
{
    DriveKit::DKRecord record;
    ResultSetMock resultSet;
    fileDataConvertor_->type_ = OperationType::FILE_CREATE;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetString(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetLong(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetDouble(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(resultSet,  GetInt(_, _)).WillRepeatedly(Return(0));
    auto ret = fileDataConvertor_->Convert(record, resultSet);
    EXPECT_EQ(E_PATH, ret);
}

HWTEST_F(FileDataConvertorTest, GetLowerPathTest, TestSize.Level1)
{
    string path = "";
    auto ret = fileDataConvertor_->GetLowerPath(path);
    string out = "";
    EXPECT_EQ(out, ret);
}

HWTEST_F(FileDataConvertorTest, GetLowerPathTest1, TestSize.Level1)
{
    string path = "/storage/cloud/files/1";
    auto ret = fileDataConvertor_->GetLowerPath(path);
    string out = "/data/service/el2/100/hmdfs/account/files/1";
    EXPECT_EQ(out, ret);
}

HWTEST_F(FileDataConvertorTest, GetLowerTmpPathTest, TestSize.Level1)
{
    string path = "";
    auto ret = fileDataConvertor_->GetLowerTmpPath(path);
    string out = "";
    EXPECT_EQ(out, ret);
}

HWTEST_F(FileDataConvertorTest, GetLowerTmpPathTest1, TestSize.Level1)
{
    string path = "/storage/cloud/files/1";
    auto ret = fileDataConvertor_->GetLowerTmpPath(path);
    string out = "/data/service/el2/100/hmdfs/account/files/1.temp.download";
    EXPECT_EQ(out, ret);
}

HWTEST_F(FileDataConvertorTest, GetSandboxPathTest, TestSize.Level1)
{
    string path = "";
    auto ret = fileDataConvertor_->GetSandboxPath(path);
    string out = "";
    EXPECT_EQ(out, ret);
}

HWTEST_F(FileDataConvertorTest, GetSandboxPathTest1, TestSize.Level1)
{
    string path = "/data/service/el2/100/hmdfs/account/files/1";
    auto ret = fileDataConvertor_->GetSandboxPath(path);
    string out = "/storage/cloud/files/1";
    EXPECT_EQ(out, ret);
}

HWTEST_F(FileDataConvertorTest, GetThumbPathTest, TestSize.Level1)
{
    string key;
    string path = "";
    auto ret = fileDataConvertor_->GetThumbPath(path, key);
    string out = "";
    EXPECT_EQ(out, ret);
}

HWTEST_F(FileDataConvertorTest, GetThumbPathTest1, TestSize.Level1)
{
    string key;
    string path = "/storage/cloud/files/1";
    auto ret = fileDataConvertor_->GetThumbPath(path, key);
    string out = "/mnt/hmdfs/100/account/device_view/local/files/.thumbs/1/.jpg";
    EXPECT_EQ(out, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateDataTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->CompensateData(data, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateDataTest2, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[PhotoColumn::MEDIA_FILE_PATH] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    data[FILE_PROPERTIES] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->CompensateData(data, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateTitleTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->CompensateTitle(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateTitleTest2, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    data[FILE_PROPERTIES] = DriveKit::DKRecordField();
    auto ret = fileDataConvertor_->CompensateTitle(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateTitleTest3, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_SOURCE_FILE_NAME] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    data[FILE_PROPERTIES] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->CompensateTitle(data, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateMediaTypeTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->CompensateMediaType(data, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateMediaTypeeTest2, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_FILETYPE] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    data[FILE_FILETYPE] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->CompensateMediaType(data, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateSubtypeTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->CompensateSubtype(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateSubtypeTest2, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    data[FILE_PROPERTIES] = DriveKit::DKRecordField();
    auto ret = fileDataConvertor_->CompensateSubtype(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateSubtypeTest3, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_SOURCE_PATH] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    data[FILE_PROPERTIES] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->CompensateSubtype(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateDurationTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->CompensateDuration(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateDurationTest2, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    data[FILE_PROPERTIES] = DriveKit::DKRecordField();
    auto ret = fileDataConvertor_->CompensateDuration(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, CompensateDurationTest3, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_DURATION] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    data[FILE_PROPERTIES] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->CompensateDuration(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractCompatibleValueTest1, TestSize.Level1)
{
    DriveKit::DKRecord record;
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractCompatibleValue(record, data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractOrientationTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractOrientation(map, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractOrientationTest2, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_ROTATION] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    map[FILE_ROTATION] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->ExtractOrientation(map, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractPositionTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractPosition(map, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractPositionTest2, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_POSITION] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    map[FILE_POSITION] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->ExtractPosition(map, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractHeightTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractHeight(map, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractHeightTest2, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_HEIGHT] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    map[FILE_HEIGHT] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->ExtractHeight(map, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractWidthTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractWidth(map, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractWidthTest2, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_WIDTH] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    map[FILE_WIDTH] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->ExtractWidth(map, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractSizeTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractSize(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractSizeTest2, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_SIZE] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    map[FILE_SIZE] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->ExtractSize(map, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractDisplayNameTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractDisplayName(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractDisplayNameTest2, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_FILE_NAME] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    data[FILE_FILE_NAME] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->ExtractDisplayName(data, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractMimeTypeTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractMimeType(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractMimeTypeTest2, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_MIME_TYPE] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    data[FILE_MIME_TYPE] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->ExtractMimeType(data, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractDeviceNameTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractDeviceName(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractDeviceNameTest2, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    DriveKit::DKRecordFieldMap fieldValue1;
    fieldValue1[FILE_SOURCE] = DriveKit::DKRecordField();
    DriveKit::DKFieldValue fieldValue = fieldValue1;
    data[FILE_SOURCE] = DriveKit::DKRecordField(fieldValue);
    auto ret = fileDataConvertor_->ExtractDeviceName(data, valueBucket);
    EXPECT_EQ(E_INVAL_ARG, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractFavoriteTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractFavorite(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ExtractDateTrashedTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    NativeRdb::ValuesBucket valueBucket;
    auto ret = fileDataConvertor_->ExtractDateTrashed(data, valueBucket);
    EXPECT_EQ(E_OK, ret);
}
} // namespace OHOS::FileManagement::CloudSync::Test