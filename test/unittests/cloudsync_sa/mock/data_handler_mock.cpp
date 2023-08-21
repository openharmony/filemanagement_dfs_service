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

#include <gtest/gtest.h>

#include "dfs_error.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

constexpr int32_t TEST_NUM = 100;

DataHandler::DataHandler(int32_t userId, const string &bundleName, const std::string &table)
    : cloudPrefImpl_(userId, bundleName, table)
{
}

void DataHandler::GetNextCursor(DriveKit::DKQueryCursor &cursor) {}

void DataHandler::SetTempStartCursor(const DriveKit::DKQueryCursor &cursor) {}

void DataHandler::GetTempStartCursor(DriveKit::DKQueryCursor &cursor) {}

void DataHandler::SetTempNextCursor(const DriveKit::DKQueryCursor &cursor, bool isFinish) {}

int32_t DataHandler::GetBatchNo()
{
    return batchNo_;
}

bool DataHandler::IsPullRecords()
{
    return startCursor_.empty();
}

void DataHandler::ClearCursor() {}

void DataHandler::FinishPull(const int32_t batchNo) {}

void DataHandler::SetRecordSize(const int32_t recordSize) {}

int32_t DataHandler::GetRecordSize()
{
    return recordSize_;
}

bool DataHandler::GetCheckFlag()
{
    return isChecking_;
}

void DataHandler::SetChecking() {}

int32_t DataHandler::GetCheckRecords(std::vector<DriveKit::DKRecordId> &checkRecords,
                                     const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records)
{
    return E_OK;
}

int32_t DataHandler::GetFileModifiedRecords(vector<DriveKit::DKRecord> &records)
{
    return E_OK;
}

int32_t DataHandler::OnModifyFdirtyRecords(const map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map)
{
    return E_OK;
}

int32_t DataHandler::GetAssetsToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
{
    return E_OK;
}

int32_t DataHandler::GetDownloadAsset(string cloudId, vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
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

int32_t DataHandler::OnDownloadThumb(const DriveKit::DKDownloadAsset &asset)
{
    return E_OK;
}

int32_t DataHandler::Clean(const int action)
{
    return E_OK;
}

int32_t DataHandler::GetRetryRecords(std::vector<DriveKit::DKRecordId> &records)
{
    if (recordSize_ == TEST_NUM) {
        return 1;
    }
    return E_OK;
}

int32_t DataHandler::OnModifyMdirtyRecords(const map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map)
{
    return E_OK;
}

int32_t DataHandler::OnDeleteRecords(const map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map)
{
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS