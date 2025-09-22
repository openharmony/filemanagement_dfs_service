/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ohclouddiskmanagerndk_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "cloud_fuzzer_helper.h"
#include "oh_cloud_disk_manager.h"

using namespace std;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t MAX_BUFFER_LENGTH = 10000;
constexpr size_t MAX_PATH_LENGTH = 4096;

static void OnChangeDataCallback(
    const CloudDisk_SyncFolderPath syncFolderPath, const CloudDisk_ChangeData changeDatas[], size_t length)
{
    if (syncFolderPath.value != nullptr) {
        delete[] syncFolderPath.value;
    }
    if (length == 0) {
        return;
    }
    for (size_t index = 0; index < length; ++index) {
        delete[] changeDatas[index].fileId.value;
        delete[] changeDatas[index].parentFileId.value;
        delete[] changeDatas[index].relativePathInfo.value;
    }
    delete[] changeDatas;
}

void OHRegisterSyncFolderChangesFuzzTest(const uint8_t *data, size_t size)
{
    OHOS::FuzzData fuzzData(data, size);
    size_t syncLength = fuzzData.GetRemainSize();
    if (syncLength > MAX_PATH_LENGTH) {
        return;
    }
    std::string path = fuzzData.GetStringFromData(syncLength);
    CloudDisk_SyncFolderPath syncFolderPath = {.value = path.data(), .length = path.length()};
    OH_CloudDisk_RegisterSyncFolderChanges(syncFolderPath, &OnChangeDataCallback);
}

void OHUnregisterSyncFolderChangesFuzzTest(const uint8_t *data, size_t size)
{
    OHOS::FuzzData fuzzData(data, size);
    size_t syncLength = fuzzData.GetRemainSize();
    if (syncLength > MAX_PATH_LENGTH) {
        return;
    }
    std::string path = fuzzData.GetStringFromData(syncLength);
    CloudDisk_SyncFolderPath syncFolderPath = {.value = path.data(), .length = path.length()};
    OH_CloudDisk_UnregisterSyncFolderChanges(syncFolderPath);
}

void OHGetSyncFolderChangesFuzzTest(const uint8_t *data, size_t size)
{
    OHOS::FuzzData fuzzData(data, size);
    uint64_t startUsn = fuzzData.GetData<uint64_t>();
    size_t count = fuzzData.GetData<size_t>();
    size_t syncLength = fuzzData.GetRemainSize();
    if (syncLength > MAX_PATH_LENGTH) {
        return;
    }
    std::string path = fuzzData.GetStringFromData(syncLength);
    CloudDisk_SyncFolderPath syncFolderPath = {.value = path.data(), .length = path.length()};
    CloudDisk_ChangesResult *changesResult;
    OH_CloudDisk_GetSyncFolderChanges(syncFolderPath, startUsn, count, &changesResult);
    if (changesResult != nullptr) {
        for (size_t index = 0; index < changesResult->bufferLength; ++index) {
            delete[] changesResult->changeDatas[index].fileId.value;
            delete[] changesResult->changeDatas[index].parentFileId.value;
            delete[] changesResult->changeDatas[index].relativePathInfo.value;
        }
        delete changesResult;
    }
}

void OHSetFileSyncStatesFuzzTest(const uint8_t *data, size_t size)
{
    OHOS::FuzzData fuzzData(data, size);
    size_t bufferLength = fuzzData.GetData<size_t>();
    size_t pathLength = fuzzData.GetData<size_t>();
    size_t syncLength = fuzzData.GetData<size_t>();
    if (bufferLength > MAX_BUFFER_LENGTH || pathLength > MAX_PATH_LENGTH || syncLength > MAX_PATH_LENGTH) {
        return;
    }
    if (bufferLength * (sizeof(CloudDisk_FileSyncState) + pathLength) + syncLength > fuzzData.GetRemainSize()) {
        return;
    }
    vector<string> tmpPaths;
    for (size_t index = 0; index < bufferLength; ++index) {
        tmpPaths.emplace_back(fuzzData.GetStringFromData(pathLength));
    }
    std::vector<CloudDisk_FileSyncState> fileSyncStates(bufferLength);
    for (size_t index = 0; index < bufferLength; ++index) {
        int32_t tmpData = fuzzData.GetData<int32_t>();
        fileSyncStates[index].syncState = static_cast<CloudDisk_SyncState>(
            tmpData % (static_cast<int32_t>(CloudDisk_SyncState::SYNC_CONFLICTED) + 1));
        fileSyncStates[index].filePathInfo.value = tmpPaths[index].data();
        fileSyncStates[index].filePathInfo.length = tmpPaths[index].length();
    }
    std::string path = fuzzData.GetStringFromData(syncLength);
    CloudDisk_SyncFolderPath syncFolderPath = {.value = path.data(), .length = path.length()};

    CloudDisk_FailedList *failedLists;
    size_t failedCount;
    OH_CloudDisk_SetFileSyncStates(syncFolderPath, fileSyncStates.data(), bufferLength, &failedLists, &failedCount);
    if (failedLists != nullptr) {
        for (size_t index = 0; index < failedCount; ++index) {
            delete[] failedLists[index].pathInfo.value;
        }
        delete[] failedLists;
    }
}

void OHGetFileSyncStatesFuzzTest(const uint8_t *data, size_t size)
{
    OHOS::FuzzData fuzzData(data, size);
    size_t bufferLength = fuzzData.GetData<size_t>();
    size_t pathLength = fuzzData.GetData<size_t>();
    size_t syncLength = fuzzData.GetData<size_t>();
    if (bufferLength > MAX_BUFFER_LENGTH || pathLength > MAX_PATH_LENGTH || syncLength > MAX_PATH_LENGTH) {
        return;
    }
    if (bufferLength * (sizeof(CloudDisk_PathInfo) + pathLength) + syncLength > fuzzData.GetRemainSize()) {
        return;
    }
    vector<string> tmpPaths;
    for (size_t index = 0; index < bufferLength; ++index) {
        tmpPaths.emplace_back(fuzzData.GetStringFromData(pathLength));
    }
    std::vector<CloudDisk_PathInfo> paths(bufferLength);
    for (size_t index = 0; index < bufferLength; ++index) {
        paths[index].value = tmpPaths[index].data();
        paths[index].length = tmpPaths[index].length();
    }
    std::string path = fuzzData.GetStringFromData(syncLength);
    CloudDisk_SyncFolderPath syncFolderPath = {.value = path.data(), .length = path.length()};

    CloudDisk_ResultList *resultLists;
    size_t resultCount = 0;
    OH_CloudDisk_GetFileSyncStates(syncFolderPath, paths.data(), bufferLength, &resultLists, &resultCount);
    if (resultLists != nullptr) {
        for (size_t index = 0; index < resultCount; ++index) {
            delete[] resultLists[index].pathInfo.value;
        }
        delete[] resultLists;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= (U32_AT_SIZE << 1)) {
        return 0;
    }

    /* 用例中通过length确定char*长度或数组大小，没有考虑访问越界情况 */
    OHRegisterSyncFolderChangesFuzzTest(data, size);
    OHUnregisterSyncFolderChangesFuzzTest(data, size);
    OHGetSyncFolderChangesFuzzTest(data, size);
    OHSetFileSyncStatesFuzzTest(data, size);
    OHGetFileSyncStatesFuzzTest(data, size);
    return 0;
}
