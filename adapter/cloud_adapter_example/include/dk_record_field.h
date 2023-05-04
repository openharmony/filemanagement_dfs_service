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

#ifndef DRIVE_KIT_RECORD_FIELD_H
#define DRIVE_KIT_RECORD_FIELD_H
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "dk_asset.h"
#include "dk_error.h"
#include "dk_reference.h"

namespace DriveKit {
enum class DKRecordFieldType {
    FIELD_TYPE_NULL = 0,
    FIELD_TYPE_INT,
    FIELD_TYPE_DOUBLE,
    FIELD_TYPE_STRING,
    FIELD_TYPE_BOOL,
    FIELD_TYPE_BLOB,
    FIELD_TYPE_LIST,
    FIELD_TYPE_MAP,
    FIELD_TYPE_ASSET,
    FIELD_TYPE_REFERENCE,
};
class DKRecordField;
using DKRecordFieldList = std::vector<DKRecordField>;
using DKRecordFieldMap = std::map<std::string, DKRecordField>;
using DKFieldValue = std::variant<std::monostate,
                                  int64_t,
                                  double,
                                  std::string,
                                  bool,
                                  std::vector<uint8_t>,
                                  DKRecordFieldList,
                                  DKRecordFieldMap,
                                  DKAsset,
                                  DKReference>;
class DKRecordField {
public:
    DKRecordField();
    ~DKRecordField();
    DKRecordField(DKFieldValue fieldValue) noexcept;
    DKRecordField(DKRecordField &&recordField) noexcept;
    DKRecordField(const DKRecordField &recordField);

    explicit DKRecordField(int val);
    explicit DKRecordField(int64_t val);
    explicit DKRecordField(double val);
    explicit DKRecordField(bool val);
    explicit DKRecordField(const char *val);
    explicit DKRecordField(const std::string &val);
    explicit DKRecordField(const std::vector<uint8_t> &val);
    explicit DKRecordField(DKRecordFieldMap &val);
    explicit DKRecordField(DKRecordFieldList &val);
    explicit DKRecordField(DKAsset &val);
    explicit DKRecordField(DKReference &val);
    DKRecordField &operator=(DKRecordField &&recordField) noexcept;
    DKRecordField &operator=(const DKRecordField &recordField);
    DKRecordFieldType GetType() const;
    DKFieldValue GetFieldValue() const;
    void StealDKFiledValue(DKFieldValue &value);
    DKLocalErrorCode GetInt(int &val) const;
    DKLocalErrorCode GetLong(int64_t &val) const;
    DKLocalErrorCode GetDouble(double &val) const;
    DKLocalErrorCode GetBool(bool &val) const;
    DKLocalErrorCode GetString(std::string &val) const;
    DKLocalErrorCode GetBlob(std::vector<uint8_t> &val) const;
    DKLocalErrorCode GetRecordList(DKRecordFieldList &val) const;
    DKLocalErrorCode GetRecordMap(DKRecordFieldMap &val) const;
    DKLocalErrorCode GetAsset(DKAsset &val) const;
    DKLocalErrorCode GetReference(DKReference &val) const;

    operator int() const
    {
        return static_cast<int>(int64_t(std::get<int64_t>(value_)));
    }
    operator int64_t() const
    {
        return std::get<int64_t>(value_);
    }
    operator double() const
    {
        return std::get<double>(value_);
    }
    operator bool() const
    {
        return std::get<bool>(value_);
    }
    operator std::string() const
    {
        return std::get<std::string>(value_);
    }
    operator std::vector<uint8_t>() const
    {
        return std::get<std::vector<uint8_t>>(value_);
    }
    operator DKRecordFieldList() const
    {
        return std::get<DKRecordFieldList>(value_);
    }
    operator DKRecordFieldMap() const
    {
        return std::get<DKRecordFieldMap>(value_);
    }
    operator DKAsset() const
    {
        return std::get<DKAsset>(value_);
    }
    operator DKReference() const
    {
        return std::get<DKReference>(value_);
    }
    operator DKFieldValue() const
    {
        return value_;
    }

private:
    DKRecordFieldType type_;
    DKFieldValue value_;
};
} // namespace DriveKit
#endif
