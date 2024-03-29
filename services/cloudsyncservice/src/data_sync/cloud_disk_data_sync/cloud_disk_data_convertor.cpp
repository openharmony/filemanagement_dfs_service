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
    if (!rootId_.empty()) {
        return E_OK;
    }
    CloudPrefImpl cloudPrefImpl(userId_, bundleName_, FileColumn::FILES_TABLE);
    cloudPrefImpl.GetString("rootId", rootId_);
    LOGD("local rootId is %{public}s", rootId_.c_str());
    if (rootId_.empty()) {
        auto driveKit = DriveKit::DriveKitNative::GetInstance(userId_);
        if (driveKit == nullptr) {
            LOGE("sdk helper get drive kit instance fail");
            return E_INVAL_ARG;
        }
        auto container = driveKit->GetDefaultContainer(bundleName_);
        if (container == nullptr) {
            LOGE("sdk helper get drive kit container fail");
            return E_INVAL_ARG;
        }
        shared_ptr<DriveKit::DKDatabase> database = container->GetPrivateDatabase();
        if (database == nullptr) {
            LOGE("sdk helper get drive kit database fail");
            return E_INVAL_ARG;
        }
        DriveKit::DKError dkErr = database->GetRootId(rootId_);
        LOGD("database rootId is %{public}s", rootId_.c_str());
        if (dkErr.dkErrorCode != DriveKit::DKLocalErrorCode::NO_ERROR || rootId_.empty()) {
            LOGE("get root id failed, err %{public}d", dkErr.dkErrorCode);
            return E_INVAL_ARG;
        }
        cloudPrefImpl.SetString("rootId", rootId_);
    }
    return E_OK;
}
std::string CloudDiskDataConvertor::GetParentCloudId(const DriveKit::DKRecordData &data)
{
    string parentFolder;
    if (data.find(DK_PARENT_CLOUD_ID) == data.end() ||
        data.at(DK_PARENT_CLOUD_ID).GetString(parentFolder) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract parent cloudId error");
        return "";
    }
    if (parentFolder == rootId_) {
        parentFolder = "rootId";
    }
    return parentFolder;
}

int32_t CloudDiskDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
{
    RETURN_ON_ERR(InitRootId());
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    ExtractCompatibleValue(record, data, valueBucket);
    return E_OK;
}
int32_t CloudDiskDataConvertor::Convert(DriveKit::DKRecord &record, NativeRdb::ResultSet &resultSet)
{
    RETURN_ON_ERR(InitRootId());
    DriveKit::DKRecordData data;
    RETURN_ON_ERR(FillRecordId(record, resultSet));
    RETURN_ON_ERR(FillCreatedTime(record, resultSet));
    RETURN_ON_ERR(FillMetaEditedTime(record, resultSet));
    RETURN_ON_ERR(FillVersion(record, resultSet));
    RETURN_ON_ERR(HandleCompatibleFileds(data, resultSet));
    RETURN_ON_ERR(HandleAttributes(data, resultSet));
    record.SetRecordData(data);
    record.SetRecordType(recordType_);
    if (type_ == FILE_CREATE) {
        record.SetNewCreate(true);
    }
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractCompatibleValue(const DriveKit::DKRecord &record,
                                                       const DriveKit::DKRecordData &data,
                                                       NativeRdb::ValuesBucket &valueBucket)
{
    RETURN_ON_ERR(ExtractCloudId(record, valueBucket));
    if (record.GetIsDelete()) {
        return E_OK;
    }
    RETURN_ON_ERR(ExtractFileName(data, valueBucket));
    RETURN_ON_ERR(ExtractFileParentCloudId(data, valueBucket));
    RETURN_ON_ERR(ExtractDirectlyRecycled(data, valueBucket));
    RETURN_ON_ERR(ExtractIsDirectory(data, valueBucket));
    RETURN_ON_ERR(ExtractVersion(record, valueBucket));
    CompensateAttributes(data, record, valueBucket);
    ExtractFileTimeRecycled(data, valueBucket);
    return E_OK;
}
int32_t CloudDiskDataConvertor::CompensateAttributes(const DriveKit::DKRecordData &data,
                                                     const DriveKit::DKRecord &record,
                                                     NativeRdb::ValuesBucket &valueBucket)
{
    int64_t fileTimeAdded = 0;
    int64_t fileTimeEdited = 0;
    int64_t metaTimeEdited = 0;
    DriveKit::DKRecordFieldMap attributes;
    if (data.find(DK_FILE_ATTRIBUTES) == data.end()) {
        fileTimeAdded = static_cast<int64_t>(record.GetCreateTime());
        fileTimeEdited = static_cast<int64_t>(record.GetEditedTime());
        metaTimeEdited = static_cast<int64_t>(record.GetEditedTime());
    } else {
        data.at(DK_FILE_ATTRIBUTES).GetRecordMap(attributes);
        if (attributes.find(DK_FILE_TIME_ADDED) == attributes.end() ||
            attributes[DK_FILE_TIME_ADDED].GetLong(fileTimeAdded) != DKLocalErrorCode::NO_ERROR) {
            fileTimeAdded = static_cast<int64_t>(record.GetCreateTime());
        }
        if (attributes.find(DK_FILE_TIME_EDITED) == attributes.end() ||
            attributes[DK_FILE_TIME_EDITED].GetLong(fileTimeEdited) != DKLocalErrorCode::NO_ERROR) {
            fileTimeEdited = static_cast<int64_t>(record.GetEditedTime());
        }
        if (attributes.find(DK_META_TIME_EDITED) == attributes.end() ||
            attributes[DK_META_TIME_EDITED].GetLong(metaTimeEdited) != DKLocalErrorCode::NO_ERROR) {
            metaTimeEdited = static_cast<int64_t>(record.GetEditedTime());
        }
    }
    valueBucket.PutLong(FileColumn::FILE_TIME_ADDED, fileTimeAdded);
    valueBucket.PutLong(FileColumn::FILE_TIME_EDITED, fileTimeEdited);
    valueBucket.PutLong(FileColumn::META_TIME_EDITED, metaTimeEdited);
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractCloudId(const DriveKit::DKRecord &record,
                                               NativeRdb::ValuesBucket &valueBucket)
{
    std::string cloudId = record.GetRecordId();
    valueBucket.PutString(FileColumn::CLOUD_ID, cloudId);
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractFileName(const DriveKit::DKRecordData &data,
                                                NativeRdb::ValuesBucket &valueBucket)
{
    string fileName;
    if (data.find(DK_FILE_NAME) == data.end() ||
        data.at(DK_FILE_NAME).GetString(fileName) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract file name error");
        return E_INVAL_ARG;
    }
    size_t pos = fileName.find_last_of('.');
    if (pos != string::npos) {
        string extension = fileName.substr(fileName.find_last_of('.') + 1, fileName.length());
        valueBucket.PutString(FileColumn::FILE_CATEGORY, extension);
    }
    valueBucket.PutString(FileColumn::FILE_NAME, fileName);
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractFileParentCloudId(const DriveKit::DKRecordData &data,
                                                         NativeRdb::ValuesBucket &valueBucket)
{
    string parentFolder = GetParentCloudId(data);
    if (parentFolder.empty()) {
        LOGE("extract parent cloudId error");
        return E_INVAL_ARG;
    }
    valueBucket.PutString(FileColumn::PARENT_CLOUD_ID, parentFolder);
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractFileSize(const DriveKit::DKRecordData &data,
                                                NativeRdb::ValuesBucket &valueBucket)
{
    int64_t fileSize;
    if (data.find(DK_FILE_SIZE) == data.end() ||
        data.at(DK_FILE_SIZE).GetLong(fileSize) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract fileSize cloudId error");
        return E_INVAL_ARG;
    }
    valueBucket.PutLong(FileColumn::FILE_SIZE, fileSize);
    return E_OK;
}

int32_t CloudDiskDataConvertor::ExtractSha256(const DriveKit::DKRecordData &data,
                                              NativeRdb::ValuesBucket &valueBucket)
{
    string fileSha256;
    if (data.find(DK_FILE_SHA256) == data.end() ||
        data.at(DK_FILE_SHA256).GetString(fileSha256) != DKLocalErrorCode::NO_ERROR) {
        LOGW("extract sha256 error");
        return E_INVAL_ARG;
    }
    valueBucket.PutString(FileColumn::FILE_SHA256, fileSha256);
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractFileTimeRecycled(const DriveKit::DKRecordData &data,
                                                        NativeRdb::ValuesBucket &valueBucket)
{
    int64_t fileTimeRecycled = 0;
    bool isRecyled = false;
    if (data.find(DK_IS_RECYCLED) == data.end()
        || data.at(DK_IS_RECYCLED).GetBool(isRecyled) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract DK_IS_RECYCLED error");
        return E_INVAL_ARG;
    }
    if (isRecyled) {
        if (data.find(DK_FILE_TIME_RECYCLED) != data.end() &&
            data.at(DK_FILE_TIME_RECYCLED).GetLong(fileTimeRecycled) != DKLocalErrorCode::NO_ERROR) {
            LOGE("extract fileTimeRecycled error");
            return E_INVAL_ARG;
        }
    }
    valueBucket.PutLong(FileColumn::FILE_TIME_RECYCLED, fileTimeRecycled);
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractDirectlyRecycled(const DriveKit::DKRecordData &data,
                                                        NativeRdb::ValuesBucket &valueBucket)
{
    bool directoryRecycled;
    if (data.find(DK_DIRECTLY_RECYCLED) == data.end() ||
        data.at(DK_DIRECTLY_RECYCLED).GetBool(directoryRecycled) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract directoryRecycled error");
        return E_INVAL_ARG;
    }
    valueBucket.PutInt(FileColumn::DIRECTLY_RECYCLED, directoryRecycled);
    return E_OK;
}

int32_t CloudDiskDataConvertor::ExtractVersion(const DriveKit::DKRecord &record,
                                               NativeRdb::ValuesBucket &valueBucket)
{
    unsigned long version = record.GetVersion();
    valueBucket.PutLong(FileColumn::VERSION, version);
    return E_OK;
}
int32_t CloudDiskDataConvertor::ExtractIsDirectory(const DriveKit::DKRecordData &data,
                                                   NativeRdb::ValuesBucket &valueBucket)
{
    string fileType;
    if (data.find(DK_IS_DIRECTORY) == data.end() ||
        data.at(DK_IS_DIRECTORY).GetString(fileType) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract type error");
        return E_INVAL_ARG;
    }
    int32_t type = DIRECTORY;
    if (fileType == "file") {
        type = FILE;
        ExtractSha256(data, valueBucket);
        RETURN_ON_ERR(ExtractFileSize(data, valueBucket));
    }
    valueBucket.PutInt(FileColumn::IS_DIRECTORY, type);
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleCompatibleFileds(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    RETURN_ON_ERR(HandleFileName(data, resultSet));
    RETURN_ON_ERR(HandleParentId(data, resultSet));
    RETURN_ON_ERR(HandleDirectlyRecycled(data, resultSet));
    RETURN_ON_ERR(HandleRecycleTime(data, resultSet));
    RETURN_ON_ERR(HandleType(data, resultSet));
    RETURN_ON_ERR(HandleOperateType(data, resultSet));
    RETURN_ON_ERR(HandleFileSize(data, resultSet));
    RETURN_ON_ERR(HandleAttachments(data, resultSet));
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleAttachments(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ != FILE_CREATE && type_ != FILE_DATA_MODIFY) {
        return E_OK;
    }
    int32_t isDirectory;
    if (GetInt(FileColumn::IS_DIRECTORY, isDirectory, resultSet)) {
        LOGE("Get File Type is failed");
        return E_RDB;
    }
    if (isDirectory == FILE) {
        string cloudId;
        string filePath;
        if (!GetString(FileColumn::CLOUD_ID, cloudId, resultSet)) {
            filePath = CloudFileUtils::GetLocalFilePath(cloudId, bundleName_, userId_);
        } else {
            LOGE("Get File Path is failed");
            return E_RDB;
        }
        int32_t ret = HandleContent(data, filePath);
        if (ret != E_OK) {
            LOGE("handle content err %{public}d", ret);
            return ret;
        }
    }
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleAttributes(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    DriveKit::DKRecordFieldMap map;
    RETURN_ON_ERR(HandleCreateTime(map, resultSet));
    RETURN_ON_ERR(HandleMetaEditedTime(map, resultSet));
    RETURN_ON_ERR(HandleEditedTime(map, resultSet));
    data[DK_FILE_ATTRIBUTES] = DriveKit::DKRecordField(map);
    return E_OK;
}
int32_t CloudDiskDataConvertor::FillRecordId(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    string val;
    int32_t ret = GetString(FileColumn::CLOUD_ID, val, resultSet);
    if (ret != E_OK) {
        LOGE("Fill record id failed, ret %{public}d", ret);
        return ret;
    }
    record.SetRecordId(val);
    return E_OK;
}
int32_t CloudDiskDataConvertor::FillCreatedTime(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    int64_t createdTime;
    int32_t ret = GetLong(FileColumn::FILE_TIME_ADDED, createdTime, resultSet);
    if (ret != E_OK) {
        LOGE("Fill CreatedTime failed, ret = %{public}d", ret);
        return ret;
    }
    record.SetCreateTime(static_cast<uint64_t>(createdTime));
    return E_OK;
}
int32_t CloudDiskDataConvertor::FillMetaEditedTime(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    int64_t metaEditedTime;
    int32_t ret = GetLong(FileColumn::META_TIME_EDITED, metaEditedTime, resultSet);
    if (ret != E_OK) {
        LOGE("Fill MetaEditedTime failed, ret = %{public}d", ret);
        return ret;
    }
    record.SetEditedTime(static_cast<uint64_t>(metaEditedTime));
    return E_OK;
}
int32_t CloudDiskDataConvertor::FillVersion(DriveKit::DKRecord &record,
    NativeRdb::ResultSet &resultSet)
{
    int64_t version;
    int32_t ret = GetLong(FileColumn::VERSION, version, resultSet);
    if (ret != E_OK) {
        LOGE("Fill Version failed, ret = %{public}d", ret);
        return ret;
    }
    record.SetVersion(static_cast<unsigned long>(version));
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleFileName(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    std::string displayName;
    int32_t ret = GetString(FileColumn::FILE_NAME, displayName, resultSet);
    if (ret != E_OK) {
        LOGE("handler FileName failed, ret = %{public}d", ret);
        return ret;
    }
    data[DK_FILE_NAME] = DriveKit::DKRecordField(displayName);
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleParentId(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    std::string parentFolder;
    int32_t ret = GetString(FileColumn::PARENT_CLOUD_ID, parentFolder, resultSet);
    if (ret != E_OK) {
        LOGE("handler ParentId failed, ret = %{public}d", ret);
        return ret;
    }
    if (parentFolder == "rootId") {
        parentFolder = rootId_;
    }
    data[DK_PARENT_CLOUD_ID] = DriveKit::DKRecordField(parentFolder);
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleDirectlyRecycled(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    int32_t directlyRecycled;
    int32_t ret = GetInt(FileColumn::DIRECTLY_RECYCLED, directlyRecycled, resultSet);
    if (ret != E_OK) {
        LOGE("handler DirectlyRecycled failed, ret = %{public}d", ret);
        return ret;
    }
    data[DK_DIRECTLY_RECYCLED] = DriveKit::DKRecordField((directlyRecycled != 0));
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleRecycleTime(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    int64_t recycleTime;
    int32_t ret = GetLong(FileColumn::FILE_TIME_RECYCLED, recycleTime, resultSet);
    if (ret != E_OK) {
        LOGE("handler RecycleTime failed, ret = %{public}d", ret);
        return ret;
    }
    data[DK_IS_RECYCLED] = DriveKit::DKRecordField((recycleTime != 0));
    if (recycleTime != 0) {
        data[DK_FILE_TIME_RECYCLED] = DriveKit::DKRecordField(recycleTime);
    }
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleType(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    int32_t type;
    int32_t ret = GetInt(FileColumn::IS_DIRECTORY, type, resultSet);
    if (ret != E_OK) {
        LOGE("handler type failed, ret = %{public}d", ret);
        return ret;
    }
    data[DK_IS_DIRECTORY] = DriveKit::DKRecordField(type == DIRECTORY ? "directory" : "file");
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleOperateType(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    int64_t operateType;
    int32_t ret = GetLong(FileColumn::OPERATE_TYPE, operateType, resultSet);
    if (ret != E_OK) {
        LOGE("handler operateType failed, ret = %{public}d", ret);
        return ret;
    }
    data[DK_FILE_OPERATE_TYPE] = DriveKit::DKRecordField(operateType);
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleFileSize(DriveKit::DKRecordData &data,
    NativeRdb::ResultSet &resultSet)
{
    if (type_ == FILE_DELETE) {
        return E_OK;
    }
    int64_t fileSize;
    int32_t ret = GetLong(FileColumn::FILE_SIZE, fileSize, resultSet);
    if (ret != E_OK) {
        LOGE("handler fileSize failed, ret = %{public}d", ret);
        return ret;
    }
    data[DK_FILE_SIZE] = DriveKit::DKRecordField(fileSize);
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleCreateTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t createTime;
    int32_t ret = GetLong(FileColumn::FILE_TIME_ADDED, createTime, resultSet);
    if (ret != E_OK) {
        LOGE("handler CreateTime failed, ret = %{public}d", ret);
        return ret;
    }
    map[DK_FILE_TIME_ADDED] = DriveKit::DKRecordField(createTime);
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleMetaEditedTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t metaEditedTime;
    int32_t ret = GetLong(FileColumn::META_TIME_EDITED, metaEditedTime, resultSet);
    if (ret != E_OK) {
        LOGE("handler MetaEditedTime failed, ret = %{public}d", ret);
        return ret;
    }
    map[DK_META_TIME_EDITED] = DriveKit::DKRecordField(metaEditedTime);
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleEditedTime(DriveKit::DKRecordFieldMap &map,
    NativeRdb::ResultSet &resultSet)
{
    int64_t editedTime;
    int32_t ret = GetLong(FileColumn::FILE_TIME_EDITED, editedTime, resultSet);
    if (ret != E_OK) {
        LOGE("handler EditedTime failed, ret = %{public}d", ret);
        return ret;
    }
    map[DK_FILE_TIME_EDITED] = DriveKit::DKRecordField(editedTime);
    return E_OK;
}
int32_t CloudDiskDataConvertor::HandleContent(DriveKit::DKRecordData &data,
    string &path)
{
    if (access(path.c_str(), F_OK)) {
        LOGE("content %{private}s doesn't exist", path.c_str());
        return E_PATH;
    }
    DriveKit::DKAsset content;
    content.uri = move(path);
    content.assetName = DK_FILE_CONTENT;
    content.operationType = DriveKit::DKAssetOperType::DK_ASSET_ADD;
    data[DK_FILE_CONTENT] = DriveKit::DKRecordField(content);
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS