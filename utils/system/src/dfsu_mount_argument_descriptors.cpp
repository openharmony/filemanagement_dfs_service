/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "dfsu_mount_argument_descriptors.h"

#include <sstream>
#include <sys/mount.h>
#include <sys/stat.h>

#include "dfs_error.h"
#include "utils_log.h"
namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
using namespace std;
namespace {
    static const std::string DATA_POINT = "/data/service/el2/";
    static const std::string BASE_MOUNT_POINT = "/mnt/hmdfs/";
    static const std::string SYSFS_HMDFS_PATH = "/sys/fs/hmdfs/";
} // namespace

string MountArgument::GetFullSrc() const
{
    stringstream ss;
    ss << DATA_POINT << userId_ << "/hmdfs/" << relativePath_;
    return ss.str();
}

string MountArgument::GetFullDst() const
{
    stringstream ss;
    ss << BASE_MOUNT_POINT << userId_ << "/" << relativePath_;
    return ss.str();
}

string MountArgument::GetCachePath() const
{
    stringstream ss;
    ss << DATA_POINT << userId_ << "/hmdfs/" << relativePath_ << "/cache/";
    return ss.str();
}

static uint64_t MocklispHash(const string &str)
{
    struct stat statBuf;
    auto err = stat(str.c_str(), &statBuf);
    if (err != 0) {
        LOGE("stat failed %{public}s error, err: %{public}d", GetAnonyString(str).c_str(), err);
        return static_cast<uint64_t>(FileManagement::ERR_BAD_VALUE);
    }
    LOGI("statBuf dev id: %{public}lu", static_cast<unsigned long>(statBuf.st_dev));
    return statBuf.st_dev;
}

string MountArgument::GetCtrlPath() const
{
    auto dst = GetFullDst();
    auto res = MocklispHash(dst);
    stringstream ss;
    ss << SYSFS_HMDFS_PATH << res << "/cmd";
    return ss.str();
}

string MountArgument::OptionsToString() const
{
    stringstream ss;
    ss << "local_dst=" << GetFullDst();
    if (useCache_) {
        ss << ",cache_dir=" << GetCachePath();
    }
    if (caseSensitive_) {
        ss << ",sensitive";
    }
    if (enableMergeView_) {
        ss << ",merge";
    }
    if (enableFixupOwnerShip_) {
        ss << ",fixupownership";
    }
    if (!enableOfflineStash_) {
        ss << ",no_offline_stash";
    }
    if (externalFS_) {
        ss << ",external_fs";
    }
    return ss.str();
}

unsigned long MountArgument::GetFlags() const
{
    return MS_NODEV;
}

MountArgument DfsuMountArgumentDescriptors::Alpha(int userId, string relativePath)
{
    MountArgument mountArgument = {
        .userId_ = userId,
        .needInitDir_ = true,
        .useCache_ = true,
        .enableMergeView_ = true,
        .enableFixupOwnerShip_ = false,
        .enableOfflineStash_ = true,
        .externalFS_ = false,
        .relativePath_ = relativePath,
    };

    if (relativePath == "non_account") {
        mountArgument.accountless_ = true;
    }
    return mountArgument;
};
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
