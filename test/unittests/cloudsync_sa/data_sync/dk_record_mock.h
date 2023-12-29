/*
 * Copyright (C) 2023 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef OHOS_CLOUD_SYNC_SA_DRIVE_KIT_RECORD_MOCK_H
#define OHOS_CLOUD_SYNC_SA_DRIVE_KIT_RECORD_MOCK_H

#include "dk_record.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace DriveKit;
using DKFieldKey = std::string;
using DKRecordData = std::map<DKFieldKey, DKRecordField>;

class DKRecordMock : public DKRecord {
public:
    ~DKRecordMock() {}
    MOCK_METHOD0(GetRecordId, DKRecordId());
    MOCK_METHOD0(GetRecordType, DKRecordType());
    MOCK_METHOD0(GetRecordCreateInfo, DKRecordsResponse());
    MOCK_METHOD0(GetRecordModifiedInfo, DKRecordsResponse());
    MOCK_METHOD1(GetRecordData, void(DKRecordData &fields));
    MOCK_METHOD1(StealRecordData, void(DKRecordData &fields));
    MOCK_METHOD0(GetIsDelete, bool());
    MOCK_METHOD0(GetVersion, unsigned long());
    MOCK_METHOD0(GetCreateTime, uint64_t());
    MOCK_METHOD0(GetEditedTime, uint64_t());
    MOCK_METHOD1(SetRecordId, void(DKRecordId id));
    MOCK_METHOD1(SetRecordType, void(DKRecordType type));
    MOCK_METHOD1(SetCreateInfo, void(DKRecordsResponse create));
    MOCK_METHOD1(SetModifiedInfo, void(DKRecordsResponse modified));
    MOCK_METHOD1(SetRecordData, void(const DKRecordData &fields));
    MOCK_METHOD1(SetRecordData, void(DKRecordData &&fields));
    MOCK_METHOD1(SetDelete, void(bool isDelete));
    MOCK_METHOD1(SetVersion, void(unsigned long version));
    MOCK_METHOD1(SetNewCreate, void(bool isNew));
    MOCK_METHOD1(SetCreateTime, void(uint64_t createdTime));
    MOCK_METHOD1(SetEditedTime, void(uint64_t editedTime));
    MOCK_METHOD1(SetBaseCursor, void(std::string cursor));
};
} // namespace OHOS::FileManagement::CloudSync::Test

#endif
