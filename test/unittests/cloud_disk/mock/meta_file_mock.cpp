/*
 * Copyright (c) 2026. Huawei Device Co., Ltd.
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

#include "meta_file.h"

#include "dfs_error.h"

namespace OHOS {
namespace FileManagement {
MetaFileMgr& MetaFileMgr::GetInstance()
{
    static MetaFileMgr instance_;
    return instance_;
}
void MetaFileMgr::CloudDiskClearAll()
{
}

CloudDiskMetaFile::CloudDiskMetaFile(uint32_t userId, const std::string &bundleName, const std::string &cloudId)
{
}

std::string CloudDiskMetaFile::GetDentryFilePath()
{
    return "";
}

int32_t CloudDiskMetaFile::DoLookupAndCreate(const std::string &name, CloudDiskMetaFileCallBack metaFileCallBack)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoLookupAndUpdate(const std::string &name, CloudDiskMetaFileCallBack callback)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoChildUpdate(const std::string &name, CloudDiskMetaFileCallBack callback)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoLookupAndRemove(MetaBase &metaBase)
{
    return E_OK;
}

CloudDiskMetaFile::~CloudDiskMetaFile()
{
}

int32_t CloudDiskMetaFile::HandleFileByFd(unsigned long &endBlock, uint32_t &level)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::GetCreateInfo(const MetaBase &base, uint32_t &bitPos, uint32_t &namehash,
    unsigned long &bidx, struct HmdfsDentryGroup &dentryBlk)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoCreate(const MetaBase &base)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoRemove(const MetaBase &base)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoLookup(MetaBase &base)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoUpdate(const MetaBase &base)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::DoRename(MetaBase &metaBase, const std::string &newName,
    std::shared_ptr<CloudDiskMetaFile> newMetaFile)
{
    return E_OK;
}

int32_t CloudDiskMetaFile::LoadChildren(std::vector<MetaBase> &bases)
{
    return E_OK;
}
} // namespace FileManagement
} // namespace OHOS