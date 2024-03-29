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

#include "cloud_disk_data_convertor.h"

#include <regex>
#include <sys/stat.h>
#include <unistd.h>

#include "cloud_disk_data_const.h"
#include "cloud_pref_impl.h"
#include "cloud_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace CloudDisk;
using DriveKit::DKLocalErrorCode;

string CloudDiskDataConvertor::recordType_ = "file";

CloudDiskDataConvertor::CloudDiskDataConvertor(int32_t userId,
                                               string &bundleName,
                                               OperationType type,
                                               const function<void(int32_t, NativeRdb::ResultSet &resultSet)> &func)
    : userId_(userId), bundleName_(bundleName), type_(type), errHandler_(func)
{
}
void CloudDiskDataConvertor::HandleErr(int32_t err, NativeRdb::ResultSet &resultSet)
{
    if (errHandler_ != nullptr) {
        errHandler_(err, resultSet);
    }
}

void CloudDiskDataConvertor::SetRootId(string rootId)
{
    rootId_ = rootId;
}

int32_t CloudDiskDataConvertor::InitRootId()
{
    return E_OK;
}
std::string CloudDiskDataConvertor::GetParentCloudId(const DriveKit::DKRecordData &data)
{
    return "";
}

int32_t CloudDiskDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
{
    if (!rootId_.empty()) {
        return E_OK;
    }
    return E_INVAL_ARG;
}
int32_t CloudDiskDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractCompatibleValue(const DriveKit::DKRecord &record,
                                                       const DriveKit::DKRecordData &data,
                                                       NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::CompensateAttributes(const DriveKit::DKRecordData &data,
                                                     const DriveKit::DKRecord &record,
                                                     NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractCloudId(const DriveKit::DKRecord &record,
                                               NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractFileName(const DriveKit::DKRecordData &data,
                                                NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractFileParentCloudId(const DriveKit::DKRecordData &data,
                                                         NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractFileSize(const DriveKit::DKRecordData &data,
                                                NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t CloudDiskDataConvertor::ExtractSha256(const DriveKit::DKRecordData &data,
                                              NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractFileTimeRecycled(const DriveKit::DKRecordData &data,
                                                        NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractDirectlyRecycled(const DriveKit::DKRecordData &data,
                                                        NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}

int32_t CloudDiskDataConvertor::ExtractVersion(const DriveKit::DKRecord &record,
                                               NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractIsDirectory(const DriveKit::DKRecordData &data,
                                                   NativeRdb::ValuesBucket &valueBucket)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleCompatibleFileds(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleAttachments(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleAttributes(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::FillRecordId(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::FillCreatedTime(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::FillMetaEditedTime(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::FillVersion(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleFileName(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleParentId(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleDirectlyRecycled(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleRecycleTime(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleType(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleOperateType(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleCreateTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleMetaEditedTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleEditedTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleContent(DriveKit::DKRecordData &data,
    string &path)
{
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS