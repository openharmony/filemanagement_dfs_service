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

#ifndef DRIVE_KIT_RECORD_H
#define DRIVE_KIT_RECORD_H

#include <map>

#include "dk_record_field.h"
#include "dk_reference.h"

namespace DriveKit {
struct DKRecordsResponse {
    int64_t time;
    std::string deviceName;
    std::string appId;
};
using DKFieldKey = std::string;
using DKRecordData = std::map<DKFieldKey, DKRecordField>;
class DKRecord {
public:
    DKRecordId GetRecordId() const
    {
        return id_;
    };
    DKRecordType GetRecordType() const
    {
        return type_;
    };
    DKRecordsResponse GetRecordCreateInfo() const
    {
        return createInfo_;
    };
    DKRecordsResponse GetRecordModifiedInfo() const
    {
        return modifiedInfo_;
    };
    void GetRecordData(DKRecordData &fields) const
    {
        fields = this->fields_;
    };
    void StealRecordData(DKRecordData &fields)
    {
        fields = std::move(fields_);
    };
    bool GetIsDelete() const
    {
        return isDelete_;
    };
    unsigned long GetVersion() const
    {
        return version_;
    };
    uint64_t GetCreateTime() const
    {
        return createdTime_;
    }
    uint64_t GetEditedTime() const
    {
        return editedTime_;
    }
    void SetRecordId(DKRecordId id)
    {
        id_ = id;
    };
    void SetRecordType(DKRecordType type)
    {
        type_ = type;
    };
    void SetCreateInfo(DKRecordsResponse create)
    {
        createInfo_ = std::move(create);
    };
    void SetModifiedInfo(DKRecordsResponse modified)
    {
        modifiedInfo_ = std::move(modified);
    };
    void SetRecordData(const DKRecordData &fields)
    {
        this->fields_ = std::move(fields);
    };
    void SetRecordData(DKRecordData &&fields)
    {
        this->fields_ = std::move(fields);
    };
    void SetDelete(bool isDelete)
    {
        isDelete_ = isDelete;
    };
    void SetVersion(unsigned long version)
    {
        version_ = version;
    };
    void SetNewCreate(bool isNew)
    {
        isNewCreate_ = isNew;
    };
    void SetCreateTime(uint64_t createdTime)
    {
        createdTime_ = createdTime;
    }
    void SetEditedTime(uint64_t editedTime)
    {
        editedTime_ = editedTime;
    }
    void SetBaseCursor(std::string cursor)
    {
        baseCursor_ = cursor;
    }
private:
    DKRecordId id_;
    DKRecordType type_;
    DKRecordsResponse createInfo_;
    DKRecordsResponse modifiedInfo_;
    DKRecordData fields_;
    bool isDelete_;
    unsigned long version_;
    bool isNewCreate_ = false;
    uint64_t createdTime_ = 0;
    uint64_t editedTime_ = 0;
    std::string baseCursor_;
};
} // namespace DriveKit
#endif
