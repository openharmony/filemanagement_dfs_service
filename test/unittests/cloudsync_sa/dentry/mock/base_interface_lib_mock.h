/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef BASE_INTERFACE_LIB_MOCK_H
#define BASE_INTERFACE_LIB_MOCK_H

#include <fcntl.h>
#include <gmock/gmock.h>
#include "meta_file.h"

namespace OHOS {
namespace FileManagement {
using namespace testing;
using namespace testing::ext;
using namespace std;

class BaseInterfaceLib {
public:
    virtual ~BaseInterfaceLib() = default;
    virtual string GetDentryfileByPath(uint32_t userId, const string &path, bool caseSense) = 0;
    virtual string GetDentryfileName(const std::string &path, bool caseSense) = 0;
    virtual uint32_t GetOverallBucket(uint32_t level) = 0;
    virtual uint32_t GetBucketaddr(uint32_t level, uint32_t buckoffset) = 0;
    virtual uint32_t GetBucketByLevel(uint32_t level) = 0;
    virtual uint32_t RoomForFilename(const uint8_t bitmap[], size_t slots, uint32_t maxSlots) = 0;
    virtual uint32_t FindNextZeroBit(const uint8_t addr[], uint32_t maxSlots, uint32_t start) = 0;
    virtual uint32_t FindNextBit(const uint8_t addr[], uint32_t maxSlots, uint32_t start) = 0;
    virtual int32_t DoLookup(MetaBase &base) = 0;
    virtual int32_t DoCreate(const MetaBase &base) = 0;
    virtual int32_t DoLookupAndRemove(MetaBase &metaBase) = 0;
public:
    static inline shared_ptr<BaseInterfaceLib> baseInterfaceLib_ = nullptr;
};

class InterfaceLibMock : public BaseInterfaceLib {
public:
    MOCK_METHOD3(GetDentryfileByPath, string(uint32_t userId, const string &path, bool caseSense));
    MOCK_METHOD2(GetDentryfileName, string(const string &path, bool caseSense));
    MOCK_METHOD1(GetOverallBucket, uint32_t(uint32_t level));
    MOCK_METHOD2(GetBucketaddr, uint32_t(uint32_t level, uint32_t buckoffset));
    MOCK_METHOD1(GetBucketByLevel, uint32_t(uint32_t level));
    MOCK_METHOD3(RoomForFilename, uint32_t(const uint8_t bitmap[], size_t slots, uint32_t maxSlot));
    MOCK_METHOD3(FindNextZeroBit, uint32_t(const uint8_t addr[], uint32_t maxSlots, uint32_t start));
    MOCK_METHOD3(FindNextBit, uint32_t(const uint8_t addr[], uint32_t maxSlots, uint32_t start));
    MOCK_METHOD1(DoLookup, int32_t(MetaBase &base));
    MOCK_METHOD1(DoCreate, int32_t(const MetaBase &base));
    MOCK_METHOD1(DoLookupAndRemove, int32_t(MetaBase &metaBase));
};

enum ErrStatus {
    E_FAIL = -1,
    E_SUCCESS = 0,
    E_EINVAL = 22,
    DENTRY_PER_GROUP = 60,
    MAX_BUCKET_LEVEL = 63
};
} // FileManagement
} // OHOS
#endif  // BASE_INTERFACE_LIB_MOCK_H