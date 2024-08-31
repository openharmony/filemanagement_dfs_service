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
#include "base_interface_lib_mock.h"

namespace OHOS {
namespace FileManagement {
string GetDentryfileByPath(uint32_t userId, const string &path, bool caseSense)
{
    if (BaseInterfaceLib::baseInterfaceLib_ == nullptr) {
        if (path == "") {
            caseSense = true;
            return "";
        }
        caseSense = false;
        return "";
    }

    return BaseInterfaceLib::baseInterfaceLib_->GetDentryfileByPath(userId, path, caseSense);
}

string GetDentryfileName(const std::string &path, bool caseSense)
{
    if (BaseInterfaceLib::baseInterfaceLib_ == nullptr) {
        return "";
    }

    return BaseInterfaceLib::baseInterfaceLib_->GetDentryfileName(path, caseSense);
}

uint32_t GetOverallBucket(uint32_t level)
{
    if (BaseInterfaceLib::baseInterfaceLib_ == nullptr) {
        if (level >= MAX_BUCKET_LEVEL) {
            return E_SUCCESS;
        }
        return E_SUCCESS;
    }

    return BaseInterfaceLib::baseInterfaceLib_->GetOverallBucket(level);
}

uint32_t GetBucketaddr(uint32_t level, uint32_t buckoffset)
{
    if (BaseInterfaceLib::baseInterfaceLib_ == nullptr) {
        if (level >= MAX_BUCKET_LEVEL) {
            return E_SUCCESS;
        }

        uint64_t curLevelMaxBucks = level;
        if (buckoffset >= curLevelMaxBucks) {
            return E_FAIL;
        }

        return MAX_BUCKET_LEVEL;
    }

    return BaseInterfaceLib::baseInterfaceLib_->GetBucketaddr(level, buckoffset);
}

uint32_t GetBucketByLevel(uint32_t level)
{
    if (BaseInterfaceLib::baseInterfaceLib_ == nullptr) {
        if (level >= MAX_BUCKET_LEVEL) {
            return E_SUCCESS;
        } else {
            return MAX_BUCKET_LEVEL;
        }
    }

    return BaseInterfaceLib::baseInterfaceLib_->GetBucketByLevel(level);
}

uint32_t RoomForFilename(const uint8_t bitmap[], size_t slots, uint32_t maxSlots)
{
    if (BaseInterfaceLib::baseInterfaceLib_ == nullptr) {
        if (maxSlots == E_SUCCESS) {
            return E_SUCCESS;
        } else {
            return MAX_BUCKET_LEVEL;
        }
    }

    return BaseInterfaceLib::baseInterfaceLib_->RoomForFilename(bitmap, slots, maxSlots);
}

uint32_t FindNextZeroBit(const uint8_t addr[], uint32_t maxSlots, uint32_t start)
{
    if (BaseInterfaceLib::baseInterfaceLib_ == nullptr) {
        if (maxSlots == DENTRY_PER_GROUP) {
            return DENTRY_PER_GROUP;
        } else {
            return MAX_BUCKET_LEVEL;
        }
    }

    return BaseInterfaceLib::baseInterfaceLib_->FindNextZeroBit(addr, maxSlots, start);
}

uint32_t FindNextBit(const uint8_t addr[], uint32_t maxSlots, uint32_t start)
{
    if (BaseInterfaceLib::baseInterfaceLib_ == nullptr) {
        if (maxSlots == DENTRY_PER_GROUP) {
            return DENTRY_PER_GROUP;
        } else {
            return MAX_BUCKET_LEVEL;
        }
    }

    return BaseInterfaceLib::baseInterfaceLib_->FindNextBit(addr, maxSlots, start);
}

int32_t DoLookup(MetaBase &base)
{
    return E_SUCCESS;
}

int32_t DoCreate(const MetaBase &base)
{
    return E_SUCCESS;
}

int32_t DoLookupAndRemove(MetaBase &metaBase)
{
    return E_SUCCESS;
}
} // FileManagement
} // OHOS