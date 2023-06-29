/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

namespace OHOS {
using namespace std;
bool Parcel::WriteInt32(int32_t value)
{
    if (value == 0) {
        return false;
    }
        return true;
}

bool Parcel::WriteUint32(uint32_t value)
{
    if (value == 0) {
        return false;
    }
        return true;
}

bool Parcel::WriteInt64(int64_t value)
{
    if (value == 0) {
        return false;
    }
        return true;
}

bool Parcel::WriteString(const std::string &value)
{
    if (value == "continue") {
        return true;
    }
    return false;
}

bool Parcel::WriteBool(bool value)
{
    if (value == false) {
        return false;
    }
        return true;
}

bool Parcel::WriteParcelable(const Parcelable *)
{
    return true;
}

bool Parcel::ReadInt32(int32_t &value)
{
    value = Parcel::readCursor_;
    if (Parcel::readCursor_ == 0) {
        return false;
    }
        return true;
}

bool Parcel::ReadUint32(uint32_t &value)
{
    value = Parcel::dataSize_;
    if (Parcel::dataSize_ == 0) {
        return false;
    }
        return true;
}

bool Parcel::ReadInt64(int64_t &value)
{
    if (value == 0) {
        return false;
    }
    return true;
}

bool Parcel::ReadString(string &value)
{
    if (Parcel::data_ == nullptr) {
        return false;
    }
    return true;
}

bool Parcel::ReadBool(bool &value)
{
    value = Parcel::writable_;
    if (value == false) {
        return false;
    }
    return true;
}
} // namespace OHOS::FileManagement::Backup
