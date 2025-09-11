/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "oh_cloud_disk_manager.h"

#include <cstring>
#include <functional>
#include <new>
#include <vector>

#include <securec.h>

#include "cloud_disk_common.h"
#include "cloud_disk_service_callback.h"
#include "cloud_disk_service_manager.h"
#include "oh_cloud_disk_utils.h"
#include "utils_log.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::FileManagement;
using namespace OHOS::FileManagement::CloudDiskService;

static char *AllocField(const char *value, size_t length)
{
    char *field = new(std::nothrow) char[length + 1];
    if (field == nullptr) {
        LOGE("new failed");
        return nullptr;
    }

    if (strncpy_s(field, length + 1, value, length) != 0) {
        LOGE("strncpy_s failed");
        delete[] field;
        return nullptr;
    }
    field[length] = '\0';
    return field;
}

static bool IsValidPathInfo(const char *path, size_t length)
{
    if (path == nullptr) {
        LOGE("path is nullptr");
        return false;
    }
    if (length == 0) {
        LOGE("length is invalid: %{public}zu", length);
        return false;
    }
    size_t actualLen = strnlen(path, length + 1);
    if (actualLen != length) {
        LOGE("length not equal to actual string length: %{public}zu", actualLen);
        return false;
    }
    return true;
}

class CloudDiskServiceCallbackImpl : public CloudDiskServiceCallback {
public:
    using OnChangeDataCallback =
        function<void(const CloudDisk_SyncFolderPath syncFolderPath, const CloudDisk_ChangeData changeDatas[],
                      size_t length)>;
    explicit CloudDiskServiceCallbackImpl(OnChangeDataCallback callback)
        : callback_(callback) {};
    void OnChangeData(const std::string &syncFolder, const std::vector<ChangeData> &changeData) override;
    ~CloudDiskServiceCallbackImpl() override = default;

private:
    OnChangeDataCallback callback_;
};

void CloudDiskServiceCallbackImpl::OnChangeData(const std::string &syncFolder,
                                                const std::vector<ChangeData> &changeData)
{
    if (callback_ == nullptr) {
        LOGE("Can not find callback, just return");
        return;
    }
    char *path = new (std::nothrow) char[syncFolder.length() + 1];
    if (path == nullptr) {
        LOGE("Failed to allocate memory for path");
        return;
    }
    if (strncpy_s(path, syncFolder.length() + 1, syncFolder.c_str(), syncFolder.length()) != 0) {
        LOGE("strncpy_s failed");
        delete[] path;
        return;
    }
    CloudDisk_SyncFolderPath syncFolderPath = {.value = path, .length = syncFolder.length()};
    CloudDisk_ChangeData *dataDatas = new (std::nothrow) CloudDisk_ChangeData[changeData.size()];
    if (dataDatas == nullptr) {
        LOGE("Failed to allocate memory for changeDatas");
        delete[] syncFolderPath.value = nullptr;
        syncFolderPath.value = nullptr;
        return;
    }
    LOGD("OnChangeData callback, syncFolder: %{public}s, change datas size: %{public}zu", syncFolder.c_str(),
         changeData.size());
    size_t length = 0;
    for (auto &item : changeData) {
        CloudDisk_ChangeData data;
        data.fileId.value = AllocField(item.fileId.c_str(), item.fileId.length());
        CHECK_NULLPTR_AND_CONTINUE(data.fileId.value);
        data.fileId.length = item.fileId.length();
        data.parentFileId.value = AllocField(item.parentFileId.c_str(), item.parentFileId.length());
        CHECK_NULLPTR_AND_CONTINUE(data.parentFileId.value, delete[] data.fileId.value; data.fileId.value = nullptr;);
        data.parentFileId.length = item.parentFileId.length();
        data.relativePathInfo.value = AllocField(item.relativePath.c_str(), item.relativePath.length());
        CHECK_NULLPTR_AND_CONTINUE(data.relativePathInfo.value, delete[] data.fileId.value; data.fileId.value = nullptr;
                                   delete[] data.parentFileId.value; data.parentFileId.value = nullptr;);
        data.relativePathInfo.length = item.relativePath.length();
        data.updateSequenceNumber = item.updateSequenceNumber;
        data.operationType = static_cast<CloudDisk_OperationType>(item.operationType);
        data.size = item.size;
        data.mtime = item.mtime;
        data.timeStamp = item.timeStamp;
        dataDatas[length++] = data;
    }
    callback_(syncFolderPath, dataDatas, length);
}

CloudDisk_ErrorCode
    OH_CloudDisk_RegisterSyncFolderChanges(const CloudDisk_SyncFolderPath syncFolderPath,
                                           void (*callback)(const CloudDisk_SyncFolderPath syncFolderPath,
                                                            const CloudDisk_ChangeData changeDatas[], size_t length))
{
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    if (callback == nullptr) {
        LOGE("Invalid argument, callback is nullptr");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    auto callbackInner = [callback](const CloudDisk_SyncFolderPath syncFolderPath,
        const CloudDisk_ChangeData changeDatas[], size_t length) {
        callback(syncFolderPath, changeDatas, length);
    };
    shared_ptr<CloudDiskServiceCallback> callbackImpl = make_shared<CloudDiskServiceCallbackImpl>(callbackInner);
    int32_t ret = CloudDiskServiceManager::GetInstance().RegisterSyncFolderChanges(
        string(syncFolderPath.value, syncFolderPath.length), callbackImpl);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Register sync folder change failed, ret: %{public}d", ret);
        return CovertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_ERR_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_UnregisterSyncFolderChanges(const CloudDisk_SyncFolderPath syncFolderPath)
{
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    int32_t ret = CloudDiskServiceManager::GetInstance().UnregisterSyncFolderChanges(
        string(syncFolderPath.value, syncFolderPath.length));
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Unegister sync folder change failed, ret: %{public}d", ret);
        return CovertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_ERR_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_GetSyncFolderChanges(const CloudDisk_SyncFolderPath syncFolderPath,
                                                      uint64_t startUsn,
                                                      size_t count,
                                                      CloudDisk_ChangesResult **changesResult)
{
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    ChangesResult changesRes;
    auto ret = CloudDiskServiceManager::GetInstance().GetSyncFolderChanges(
        string(syncFolderPath.value, syncFolderPath.length), count, startUsn, changesRes);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Get sync folder change failed, ret: %{public}d", ret);
        return CovertToErrorCode(ret);
    }
    *changesResult = (CloudDisk_ChangesResult *)malloc(sizeof(CloudDisk_ChangesResult) +
                                                       sizeof(CloudDisk_ChangeData) * changesRes.changesData.size());
    if (*changesResult == nullptr) {
        LOGE("Failed to allocate memory for changesResult");
        return CloudDisk_ErrorCode::CLOUD_DISK_INTERNAL_ERROR;
    }
    (*changesResult)->length = 0;
    (*changesResult)->nextUsn = changesRes.nextUsn;
    (*changesResult)->isEof = changesRes.isEof;
    for (auto &item : changesRes.changesData) {
        LOGD("change data: fileId=%{public}s, parentFileId=%{public}s, relativePath=%{public}s, "
             "updateSequenceNumber=%{public}lu, operationType=%{public}hhu",
             item.fileId.c_str(), item.parentFileId.c_str(), item.relativePath.c_str(), item.updateSequenceNumber,
             item.operationType);
        CloudDisk_ChangeData data;
        data.fileId.value = AllocField(item.fileId.c_str(), item.fileId.length());
        CHECK_NULLPTR_AND_CONTINUE(data.fileId.value);
        data.fileId.length = item.fileId.length();
        data.parentFileId.value = AllocField(item.parentFileId.c_str(), item.parentFileId.length());
        CHECK_NULLPTR_AND_CONTINUE(data.parentFileId.value, delete[] data.fileId.value; data.fileId.value = nullptr;);
        data.parentFileId.length = item.parentFileId.length();
        data.relativePathInfo.value = AllocField(item.relativePath.c_str(), item.relativePath.length());
        CHECK_NULLPTR_AND_CONTINUE(data.relativePathInfo.value, delete[] data.fileId.value; data.fileId.value = nullptr;
                                   delete[] data.parentFileId.value; data.parentFileId.value = nullptr;);
        data.relativePathInfo.length = item.relativePath.length();
        data.updateSequenceNumber = item.updateSequenceNumber;
        data.operationType = static_cast<CloudDisk_OperationType>(item.operationType);
        data.size = item.size;
        data.mtime = item.mtime;
        data.timeStamp = item.timeStamp;
        (*changesResult)->dataDatas[(*changesResult)->length++] = data;
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_ERR_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_SetFileSyncStates(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const CloudDisk_FileSyncState fileSyncStates[],
                                                   size_t length,
                                                   CloudDisk_FailedList **failedLists,
                                                   size_t *failedCount)
{
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    vector<FileSyncState> syncStatesVec;
    for (size_t i = 0; i < length; ++i) {
        FileSyncState state;
        state.path = string(fileSyncStates[i].filePathInfo.value, fileSyncStates[i].filePathInfo.length);
        state.state = static_cast<SyncState>(fileSyncStates[i].syncState);
        syncStatesVec.emplace_back(state);
    }
    vector<FailedList> failedVec;
    int32_t ret = CloudDiskServiceManager::GetInstance().SetFileSyncStates(
        string(syncFolderPath.value, syncFolderPath.length), syncStatesVec, failedVec);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Get sync folder change failed, ret: %{public}d", ret);
        return CovertToErrorCode(ret);
    }
    *failedCount = failedVec.size();
    if (*failedCount == 0) {
        *failedLists = nullptr;
        return CloudDisk_ErrorCode::CLOUD_DISK_ERR_OK;
    }
    *failedLists = new (std::nothrow) CloudDisk_FailedList[*failedCount];
    if (*failedLists == nullptr) {
        LOGE("Failed to allocate memory for failedLists");
        return CloudDisk_ErrorCode::CLOUD_DISK_INTERNAL_ERROR;
    }
    for (size_t index = 0; index < *failedCount; ++index) {
        LOGD("failed list: path=%{public}s, errorReason=%{public}d",
             failedVec[index].path.c_str(), failedVec[index].error);
        (*failedLists)[index].pathInfo.value =
            AllocField(failedVec[index].path.c_str(), failedVec[index].path.length());
        CHECK_NULLPTR_AND_CONTINUE((*failedLists)[index].pathInfo.value);
        (*failedLists)[index].pathInfo.length = failedVec[index].path.length();
        (*failedLists)[index].errorReason = static_cast<CloudDisk_ErrorReason>(failedVec[index].error);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_ERR_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_GetFileSyncStates(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const CloudDisk_PathInfo paths[],
                                                   size_t length,
                                                   CloudDisk_ResultList **resultLists,
                                                   size_t *resultCount)
{
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    vector<string> pathVec;
    for (size_t i = 0; i < length; ++i) {
        if (!IsValidPathInfo(paths[i].value, paths[i].length)) {
            LOGE("Invalid argument, paths[%{public}zu] is nullptr", i);
            return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
        }
        pathVec.emplace_back(string(paths[i].value, paths[i].length));
    }
    vector<ResultList> resultVec;
    int32_t ret = CloudDiskServiceManager::GetInstance().GetFileSyncStates(
        string(syncFolderPath.value, syncFolderPath.length), pathVec, resultVec);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Get file sync state failed, ret: %{public}d", ret);
        return CovertToErrorCode(ret);
    }
    *resultCount = resultVec.size();
    if (*resultCount == 0) {
        *resultLists = nullptr;
        return CloudDisk_ErrorCode::CLOUD_DISK_ERR_OK;
    }
    *resultLists = new (std::nothrow) CloudDisk_ResultList[*resultCount];
    if (*resultLists == nullptr) {
        LOGE("Failed to allocate memory for resultLists");
        return CloudDisk_ErrorCode::CLOUD_DISK_INTERNAL_ERROR;
    }
    for (size_t index = 0; index < *resultCount; ++index) {
        LOGD("result list: path=%{public}s, isSuccess=%{public}d, syncState=%{public}d, errorReason=%{public}d",
             resultVec[index].path.c_str(), resultVec[index].isSuccess, resultVec[index].state,
             resultVec[index].error);
        (*resultLists)[index].pathInfo.value =
            AllocField(resultVec[index].path.c_str(), resultVec[index].path.length());
        (*resultLists)[index].pathInfo.length = resultVec[index].path.length();
        CHECK_NULLPTR_AND_CONTINUE((*resultLists)[index].pathInfo.value);
        (*resultLists)[index].isSuccess = resultVec[index].isSuccess;
        (*resultLists)[index].syncState = static_cast<CloudDisk_SyncState>(resultVec[index].state);
        (*resultLists)[index].errorReason = static_cast<CloudDisk_ErrorReason>(resultVec[index].error);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_ERR_OK;
}
