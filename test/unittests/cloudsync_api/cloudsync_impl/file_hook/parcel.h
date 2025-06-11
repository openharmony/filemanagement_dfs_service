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

#ifndef MOCK_PARCEL_H
#define MOCK_PARCEL_H

#include <gmock/gmock.h>

namespace OHOS::FileManagement::CloudSync {

class IParcel {
public:
    IParcel() = default;
    virtual ~IParcel() = default;
    virtual bool WriteBool(bool value) = 0;
    virtual bool ReadBool(bool &value) = 0;
    virtual bool WriteInt32(int32_t value) = 0;
    virtual bool ReadInt32(int32_t &value) = 0;
    virtual bool WriteInt64(int64_t value) = 0;
    virtual bool ReadInt64(int64_t &value) = 0;
    virtual bool WriteUint32(uint32_t value) = 0;
    virtual bool ReadUint32(uint32_t &value) = 0;
    virtual bool WriteString(const std::string &value) = 0;
    virtual bool ReadString(std::string &value) = 0;
    virtual bool WriteStringVector(const std::vector<std::string> &value) = 0;
    virtual bool ReadStringVector(std::vector<std::string> *value) = 0;

    static inline std::shared_ptr<IParcel> parcel_{nullptr};
};

class ParcelMock : public IParcel {
public:
    MOCK_METHOD1(WriteBool, bool(bool value));
    MOCK_METHOD1(ReadBool, bool(bool &value));
    MOCK_METHOD1(WriteInt32, bool(int32_t value));
    MOCK_METHOD1(ReadInt32, bool(int32_t &value));
    MOCK_METHOD1(WriteInt64, bool(int64_t value));
    MOCK_METHOD1(ReadInt64, bool(int64_t &value));
    MOCK_METHOD1(WriteUint32, bool(uint32_t value));
    MOCK_METHOD1(ReadUint32, bool(uint32_t &value));
    MOCK_METHOD1(WriteString, bool(const std::string &value));
    MOCK_METHOD1(ReadString, bool(std::string &value));
    MOCK_METHOD1(WriteStringVector, bool(const std::vector<std::string> &value));
    MOCK_METHOD1(ReadStringVector, bool(std::vector<std::string> *value));
};

class Parcel {
public:
    Parcel() = default;
    ~Parcel() = default;
    bool WriteBool(bool value);
    bool ReadBool(bool &value);
    bool WriteInt32(int32_t value);
    bool ReadInt32(int32_t &value);
    bool WriteInt64(int64_t value);
    bool ReadInt64(int64_t &value);
    bool WriteUint32(uint32_t value);
    bool ReadUint32(uint32_t &value);
    bool WriteString(const std::string &value);
    bool ReadString(std::string &value);
    bool WriteStringVector(const std::vector<std::string> &value);
    bool ReadStringVector(std::vector<std::string> *value);
};

class Parcelable {
public:
    Parcelable() = default;
    virtual ~Parcelable() = default;
    virtual bool Marshalling(Parcel &parcel) const = 0;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // MOCK_PARCEL_H
