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

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
DataHandler::DataHandler(int32_t userId, const string &bundleName, const std::string &table)
    : cloudPrefImpl_(userId, bundleName, table)
{
    cloudPrefImpl_.GetString(START_CURSOR, startCursor_);
    cloudPrefImpl_.GetString(NEXT_CURSOR, nextCursor_);
}

void DataHandler::SetNextCursor(const DriveKit::DKQueryCursor &cursor)
{
    nextCursor_ = cursor;
    cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
}

void DataHandler::GetNextCursor(DriveKit::DKQueryCursor &cursor)
{
    if (nextCursor_.empty()) {
        cursor = startCursor_;
        return;
    }
    cursor = nextCursor_;
}

void DataHandler::SetTempStartCursor(const DriveKit::DKQueryCursor &cursor)
{
    tempStartCursor_ = cursor;
    cloudPrefImpl_.SetString(TEMP_START_CURSOR, nextCursor_);
}

void DataHandler::GetTempStartCursor(DriveKit::DKQueryCursor &cursor)
{
    cursor = tempStartCursor_;
}

bool DataHandler::IsPullRecords()
{
    return startCursor_.empty();
}

void DataHandler::ClearCursor()
{
    startCursor_.clear();
    nextCursor_.clear();
    cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
    cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
    if (!tempStartCursor_.empty()) {
        tempStartCursor_.clear();
        cloudPrefImpl_.Delete(TEMP_START_CURSOR);
    }
}

void DataHandler::GetPullCount(int32_t &totalPullCount, int32_t &downloadThumbLimit)
{
    totalPullCount = totalPullCount_;
    downloadThumbLimit = downloadThumbLimit_;
}

void DataHandler::SetTotalPullCount(const int32_t totalPullCount)
{
    totalPullCount_ = totalPullCount;
    cloudPrefImpl_.SetInt(TOTAL_PULL_COUNT, totalPullCount_);
}

void DataHandler::FinishPull(const DriveKit::DKQueryCursor &nextCursor)
{
    if (IsPullRecords()) {
        startCursor_ = tempStartCursor_;
        nextCursor_.clear();
        tempStartCursor_.clear();
        totalPullCount_ = 0;
        cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
        cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
        cloudPrefImpl_.Delete(TEMP_START_CURSOR);
        cloudPrefImpl_.Delete(TOTAL_PULL_COUNT);
    } else {
        startCursor_ = nextCursor;
        nextCursor_.clear();
        cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
        cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
    }
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

int32_t DataHandler::OnDownloadThumbSuccess(const DriveKit::DKDownloadAsset &asset)
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