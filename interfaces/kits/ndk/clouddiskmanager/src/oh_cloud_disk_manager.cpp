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

#include <algorithm>
#include <cstring>
#include <functional>
#include <new>
#include <vector>

#include <securec.h>

#include "cloud_disk_common.h"
#include "cloud_disk_service_callback.h"
#include "cloud_disk_service_callback_table.h"
#include "cloud_disk_service_manager.h"
#ifdef SUPPORT_CLOUD_DISK_SERVICE
#include "cloud_disk_sync_folder_manager.h"
#endif
#include "oh_cloud_disk_utils.h"
#include "utils_log.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::FileManagement;
using namespace OHOS::FileManagement::CloudDiskService;

class CloudDiskServiceCallbackImpl : public CloudDiskServiceCallback {
public:
    using OnChangeDataCallback =
        function<void(const CloudDisk_SyncFolderPath syncFolderPath, const CloudDisk_ChangeData changeDatas[],
                      size_t bufferLength)>;
    explicit CloudDiskServiceCallbackImpl(OnChangeDataCallback callback)
        : callback_(callback) {};
    void OnChangeData(const std::string &syncFolder, const std::vector<ChangeData> &changeData) override;
    ~CloudDiskServiceCallbackImpl() override = default;

private:
    OnChangeDataCallback callback_;
};

class CloudDiskServiceCallbackTableImpl : public CloudDiskServiceCallbackTable {
public:
    using Callback = void (*)(const OH_CloudDisk_CallbackReqHead reqHead, OH_CloudDisk_CallbackContext reqContext);

    explicit CloudDiskServiceCallbackTableImpl(Callback callback) : callback_(callback) {}
    ~CloudDiskServiceCallbackTableImpl() override = default;

    void OnCallback(const CloudDiskCallbackReqHead &reqHead, CloudDiskCallbackContext &reqContext) override;

private:
    void HandleFetchData(const OH_CloudDisk_CallbackReqHead &reqHead, CloudDiskCallbackContext &context);
    void HandleCancelFetchData(const OH_CloudDisk_CallbackReqHead &reqHead, CloudDiskCallbackContext &context);
    void HandleFetchRangeData(const OH_CloudDisk_CallbackReqHead &reqHead, CloudDiskCallbackContext &context);
    void HandleDehydrate(const OH_CloudDisk_CallbackReqHead &reqHead, CloudDiskCallbackContext &context);
    Callback callback_{nullptr};
};

namespace {
constexpr size_t PLACEHOLDER_CUSTOM_INFO_MAX_SIZE = 4096;

CloudDisk_PathInfo ToPublicPathInfo(const CloudDiskPathInfo &pathInfo)
{
    return {pathInfo.value, pathInfo.length};
}

OH_CloudDisk_DataBuf ToPublicDataBuf(const CloudDiskDataBuf &dataBuf)
{
    return {dataBuf.data, dataBuf.dataSize};
}

bool ConvertPlaceholderCustomInfo(const OH_CloudDisk_PlaceholderCustomInfo *customInfo,
                                  PlaceholderCustomInfo &innerCustomInfo)
{
    innerCustomInfo.data.clear();
    if (customInfo == nullptr || customInfo->data == nullptr || customInfo->dataLength == 0) {
        return true;
    }
    if (customInfo->dataLength > PLACEHOLDER_CUSTOM_INFO_MAX_SIZE) {
        LOGE("Placeholder custom info is too large, size: %{public}zu", customInfo->dataLength);
        return false;
    }
    innerCustomInfo.data.assign(customInfo->data, customInfo->data + customInfo->dataLength);
    return true;
}
} // namespace

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
    CloudDisk_ChangeData *changeDatas = new (std::nothrow) CloudDisk_ChangeData[changeData.size()];
    if (changeDatas == nullptr) {
        LOGE("Failed to allocate memory for changeDatas");
        delete[] syncFolderPath.value;
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
        changeDatas[length++] = data;
    }
    callback_(syncFolderPath, changeDatas, length);
}

void CloudDiskServiceCallbackTableImpl::OnCallback(const CloudDiskCallbackReqHead &reqHead,
                                                   CloudDiskCallbackContext &reqContext)
{
    if (callback_ == nullptr) {
        LOGE("Callback table function is nullptr");
        return;
    }

    OH_CloudDisk_CallbackReqHead publicReqHead{
        ToPublicPathInfo(reqHead.syncFolderPath),
        static_cast<OH_CloudDisk_CallbackType>(reqHead.callbackType),
        ToPublicDataBuf(reqHead.reqKey),
    };
    switch (reqHead.callbackType) {
        case CloudDiskCallbackType::FETCH_DATA:
            HandleFetchData(publicReqHead, reqContext);
            break;
        case CloudDiskCallbackType::CANCEL_FETCH_DATA:
            HandleCancelFetchData(publicReqHead, reqContext);
            break;
        case CloudDiskCallbackType::FETCH_RANGE_DATA:
            HandleFetchRangeData(publicReqHead, reqContext);
            break;
        case CloudDiskCallbackType::DEHYDRATE:
            HandleDehydrate(publicReqHead, reqContext);
            break;
        default:
            LOGE("Invalid callback table type");
            break;
    }
}

void CloudDiskServiceCallbackTableImpl::HandleFetchData(const OH_CloudDisk_CallbackReqHead &reqHead,
    CloudDiskCallbackContext &context)
{
    if (context.fetchData == nullptr) {
        LOGE("Fetch data callback context is nullptr");
        return;
    }
    OH_CloudDisk_FetchDataRequest request{ToPublicPathInfo(context.fetchData->filePath),
        static_cast<OH_CloudDisk_HydratePriority>(context.fetchData->priority)};
    OH_CloudDisk_CallbackContext publicContext{};
    publicContext.fetchData = &request;
    callback_(reqHead, publicContext);
}

void CloudDiskServiceCallbackTableImpl::HandleCancelFetchData(const OH_CloudDisk_CallbackReqHead &reqHead,
    CloudDiskCallbackContext &context)
{
    if (context.cancelFetchData == nullptr) {
        LOGE("Cancel fetch data callback context is nullptr");
        return;
    }
    CloudDisk_PathInfo pathInfo = ToPublicPathInfo(*context.cancelFetchData);
    OH_CloudDisk_CallbackContext publicContext{};
    publicContext.cancelFetchData = &pathInfo;
    callback_(reqHead, publicContext);
}

void CloudDiskServiceCallbackTableImpl::HandleFetchRangeData(const OH_CloudDisk_CallbackReqHead &reqHead,
    CloudDiskCallbackContext &context)
{
    if (context.fetchRangeData == nullptr) {
        LOGE("Fetch range data callback context is nullptr");
        return;
    }
    OH_CloudDisk_RangeInfo rangeInfo{ToPublicPathInfo(context.fetchRangeData->filePath),
        context.fetchRangeData->offset, context.fetchRangeData->size, ToPublicDataBuf(context.fetchRangeData->data)};
    uint8_t *rangeData = rangeInfo.data.data;
    uint64_t rangeDataCapacity = rangeInfo.data.dataSize;
    OH_CloudDisk_CallbackContext publicContext{};
    publicContext.fetchRangeData = &rangeInfo;
    callback_(reqHead, publicContext);
    if (rangeInfo.data.data != rangeData) {
        LOGE("Callback changed the range data buffer pointer");
        context.fetchRangeData->data.dataSize = 0;
        return;
    }
    context.fetchRangeData->data.dataSize = std::min(rangeInfo.data.dataSize, rangeDataCapacity);
}

void CloudDiskServiceCallbackTableImpl::HandleDehydrate(const OH_CloudDisk_CallbackReqHead &reqHead,
    CloudDiskCallbackContext &context)
{
    if (context.dehydrateData == nullptr) {
        LOGE("Dehydrate callback context is nullptr");
        return;
    }
    OH_CloudDisk_DehydrateInfo dehydrateInfo{ToPublicPathInfo(context.dehydrateData->filePath), false};
    OH_CloudDisk_CallbackContext publicContext{};
    publicContext.dehydrateData = &dehydrateInfo;
    callback_(reqHead, publicContext);
    context.dehydrateData->allow = dehydrateInfo.allow;
}

CloudDisk_ErrorCode OH_CloudDisk_RegisterSyncFolderChanges(const CloudDisk_SyncFolderPath syncFolderPath,
    void (*callback)(const CloudDisk_SyncFolderPath syncFolderPath,
                     const CloudDisk_ChangeData changeDatas[],
                     size_t bufferLength))
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
        const CloudDisk_ChangeData changeDatas[], size_t bufferLength) {
        callback(syncFolderPath, changeDatas, bufferLength);
    };
    shared_ptr<CloudDiskServiceCallback> callbackImpl = make_shared<CloudDiskServiceCallbackImpl>(callbackInner);
    int32_t ret = CloudDiskServiceManager::GetInstance().RegisterSyncFolderChanges(
        string(syncFolderPath.value, syncFolderPath.length), callbackImpl);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Register sync folder change failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
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
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_GetSyncFolderChanges(const CloudDisk_SyncFolderPath syncFolderPath,
                                                      uint64_t startUsn,
                                                      size_t count,
                                                      CloudDisk_ChangesResult **changesResult)
{
    if (changesResult == nullptr) {
        LOGE("Invalid argument, changesResult is nullptr");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    ChangesResult changesRes;
    auto ret = CloudDiskServiceManager::GetInstance().GetSyncFolderChanges(
        string(syncFolderPath.value, syncFolderPath.length), count, startUsn, changesRes);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Get sync folder change failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    *changesResult = (CloudDisk_ChangesResult *)malloc(sizeof(CloudDisk_ChangesResult) +
                                                       sizeof(CloudDisk_ChangeData) * changesRes.changesData.size());
    if (*changesResult == nullptr) {
        LOGE("Failed to allocate memory for changesResult");
        return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
    }
    (*changesResult)->bufferLength = 0;
    (*changesResult)->nextUsn = changesRes.nextUsn;
    (*changesResult)->isEof = changesRes.isEof;
    for (auto &item : changesRes.changesData) {
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
        (*changesResult)->changeDatas[(*changesResult)->bufferLength++] = data;
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_SetFileSyncStates(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const CloudDisk_FileSyncState fileSyncStates[],
                                                   size_t bufferLength,
                                                   CloudDisk_FailedList **failedLists,
                                                   size_t *failedCount)
{
    if (fileSyncStates == nullptr || failedLists == nullptr || failedCount == nullptr) {
        LOGE("Invalid argument, contains nullptr");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    vector<FileSyncState> syncStatesVec;
    for (size_t i = 0; i < bufferLength; ++i) {
        if (fileSyncStates[i].filePathInfo.value == nullptr) {
            LOGE("Invalid argument, contains nullptr");
            return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
        }
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
        return ConvertToErrorCode(ret);
    }
    *failedCount = failedVec.size();
    if (*failedCount == 0) {
        *failedLists = nullptr;
        return CloudDisk_ErrorCode::CLOUD_DISK_OK;
    }
    *failedLists = new (std::nothrow) CloudDisk_FailedList[*failedCount];
    if (*failedLists == nullptr) {
        LOGE("Failed to allocate memory for failedLists");
        return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
    }
    for (size_t index = 0; index < *failedCount; ++index) {
        (*failedLists)[index].pathInfo.value =
            AllocField(failedVec[index].path.c_str(), failedVec[index].path.length());
        CHECK_NULLPTR_AND_CONTINUE((*failedLists)[index].pathInfo.value);
        (*failedLists)[index].pathInfo.length = failedVec[index].path.length();
        (*failedLists)[index].errorReason = static_cast<CloudDisk_ErrorReason>(failedVec[index].error);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_GetFileSyncStates(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const CloudDisk_PathInfo paths[],
                                                   size_t bufferLength,
                                                   CloudDisk_ResultList **resultLists,
                                                   size_t *resultCount)
{
    if (paths == nullptr || resultLists == nullptr || resultCount == nullptr) {
        LOGE("Invalid argument, contains nullptr");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    vector<string> pathVec;
    for (size_t i = 0; i < bufferLength; ++i) {
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
        return ConvertToErrorCode(ret);
    }
    *resultCount = resultVec.size();
    if (*resultCount == 0) {
        *resultLists = nullptr;
        return CloudDisk_ErrorCode::CLOUD_DISK_OK;
    }
    *resultLists = new (std::nothrow) CloudDisk_ResultList[*resultCount];
    if (*resultLists == nullptr) {
        LOGE("Failed to allocate memory for resultLists");
        return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
    }
    for (size_t index = 0; index < *resultCount; ++index) {
        (*resultLists)[index].pathInfo.value =
            AllocField(resultVec[index].path.c_str(), resultVec[index].path.length());
        (*resultLists)[index].pathInfo.length = resultVec[index].path.length();
        CHECK_NULLPTR_AND_CONTINUE((*resultLists)[index].pathInfo.value);
        (*resultLists)[index].isSuccess = resultVec[index].isSuccess;
        (*resultLists)[index].syncState = static_cast<CloudDisk_SyncState>(resultVec[index].state);
        (*resultLists)[index].errorReason = static_cast<CloudDisk_ErrorReason>(resultVec[index].error);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_CreatePlaceholder(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const CloudDisk_PathInfo relativePathInfo,
                                                   const OH_CloudDisk_PlaceholderInfo placeholderInfo,
                                                   const OH_CloudDisk_PlaceholderCustomInfo *customInfo)
{
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length) ||
        !IsValidPathInfo(relativePathInfo.value, relativePathInfo.length)) {
        LOGE("CreatePlaceholderFile branch=invalid_arg");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    PlaceholderInfo innerInfo;
    innerInfo.logicalSize = placeholderInfo.logicalSize;
    innerInfo.atimeMs = placeholderInfo.atimeMs;
    innerInfo.mtimeMs = placeholderInfo.mtimeMs;
    PlaceholderCustomInfo innerCustomInfo;
    if (!ConvertPlaceholderCustomInfo(customInfo, innerCustomInfo)) {
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    int32_t ret = CloudDiskServiceManager::GetInstance().CreatePlaceholderFile(
        string(syncFolderPath.value, syncFolderPath.length), string(relativePathInfo.value, relativePathInfo.length),
        innerInfo, innerCustomInfo);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("CreatePlaceholderFile branch=service_failed ret=%{public}d", ret);
        return ConvertToErrorCode(ret);
    }

    LOGI("CreatePlaceholderFile branch=success");
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_IsPlaceholderFile(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const CloudDisk_PathInfo path,
                                                   bool *isPlaceholder)
{
    if (isPlaceholder == nullptr) {
        LOGE("IsPlaceholderFile branch=invalid_arg_result_null");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    *isPlaceholder = false;
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("IsPlaceholderFile branch=invalid_arg_sync_folder");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    if (!IsValidPathInfo(path.value, path.length)) {
        LOGE("IsPlaceholderFile branch=invalid_arg_path");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    std::string syncFolder(syncFolderPath.value, syncFolderPath.length);
    std::string filePath(path.value, path.length);

    int32_t ret = CloudDiskServiceManager::GetInstance().IsPlaceholderFile(syncFolder, filePath, *isPlaceholder);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("IsPlaceholderFile branch=service_failed ret=%{public}d", ret);
        return ConvertToErrorCode(ret);
    }

    LOGI("IsPlaceholderFile branch=success isPlaceholder=%{public}d", *isPlaceholder);
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_GetPlaceholderState(const CloudDisk_SyncFolderPath syncFolderPath,
                                                     const CloudDisk_PathInfo relativePathInfo,
                                                     OH_CloudDisk_PlaceholderState *state)
{
    if (state == nullptr) {
        LOGE("GetPlaceholderState branch=invalid_arg_state_null");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    *state = OH_CLOUD_DISK_PLACEHOLDER_STATE_NONE;
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length) ||
        !IsValidPathInfo(relativePathInfo.value, relativePathInfo.length)) {
        LOGE("GetPlaceholderState branch=invalid_path_info");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    std::string syncFolder(syncFolderPath.value, syncFolderPath.length);
    std::string relativePath(relativePathInfo.value, relativePathInfo.length);
    int32_t innerState = OH_CLOUD_DISK_PLACEHOLDER_STATE_NONE;
    int32_t ret = CloudDiskServiceManager::GetInstance().GetPlaceholderState(syncFolder, relativePath, innerState);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("GetPlaceholderState branch=service_failed ret=%{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    *state = static_cast<OH_CloudDisk_PlaceholderState>(innerState);
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}

CloudDisk_ErrorCode OH_CloudDisk_RegisterSyncFolder(const CloudDisk_SyncFolder *syncFolder)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolder == nullptr) {
        LOGE("Invalid argument, syncFolder is nullptr");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    if (!IsValidPathInfo(syncFolder->path.value, syncFolder->path.length)) {
        LOGE("Invalid argument, path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    OHOS::FileManagement::SyncFolder folder;
    folder.path_ = std::string(syncFolder->path.value, syncFolder->path.length);
    if (syncFolder->displayNameInfo.customAlias != nullptr && syncFolder->displayNameInfo.customAliasLength != 0) {
        if (!IsValidPathInfo(syncFolder->displayNameInfo.customAlias, syncFolder->displayNameInfo.customAliasLength)) {
            LOGE("Invalid argument, displayName is invalid");
            return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
        }
        folder.displayName_ =
            std::string(syncFolder->displayNameInfo.customAlias, syncFolder->displayNameInfo.customAliasLength);
    }
    if (syncFolder->displayNameInfo.displayNameResId != 0) {
        folder.displayNameResId_ = syncFolder->displayNameInfo.displayNameResId;
    }
    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().Register(folder);
    LOGI("Register sync folder, ret: %{public}d", ret);
    return ConvertToErrorCode(ret);
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_UnregisterSyncFolder(const CloudDisk_SyncFolderPath syncFolderPath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().Unregister(
        string(syncFolderPath.value, syncFolderPath.length));
    LOGI("Unregister sync folder, ret: %{public}d", ret);
    return ConvertToErrorCode(ret);
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_ActiveSyncFolder(const CloudDisk_SyncFolderPath syncFolderPath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().Active(
        string(syncFolderPath.value, syncFolderPath.length));
    LOGI("Active sync folder, ret: %{public}d", ret);
    return ConvertToErrorCode(ret);
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}
CloudDisk_ErrorCode OH_CloudDisk_DeactiveSyncFolder(const CloudDisk_SyncFolderPath syncFolderPath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().Deactive(
        string(syncFolderPath.value, syncFolderPath.length));
    LOGI("Deactive sync folder, ret: %{public}d", ret);
    return ConvertToErrorCode(ret);
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

#ifdef SUPPORT_CLOUD_DISK_SERVICE
CloudDisk_ErrorCode OH_CloudDisk_GetSyncFolders(CloudDisk_SyncFolder **syncFolders, size_t *count)
{
    if (syncFolders == nullptr || count == nullptr) {
        LOGE("Invalid argument, syncFolders or count is nullptr");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    std::vector<OHOS::FileManagement::SyncFolder> folderVec;
    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().GetSyncFolders(folderVec);
    if (ret != 0) {
        LOGE("Get sync folders failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    *syncFolders = new (std::nothrow) CloudDisk_SyncFolder[folderVec.size()];
    if (*syncFolders == nullptr) {
        LOGE("Memory allocation failed for syncFolders");
        return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
    }
    for (size_t i = 0; i < folderVec.size(); ++i) {
        (*syncFolders)[i].path.value = AllocField(folderVec[i].path_.c_str(), folderVec[i].path_.length());
        if ((*syncFolders)[i].path.value == nullptr) {
            for (size_t j = 0; j < i; ++j) {
                delete[] (*syncFolders)[j].path.value;
            }
            delete[] (*syncFolders);
            *syncFolders = nullptr;
            LOGE("Memory allocation failed for path");
            return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
        }
        (*syncFolders)[i].path.length = folderVec[i].path_.length();
        (*syncFolders)[i].state = static_cast<CloudDisk_SyncFolderState>(folderVec[i].state_);
        (*syncFolders)[i].displayNameInfo.displayNameResId = folderVec[i].displayNameResId_;
        (*syncFolders)[i].displayNameInfo.customAlias =
            AllocField(folderVec[i].displayName_.c_str(), folderVec[i].displayName_.length());
        if ((*syncFolders)[i].displayNameInfo.customAlias == nullptr) {
            delete[] (*syncFolders)[i].path.value;
            for (size_t j = 0; j < i; ++j) {
                delete[] (*syncFolders)[j].path.value;
                delete[] (*syncFolders)[j].displayNameInfo.customAlias;
            }
            delete[] (*syncFolders);
            *syncFolders = nullptr;
            LOGE("Memory allocation failed for displayName");
            return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
        }
        (*syncFolders)[i].displayNameInfo.customAliasLength = folderVec[i].displayName_.length();
    }
    *count = folderVec.size();
    LOGI("Get sync folders success, count: %{public}zu", *count);
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
}
#else
CloudDisk_ErrorCode OH_CloudDisk_GetSyncFolders(CloudDisk_SyncFolder **syncFolders, size_t *count)
{
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
}
#endif

CloudDisk_ErrorCode OH_CloudDisk_UpdateCustomAlias(const CloudDisk_SyncFolderPath syncFolderPath,
                                                   const char *customAlias,
                                                   size_t customAliasLength)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    if (customAlias != nullptr && strnlen(customAlias, customAliasLength + 1) != customAliasLength) {
        LOGE("Invalid argument, customAlias is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    std::string customAliasStr = "";
    if (customAlias != nullptr) {
        customAliasStr = string(customAlias, customAliasLength);
    }
    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().UpdateDisplayName(
        string(syncFolderPath.value, syncFolderPath.length), customAliasStr);
    LOGI("Update display name, ret: %{public}d", ret);
    return ConvertToErrorCode(ret);
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_ConvertPlaceholderToFile(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    if (!IsValidPathInfo(relativePathInfo.value, relativePathInfo.length)) {
        LOGE("Invalid argument, pathInfo is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    string syncFolder(syncFolderPath.value, syncFolderPath.length);
    string relativePath(relativePathInfo.value, relativePathInfo.length);
    int32_t ret = OHOS::FileManagement::CloudDiskService::CloudDiskServiceManager::GetInstance()
        .ConvertPlaceholderToFile(syncFolder, relativePath);
    if (ret != OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_OK) {
        LOGE("Convert placeholder to file failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_MarkFileAsPlaceholder(const CloudDisk_SyncFolderPath syncFolderPath,
                                                       const CloudDisk_PathInfo relativePathInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length) ||
        !IsValidPathInfo(relativePathInfo.value, relativePathInfo.length)) {
        LOGE("Invalid placeholder mark arguments");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    std::string syncFolder(syncFolderPath.value, syncFolderPath.length);
    std::string relativePath(relativePathInfo.value, relativePathInfo.length);
    int32_t ret = CloudDiskServiceManager::GetInstance().MarkFileAsPlaceholder(syncFolder, relativePath);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Mark file as placeholder failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_UnmarkPlaceholderFile(const CloudDisk_SyncFolderPath syncFolderPath,
                                                       const CloudDisk_PathInfo relativePathInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length) ||
        !IsValidPathInfo(relativePathInfo.value, relativePathInfo.length)) {
        LOGE("Invalid placeholder unmark arguments");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    std::string syncFolder(syncFolderPath.value, syncFolderPath.length);
    std::string relativePath(relativePathInfo.value, relativePathInfo.length);
    int32_t ret = CloudDiskServiceManager::GetInstance().UnmarkPlaceholderFile(syncFolder, relativePath);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Unmark placeholder file failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_HydratePlaceholder(const CloudDisk_SyncFolderPath *syncFolderPath,
                                                    const CloudDisk_PathInfo *filePath,
                                                    OH_CloudDisk_CallbackType type,
                                                    OH_CloudDisk_HydratePriority priority)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolderPath == nullptr || filePath == nullptr ||
        !IsValidPathInfo(syncFolderPath->value, syncFolderPath->length) ||
        !IsValidPathInfo(filePath->value, filePath->length) ||
        (type != CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA && type != CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA) ||
        priority < ::CLOUD_DISK_HYDRATE_PRIORITY_LOW || priority > ::CLOUD_DISK_HYDRATE_PRIORITY_HIGH) {
        LOGE("Invalid placeholder hydrate arguments");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    std::string syncFolder(syncFolderPath->value, syncFolderPath->length);
    std::string relativePath(filePath->value, filePath->length);
    int32_t ret = E_INVALID_ARG;
    if (type == CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA) {
        ret = CloudDiskServiceManager::GetInstance().StartHydration(
            syncFolder, relativePath, static_cast<CloudDiskHydratePriority>(priority));
    } else {
        ret = CloudDiskServiceManager::GetInstance().CancelHydration(syncFolder, relativePath);
    }
    return ConvertToErrorCode(ret);
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

#ifdef SUPPORT_CLOUD_DISK_SERVICE
static bool BuildExecuteFetchData(const OH_CloudDisk_FetchData *fetchData, CallbackExecuteRequest &request)
{
    if (fetchData == nullptr || fetchData->size != fetchData->data.dataSize ||
        fetchData->size > MAX_EXECUTE_DATA_SIZE || fetchData->offset > fetchData->totalSize ||
        fetchData->size > fetchData->totalSize - fetchData->offset ||
        (fetchData->size != 0 && fetchData->data.data == nullptr)) {
        return false;
    }
    request.offset = fetchData->offset;
    request.size = fetchData->size;
    request.totalSize = fetchData->totalSize;
    request.isComplete = fetchData->isComplete;
    if (fetchData->size != 0) {
        request.data.assign(fetchData->data.data, fetchData->data.data + fetchData->size);
    }
    return true;
}

static bool BuildCallbackExecuteRequest(const OH_CloudDisk_CallbackReqHead &reqHead,
    const OH_CloudDisk_CallbackContext &reqContext, const OH_CloudDisk_CallbackResponse &rsp,
    CallbackExecuteRequest &request)
{
    if (reqHead.reqKey.data == nullptr || reqHead.reqKey.dataSize == 0 ||
        reqHead.reqKey.dataSize > MAX_CALLBACK_REQUEST_KEY_SIZE ||
        !IsValidPathInfo(reqHead.syncFolderPath.value, reqHead.syncFolderPath.length)) {
        return false;
    }
    const CloudDisk_PathInfo *filePath = nullptr;
    if (reqHead.callbackType == CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA) {
        if (reqContext.fetchData == nullptr || !BuildExecuteFetchData(rsp.fetchData, request)) {
            return false;
        }
        filePath = &reqContext.fetchData->filePath;
    } else if (reqHead.callbackType == CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA) {
        filePath = reqContext.cancelFetchData;
    } else {
        return false;
    }
    if (filePath == nullptr || !IsValidPathInfo(filePath->value, filePath->length)) {
        return false;
    }
    request.reqKey.assign(reqHead.reqKey.data, reqHead.reqKey.data + reqHead.reqKey.dataSize);
    request.syncFolder.assign(reqHead.syncFolderPath.value, reqHead.syncFolderPath.length);
    request.filePath.assign(filePath->value, filePath->length);
    request.callbackType = static_cast<int32_t>(reqHead.callbackType);
    return true;
}
#endif

CloudDisk_ErrorCode OH_CloudDisk_Execute(const OH_CloudDisk_CallbackReqHead reqHead,
                                         OH_CloudDisk_CallbackContext reqContext,
                                         OH_CloudDisk_CallbackResponse rsp)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    CallbackExecuteRequest request;
    if (!BuildCallbackExecuteRequest(reqHead, reqContext, rsp, request)) {
        LOGE("Invalid placeholder execute arguments");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    int32_t ret = CloudDiskServiceManager::GetInstance().Execute(request);
    return ConvertToErrorCode(ret);
#else
    (void)reqContext;
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_DehydrateFile(const CloudDisk_SyncFolderPath *syncFolderPath,
                                               const CloudDisk_PathInfo *filePath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolderPath == nullptr || filePath == nullptr ||
        !IsValidPathInfo(syncFolderPath->value, syncFolderPath->length) ||
        !IsValidPathInfo(filePath->value, filePath->length)) {
        LOGE("Invalid placeholder dehydrate arguments");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    std::string syncFolder(syncFolderPath->value, syncFolderPath->length);
    std::string relativePath(filePath->value, filePath->length);
    int32_t ret = CloudDiskServiceManager::GetInstance().DehydrateFile(syncFolder, relativePath);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Dehydrate placeholder file failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_UpdatePlaceholder(
    const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo,
    const OH_CloudDisk_PlaceholderInfo placeholderInfo,
    const OH_CloudDisk_PlaceholderCustomInfo *customInfo)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid argument, syncFolder path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    if (!IsValidPathInfo(relativePathInfo.value, relativePathInfo.length)) {
        LOGE("Invalid argument, relativePathInfo is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    string syncFolder(syncFolderPath.value, syncFolderPath.length);
    string relativePath(relativePathInfo.value, relativePathInfo.length);
    OHOS::FileManagement::CloudDiskService::PlaceholderInfo metaData;
    metaData.logicalSize = placeholderInfo.logicalSize;
    metaData.mtimeMs = placeholderInfo.mtimeMs;
    metaData.atimeMs = placeholderInfo.atimeMs;
    PlaceholderCustomInfo innerCustomInfo;
    if (!ConvertPlaceholderCustomInfo(customInfo, innerCustomInfo)) {
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    int32_t ret = OHOS::FileManagement::CloudDiskService::CloudDiskServiceManager::GetInstance()
        .UpdatePlaceholder(syncFolder, relativePath, metaData, innerCustomInfo);
    if (ret != OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_OK) {
        LOGE("Update placeholder to file failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_RegisterSyncFolderEx(const OH_CloudDisk_SyncFolderEx *syncFolder)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolder == nullptr) {
        LOGE("Invalid argument, syncFolder is nullptr");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    if (syncFolder->version < OH_CLOUD_DISK_SYNC_FOLDER_EX_VERSION_1) {
        LOGE("Invalid argument, version %{public}u is less than minimum %{public}u",
             syncFolder->version, OH_CLOUD_DISK_SYNC_FOLDER_EX_VERSION_1);
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    if (!IsValidPathInfo(syncFolder->path.value, syncFolder->path.length)) {
        LOGE("Invalid argument, path is invalid");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    OHOS::FileManagement::SyncFolder folder;
    folder.path_ = std::string(syncFolder->path.value, syncFolder->path.length);
    if (syncFolder->displayNameInfo.customAlias != nullptr && syncFolder->displayNameInfo.customAliasLength != 0) {
        if (!IsValidPathInfo(syncFolder->displayNameInfo.customAlias, syncFolder->displayNameInfo.customAliasLength)) {
            LOGE("Invalid argument, displayName is invalid");
            return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
        }
        folder.displayName_ =
            std::string(syncFolder->displayNameInfo.customAlias, syncFolder->displayNameInfo.customAliasLength);
    }
    if (syncFolder->displayNameInfo.displayNameResId != 0) {
        folder.displayNameResId_ = syncFolder->displayNameInfo.displayNameResId;
    }
    folder.isSupportPlaceHolder_ = syncFolder->isSupportPlaceHolder;
    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().Register(folder);
    LOGI("Register sync folder ex, ret: %{public}d", ret);
    return ConvertToErrorCode(ret);
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}
 
#ifdef SUPPORT_CLOUD_DISK_SERVICE
static void ReleaseSyncFoldersEx(OH_CloudDisk_SyncFolderEx *folders, size_t count)
{
    for (size_t j = 0; j < count; ++j) {
        delete[] folders[j].path.value;
        delete[] folders[j].displayNameInfo.customAlias;
    }
    delete[] folders;
}
 
static bool FillSyncFolderEx(OH_CloudDisk_SyncFolderEx &folder, const OHOS::FileManagement::SyncFolder &src)
{
    folder.version = OH_CLOUD_DISK_SYNC_FOLDER_EX_VERSION_1;
    folder.path.value = AllocField(src.path_.c_str(), src.path_.length());
    if (folder.path.value == nullptr) {
        LOGE("folder path value alloc failed.");
        return false;
    }
    folder.path.length = src.path_.length();
    folder.state = static_cast<CloudDisk_SyncFolderState>(src.state_);
    folder.displayNameInfo.displayNameResId = src.displayNameResId_;
    folder.displayNameInfo.customAlias = AllocField(src.displayName_.c_str(), src.displayName_.length());
    if (folder.displayNameInfo.customAlias == nullptr) {
        LOGE("folder displayNameInfo customAlias alloc failed.");
        delete[] folder.path.value;
        folder.path.value = nullptr;
        return false;
    }
    folder.displayNameInfo.customAliasLength = src.displayName_.length();
    folder.isSupportPlaceHolder = src.isSupportPlaceHolder_;
    return true;
}
#endif
 
CloudDisk_ErrorCode OH_CloudDisk_GetSyncFoldersEx(OH_CloudDisk_SyncFolderEx **syncFolders, size_t *count)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (syncFolders == nullptr || count == nullptr) {
        LOGE("Invalid argument, syncFolders or count is nullptr");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    std::vector<OHOS::FileManagement::SyncFolder> folderVec;
    int32_t ret = OHOS::FileManagement::CloudDiskSyncFolderManager::GetInstance().GetSyncFolders(folderVec);
    if (ret != 0) {
        LOGE("Get sync folders ex failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    *syncFolders = new (std::nothrow) OH_CloudDisk_SyncFolderEx[folderVec.size()]();
    if (*syncFolders == nullptr) {
        LOGE("Memory allocation failed for syncFolders");
        return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
    }
    size_t i = 0;
    for (; i < folderVec.size(); ++i) {
        if (!FillSyncFolderEx((*syncFolders)[i], folderVec[i])) {
            ReleaseSyncFoldersEx(*syncFolders, i);
            *syncFolders = nullptr;
            return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
        }
    }
    *count = folderVec.size();
    LOGI("Get sync folders ex success, count: %{public}zu", *count);
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_GetPlaceholderCustomInfo(const CloudDisk_SyncFolderPath syncFolderPath,
    const CloudDisk_PathInfo relativePathInfo, uint8_t *dataBuf, size_t *inOutDataLength)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (inOutDataLength == nullptr || !IsValidPathInfo(syncFolderPath.value, syncFolderPath.length) ||
        !IsValidPathInfo(relativePathInfo.value, relativePathInfo.length)) {
        LOGE("Invalid placeholder custom info query arguments");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    size_t capacity = *inOutDataLength;
    *inOutDataLength = 0;
    if (dataBuf == nullptr && capacity != 0) {
        LOGE("Custom info output buffer is nullptr with non-zero capacity");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }

    PlaceholderCustomInfo customInfo;
    std::string syncFolder(syncFolderPath.value, syncFolderPath.length);
    std::string relativePath(relativePathInfo.value, relativePathInfo.length);
    int32_t ret = CloudDiskServiceManager::GetInstance().GetPlaceholderCustomInfo(
        syncFolder, relativePath, customInfo);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Get placeholder custom info failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }

    size_t actualLength = customInfo.data.size();
    *inOutDataLength = actualLength;
    if (capacity < actualLength || (actualLength != 0 && dataBuf == nullptr)) {
        LOGE("Custom info output buffer is too small, capacity: %{public}zu, actual: %{public}zu",
             capacity, actualLength);
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    if (actualLength != 0 && memcpy_s(dataBuf, capacity, customInfo.data.data(), actualLength) != EOK) {
        LOGE("Failed to copy placeholder custom info");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    if (inOutDataLength != nullptr) {
        *inOutDataLength = 0;
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_RegisterCallbackTable(const CloudDisk_SyncFolderPath syncFolderPath,
                                                       void (*callback)(const OH_CloudDisk_CallbackReqHead reqHead,
                                                                        OH_CloudDisk_CallbackContext reqContext))
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length) || callback == nullptr) {
        LOGE("Invalid callback table registration arguments");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    auto callbackTable = make_shared<CloudDiskServiceCallbackTableImpl>(callback);
    int32_t ret = CloudDiskServiceManager::GetInstance().RegisterCallbackTable(
        string(syncFolderPath.value, syncFolderPath.length), callbackTable);
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Register callback table failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}

CloudDisk_ErrorCode OH_CloudDisk_UnregisterCallbackTable(const CloudDisk_SyncFolderPath syncFolderPath)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    if (!IsValidPathInfo(syncFolderPath.value, syncFolderPath.length)) {
        LOGE("Invalid callback table unregistration arguments");
        return CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG;
    }
    int32_t ret = CloudDiskServiceManager::GetInstance().UnregisterCallbackTable(
        string(syncFolderPath.value, syncFolderPath.length));
    if (ret != CloudDiskServiceErrCode::E_OK) {
        LOGE("Unregister callback table failed, ret: %{public}d", ret);
        return ConvertToErrorCode(ret);
    }
    return CloudDisk_ErrorCode::CLOUD_DISK_OK;
#else
    return CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED;
#endif
}
