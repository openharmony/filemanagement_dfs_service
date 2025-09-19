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

CloudDiskServiceMetaFile::CloudDiskServiceMetaFile(const int32_t userId, const uint32_t syncFolderIndex,
                                                   const uint64_t inode)
{
    userId_ = userId;
}

MetaFileMgr& MetaFileMgr::GetInstance()
{
    static MetaFileMgr instance_;
    return instance_;
}

std::shared_ptr<CloudDiskServiceMetaFile> MetaFileMgr::GetCloudDiskServiceMetaFile(int32_t userId,
    const uint32_t syncFolderIndex, const uint64_t inode)
{
    std::shared_ptr<CloudDiskServiceMetaFile> metaFile =
        std::make_shared<CloudDiskServiceMetaFile>(userId, syncFolderIndex, inode);
    return metaFile;
}

int32_t CloudDiskServiceMetaFile::DoRemove(const MetaBase &base, std::string &recordId, unsigned long &bidx,
                                           uint32_t &bitPos)
{
    if (recordId == "") {
        return E_OK;
    } else {
        return 1;
    }
}

int32_t CloudDiskServiceMetaFile::DoRenameOld(const MetaBase &base, std::string &recordId, unsigned long &bidx,
                                              uint32_t &bitPos)
{
    if (recordId == "") {
        return E_OK;
    } else {
        return 1;
    }
}

int32_t CloudDiskServiceMetaFile::DoRenameNew(const MetaBase &base, std::string &recordId, unsigned long &bidx,
                                              uint32_t &bitPos)
{
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

int32_t CloudDiskServiceMetaFile::DoUpdate(const MetaBase &base, std::string &recordId, unsigned long &bidx,
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

void MetaFileMgr::CloudDiskServiceClearAll()
{
}
}