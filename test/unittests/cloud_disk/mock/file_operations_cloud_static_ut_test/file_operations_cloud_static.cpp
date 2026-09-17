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

#include "file_operations_cloud_static.h"

#include "dfs_error.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace OHOS::NativeRdb;

/* ---------------- FileOperationsHelper ---------------- */
int32_t FileOperationsHelper::GetNextLayer(std::shared_ptr<CloudDiskInode> inoPtr, fuse_ino_t parent)
{
    return 0;
}

std::shared_ptr<CloudDiskInode> FileOperationsHelper::FindCloudDiskInode(struct CloudDiskFuseData *data, int64_t key)
{
    if (FileOperationsHelperMock::proxy_ != nullptr) {
        return FileOperationsHelperMock::proxy_->FindCloudDiskInode(data, key);
    }
    return nullptr;
}

std::shared_ptr<CloudDiskFile> FileOperationsHelper::FindCloudDiskFile(struct CloudDiskFuseData *data, int64_t key)
{
    return nullptr;
}

int64_t FileOperationsHelper::FindLocalId(struct CloudDiskFuseData *data, const std::string &key)
{
    return 0;
}

void FileOperationsHelper::GetInodeAttr(std::shared_ptr<CloudDiskInode> ino, struct stat *statBuf)
{
    return;
}

void FileOperationsHelper::PutCloudDiskFile(struct CloudDiskFuseData *data,
    std::shared_ptr<CloudDiskFile> filePtr, int64_t key)
{
    return;
}

int32_t FileOperationsHelper::GetFixedLayerRootId(int32_t layer)
{
    return 0;
}

std::string FileOperationsHelper::GetCloudDiskRootPath(int32_t userId)
{
    return "";
}

std::string FileOperationsHelper::GetCloudDiskLocalPath(int32_t userId, std::string fileName)
{
    return "";
}

void FileOperationsHelper::AddDirEntry(fuse_req_t req, std::string &buf, size_t &size, const char *name,
    std::shared_ptr<CloudDiskInode> ino)
{
    return;
}

void FileOperationsHelper::FuseReplyLimited(fuse_req_t req, const char *buf, size_t bufSize,
    off_t off, size_t maxSize)
{
    return;
}

void FileOperationsHelper::PutCloudDiskInode(struct CloudDiskFuseData *data,
    std::shared_ptr<CloudDiskInode> inoPtr, uint64_t num, int64_t key)
{
    return;
}

void FileOperationsHelper::PutLocalId(struct CloudDiskFuseData *data,
    std::shared_ptr<CloudDiskInode> inoPtr, const std::string &key)
{
    return;
}

std::shared_ptr<CloudDiskInode> FileOperationsHelper::GenerateCloudDiskInode(struct CloudDiskFuseData *data,
    fuse_ino_t parent, const std::string &fileName, const std::string &path)
{
    return nullptr;
}

/* ---------------- DatabaseManager ---------------- */
DatabaseManager &DatabaseManager::GetInstance()
{
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::IsBundleCloudSyncEnabled(int32_t userId, const std::string &bundleName)
{
    if (DatabaseManagerMock::proxy_ != nullptr) {
        return DatabaseManagerMock::proxy_->IsBundleCloudSyncEnabled(userId, bundleName);
    }
    return false;
}

std::shared_ptr<CloudDiskRdbStore> DatabaseManager::GetRdbStore(const std::string &bundleName, int32_t userId)
{
    if (DatabaseManagerMock::proxy_ != nullptr) {
        return DatabaseManagerMock::proxy_->GetRdbStore(bundleName, userId);
    }
    return nullptr;
}

void DatabaseManager::ClearRdbStore()
{
    return;
}

/* ---------------- CloudDiskRdbStore ---------------- */
CloudDiskRdbStore::CloudDiskRdbStore(const std::string &bundleName, const int32_t &userId)
    : bundleName_(bundleName), userId_(userId)
{
    RdbInit();
}

CloudDiskRdbStore::~CloudDiskRdbStore()
{
    Stop();
}

int32_t CloudDiskRdbStore::ReBuildDatabase(const string &databasePath)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::RdbInit()
{
    return E_OK;
}

void CloudDiskRdbStore::Stop()
{
    if (rdbStore_ == nullptr) {
        return;
    }
    rdbStore_ = nullptr;
}

shared_ptr<RdbStore> CloudDiskRdbStore::GetRaw()
{
    return rdbStore_;
}

void CloudDiskRdbStore::DatabaseRestore()
{
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
    const std::string &cloudId, const struct stat *attr, const int valid)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::ReadDir(const std::string &cloudId, vector<CloudDiskFileInfo> &infos)
{
    if (CloudDiskRdbStoreMock::proxy_ != nullptr) {
        return CloudDiskRdbStoreMock::proxy_->ReadDir(cloudId, infos);
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::Create(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &fileName, bool noNeedUpload)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::MkDir(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &directoryName, bool noNeedUpload)
{
    return E_OK;
}

void CloudDiskRdbStore::TriggerSyncForWrite(const std::string &fileName, const std::string &parentCloudId)
{
}

int32_t CloudDiskRdbStore::Write(const std::string &fileName, const std::string &parentCloudId,
    const std::string &cloudId, bool isWrite)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::LocationSetXattr(const std::string &name, const std::string &parentCloudId,
    const std::string &cloudId, const std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetRowId(const std::string &cloudId, int64_t &rowId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GenerateNewRowId(const std::string &cloudId, const std::string &fileName, int64_t &rowId,
                                            const std::string &parentCloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetParentCloudId(const std::string &cloudId, std::string &parentCloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::RecycleSetXattr(std::string &name, const std::string &parentCloudId,
    const std::string &cloudId, const std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::FavoriteSetXattr(const std::string &cloudId, const std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::LocationGetXattr(const std::string &name, const std::string &key, std::string &value,
    const std::string &parentCloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::FavoriteGetXattr(const std::string &cloudId, const std::string &key, std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::FileStatusGetXattr(const std::string &cloudId, const std::string &key, std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetExtAttrValue(const std::string &cloudId, const std::string &key, std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetExtAttr(const std::string &cloudId, std::string &value, int32_t &position,
    int32_t &dirtyType)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetXAttr(const std::string &cloudId, const std::string &key, std::string &value,
    const CacheNode &node, const std::string &extAttrKey)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::ExtAttributeSetXattr(const std::string &cloudId, const std::string &value,
    const std::string &key)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::SetXAttr(const std::string &cloudId, const std::string &key, const std::string &value,
    std::string &name, const std::string &parentCloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::Rename(const std::string &oldParentCloudId, const std::string &oldFileName,
    const std::string &newParentCloudId, const std::string &newFileName, bool newFileNoNeedUpload,
    bool needSyncAndNotify)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetHasChild(const std::string &cloudId, bool &hasChild)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::UnlinkSynced(const std::string &cloudId, const int64_t &visitTime)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::UnlinkLocal(const std::string &cloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::Unlink(const std::string &cloudId, const int32_t &position, const int64_t &visitTime)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetDirtyType(const std::string &cloudId, int32_t &fileStatus)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetCurNode(const std::string &cloudId, CacheNode &curNode)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetParentNode(const std::string &parentCloudId, std::string &nextCloudId,
    std::string &fileName)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetUriFromDB(const std::string &parentCloudId, std::string &uri)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetNotifyUri(const CacheNode &cacheNode, std::string &uri)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetNotifyData(const CacheNode &cacheNode, NotifyData &notifyData)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::UpdateTHMStatus(shared_ptr<CloudDiskMetaFile> metaFile,
    MetaBase &metaBase, int32_t status, const string &filePath)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetSrcCloudId(const std::string &cloudId, std::string &srcCloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::CopyFile(std::string srcCloudId, std::string destCloudId, std::string bundleName,
                                    const int userId, std::string destPath)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::CheckRootIdValid()
{
    return E_OK;
}

int32_t CloudDiskDataCallBack::OnCreate(RdbStore &store)
{
    return E_OK;
}

int32_t CloudDiskDataCallBack::OnUpgrade(RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetSourcePathFromAttr(const std::string &cloudId, std::string &sourcePath)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::HandleRestore(string &name, const string &parentCloudId,
    const string &cloudId, string &newName, int32_t &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::HandleRecycle(const string &name, const string &parentCloudId,
    const string &cloudId, int32_t &value)
{
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudDisk
