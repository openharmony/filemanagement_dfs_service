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

#include "data_handler.h"

#include "dfs_error.h"
#include "utils_log.h"
namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
DataHandler::DataHandler(int32_t userId, const string &bundleName, const std::string &table)
    : cloudPrefImpl_(userId, bundleName, table)
{
    cloudPrefImpl_.GetString(START_CURSOR, startCursor_);
    cloudPrefImpl_.GetString(NEXT_CURSOR, nextCursor_);
    cloudPrefImpl_.GetString(NEXT_CURSOR, tempNextCursor_);
    cloudPrefImpl_.GetInt(BATCH_NO, batchNo_);
    cloudPrefImpl_.GetInt(RECORD_SIZE, recordSize_);
}

void DataHandler::GetNextCursor(DriveKit::DKQueryCursor &cursor)
{
    if (tempNextCursor_.empty()) {
        cursor = startCursor_;
        return;
    }
    cursor = tempNextCursor_;
}

void DataHandler::SetTempStartCursor(const DriveKit::DKQueryCursor &cursor)
{
    tempStartCursor_ = cursor;
    cloudPrefImpl_.SetString(TEMP_START_CURSOR, tempStartCursor_);
}

void DataHandler::GetTempStartCursor(DriveKit::DKQueryCursor &cursor)
{
    cursor = tempStartCursor_;
}

void DataHandler::SetTempNextCursor(const DriveKit::DKQueryCursor &cursor, bool isFinish)
{
    LOGI("batchNo_ %{public}d set temp next cursor %{public}s, isFinish %{public}d",
        batchNo_, cursor.c_str(), isFinish);
    tempNextCursor_ = cursor;
    cursorMap_.insert(std::pair<int32_t, DriveKit::DKQueryCursor>(batchNo_, cursor));
    cursorFinishMap_.insert(std::pair<int32_t, bool>(batchNo_, false));
    if (!isFinish) {
        batchNo_ ++;
    } else {
        isFinish_ = true;
    }
}

int32_t DataHandler::GetBatchNo()
{
    return batchNo_;
}

bool DataHandler::IsPullRecords()
{
    return startCursor_.empty();
}

void DataHandler::ClearCursor()
{
    startCursor_.clear();
    nextCursor_.clear();
    tempStartCursor_.clear();
    tempNextCursor_.clear();
    batchNo_ = 0;
    recordSize_ = 0;
    isFinish_ = false;
    cursorMap_.clear();
    cursorFinishMap_.clear();
    cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
    cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
    cloudPrefImpl_.SetInt(BATCH_NO, batchNo_);
    cloudPrefImpl_.Delete(TEMP_START_CURSOR);
    cloudPrefImpl_.Delete(RECORD_SIZE);
}

void DataHandler::FinishPull(const int32_t batchNo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    LOGI("batchNo %{public}d finish pull", batchNo);
    cursorFinishMap_[batchNo] = true;
    if (cursorFinishMap_.begin()->first == batchNo) {
        while (!cursorFinishMap_.empty() && cursorFinishMap_.begin()->second) {
            nextCursor_ = cursorMap_.begin()->second;
            cloudPrefImpl_.SetInt(BATCH_NO, cursorFinishMap_.begin()->first);
            cursorMap_.erase(cursorMap_.begin()->first);
            cursorFinishMap_.erase(cursorFinishMap_.begin()->first);
        }
    }
    LOGD("The next cursor is %{public}s", nextCursor_.c_str());
    cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);

    if (cursorMap_.empty() && isFinish_) {
        if (IsPullRecords()) {
            LOGD("PullRecords finish all");
            startCursor_ = tempStartCursor_;
            tempStartCursor_.clear();
            recordSize_ = 0;
            cloudPrefImpl_.Delete(TEMP_START_CURSOR);
            cloudPrefImpl_.Delete(RECORD_SIZE);
        } else {
            LOGD("PullDatabaseChanged finish all");
            startCursor_ = nextCursor_;
        }
        nextCursor_.clear();
        tempNextCursor_.clear();
        cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
        cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
        cloudPrefImpl_.Delete(BATCH_NO);
        batchNo_ = 0;
        isFinish_ = false;
    }
    LOGD("When batchNo finish pull startCursor %{public}s, nextCursor %{public}s, tempStartCursor %{public}s,",
        startCursor_.c_str(), nextCursor_.c_str(), tempStartCursor_.c_str());
    LOGD("tempNextCursor %{public}s, batchNo_ %{public}d", tempNextCursor_.c_str(), batchNo_);
}

void DataHandler::SetRecordSize(const int32_t recordSize)
{
    recordSize_ = recordSize;
    cloudPrefImpl_.SetInt(RECORD_SIZE, recordSize_);
}

int32_t DataHandler::GetRecordSize()
{
    return recordSize_;
}

int32_t DataHandler::GetFileModifiedRecords(vector<DriveKit::DKRecord> &records)
{
    return E_OK;
}

int32_t DataHandler::OnModifyFdirtyRecords(const map<DriveKit::DKRecordId,
    DriveKit::DKRecordOperResult> &map)
{
    return E_OK;
}

int32_t DataHandler::GetAssetsToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
{
    return E_OK;
}

int32_t DataHandler::GetDownloadAsset(string cloudId,
    vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
{
    return E_OK;
}

int32_t DataHandler::OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset)
{
    return E_OK;
}

int32_t DataHandler::OnDownloadThumb(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap)
{
    return E_OK;
}

int32_t DataHandler::Clean(const int action)
{
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS