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

#include "dk_record_field.h"

namespace DriveKit {
DKRecordField::DKRecordField() : type_(DKRecordFieldType::FIELD_TYPE_NULL) {}

DKRecordField::~DKRecordField() {}

DKRecordField::DKRecordField(DKFieldValue fieldValue) noexcept : value_(std::move(fieldValue))
{
    type_ = DKRecordFieldType(value_.index());
}

DKRecordField::DKRecordField(DKRecordField &&recordField) noexcept
{
    if (this == &recordField) {
        return;
    }
    type_ = recordField.type_;
    value_ = std::move(recordField.value_);
    recordField.type_ = DKRecordFieldType::FIELD_TYPE_NULL;
}

DKRecordField::DKRecordField(const DKRecordField &recordField)
{
    if (this == &recordField) {
        return;
    }
    type_ = recordField.type_;
    value_ = recordField.value_;
}

DKRecordField::DKRecordField(int val) : type_(DKRecordFieldType::FIELD_TYPE_INT)
{
    value_ = static_cast<int64_t>(val);
}
DKRecordField::DKRecordField(int64_t val) : type_(DKRecordFieldType::FIELD_TYPE_INT)
{
    value_ = val;
}
DKRecordField::DKRecordField(double val) : type_(DKRecordFieldType::FIELD_TYPE_DOUBLE)
{
    value_ = val;
}
DKRecordField::DKRecordField(bool val) : type_(DKRecordFieldType::FIELD_TYPE_BOOL)
{
    value_ = val;
}
DKRecordField::DKRecordField(const char *val) : type_(DKRecordFieldType::FIELD_TYPE_STRING)
{
    value_ = std::string(val);
}
DKRecordField::DKRecordField(const std::string &val) : type_(DKRecordFieldType::FIELD_TYPE_STRING)
{
    value_ = val;
}
DKRecordField::DKRecordField(const std::vector<uint8_t> &val) : type_(DKRecordFieldType::FIELD_TYPE_BLOB)
{
    std::vector<uint8_t> blob = val;
    value_ = blob;
}
DKRecordField::DKRecordField(DKRecordFieldMap &val) : type_(DKRecordFieldType::FIELD_TYPE_MAP)
{
    value_ = val;
}
DKRecordField::DKRecordField(DKRecordFieldList &val) : type_(DKRecordFieldType::FIELD_TYPE_LIST)
{
    value_ = val;
}
DKRecordField::DKRecordField(DKAsset &val) : type_(DKRecordFieldType::FIELD_TYPE_ASSET)
{
    value_ = val;
}
DKRecordField::DKRecordField(DKReference &val) : type_(DKRecordFieldType::FIELD_TYPE_REFERENCE)
{
    value_ = val;
}
DKRecordField &DKRecordField::operator=(DKRecordField &&recordField) noexcept
{
    if (this == &recordField) {
        return *this;
    }
    type_ = recordField.type_;
    value_ = std::move(recordField.value_);
    recordField.type_ = DKRecordFieldType::FIELD_TYPE_NULL;
    return *this;
}
DKRecordField &DKRecordField::operator=(const DKRecordField &recordField)
{
    if (this == &recordField) {
        return *this;
    }
    type_ = recordField.type_;
    value_ = recordField.value_;
    return *this;
}
DKRecordFieldType DKRecordField::GetType() const
{
    return type_;
}
DKFieldValue DKRecordField::GetFieldValue() const
{
    return value_;
}
void DKRecordField::StealDKFiledValue(DKFieldValue &value)
{
    value = std::move(value_);
    type_ = DKRecordFieldType::FIELD_TYPE_NULL;
    return;
}
DKLocalErrorCode DKRecordField::GetInt(int &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_INT) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }

    int64_t v = std::get<int64_t>(value_);
    val = static_cast<int>(v);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetLong(int64_t &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_INT) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }

    val = std::get<int64_t>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetDouble(double &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_DOUBLE) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }

    val = std::get<double>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetBool(bool &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_BOOL) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }

    val = std::get<bool>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetString(std::string &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_STRING) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }
    val = std::get<std::string>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetBlob(std::vector<uint8_t> &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_BLOB) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }
    val = std::get<std::vector<uint8_t>>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetRecordList(DKRecordFieldList &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_LIST) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }
    val = std::get<DKRecordFieldList>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetRecordMap(DKRecordFieldMap &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_MAP) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }
    val = std::get<DKRecordFieldMap>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetAsset(DKAsset &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_ASSET) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }
    val = std::get<DKAsset>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
DKLocalErrorCode DKRecordField::GetReference(DKReference &val) const
{
    if (type_ != DKRecordFieldType::FIELD_TYPE_REFERENCE) {
        return DKLocalErrorCode::DATA_TYPE_ERROR;
    }
    val = std::get<DKReference>(value_);
    return DKLocalErrorCode::NO_ERROR;
}
}