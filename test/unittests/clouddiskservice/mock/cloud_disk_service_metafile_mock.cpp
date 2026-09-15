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
#include "cloud_disk_service_metafile.h"

namespace OHOS::FileManagement::CloudDiskService {
#ifndef E_OK
#define E_OK 0
#endif

namespace {
int32_t placeholderLookupResult = E_OK;
uint8_t placeholderLookupState = 0;
int32_t placeholderUpdateResult = E_OK;
uint32_t placeholderUpdateCount = 0;
uint8_t lastPlaceholderUpdateState = 0;
std::string lastPlaceholderUpdateName;
bool createResultOverridden = false;
int32_t createResult = E_OK;
uint8_t lastCreatePlaceholderState = 0;
bool removeResultOverridden = false;
int32_t removeResult = E_OK;
bool renameOldResultOverridden = false;
int32_t renameOldResult = E_OK;
bool renameNewResultOverridden = false;
int32_t renameNewResult = E_OK;
uint8_t lastRenameNewPlaceholderState = 0;
} // namespace

void ResetPlaceholderMetaFileMock()
{
    placeholderLookupResult = E_OK;
    placeholderLookupState = 0;
    placeholderUpdateResult = E_OK;
    placeholderUpdateCount = 0;
    lastPlaceholderUpdateState = 0;
    lastPlaceholderUpdateName.clear();
    createResultOverridden = false;
    createResult = E_OK;
    lastCreatePlaceholderState = 0;
    removeResultOverridden = false;
    removeResult = E_OK;
    renameOldResultOverridden = false;
    renameOldResult = E_OK;
    renameNewResultOverridden = false;
    renameNewResult = E_OK;
    lastRenameNewPlaceholderState = 0;
}

void SetPlaceholderMetaFileLookupResult(int32_t result, uint8_t state)
{
    placeholderLookupResult = result;
    placeholderLookupState = state;
}

void SetPlaceholderMetaFileUpdateResult(int32_t result)
{
    placeholderUpdateResult = result;
}

uint32_t GetPlaceholderMetaFileUpdateCount()
{
    return placeholderUpdateCount;
}

uint8_t GetLastPlaceholderMetaFileUpdateState()
{
    return lastPlaceholderUpdateState;
}

std::string GetLastPlaceholderMetaFileUpdateName()
{
    return lastPlaceholderUpdateName;
}

void SetMetaFileCreateResult(int32_t result)
{
    createResultOverridden = true;
    createResult = result;
}

uint8_t GetLastMetaFileCreatePlaceholderState()
{
    return lastCreatePlaceholderState;
}

void SetMetaFileRemoveResult(int32_t result)
{
    removeResultOverridden = true;
    removeResult = result;
}

void SetMetaFileRenameOldResult(int32_t result)
{
    renameOldResultOverridden = true;
    renameOldResult = result;
}

void SetMetaFileRenameNewResult(int32_t result)
{
    renameNewResultOverridden = true;
    renameNewResult = result;
}

uint8_t GetLastMetaFileRenameNewPlaceholderState()
{
    return lastRenameNewPlaceholderState;
}

CloudDiskServiceMetaFile::CloudDiskServiceMetaFile(const int32_t userId,
                                                   const uint32_t syncFolderIndex,
                                                   const uint64_t inode)
{
    userId_ = userId;
}

MetaFileMgr &MetaFileMgr::GetInstance()
{
    static MetaFileMgr instance_;
    return instance_;
}

std::shared_ptr<CloudDiskServiceMetaFile> MetaFileMgr::GetCloudDiskServiceMetaFile(
    int32_t userId, const uint32_t syncFolderIndex, const uint64_t inode)
{
    std::shared_ptr<CloudDiskServiceMetaFile> metaFile =
        std::make_shared<CloudDiskServiceMetaFile>(userId, syncFolderIndex, inode);
    return metaFile;
}

int32_t CloudDiskServiceMetaFile::DoRemove(const MetaBase &base,
                                           std::string &recordId,
                                           unsigned long &bidx,
                                           uint32_t &bitPos)
{
    if (removeResultOverridden) {
        return removeResult;
    }
    if (recordId == "") {
        return E_OK;
    } else {
        return 1;
    }
}

int32_t CloudDiskServiceMetaFile::DoRenameOld(const MetaBase &base,
                                              std::string &recordId,
                                              unsigned long &bidx,
                                              uint32_t &bitPos)
{
    if (renameOldResultOverridden) {
        return renameOldResult;
    }
    if (recordId == "") {
        return E_OK;
    } else {
        return 1;
    }
}

int32_t CloudDiskServiceMetaFile::DoRenameNew(const MetaBase &base,
                                              std::string &recordId,
                                              unsigned long &bidx,
                                              uint32_t &bitPos)
{
    lastRenameNewPlaceholderState = base.placeholder;
    if (renameNewResultOverridden) {
        return renameNewResult;
    }
    if (recordId == "") {
        return E_OK;
    } else {
        return 1;
    }
}

int32_t MetaFileMgr::GetRelativePath(const std::shared_ptr<CloudDiskServiceMetaFile> metaFile, std::string &path)
{
    if (metaFile->userId_ == 1) {
        return E_OK;
    } else {
        return -1;
    }
}

int32_t CloudDiskServiceMetaFile::DoUpdate(const MetaBase &base,
                                           std::string &recordId,
                                           unsigned long &bidx,
                                           uint32_t &bitPos)
{
    if (recordId == "") {
        return E_OK;
    } else {
        return 1;
    }
}

int32_t CloudDiskServiceMetaFile::GenericDentryHeader()
{
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoCreate(const MetaBase &base, unsigned long &bidx, uint32_t &bitPos)
{
    lastCreatePlaceholderState = base.placeholder;
    if (createResultOverridden) {
        return createResult;
    }
    if (base.name == "") {
        return E_OK;
    } else {
        return 1;
    }
}

int32_t CloudDiskServiceMetaFile::DoLookupByRecordId(MetaBase &base, uint8_t revalidate)
{
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoLookupByOffset(MetaBase &base, unsigned long bidx, uint32_t bitPos)
{
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoLookupPlaceholderByName(const MetaBase &base, uint8_t &placeholderState)
{
    (void)base;
    placeholderState = placeholderLookupState;
    return placeholderLookupResult;
}

int32_t CloudDiskServiceMetaFile::DoUpdatePlaceholderState(const MetaBase &base, uint8_t placeholderState)
{
    ++placeholderUpdateCount;
    lastPlaceholderUpdateName = base.name;
    lastPlaceholderUpdateState = placeholderState;
    return placeholderUpdateResult;
}

void MetaFileMgr::CloudDiskServiceClearAll() {}
} // namespace OHOS::FileManagement::CloudDiskService
