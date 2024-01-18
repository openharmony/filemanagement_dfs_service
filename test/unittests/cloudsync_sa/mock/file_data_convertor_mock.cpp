/*
 * Copyright (C) 2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "file_data_convertor.h"

#include <regex>
#include <unistd.h>
#include <sys/stat.h>

#include "medialibrary_db_const.h"
#include "thumbnail_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace Media;
using DriveKit::DKLocalErrorCode;

FileDataConvertor::FileDataConvertor(int32_t userId, string &bundleName, OperationType type,
    const function<void(int32_t, NativeRdb::ResultSet &resultSet)> &func) : userId_(userId),
    bundleName_(bundleName), type_(type), errHandler_(func)
{
}

int32_t FileDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

void FileDataConvertor::HandleErr(int32_t err, NativeRdb::ResultSet &resultSet)
{
}

int32_t FileDataConvertor::ConvertAsset(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

int32_t FileDataConvertor::HandleUniqueFileds(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

/* Cloud sync unique filed processing */
int32_t FileDataConvertor::HandleAttributes(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

int32_t FileDataConvertor::HandleCompatibleFileds(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

/* properties */
int32_t FileDataConvertor::HandleProperties(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

/* attachments */
int32_t FileDataConvertor::HandleAttachments(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

/* record id */
int32_t FileDataConvertor::FillRecordId(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

int32_t FileDataConvertor::HandleThumbSize(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

int32_t FileDataConvertor::HandleLcdSize(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

string FileDataConvertor::StrCreateTime(const string &format, int64_t time)
{
    return "";
}

int32_t FileDataConvertor::HandleFormattedDate(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

int32_t FileDataConvertor::HandleDetailTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

int32_t FileDataConvertor::HandlePosition(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

int32_t FileDataConvertor::HandleRotate(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

/* properties - general */
int32_t FileDataConvertor::HandleGeneral(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}

/* attachments */
int32_t FileDataConvertor::HandleContent(DriveKit::DKRecordData &data,
    string &path)
{
    return E_OK;
}

int32_t FileDataConvertor::HandleThumbnail(DriveKit::DKRecordData &data,
    string &path)
{
    return E_OK;
}

int32_t FileDataConvertor::HandleLcd(DriveKit::DKRecordData &data,
    string &path)
{
    return E_OK;
}

string FileDataConvertor::GetLowerPath(const string &path)
{
    return "";
}

string FileDataConvertor::GetThumbParentPath(const string &path)
{
    return "";
}

string FileDataConvertor::GetLowerTmpPath(const string &path)
{
    return "";
}

string FileDataConvertor::GetSandboxPath(const string &path)
{
    return "";
}

string FileDataConvertor::GetThumbPath(const string &path, const string &key)
{
    return "";
}

string FileDataConvertor::GetHmdfsLocalPath(const string &path)
{
    return "";
}

string FileDataConvertor::GetThumbPathInCloud(const std::string &path, const std::string &key)
{
    return "";
}

string FileDataConvertor::GetLocalPathToCloud(const std::string &path)
{
    return "";
}

bool FileDataConvertor::IfContainsAttributes(const DriveKit::DKRecord &record)
{
    return false;
}

int32_t FileDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::TryCompensateValue(const DriveKit::DKRecord &record,
    DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateData(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateTitle(DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateMediaType(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateDataAdded(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateMetaDateModified(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateSubtype(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateDuration(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateFormattedDate(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::CompensateFormattedDate(uint64_t dateAdded, NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractAttributeValue(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractCompatibleValue(const DriveKit::DKRecord &record,
    DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractOrientation(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractPosition(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractHeight(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractWidth(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractSize(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractDisplayName(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractMimeType(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractDeviceName(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractDateModified(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractDateTaken(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractFavorite(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractDateTrashed(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractCloudId(const DriveKit::DKRecord &record,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t FileDataConvertor::ExtractDescription(DriveKit::DKRecordData &data,
    NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
