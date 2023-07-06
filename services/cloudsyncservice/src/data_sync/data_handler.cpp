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

void DataHandler::SetNextCursor(const DriveKit::DKQueryCursor &cursor)
{
    nextCursor_ = cursor;
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
    tempStartCursor_.clear();
}

void DataHandler::GetPullCount(int32_t &totalPullCount, int32_t &downloadThumbLimit)
{
    totalPullCount = totalPullCount_;
    downloadThumbLimit = downloadThumbLimit_;
}

void DataHandler::SetTotalPullCount(const int32_t totalPullCount)
{
    totalPullCount_ = totalPullCount;
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
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS