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

#include "clouddisk_rdbstore.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace OHOS::NativeRdb;

CloudDiskRdbStore::CloudDiskRdbStore(const std::string &bundleName, const int32_t &userId)
    : bundleName_(bundleName), userId_(userId)
{}

CloudDiskRdbStore::~CloudDiskRdbStore()
{}

int32_t CloudDiskRdbStore::RdbInit()
{
    return E_OK;
}

void CloudDiskRdbStore::Stop()
{
    rdbStore_ = nullptr;
}

shared_ptr<RdbStore> CloudDiskRdbStore::GetRaw()
{
    return rdbStore_;
}

int32_t CloudDiskRdbStore::LookUp(const std::string &parentCloudId,
    const std::string &fileName, CloudDiskFileInfo &info)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetAttr(const std::string &cloudId, CloudDiskFileInfo &info)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::SetAttr(const std::string &fileName, const std::string &parentCloudId,
    const std::string &cloudId, const unsigned long long &size)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::ReadDir(const std::string &cloudId, vector<CloudDiskFileInfo> &infos)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::Create(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &fileName)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::MkDir(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &directoryName)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::Write(const std::string &fileName, const std::string &parentCloudId,
    const std::string &cloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetXAttr(const std::string &cloudId, const std::string &key, std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::SetXAttr(const std::string &cloudId, const std::string &key, const std::string &value,
    const std::string &name, const std::string &parentCloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::Rename(const std::string &oldParentCloudId, const std::string &oldFileName,
    const std::string &newParentCloudId, const std::string &newFileName)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::UnlinkSynced(const std::string &cloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::UnlinkLocal(const std::string &cloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::Unlink(const std::string &cloudId, const int32_t &position)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetHasChild(const std::string &cloudId, bool &hasChild)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetRowId(const std::string &cloudId, int64_t &rowId)
{
    return E_OK;
}

int32_t CloudDiskDataCallBack::OnCreate(RdbStore &store)
{
    return NativeRdb::E_OK;
}

int32_t CloudDiskDataCallBack::OnUpgrade(RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    return NativeRdb::E_OK;
}
}