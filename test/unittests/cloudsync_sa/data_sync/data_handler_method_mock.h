/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_SYNC_SA_DATA_HANDER_METHOD_MOCK_H
#define OHOS_CLOUD_SYNC_SA_DATA_HANDER_METHOD_MOCK_H

#include "data_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
class DataHandlerMethodMock final : public DataHandler {
public:
    DataHandlerMethodMock(int32_t userId, const std::string &bundleName, const std::string &table)
        : DataHandler(userId, bundleName, table)
    {
    }
    MOCK_METHOD1(GetFetchCondition, void(FetchCondition &cond));
    MOCK_METHOD2(OnFetchRecords, int32_t(std::shared_ptr<std::vector<DriveKit::DKRecord>> &, OnFetchParams &));
    MOCK_METHOD1(GetRetryRecords, int32_t(std::vector<DriveKit::DKRecordId> &records));
    MOCK_METHOD2(GetCheckRecords,
                 int32_t(std::vector<DriveKit::DKRecordId> &checkRecords,
                         const std::shared_ptr<std::vector<DriveKit::DKRecord>> &records));
    MOCK_METHOD1(GetAssetsToDownload, int32_t(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload));

    MOCK_METHOD3(GetDownloadAsset,
                 int32_t(std::string cloudId, std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload,
                 std::shared_ptr<DentryContext> dentryContext));
    /* upload */
    MOCK_METHOD1(GetCreatedRecords, int32_t(std::vector<DriveKit::DKRecord> &records));
    MOCK_METHOD1(GetDeletedRecords, int32_t(std::vector<DriveKit::DKRecord> &records));
    MOCK_METHOD1(GetMetaModifiedRecords, int32_t(std::vector<DriveKit::DKRecord> &records));
    MOCK_METHOD1(GetFileModifiedRecords, int32_t(std::vector<DriveKit::DKRecord> &records));

    /* upload callback */
    MOCK_METHOD1(OnCreateRecords, int32_t(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map));
    MOCK_METHOD1(OnDeleteRecords, int32_t(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map));
    MOCK_METHOD1(OnModifyMdirtyRecords,
                 int32_t(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map));
    MOCK_METHOD1(OnModifyFdirtyRecords,
                 int32_t(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map));
    MOCK_METHOD2(OnDownloadSuccess, int32_t(const DriveKit::DKDownloadAsset &asset,
                 std::shared_ptr<DriveKit::DKContext> context));
    MOCK_METHOD1(OnDownloadThumb,
                 int32_t(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap));
    MOCK_METHOD1(OnDownloadThumb, int32_t(const DriveKit::DKDownloadAsset &asset));
    /*clean*/
    MOCK_METHOD1(Clean, int32_t(const int action));

    /* cursor */
    MOCK_METHOD1(GetNextCursor, void(DriveKit::DKQueryCursor &cursor));
    MOCK_METHOD1(SetTempStartCursor, void(const DriveKit::DKQueryCursor &cursor));
    MOCK_METHOD1(GetTempStartCursor, void(DriveKit::DKQueryCursor &cursor));
    MOCK_METHOD2(SetTempNextCursor, void(const DriveKit::DKQueryCursor &cursor, bool isFinish));
    MOCK_METHOD0(GetBatchNo, int32_t());
    MOCK_METHOD0(IsPullRecords, bool());
    MOCK_METHOD0(ClearCursor, void());
    MOCK_METHOD1(FinishPull, void(const int32_t barchNo));
    MOCK_METHOD1(SetRecordSize, void(const int32_t recordSize));
    MOCK_METHOD0(GetRecordSize, int32_t());
    MOCK_METHOD0(GetCheckFlag, bool());
    MOCK_METHOD0(SetChecking, void());
};
} // namespace OHOS::FileManagement::CloudSync::Test
#endif
