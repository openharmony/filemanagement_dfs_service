/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "parcel.h"

namespace OHOS::FileManagement::CloudSync {

bool Parcel::WriteBool(bool value)
{
    return IParcel::parcel_->WriteBool(value);
}

bool Parcel::ReadBool(bool &value)
{
    return IParcel::parcel_->ReadBool(value);
}

bool Parcel::WriteInt32(int32_t value)
{
    return IParcel::parcel_->WriteInt32(value);
}

bool Parcel::ReadInt32(int32_t &value)
{
    return IParcel::parcel_->ReadInt32(value);
}

bool Parcel::WriteInt64(int64_t value)
{
    return IParcel::parcel_->WriteInt64(value);
}

bool Parcel::WriteUint64(uint64_t value)
{
    return IParcel::parcel_->WriteUint64(value);
}

bool Parcel::ReadInt64(int64_t &value)
{
    return IParcel::parcel_->ReadInt64(value);
}

bool Parcel::ReadUint64(uint64_t &value)
{
    return IParcel::parcel_->ReadUint64(value);
}

bool Parcel::WriteUint32(uint32_t value)
{
    return IParcel::parcel_->WriteUint32(value);
}

bool Parcel::ReadUint32(uint32_t &value)
{
    return IParcel::parcel_->ReadUint32(value);
}

bool Parcel::WriteString(const std::string &value)
{
    return IParcel::parcel_->WriteString(value);
}

bool Parcel::ReadString(std::string &value)
{
    return IParcel::parcel_->ReadString(value);
}

bool Parcel::WriteStringVector(const std::vector<std::string> &value)
{
    return IParcel::parcel_->WriteStringVector(value);
}

bool Parcel::ReadStringVector(std::vector<std::string> *value)
{
    return IParcel::parcel_->ReadStringVector(value);
}

bool Parcel::WriteUInt8Vector(const std::vector<uint8_t> &value)
{
    return IParcel::parcel_->WriteUInt8Vector(value);
}

bool Parcel::ReadUInt8Vector(std::vector<uint8_t> *value)
{
    return IParcel::parcel_->ReadUInt8Vector(value);
}
} // namespace OHOS::FileManagement::CloudSync