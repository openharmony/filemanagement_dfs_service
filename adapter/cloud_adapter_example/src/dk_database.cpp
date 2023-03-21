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

#include <functional>
#include <iostream>

#include "dk_context.h"
#include "dk_database.h"

namespace DriveKit {
DKLocalErrorCode
    DKDatabase::SaveRecords(std::shared_ptr<DKContext> context,
                            std::vector<DKRecord> &records,
                            DKSavePolicy policy,
                            std::function<void(std::shared_ptr<DKContext>,
                                               std::shared_ptr<const DKDatabase>,
                                               std::shared_ptr<const std::map<DKRecordId, DKRecordOperResult>>,
                                               const DKError &)> callback)
{
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKDatabase::SaveRecord(std::shared_ptr<DKContext> context,
                                        DKRecord &record,
                                        DKSavePolicy policy,
                                        std::function<void(std::shared_ptr<DKContext>,
                                                           std::shared_ptr<const DKDatabase>,
                                                           DKRecordId,
                                                           DKRecordOperResult,
                                                           const DKError &)> callback)
{
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKDatabase::FetchRecords(std::shared_ptr<DKContext> context,
                                          DKRecordType recordType,
                                          DKFieldKeyArray &desiredKeys,
                                          int resultLimit,
                                          DKQueryCursor cursor,
                                          std::function<void(std::shared_ptr<DKContext>,
                                                             std::shared_ptr<const DKDatabase>,
                                                             std::shared_ptr<const std::vector<DKRecord>>,
                                                             DKQueryCursor nextcursor,
                                                             const DKError &)> callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::FetchRecordWithId(std::shared_ptr<DKContext> context,
                                               DKRecordType recordType,
                                               DKRecordId recordId,
                                               DKFieldKeyArray &desiredKeys,
                                               std::function<void(std::shared_ptr<DKContext>,
                                                                  std::shared_ptr<const DKDatabase>,
                                                                  DKRecordId,
                                                                  const DKRecord &,
                                                                  const DKError &)> callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode
    DKDatabase::DeleteRecords(std::shared_ptr<DKContext> context,
                              std::vector<DKRecord> &records,
                              DKSavePolicy policy,
                              std::function<void(std::shared_ptr<DKContext>,
                                                 std::shared_ptr<const DKDatabase>,
                                                 std::shared_ptr<const std::map<DKRecordId, DKRecordOperResult>>,
                                                 const DKError &)> callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::ModifyRecords(
    std::shared_ptr<DKContext> context,
    std::vector<DKRecord> &recordsToSave,
    std::vector<DKRecord> &recordsToDelete,
    DKSavePolicy policy,
    bool atomically,
    std::function<void(std::shared_ptr<DKContext>,
                       std::shared_ptr<const DKDatabase>,
                       std::shared_ptr<const std::map<DKRecordId, DKRecordOperResult>> saveResult,
                       std::shared_ptr<const std::map<DKRecordId, DKRecordOperResult>> delResult,
                       const DKError &)> callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKDatabase::FetchRecordsWithQuery(std::shared_ptr<DKContext> context,
                                                   DKFieldKeyArray &desiredKeys,
                                                   DKQuery query,
                                                   int resultLimit,
                                                   DKQueryCursor cursor,
                                                   std::function<void(std::shared_ptr<DKContext>,
                                                                      std::shared_ptr<const DKDatabase>,
                                                                      std::shared_ptr<const std::vector<DKRecord>>,
                                                                      DKQueryCursor nextcursor,
                                                                      const DKError &)> callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode
    DKDatabase::FetchDatabaseChanges(std::shared_ptr<DKContext> context,
                                     DKRecordType recordType,
                                     DKFieldKeyArray &desiredKeys,
                                     int resultLimit,
                                     DKQueryCursor cursor,
                                     std::function<void(std::shared_ptr<DKContext>,
                                                        std::shared_ptr<const DKDatabase>,
                                                        std::shared_ptr<const std::map<DKRecordId, DKRecord>>,
                                                        DKQueryCursor nextcursor,
                                                        bool hasMore,
                                                        const DKError &)>)
{
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
} // namespace DriveKit
