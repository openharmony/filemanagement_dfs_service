/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "WarmupMetaFileMock.h"

#include <cerrno>
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <utility>

#include "cloud_disk_service_error.h"
#include "cloud_disk_service_metafile.h"
#include "convertor.h"

namespace OHOS::FileManagement::CloudDiskService {
namespace {
using MetaKey = std::pair<uint32_t, uint64_t>;

constexpr uint32_t WARMUP_RECORD_HASH = 42;
std::map<MetaKey, std::shared_ptr<CloudDiskServiceMetaFile>> g_metaFiles;
std::set<uint64_t> g_existingInodes;
std::deque<int32_t> g_lookupByNameResults;
int32_t g_genericDentryResult = E_OK;
uint32_t g_genericDentryCalls = 0;
uint32_t g_lookupByNameCalls = 0;
uint32_t g_createMetaFileCalls = 0;
} // namespace

namespace Test {
void ResetWarmupMetaFileMock()
{
    g_metaFiles.clear();
    g_existingInodes.clear();
    g_lookupByNameResults.clear();
    g_genericDentryResult = E_OK;
    g_genericDentryCalls = 0;
    g_lookupByNameCalls = 0;
    g_createMetaFileCalls = 0;
}

void SetWarmupMetaFileExists(uint64_t inode, bool exists)
{
    if (exists) {
        g_existingInodes.insert(inode);
    } else {
        g_existingInodes.erase(inode);
    }
}

void SetWarmupGenericDentryResult(int32_t result)
{
    g_genericDentryResult = result;
}

void SetWarmupLookupByNameResults(const std::vector<int32_t> &results)
{
    g_lookupByNameResults.assign(results.begin(), results.end());
}

uint32_t GetWarmupGenericDentryCalls()
{
    return g_genericDentryCalls;
}

uint32_t GetWarmupLookupByNameCalls()
{
    return g_lookupByNameCalls;
}

uint32_t GetWarmupCreateMetaFileCalls()
{
    return g_createMetaFileCalls;
}
} // namespace Test

CloudDiskServiceMetaFile::CloudDiskServiceMetaFile(const int32_t userId,
                                                   const uint32_t syncFolderIndex,
                                                   const uint64_t inode)
    : CloudDiskServiceMetaFile(userId, syncFolderIndex, inode, true)
{
}

CloudDiskServiceMetaFile::CloudDiskServiceMetaFile(const int32_t userId,
                                                   const uint32_t syncFolderIndex,
                                                   const uint64_t inode,
                                                   bool createIfMissing)
    : selfInode_(Convertor::ConvertToHex(inode)),
      syncFolderIndex_(Convertor::ConvertToHex(syncFolderIndex)),
      userId_(userId)
{
    (void)createIfMissing;
}

bool CloudDiskServiceMetaFile::IsValid() const
{
    return true;
}

int32_t CloudDiskServiceMetaFile::DoCreate(const MetaBase &base, unsigned long &bidx, uint32_t &bitPos)
{
    (void)base;
    bidx = 0;
    bitPos = 0;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoRemove(const MetaBase &base,
                                           std::string &recordId,
                                           unsigned long &bidx,
                                           uint32_t &bitPos)
{
    (void)base;
    recordId = "record-id";
    bidx = 0;
    bitPos = 0;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoUpdate(const MetaBase &base,
                                           std::string &recordId,
                                           unsigned long &bidx,
                                           uint32_t &bitPos)
{
    return DoRemove(base, recordId, bidx, bitPos);
}

int32_t CloudDiskServiceMetaFile::DoRenameOld(const MetaBase &base,
                                              std::string &recordId,
                                              unsigned long &bidx,
                                              uint32_t &bitPos)
{
    return DoRemove(base, recordId, bidx, bitPos);
}

int32_t CloudDiskServiceMetaFile::DoRenameNew(const MetaBase &base,
                                              std::string &recordId,
                                              unsigned long &bidx,
                                              uint32_t &bitPos)
{
    return DoRemove(base, recordId, bidx, bitPos);
}

int32_t CloudDiskServiceMetaFile::DoLookupByName(MetaBase &base)
{
    ++g_lookupByNameCalls;
    int32_t result = E_OK;
    if (!g_lookupByNameResults.empty()) {
        result = g_lookupByNameResults.front();
        g_lookupByNameResults.pop_front();
    }
    if (result == E_OK) {
        base.recordId = "record-id";
        base.hash = WARMUP_RECORD_HASH;
    }
    return result;
}

int32_t CloudDiskServiceMetaFile::DoLookupByRecordId(MetaBase &base, uint8_t revalidate)
{
    (void)base;
    (void)revalidate;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoLookupByOffset(MetaBase &base, unsigned long bidx, uint32_t bitPos)
{
    (void)base;
    (void)bidx;
    (void)bitPos;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::DoLookupPlaceholderByName(const MetaBase &base, uint8_t &placeholderState)
{
    (void)base;
    placeholderState = 0;
    return E_OK;
}

int32_t CloudDiskServiceMetaFile::GenericDentryHeader()
{
    ++g_genericDentryCalls;
    return g_genericDentryResult;
}

MetaFileMgr &MetaFileMgr::GetInstance()
{
    static MetaFileMgr instance;
    return instance;
}

std::shared_ptr<CloudDiskServiceMetaFile> MetaFileMgr::GetCloudDiskServiceMetaFile(const int32_t userId,
                                                                                    const uint32_t syncFolderIndex,
                                                                                    const uint64_t inode)
{
    ++g_createMetaFileCalls;
    MetaKey key(syncFolderIndex, inode);
    auto &metaFile = g_metaFiles[key];
    if (metaFile == nullptr) {
        metaFile = std::make_shared<CloudDiskServiceMetaFile>(userId, syncFolderIndex, inode);
    }
    return metaFile;
}

std::shared_ptr<CloudDiskServiceMetaFile> MetaFileMgr::GetCloudDiskServiceMetaFileIfExists(const int32_t userId,
                                                                                          const uint32_t syncFolderIndex,
                                                                                          const uint64_t inode)
{
    if (g_existingInodes.find(inode) == g_existingInodes.end()) {
        return nullptr;
    }
    return GetCloudDiskServiceMetaFile(userId, syncFolderIndex, inode);
}

int32_t MetaFileMgr::GetRelativePath(const std::shared_ptr<CloudDiskServiceMetaFile> metaFile, std::string &path)
{
    (void)metaFile;
    path = "/";
    return E_OK;
}

void MetaFileMgr::CloudDiskServiceClearAll()
{
    g_metaFiles.clear();
}

} // namespace OHOS::FileManagement::CloudDiskService
