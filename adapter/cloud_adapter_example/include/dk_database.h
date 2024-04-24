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

#ifndef DRIVE_KIT_DATABASE_H
#define DRIVE_KIT_DATABASE_H

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <list>

#include "dk_asset_read_session.h"
#include "dk_assets_downloader.h"
#include "dk_cloud_callback.h"
#include "dk_context.h"
#include "dk_error.h"
#include "dk_record.h"
#include "dk_result.h"

namespace DriveKit {
enum class DKSavePolicy {
    DK_SAVE_IF_UNCHANGED = 0,
    DK_CURRENT_WRITER_WINS,
};

enum class DKDatabaseScope {
    DK_PUBLIC_DATABASE = 0,
    DK_PRIVATE_DATABASE,
    DK_SHARED_DATABASE,
};

using DKSchemaRawData = std::string;
struct DKSchemaField {
    DKFieldKey name;
    DKRecordFieldType type;
    bool primary;
    bool nullable;
    bool sortable;
    bool searchable;
    bool queryable;
    DKRecordFieldType listType;
    DKRecordType refRecordType;
};
struct DKSchemaNode {
    DKRecordType recordType;
    std::string tableName;
    std::map<DKFieldKey, DKSchemaField> fields;
    std::vector<DKFieldKey> dupCheckFields;
};
struct DKOrderTable {
    DKRecordType recordType;
    std::string tableName;
};
struct DKSchema {
    int version;
    std::map<DKRecordType, DKSchemaNode> recordTypes;
    DKSchemaRawData schemaData;
    std::vector<DKOrderTable> orderTables;
};

struct DKPredicate {
    std::string predicateFormat;
};
struct DKSortDescriptor {
    bool ascending;
    DKFieldKey key;
};
using DKSortDescriptors = std::vector<DKSortDescriptor>;
struct DKQuery {
    DKRecordType recordType;
    DKPredicate predicate;
    DKSortDescriptors sortDescriptors;
};

class DKRecordOperResult : public DKResult {
public:
    DKRecordOperResult() {}
    DKRecordOperResult(DKLocalErrorCode code)
    {
        error_.SetLocalError(code);
    }
public:
    void SetDKRecord(const DKRecord &record)
    {
        record_ = record;
    }
    void SetDKRecord(DKRecord &&record)
    {
        record_ = std::move(record);
    }
    DKRecord GetDKRecord() const
    {
        return record_;
    }
    void StealDKRecord(DKRecord &record)
    {
        record = std::move(record_);
        return;
    }

private:
    DKRecord record_;
};

struct DKLock {
    int lockInterval;
    std::string lockSessionId;
};
using DKFieldKeyArray = std::vector<DKFieldKey>;
using DKQueryCursor = std::string;
class DKContainer;
class DKAssetsUploadHelper;
class DKDatabase : public std::enable_shared_from_this<DKDatabase> {
    friend class DKContainer;
public:
    DKDatabase(std::shared_ptr<DKContainer> container, DKDatabaseScope scope);
    ~DKDatabase() {}

    using SaveRecordsCallback = std::function<void(std::shared_ptr<DKContext>,
                                                   std::shared_ptr<DKDatabase>,
                                                   std::shared_ptr<std::map<DKRecordId, DKRecordOperResult>>,
                                                   const DKError &)>;
    DKLocalErrorCode SaveRecords(std::shared_ptr<DKContext> context,
                                 std::vector<DKRecord> &&records,
                                 DKSavePolicy policy,
                                 SaveRecordsCallback callback);

    using SaveRecordCallback = std::function<void(std::shared_ptr<DKContext>,
                                                  std::shared_ptr<DKDatabase>,
                                                  DKRecordId,
                                                  DKRecordOperResult,
                                                  const DKError &)>;
    DKLocalErrorCode SaveRecord(std::shared_ptr<DKContext> context,
                                DKRecord &&record,
                                DKSavePolicy policy,
                                SaveRecordCallback callback);

    using FetchRecordsCallback = std::function<void(std::shared_ptr<DKContext>,
                                                    std::shared_ptr<DKDatabase>,
                                                    std::shared_ptr<std::vector<DKRecord>>,
                                                    DKQueryCursor nextCursor,
                                                    const DKError &)>;
    DKLocalErrorCode FetchRecords(std::shared_ptr<DKContext> context,
                                  DKRecordType recordType,
                                  DKFieldKeyArray &desiredKeys,
                                  int resultLimit,
                                  DKQueryCursor cursor,
                                  FetchRecordsCallback callback);

    using FetchRecordCallback = std::function<void(std::shared_ptr<DKContext>,
                                                   std::shared_ptr<DKDatabase>,
                                                   DKRecordId,
                                                   DKRecord &,
                                                   const DKError &)>;
    using FetchRecordIdsCallback = std::function<void(std::shared_ptr<DKContext>,
                                                      std::shared_ptr<DKDatabase>,
                                                      std::shared_ptr<std::map<DKRecordId, DKRecordOperResult>>,
                                                      const DKError &)>;
    DKLocalErrorCode FetchRecordWithId(std::shared_ptr<DKContext> context,
                                       DKRecordType recordType,
                                       DKRecordId recordId,
                                       DKFieldKeyArray &desiredKeys,
                                       FetchRecordCallback callback);
    DKLocalErrorCode FetchRecordWithIds(std::shared_ptr<DKContext> context,
                                        std::vector<DKRecord> &&records,
                                        DKFieldKeyArray &desiredKeys,
                                        FetchRecordIdsCallback callback);
    using DeleteRecordsCallback = std::function<void(std::shared_ptr<DKContext>,
                                                     std::shared_ptr<DKDatabase>,
                                                     std::shared_ptr<std::map<DKRecordId, DKRecordOperResult>>,
                                                     const DKError &)>;
    DKLocalErrorCode DeleteRecords(std::shared_ptr<DKContext> context,
                                   std::vector<DKRecord> &&records,
                                   DKSavePolicy policy,
                                   DeleteRecordsCallback callback);

    using ModifyRecordsCallback =
        std::function<void(std::shared_ptr<DKContext>,
                           std::shared_ptr<DKDatabase>,
                           std::shared_ptr<std::map<DKRecordId, DKRecordOperResult>> saveResult,
                           std::shared_ptr<std::map<DKRecordId, DKRecordOperResult>> delResult,
                           const DKError &)>;
    DKLocalErrorCode ModifyRecords(std::shared_ptr<DKContext> context,
                                   std::vector<DKRecord> &&recordsToSave,
                                   std::vector<DKRecord> &&recordsToDelete,
                                   DKSavePolicy policy,
                                   bool atomically,
                                   ModifyRecordsCallback callback);

    DKLocalErrorCode FetchRecordsWithQuery(std::shared_ptr<DKContext> context,
                                           DKFieldKeyArray &desiredKeys,
                                           DKQuery query,
                                           int resultLimit,
                                           DKQueryCursor cursor,
                                           FetchRecordsCallback callback);

    using FetchDatabaseCallback = std::function<void(std::shared_ptr<DKContext>,
                                                     std::shared_ptr<DKDatabase>,
                                                     std::shared_ptr<std::vector<DKRecord>>,
                                                     DKQueryCursor nextCursor,
                                                     bool hasMore,
                                                     const DKError &)>;
    DKLocalErrorCode FetchDatabaseChanges(std::shared_ptr<DKContext> context,
                                          DKRecordType recordType,
                                          DKFieldKeyArray &desiredKeys,
                                          int resultLimit,
                                          DKQueryCursor cursor,
                                          FetchDatabaseCallback callback);

    DKError GetStartCursor(DKRecordType recordType, DKQueryCursor &cursor);
    DKError GenerateIds(int count, std::vector<DKRecordId> &ids);
    DKError GetRootId(DKRecordId &id);
    DKError GetLock(DKLock &lock);
    void DeleteLock(DKLock lock);
    void Release();

    std::shared_ptr<DKAssetsDownloader> GetAssetsDownloader();
    std::shared_ptr<DKAssetReadSession>
        NewAssetReadSession(DKRecordType recordType, DKRecordId recordId, DKFieldKey assetKey, DKAssetPath assetPath);
    std::list<std::shared_ptr<DKAssetsUploadHelper>> assetUploadHplperLst_;
protected:
    void Init();
private:
    DKContainerName containerName_;
};
} // namespace DriveKit
#endif