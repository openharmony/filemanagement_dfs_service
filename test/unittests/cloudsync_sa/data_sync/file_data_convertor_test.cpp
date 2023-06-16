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
    FileDataConvertor::OperationType Type = FileDataConvertor::OperationType::FILE_CREATE;
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
    auto ret = fileDataConvertor_->HandleAlbumId(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleHashIdTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "hashId";
    auto ret = fileDataConvertor_->HandleHashId(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileNameTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "fileName";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    auto ret = fileDataConvertor_->HandleFileName(key, data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileNameTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    string key = "fileName";
    auto ret = fileDataConvertor_->HandleFileName(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourceTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "source";
    auto ret = fileDataConvertor_->HandleSource(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileTypeTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "fileType";
    auto ret = fileDataConvertor_->HandleFileType(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleCreatedTimeTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "createdTime";
    auto ret = fileDataConvertor_->HandleCreatedTime(key, data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleCreatedTimeTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    string key = "createdTime";
    auto ret = fileDataConvertor_->HandleCreatedTime(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFavoriteTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "favorite";
    auto ret = fileDataConvertor_->HandleFavorite(key, data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFavoriteTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
    string key = "favorite";
    auto ret = fileDataConvertor_->HandleFavorite(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRecycleTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "recycled";
    auto ret = fileDataConvertor_->HandleRecycle(key, data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRecycleTest1, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    string key = "recycled";
    auto ret = fileDataConvertor_->HandleRecycle(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleDescriptionTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "description";
    auto ret = fileDataConvertor_->HandleDescription(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandlePropertiesTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "properties";
    auto ret = fileDataConvertor_->HandleProperties(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandlePositionTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "position";
    auto ret = fileDataConvertor_->HandlePosition(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleHeightTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "height";
    auto ret = fileDataConvertor_->HandleHeight(key, map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleHeightTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
    string key = "height";
    auto ret = fileDataConvertor_->HandleHeight(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRotationTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "rotation";
    auto ret = fileDataConvertor_->HandleRotation(key, map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRotationTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
    string key = "rotation";
    auto ret = fileDataConvertor_->HandleRotation(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleWidthTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "width";
    auto ret = fileDataConvertor_->HandleWidth(key, map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleWidthTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
    string key = "width";
    auto ret = fileDataConvertor_->HandleWidth(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleDataModifiedTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "data_modified";
    auto ret = fileDataConvertor_->HandleDataModified(key, map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleDataModifiedTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    string key = "data_modified";
    auto ret = fileDataConvertor_->HandleDataModified(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleDetailTimeTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "detail_time";
    auto ret = fileDataConvertor_->HandleDetailTime(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileCreateTimeTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "fileCreateTime";
    auto ret = fileDataConvertor_->HandleFileCreateTime(key, map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFileCreateTimeTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
    string key = "fileCreateTime";
    auto ret = fileDataConvertor_->HandleFileCreateTime(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleFirstUpdateTimeTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "first_update_time";
    auto ret = fileDataConvertor_->HandleFirstUpdateTime(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleRelativeBucketIdTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "relative_bucket_id";
    auto ret = fileDataConvertor_->HandleRelativeBucketId(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourceFileNameTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "sourceFileName";
    auto ret = fileDataConvertor_->HandleSourceFileName(key, map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourceFileNameTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    string key = "sourceFileName";
    auto ret = fileDataConvertor_->HandleSourceFileName(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourcePathTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    string key = "sourcePath";
    auto ret = fileDataConvertor_->HandleSourcePath(key, map, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleSourcePathTest1, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    string key = "sourcePath";
    auto ret = fileDataConvertor_->HandleSourcePath(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleTimeZoneTest, TestSize.Level1)
{
    DriveKit::DKRecordFieldMap map;
    ResultSetMock resultSet;
    string key = "time_zone";
    auto ret = fileDataConvertor_->HandleTimeZone(key, map, resultSet);
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
    auto ret = fileDataConvertor_->HandleGeneral(key, map, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, HandleAttachmentsTest, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "attachments";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    auto ret = fileDataConvertor_->HandleAttachments(key, data, resultSet);
    EXPECT_EQ(E_RDB, ret);
}

HWTEST_F(FileDataConvertorTest, HandleAttachments1Test, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "attachments";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
    auto ret = fileDataConvertor_->HandleAttachments(key, data, resultSet);
    EXPECT_EQ(E_PATH, ret);
}

HWTEST_F(FileDataConvertorTest, HandleAttachments2Test, TestSize.Level1)
{
    DriveKit::DKRecordData data;
    ResultSetMock resultSet;
    string key = "attachments";
    fileDataConvertor_->type_ = FileDataConvertor::OperationType::FILE_METADATA_MODIFY;
    auto ret = fileDataConvertor_->HandleAttachments(key, data, resultSet);
    EXPECT_EQ(E_OK, ret);
}

HWTEST_F(FileDataConvertorTest, ConvertTest, TestSize.Level1)
{
    DriveKit::DKRecord record;
    ResultSetMock resultSet;
    fileDataConvertor_->type_ = FileDataConvertor::OperationType::FILE_CREATE;
    auto ret = fileDataConvertor_->Convert(record, resultSet);
    EXPECT_EQ(E_PATH, ret);
}

HWTEST_F(FileDataConvertorTest, ConvertTest1, TestSize.Level1)
{
    DriveKit::DKRecord record;
    ResultSetMock resultSet;
    fileDataConvertor_->type_ = FileDataConvertor::OperationType::FILE_DELETE;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
    auto ret = fileDataConvertor_->Convert(record, resultSet);
    EXPECT_EQ(E_RDB, ret);
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
    string out = "/data/service/el2/100/hmdfs/account/files/1";
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
} // namespace OHOS::FileManagement::CloudSync::Test