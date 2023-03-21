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
#include "dk_asset.h"
#include "dk_error.h"
#include "dk_reference.h"
#include <map>
#include <string>
#include <variant>
#include <vector>

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

class DKRecordField {
public:
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
    DKRecordField() : type_(DKRecordFieldType::FIELD_TYPE_NULL){};
    ~DKRecordField(){};
    DKRecordField(DKFieldValue fieldValue) noexcept : value_(std::move(fieldValue))
    {
        type_ = DKRecordFieldType(value_.index());
    }
    DKRecordField(DKRecordField &&recordField) noexcept
    {
        if (this == &recordField) {
            return;
        }
        type_ = recordField.type_;
        value_ = std::move(recordField.value_);
        recordField.type_ = DKRecordFieldType::FIELD_TYPE_NULL;
    };
    DKRecordField(const DKRecordField &recordField)
    {
        if (this == &recordField) {
            return;
        }
        type_ = recordField.type_;
        value_ = recordField.value_;
    }

    explicit DKRecordField(int val) : type_(DKRecordFieldType::FIELD_TYPE_INT)
    {
        value_ = static_cast<int64_t>(val);
    }
    explicit DKRecordField(int64_t val) : type_(DKRecordFieldType::FIELD_TYPE_INT)
    {
        value_ = val;
    }
    explicit DKRecordField(double val) : type_(DKRecordFieldType::FIELD_TYPE_DOUBLE)
    {
        value_ = val;
    }
    explicit DKRecordField(bool val) : type_(DKRecordFieldType::FIELD_TYPE_BOOL)
    {
        value_ = val;
    }
    explicit DKRecordField(const char *val) : type_(DKRecordFieldType::FIELD_TYPE_STRING)
    {
        value_ = std::string(val);
    }
    explicit DKRecordField(const std::string &val) : type_(DKRecordFieldType::FIELD_TYPE_STRING)
    {
        value_ = val;
    }
    explicit DKRecordField(const std::vector<uint8_t> &val) : type_(DKRecordFieldType::FIELD_TYPE_BLOB)
    {
        std::vector<uint8_t> blob = val;
        value_ = blob;
    }
    explicit DKRecordField(DKRecordFieldMap &val) : type_(DKRecordFieldType::FIELD_TYPE_MAP)
    {
        value_ = val;
    }
    explicit DKRecordField(DKRecordFieldList &val) : type_(DKRecordFieldType::FIELD_TYPE_LIST)
    {
        value_ = val;
    }
    explicit DKRecordField(DKAsset &val) : type_(DKRecordFieldType::FIELD_TYPE_ASSET)
    {
        value_ = val;
    }
    explicit DKRecordField(DKReference &val) : type_(DKRecordFieldType::FIELD_TYPE_REFERENCE)
    {
        value_ = val;
    }
    DKRecordField &operetor(DKRecordField &&recordField) noexcept
    {
        if (this == &recordField) {
            return *this;
        }
        type_ = recordField.type_;
            value_ = std::move(recordField.value_);
            recordField.type_ = DKRecordFieldType::FIELD_TYPE_NULL;
            return *this;
    }
    DKRecordField &operator=(const DKRecordField &recordField)
    {
        if (this == &recordField) {
            return *this;
        }
        type_ = recordField.type_;
        value_ = recordField.value_;
        return *this;
    }
    DKRecordFieldType Gettype() const
    {
        return type_;
    }
    DKLocalErrorCode Getint(int &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_INT) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }

        int64_t v = std::get<int64_t>(value_);
        val = static_cast<int>(v);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetLong(int64_t &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_INT) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }

        val = std::get<int64_t>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetDouble(double &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_DOUBLE) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }

        val = std::get<double>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetBool(bool &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_BOOL) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }

        val = std::get<bool>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetString(std::string &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_STRING) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }
        val = std::get<std::string>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetBlob(std::vector<uint8_t> &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_BLOB) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }
        val = std::get<std::vector<uint8_t>>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetRecordList(DKRecordFieldList &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_LIST) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }
        val = std::get<DKRecordFieldList>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetRecordMap(DKRecordFieldMap &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_MAP) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }
        val = std::get<DKRecordFieldMap>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetAsset(DKAsset &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_ASSET) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }
        val = std::get<DKAsset>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }
    DKLocalErrorCode GetReference(DKReference &val) const
    {
        if (type_ != DKRecordFieldType::FIELD_TYPE_REFERENCE) {
            return DKLocalErrorCode::DATA_TYPE_ERROR;
        }
        val = std::get<DKReference>(value_);
        return DKLocalErrorCode::NO_ERROR;
    }

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
}; // namespace DriveKit
#endif
