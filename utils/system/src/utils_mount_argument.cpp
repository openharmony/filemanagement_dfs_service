/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "utils_mount_argument.h"

#include <sstream>
#include <sys/mount.h>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
using namespace std;
namespace {
static const std::string DATA_POINT = "/data/service/el2/";
static const std::string BASE_MOUNT_POINT = "/mnt/hmdfs/";
static const std::string AUTH_GROUP_MOUNT_POINT = "/mnt/hmdfs/auth_groups/";
static const std::string SYSFS_HMDFS_PATH = "sys/fs/hmdfs/";
} // namespace

string MountArgument::GetFullSrc() const
{
    stringstream ss;
    if (!accountless_) {
        ss << DATA_POINT << userId_ << "/hmdfs/storage";
    } else {
        ss << DATA_POINT << userId_ << "/hmdfs/auth_groups/" << groupId_;
    }
    return ss.str();
}

string MountArgument::GetFullDst() const
{
    stringstream ss;
    if (!accountless_) {
        ss << BASE_MOUNT_POINT << userId_ << "/";
    } else {
        ss << AUTH_GROUP_MOUNT_POINT << groupId_ << "/";
    }
    return ss.str();
}

string MountArgument::GetCachePath() const
{
    stringstream ss;
    if (!accountless_) {
        ss << DATA_POINT << userId_ << "/hmdfs/cache/";
    } else {
        ss << DATA_POINT << userId_ << "/hmdfs/auth_groups/" << groupId_ << "/cache/";
    }
    return ss.str();
}

static uint64_t MocklispHash(const string &str)
{
    uint64_t res = 0;
    constexpr int mocklispHashPos = 5;
    /* Mocklisp hash function. */
    for (auto ch : str) {
        res = (res << mocklispHashPos) - res + (uint64_t)ch;
    }
    return res;
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
    return 0;
}

MountArgument MountArgumentDescriptors::Alpha(int userId)
{
    MountArgument mountArgument = {
        .groupId_ = "default",
        .needInitDir_ = true,
        .useCache_ = true,
        .enableMergeView_ = true,
        .enableFixupOwnerShip_ = true,
        .enableOfflineStash_ = true,
        .externalFS_ = false,
    };
    mountArgument.userId_ = userId;
    return mountArgument;
}

MountArgument MountArgumentDescriptors::SetAuthGroupMountArgument(const std::string &groupId,
                                                                  const std::string &packageName,
                                                                  bool accountless)
{
    MountArgument mountArgument = {
        .accountless_ = accountless,
        .groupId_ = groupId,
        .needInitDir_ = true,
        .useCache_ = true,
        .enableMergeView_ = true,
        .enableFixupOwnerShip_ = true,
        .enableOfflineStash_ = true,
        .externalFS_ = false,
        .packageName_ = packageName,
    };
    return mountArgument;
}
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS