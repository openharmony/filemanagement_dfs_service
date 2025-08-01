/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "clouddisk_notify.h"
#include "clouddisk_notify_utils.h"
#include "clouddisk_rdbstore.h"
#include "dfs_error.h"
#include "ffrt_inner.h"
#include "file_column.h"
#include "securec.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk {
const string BUNDLENAME_FLAG = "<BundleName>";
const string CLOUDDISK_URI_PREFIX = "file://<BundleName>/data/storage/el2/cloud";
const string BACKFLASH = "/";
const string RECYCLE_BIN = ".trash";
constexpr uint32_t MAX_NOTIFY_LIST_SIZE = 32;
constexpr size_t MNOTIFY_TIME_INTERVAL = 500;
static pair<string, shared_ptr<CloudDiskRdbStore>> cacheRdbStore("", nullptr);
std::mutex cacheRdbMutex;

CloudDiskNotify &CloudDiskNotify::GetInstance()
{
    static CloudDiskNotify instance_;
    return instance_;
}

static shared_ptr<CloudDiskRdbStore> GetRdbStore(const string &bundleName, int32_t userId)
{
    std::lock_guard<std::mutex> lock(cacheRdbMutex);
    string storeKey = bundleName + to_string(userId);
    if (cacheRdbStore.first == storeKey) {
        return cacheRdbStore.second;
    }
    cacheRdbStore.first = storeKey;
    cacheRdbStore.second = make_shared<CloudDiskRdbStore>(bundleName, userId);
    return cacheRdbStore.second;
}

static int32_t GetTrashNotifyData(const NotifyParamDisk &paramDisk, NotifyData &notifyData)
{
    if (paramDisk.inoPtr == nullptr) {
        return E_INVAL_ARG;
    }
    string realPrefix = CLOUDDISK_URI_PREFIX;
    realPrefix.replace(CLOUDDISK_URI_PREFIX.find(BUNDLENAME_FLAG), BUNDLENAME_FLAG.length(),
                       paramDisk.inoPtr->bundleName);
    notifyData.uri = realPrefix + BACKFLASH + RECYCLE_BIN + BACKFLASH + paramDisk.inoPtr->fileName;
    return E_OK;
}

static int32_t GetTrashNotifyData(const CacheNode &cacheNode, const ParamServiceOther &paramOthers,
    NotifyData &notifyData)
{
    string realPrefix = CLOUDDISK_URI_PREFIX;
    realPrefix.replace(CLOUDDISK_URI_PREFIX.find(BUNDLENAME_FLAG), BUNDLENAME_FLAG.length(), paramOthers.bundleName);
    notifyData.uri = realPrefix + BACKFLASH + RECYCLE_BIN + BACKFLASH + cacheNode.fileName;
    return E_OK;
}

static int32_t TrashUriAddRowId(shared_ptr<CloudDiskRdbStore> rdbStore, const string &cloudId, string &uri)
{
    int64_t rowId = 0;
    int32_t ret = rdbStore->GetRowId(cloudId, rowId);
    if (ret != E_OK) {
        LOGE("Get rowId fail, ret: %{public}d", ret);
        return ret;
    }
    uri = uri + "_" + std::to_string(rowId);
    return E_OK;
}

static int32_t GetDataInner(const NotifyParamDisk &paramDisk, NotifyData &notifyData)
{
    int32_t ret;
    if (paramDisk.inoPtr != nullptr) {
        ret = CloudDiskNotifyUtils::GetNotifyData(paramDisk.data, paramDisk.func,
            paramDisk.inoPtr, notifyData);
    } else {
        ret = CloudDiskNotifyUtils::GetNotifyData(paramDisk.data, paramDisk.func,
            paramDisk.ino, notifyData);
    }
    if (ret != E_OK) {
        LOGI("GetNotifyData Not E_OK when GetDataInner ret = %{public}d", ret);
    }
    return ret;
}

static int32_t GetDataInnerWithName(const NotifyParamDisk &paramDisk, NotifyData &notifyData)
{
    if (paramDisk.name.empty()) {
        return E_INVAL_ARG;
    }
    int32_t ret;
    if (paramDisk.inoPtr != nullptr) {
        ret = CloudDiskNotifyUtils::GetNotifyData(paramDisk.data, paramDisk.func,
            paramDisk.inoPtr, paramDisk.name, notifyData);
    } else {
        ret = CloudDiskNotifyUtils::GetNotifyData(paramDisk.data, paramDisk.func,
            paramDisk.ino, paramDisk.name, notifyData);
    }
    if (ret != E_OK) {
        LOGI("GetNotifyData Not E_OK when GetDataInnerWithName ret = %{public}d", ret);
    }
    return ret;
}

static void HandleSetAttr(const NotifyParamDisk &paramDisk)
{
    NotifyData notifyData;
    if (GetDataInner(paramDisk, notifyData) != E_OK) {
        return;
    }
    notifyData.type = NotifyType::NOTIFY_MODIFIED;
    notifyData.isLocalOperation = true;
    CloudDiskNotify::GetInstance().AddNotify(notifyData);
}

static void HandleRecycleRestore(const NotifyParamDisk &paramDisk)
{
    NotifyData trashNotifyData;
    if (GetTrashNotifyData(paramDisk, trashNotifyData) != E_OK) {
        LOGE("Get trash notify data fail");
        return;
    }
    shared_ptr<CloudDiskRdbStore> rdbStore = GetRdbStore(paramDisk.inoPtr->bundleName, paramDisk.data->userId);
    if (rdbStore == nullptr) {
        LOGE("Get rdb store fail, bundleName: %{public}s", paramDisk.inoPtr->bundleName.c_str());
        return;
    }
    if (TrashUriAddRowId(rdbStore, paramDisk.inoPtr->cloudId, trashNotifyData.uri) != E_OK) {
        return;
    }

    NotifyData originNotifyData;
    if (GetDataInner(paramDisk, originNotifyData) != E_OK) {
        LOGE("Get origin notify data fail");
        return;
    }

    NotifyData notifyData;
    notifyData.type = NotifyType::NOTIFY_RENAMED;
    notifyData.isDir = originNotifyData.isDir;

    if (paramDisk.opsType == NotifyOpsType::DAEMON_RECYCLE) {
        notifyData.uri = trashNotifyData.uri;
        notifyData.extraUri = originNotifyData.uri;
    }
    if (paramDisk.opsType == NotifyOpsType::DAEMON_RESTORE) {
        notifyData.uri = originNotifyData.uri;
        notifyData.extraUri = trashNotifyData.uri;
    }
    notifyData.isLocalOperation = true;
    CloudDiskNotify::GetInstance().AddNotify(notifyData);
}

static void HandleWrite(const NotifyParamDisk &paramDisk, const ParamDiskOthers &paramOthers)
{
    NotifyData notifyData;
    if (GetDataInner(paramDisk, notifyData) != E_OK) {
        return;
    }

    if (paramOthers.dirtyType == static_cast<int32_t>(DirtyType::TYPE_NO_NEED_UPLOAD)) {
        notifyData.type = NotifyType::NOTIFY_ADDED;
    }
    if (paramOthers.isWrite) {
        notifyData.type = NotifyType::NOTIFY_FILE_CHANGED;
    }
    notifyData.isLocalOperation = true;
    CloudDiskNotify::GetInstance().AddNotify(notifyData);
}

static void HandleMkdir(const NotifyParamDisk &paramDisk)
{
    NotifyData notifyData;
    if (GetDataInnerWithName(paramDisk, notifyData) != E_OK) {
        return;
    }
    notifyData.type = NotifyType::NOTIFY_ADDED;
    notifyData.isDir = true;
    notifyData.isLocalOperation = true;
    CloudDiskNotify::GetInstance().AddNotify(notifyData);
}

static void HandleUnlink(const NotifyParamDisk &paramDisk)
{
    NotifyData notifyData;
    if (GetDataInnerWithName(paramDisk, notifyData) != E_OK) {
        return;
    }
    notifyData.type = NotifyType::NOTIFY_DELETED;
    notifyData.isDir = paramDisk.opsType == NotifyOpsType::DAEMON_RMDIR;
    notifyData.isLocalOperation = true;
    CloudDiskNotify::GetInstance().AddNotify(notifyData);
}

static void HandleRename(const NotifyParamDisk &paramDisk, const ParamDiskOthers &paramOthers)
{
    NotifyData oldNotifyData;
    NotifyData newNotifyData;
    int32_t ret = CloudDiskNotifyUtils::GetNotifyData(paramDisk.data, paramDisk.func, paramDisk.ino,
        paramDisk.name, oldNotifyData);
    if (ret != E_OK) {
        LOGE("get notify data fail, name: %{public}s", GetAnonyString(paramDisk.name).c_str());
        return;
    }
    ret = CloudDiskNotifyUtils::GetNotifyData(paramDisk.data, paramDisk.func, paramDisk.newIno,
        paramDisk.newName, newNotifyData);
    if (ret != E_OK) {
        LOGE("get new notify data fail, name: %{public}s", GetAnonyString(paramDisk.newName).c_str());
        return;
    }
    NotifyData notifyData;
    notifyData.uri = newNotifyData.uri;
    notifyData.extraUri = oldNotifyData.uri;
    notifyData.isDir = paramOthers.isDir;
    notifyData.type = NotifyType::NOTIFY_RENAMED;
    notifyData.isLocalOperation = true;
    CloudDiskNotify::GetInstance().AddNotify(notifyData);
}

void CloudDiskNotify::TryNotify(const NotifyParamDisk &paramDisk, const ParamDiskOthers &paramOthers)
{
    switch (paramDisk.opsType) {
        case NotifyOpsType::DAEMON_SETATTR:
        case NotifyOpsType::DAEMON_SETXATTR:
            HandleSetAttr(paramDisk);
            break;
        case NotifyOpsType::DAEMON_RECYCLE:
        case NotifyOpsType::DAEMON_RESTORE:
            HandleRecycleRestore(paramDisk);
            break;
        case NotifyOpsType::DAEMON_MKDIR:
            HandleMkdir(paramDisk);
            break;
        case NotifyOpsType::DAEMON_RMDIR:
        case NotifyOpsType::DAEMON_UNLINK:
            HandleUnlink(paramDisk);
            break;
        case NotifyOpsType::DAEMON_RENAME:
            HandleRename(paramDisk, paramOthers);
            break;
        case NotifyOpsType::DAEMON_WRITE:
            HandleWrite(paramDisk, paramOthers);
            break;
        default:
            LOGE("bad ops, opsType: %{public}d", paramDisk.opsType);
            break;
    }
}

static void HandleInsert(const NotifyParamService &paramService, const ParamServiceOther &paramOthers)
{
    shared_ptr<CloudDiskRdbStore> rdbStore = GetRdbStore(paramOthers.bundleName, paramOthers.userId);
    if (rdbStore == nullptr) {
        LOGE("Get rdb store fail, bundleName: %{public}s", paramOthers.bundleName.c_str());
        return;
    }
    NotifyData notifyData;
    int32_t ret;
    if (paramService.node.isRecycled) {
        ret = GetTrashNotifyData(paramService.node, paramOthers, notifyData);
        if (TrashUriAddRowId(rdbStore, paramService.cloudId, notifyData.uri) != E_OK) {
            return;
        }
        notifyData.isDir = paramService.node.isDir == TYPE_DIR_STR;
    } else {
        ret = rdbStore->GetNotifyData(paramService.node, notifyData);
    }
    if (ret == E_OK) {
        notifyData.type = NotifyType::NOTIFY_ADDED;
        CloudDiskNotify::GetInstance().AddNotify(notifyData);
    } else {
        LOGI("HandleInsert failed. ret = %{public}d", ret);
    }
}

static void HandleUpdate(const NotifyParamService &paramService, const ParamServiceOther &paramOthers)
{
    shared_ptr<CloudDiskRdbStore> rdbStore = GetRdbStore(paramOthers.bundleName, paramOthers.userId);
    if (rdbStore == nullptr) {
        LOGE("Get rdb store fail, bundleName: %{public}s", paramOthers.bundleName.c_str());
        return;
    }
    NotifyData inNotifyData;
    NotifyData notifyData;
    CacheNode curNode{paramService.cloudId};
    if (rdbStore->GetCurNode(paramService.cloudId, curNode) == E_OK &&
        rdbStore->GetNotifyUri(curNode, notifyData.uri) == E_OK) {
        notifyData.type = NotifyType::NOTIFY_MODIFIED;
        notifyData.isDir = curNode.isDir == TYPE_DIR_STR;
        if (paramService.notifyType == NotifyType::NOTIFY_NONE &&
            rdbStore->GetNotifyData(paramService.node, inNotifyData) == E_OK) {
            if (inNotifyData.uri != notifyData.uri) {
                notifyData.type = NotifyType::NOTIFY_DELETED;
                inNotifyData.type = NotifyType::NOTIFY_RENAMED;
            }
        }
    }
    CloudDiskNotify::GetInstance().AddNotify(notifyData);
    CloudDiskNotify::GetInstance().AddNotify(inNotifyData);
}

static void HandleUpdateRecycle(const NotifyParamService &paramService, const ParamServiceOther &paramOthers)
{
    shared_ptr<CloudDiskRdbStore> rdbStore = GetRdbStore(paramOthers.bundleName, paramOthers.userId);
    if (rdbStore == nullptr) {
        LOGE("Get rdb store fail, bundleName: %{public}s", paramOthers.bundleName.c_str());
        return;
    }
    NotifyData trashNotifyData;
    GetTrashNotifyData(paramService.node, paramOthers, trashNotifyData);
    if (TrashUriAddRowId(rdbStore, paramService.cloudId, trashNotifyData.uri) != E_OK) {
        return;
    }

    NotifyData originNotifyData;
    if (rdbStore->GetNotifyData(paramService.node, originNotifyData) != E_OK) {
        LOGE("Get origin notify data fail");
        return;
    }
    trashNotifyData.isDir = paramService.node.isDir == TYPE_DIR_STR;
    originNotifyData.isDir = trashNotifyData.isDir;
    if (paramService.node.isRecycled) {
        trashNotifyData.type = NotifyType::NOTIFY_ADDED;
        originNotifyData.type = NotifyType::NOTIFY_DELETED;
    } else {
        trashNotifyData.type = NotifyType::NOTIFY_DELETED;
        originNotifyData.type = NotifyType::NOTIFY_ADDED;
    }
    CloudDiskNotify::GetInstance().AddNotify(trashNotifyData);
    CloudDiskNotify::GetInstance().AddNotify(originNotifyData);
}

static void HandleDelete(const NotifyParamService &paramService, const ParamServiceOther &paramOthers)
{
    if (paramService.cloudId.empty()) {
        NotifyData notifyData{"", false, NotifyType::NOTIFY_DELETED};
        notifyData.uri = CLOUDDISK_URI_PREFIX;
        notifyData.uri.replace(CLOUDDISK_URI_PREFIX.find(BUNDLENAME_FLAG), BUNDLENAME_FLAG.length(),
                               paramOthers.bundleName);
        notifyData.isDir = true;
        CloudDiskNotify::GetInstance().AddNotify(notifyData);
    } else {
        for (auto notifyData : (paramOthers.notifyDataList)) {
            CloudDiskNotify::GetInstance().AddNotify(notifyData);
        }
    }
}

static void HandleDeleteBatch(const NotifyParamService &paramService, const ParamServiceOther &paramOthers)
{
    for (auto notifyData : (paramOthers.notifyDataList)) {
        CloudDiskNotify::GetInstance().AddNotify(notifyData);
    }
}

void CloudDiskNotify::TryNotifyService(const NotifyParamService &paramService, const ParamServiceOther &paramOthers)
{
    switch (paramService.opsType) {
        case NotifyOpsType::SERVICE_INSERT:
            HandleInsert(paramService, paramOthers);
            break;
        case NotifyOpsType::SERVICE_UPDATE:
            HandleUpdate(paramService, paramOthers);
            break;
        case NotifyOpsType::SERVICE_UPDATE_RECYCLE:
            HandleUpdateRecycle(paramService, paramOthers);
            break;
        case NotifyOpsType::SERVICE_DELETE:
            HandleDelete(paramService, paramOthers);
            break;
        case NotifyOpsType::SERVICE_DELETE_BATCH:
            HandleDeleteBatch(paramService, paramOthers);
            break;
        default:
            LOGE("bad ops, opsType: %{public}d", paramService.opsType);
            break;
    }
}

int32_t CloudDiskNotify::GetDeleteNotifyData(const vector<NativeRdb::ValueObject> &deleteIds,
                                             vector<NotifyData> &notifyDataList,
                                             const ParamServiceOther &paramOthers)
{
    shared_ptr<CloudDiskRdbStore> rdbStore = GetRdbStore(paramOthers.bundleName, paramOthers.userId);
    if (rdbStore == nullptr) {
        LOGE("Get rdb store fail, bundleName: %{public}s", paramOthers.bundleName.c_str());
        return E_RDB;
    }
    for (auto deleteId : deleteIds) {
        NotifyData notifyData{"", false, NotifyType::NOTIFY_DELETED};
        string cloudId = static_cast<string>(deleteId);
        CacheNode curNode{cloudId};
        if (rdbStore->GetCurNode(cloudId, curNode) == E_OK && rdbStore->GetNotifyUri(curNode, notifyData.uri) == E_OK) {
            notifyData.isDir = curNode.isDir == TYPE_DIR_STR;
            notifyDataList.push_back(notifyData);
        }
    }
    return E_OK;
}

void CloudDiskNotify::AddNotify(const NotifyData &notifyData)
{
    LOGD("push cur notify into list type: %{public}d, uri: %{public}s, isDir: %{public}d", notifyData.type,
         GetAnonyString(notifyData.uri).c_str(), notifyData.isDir);
    if (notifyData.type == NotifyType::NOTIFY_NONE) {
        return;
    }

    auto notifyFunc = [notifyData] {
        auto obsMgrClient = AAFwk::DataObsMgrClient::GetInstance();
        if (obsMgrClient == nullptr) {
            LOGE("obsMgrClient is null");
            return;
        }
        Parcel parcel;
        parcel.WriteUint32(1);
        parcel.WriteBool(notifyData.isDir);
        parcel.WriteBool(notifyData.isLocalOperation);
        parcel.WriteString(notifyData.extraUri);
        uintptr_t buf = parcel.GetData();
        if (parcel.GetDataSize() == 0) {
            LOGE("parcel.getDataSize fail");
            return;
        }

        auto *uBuf = new (std::nothrow) uint8_t[parcel.GetDataSize()];
        if (uBuf == nullptr) {
            return;
        }
        int32_t ret = memcpy_s(uBuf, parcel.GetDataSize(), reinterpret_cast<uint8_t *>(buf), parcel.GetDataSize());
        if (ret != 0) {
            LOGE("Parcel Data copy failed, err: %{public}d", ret);
            delete[] uBuf;
            return;
        }
        ChangeInfo changeInfo({static_cast<ChangeInfo::ChangeType>(notifyData.type), {Uri(notifyData.uri)}, uBuf,
                               parcel.GetDataSize()});
        obsMgrClient->NotifyChangeExt(changeInfo);
        delete[] uBuf;
    };
    ffrt::thread(notifyFunc).detach();
}

void CloudDiskNotify::NotifyChangeOuter()
{
    LOGD("Start Notify Outer");
    list<CacheNotifyInfo> tmpNfList_;
    {
        lock_guard<mutex> lock(mutex_);
        if (nfList_.empty()) {
            return;
        }
        nfList_.swap(tmpNfList_);
        nfList_.clear();
    }

    auto obsMgrClient = AAFwk::DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        LOGE("obsMgrClient is null");
        return;
    }
    for (auto cacheNode = tmpNfList_.begin(); cacheNode != tmpNfList_.end(); ++cacheNode) {
        Parcel parcel;
        parcel.WriteUint32(static_cast<uint32_t>(cacheNode->isDirList.size()));
        for (auto isDir = cacheNode->isDirList.begin(); isDir != cacheNode->isDirList.end(); ++isDir) {
            parcel.WriteBool(*isDir);
        }
        uintptr_t buf = parcel.GetData();
        if (parcel.GetDataSize() == 0) {
            LOGE("parcel.getDataSize fail");
            return;
        }

        auto *uBuf = new (std::nothrow) uint8_t[parcel.GetDataSize()];
        if (uBuf == nullptr) {
            return;
        }
        int32_t ret = memcpy_s(uBuf, parcel.GetDataSize(), reinterpret_cast<uint8_t *>(buf), parcel.GetDataSize());
        if (ret != 0) {
            LOGE("Parcel Data copy failed, err: %{public}d", ret);
            delete[] uBuf;
            return;
        }
        ChangeInfo changeInfo({static_cast<ChangeInfo::ChangeType>(cacheNode->notifyType), cacheNode->uriList, uBuf,
                               parcel.GetDataSize()});
        obsMgrClient->NotifyChangeExt(changeInfo);
        delete[] uBuf;
    }
}
} // namespace OHOS::FileManagement::CloudDisk
