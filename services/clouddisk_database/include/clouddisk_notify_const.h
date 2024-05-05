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

#ifndef OHOS_CLOUD_DISK_SERVICE_NOTIFY_CONST_H
#define OHOS_CLOUD_DISK_SERVICE_NOTIFY_CONST_H

#include "clouddisk_db_const.h"
#include "cloud_disk_inode.h"
#include "values_bucket.h"
#include <string>

namespace OHOS::FileManagement::CloudDisk {

enum class NotifyOpsType : uint32_t {
    DAEMON_SETXATTR = 0,
    DAEMON_SETATTR,
    DAEMON_MKDIR,
    DAEMON_RMDIR,
    DAEMON_UNLINK,
    DAEMON_RENAME,
    DAEMON_WRITE,
    SERVICE_INSERT,
    SERVICE_UPDATE,
    SERVICE_DELETE,
    SERVICE_DELETE_BATCH,
    OPS_NONE,
};

using FindCloudDiskInodeFunc = std::function<std::shared_ptr<CloudDiskInode>(CloudDiskFuseData*, int64_t)>;
struct NotifyParamDisk {
    CloudDiskFuseData *data = nullptr;
    FindCloudDiskInodeFunc func;
    NotifyOpsType opsType = NotifyOpsType::OPS_NONE;
    std::shared_ptr<CloudDiskInode> inoPtr = nullptr;
    fuse_ino_t ino = 0;
    std::string name = "";
    fuse_ino_t newIno = 0;
    std::string newName = "";
};

struct ParamDiskOthers {
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_NO_NEED_UPLOAD);
    bool isDir = false;
};

struct NotifyParamService {
    NotifyOpsType opsType = NotifyOpsType::OPS_NONE;
    std::string cloudId = "";
    NotifyType notifyType = NotifyType::NOTIFY_NONE;
    DriveKit::DKRecord record;
};

struct ParamServiceOther {
    int32_t userId;
    std::string bundleName;
    std::vector<NotifyData> notifyDataList = {};
};

} // namespace OHOS::FileManagement::CloudDisk
#endif // OHOS_CLOUD_DISK_SERVICE_NOTIFY_CONST_H