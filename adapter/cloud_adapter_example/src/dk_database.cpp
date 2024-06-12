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
#include "dk_database.h"

#include <functional>
#include <iostream>
#include <thread>

#include "dk_context.h"

namespace DriveKit {
DKLocalErrorCode DKDatabase::SaveRecords(std::shared_ptr<DKContext> context,
                                         std::vector<DKRecord> &&records,
                                         DKSavePolicy policy,
                                         SaveRecordsCallback callback)
{
    /* mock */
    auto result = std::make_shared<std::map<DKRecordId, DKRecordOperResult>>();
    srand(time(nullptr));
    for (auto &record : records) {
        DKRecordId recordId = record.GetRecordId();
        if (recordId.empty()) {
            recordId = std::to_string(rand() + 1);
        }
        DKRecordOperResult operResult(DKLocalErrorCode::NO_ERROR);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
    }

    DKError err;
    std::thread ([=]() { callback(context, this->shared_from_this(), result, err); }).detach();

    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKDatabase::SaveRecord(std::shared_ptr<DKContext> context,
                                        DKRecord &&record,
                                        DKSavePolicy policy,
                                        SaveRecordCallback callback)
{
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKDatabase::FetchRecords(std::shared_ptr<DKContext> context,
                                          DKRecordType recordType,
                                          DKFieldKeyArray &desiredKeys,
                                          int resultLimit,
                                          DKQueryCursor cursor,
                                          FetchRecordsCallback callback)
{
    auto result = std::make_shared<std::vector<DKRecord>>();
    DKQueryCursor newCursor;

    DKError err;
    std::thread ([=]() { callback(context, this->shared_from_this(), result, newCursor, err); }).detach();
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::FetchRecordWithId(std::shared_ptr<DKContext> context,
                                               DKRecordType recordType,
                                               DKRecordId recordId,
                                               DKFieldKeyArray &desiredKeys,
                                               FetchRecordCallback callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::FetchRecordWithIds(std::shared_ptr<DKContext> context,
                                                std::vector<DKRecord> &&records,
                                                DKFieldKeyArray &desiredKeys,
                                                FetchRecordIdsCallback callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::DeleteRecords(std::shared_ptr<DKContext> context,
                                           std::vector<DKRecord> &&records,
                                           DKSavePolicy policy,
                                           DeleteRecordsCallback callback)
{
    /* mock */
    auto result = std::make_shared<std::map<DKRecordId, DKRecordOperResult>>();
    for (auto &record : records) {
        DKRecordId recordId = record.GetRecordId();
        DKRecordOperResult operResult;
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
    }

    DKError err;
    std::thread ([=]() { callback(context, this->shared_from_this(), result, err); }).detach();
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::ModifyRecords(std::shared_ptr<DKContext> context,
                                           std::vector<DKRecord> &&recordsToSave,
                                           std::vector<DKRecord> &&recordsToDelete,
                                           DKSavePolicy policy,
                                           bool atomically,
                                           ModifyRecordsCallback callback)
{
    /* mock */
    auto result = std::make_shared<std::map<DKRecordId, DKRecordOperResult>>();
    for (auto &record : recordsToSave) {
        DKRecordId recordId = record.GetRecordId();
        DKRecordOperResult operResult;
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
    }

    DKError err;
    std::thread ([=]() { callback(context, this->shared_from_this(), result, nullptr, err); }).detach();
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::FetchRecordsWithQuery(std::shared_ptr<DKContext> context,
                                                   DKFieldKeyArray &desiredKeys,
                                                   DKQuery query,
                                                   int resultLimit,
                                                   DKQueryCursor cursor,
                                                   FetchRecordsCallback callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::FetchDatabaseChanges(std::shared_ptr<DKContext> context,
                                                  DKRecordType recordType,
                                                  DKFieldKeyArray &desiredKeys,
                                                  int resultLimit,
                                                  DKQueryCursor cursor,
                                                  FetchDatabaseCallback callback)
{
    auto result = std::make_shared<std::vector<DKRecord>>();
    bool hasMore = false;
    DKQueryCursor newCursor;

    DKError err;
    std::thread ([=]() { callback(context, this->shared_from_this(), result, newCursor, hasMore, err); }).detach();
    return DKLocalErrorCode::NO_ERROR;
}

DKError DKDatabase::GetStartCursor(DKRecordType recordType, DKQueryCursor &cursor)
{
    DKError e;
    return e;
}
DKError DKDatabase::GenerateIds(int count, std::vector<DKRecordId> &ids)
{
    DKError e;
    return e;
}
DKError DKDatabase::GetRootId(DKRecordId &id)
{
    DKError e;
    return e;
}
DKError DKDatabase::GetLock(DKLock &lock)
{
    DKError e;
    return e;
}
void DKDatabase::DeleteLock(DKLock lock) {}
std::shared_ptr<DKAssetsDownloader> DKDatabase::GetAssetsDownloader()
{
    return std::make_shared<DKAssetsDownloader>(shared_from_this());
}
std::shared_ptr<DKAssetReadSession> DKDatabase::NewAssetReadSession(DKRecordType recordType,
                                                                    DKRecordId recordId,
                                                                    DKFieldKey assetKey,
                                                                    DKAssetPath assetPath)
{
    return std::make_shared<DKAssetReadSession>();
}
DKDatabase::DKDatabase(std::shared_ptr<DKContainer> container, DKDatabaseScope scope)
{
}
void DKDatabase::Init()
{
}
void DKDatabase::Release()
{
    return;
}
} // namespace DriveKit
