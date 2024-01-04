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

#ifndef OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_DATA_CONVERTOR_H
#define OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_DATA_CONVERTOR_H

#include "cloud_disk_data_const.h"
#include "data_convertor.h"
#include "dfs_error.h"
#include "dk_record.h"
#include "file_column.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace CloudDisk;
class CloudDiskDataConvertor : public DataConvertor {
public:

    CloudDiskDataConvertor(int32_t userId,
                           std::string &bundleName,
                           OperationType type,
                           const std::function<void(int32_t err, NativeRdb::ResultSet &resultSet)> &func = nullptr);
    ~CloudDiskDataConvertor() = default;

    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);
    void HandleErr(int32_t err, NativeRdb::ResultSet &resultSet);
    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valuesBucket);
    int32_t GetMetaFilePath(const std::string &cloudId, std::string &path);
    void SetRootId(std::string rootId);
    std::string GetParentCloudId(const DriveKit::DKRecordData &data);

private:
    int32_t FillRecordId(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);
    int32_t FillCreatedTime(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);
    int32_t FillMetaEditedTime(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);
    int32_t FillVersion(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet);
    int32_t ExtractCompatibleValue(const DriveKit::DKRecord &record,
                                   const DriveKit::DKRecordData &data,
                                   NativeRdb::ValuesBucket &valueBucket);
    int32_t CompensateAttributes(const DriveKit::DKRecordData &data,
                                 const DriveKit::DKRecord &record,
                                 NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractCloudId(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractFileName(const DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractFileParentCloudId(const DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractFileSize(const DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractSha256(const DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractFileTimeRecycled(const DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractDirectlyRecycled(const DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractIsDirectory(const DriveKit::DKRecordData &data, NativeRdb::ValuesBucket &valueBucket);
    int32_t ExtractVersion(const DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket);
    int32_t HandleFileName(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleParentId(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleDirectlyRecycled(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleRecycleTime(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleOperateType(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleFileSize(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleCompatibleFileds(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleAttributes(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleCreateTime(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleMetaEditedTime(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleEditedTime(DriveKit::DKRecordFieldMap &map, NativeRdb::ResultSet &resultSet);
    int32_t HandleAttachments(DriveKit::DKRecordData &data, NativeRdb::ResultSet &resultSet);
    int32_t HandleContent(DriveKit::DKRecordData &data, std::string &path);
    int32_t InitRootId();

    int32_t userId_{0};
    std::string bundleName_;
    OperationType type_;
    DriveKit::DKRecordId rootId_ = "";
    static std::string recordType_;
    std::function<void(int32_t, NativeRdb::ResultSet &resultSet)> errHandler_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_Disk_DATA_CONVERTOR_H
