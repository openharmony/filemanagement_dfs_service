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

#include "mountpoint/mount_point.h"

#include <sys/mount.h>
#include <system_error>

#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

atomic<uint32_t> MountPoint::idGen_;

MountPoint::MountPoint(const Utils::MountArgument &mountArg) : mountArg_(mountArg)
{
    id_ = idGen_++;
    if (mountArg.accountless_) {
        authGroupId_ = mountArg.groupId_;
    }
}

void MountPoint::Mount() const
{
    auto mntArg = GetMountArgument();

    string src = mntArg.GetFullSrc();
    Utils::ForceCreateDirectory(src, S_IRWXU | S_IRWXG | S_IXOTH, Utils::UID_SYSTEM, Utils::UID_MEDIA_RW);

    string dst = mntArg.GetFullDst();
    Utils::ForceCreateDirectory(dst, S_IRWXU | S_IRWXG | S_IXOTH);

    string cache = mntArg.GetCachePath();
    Utils::ForceCreateDirectory(cache, S_IRWXU | S_IRWXG | S_IXOTH);

    unsigned long flags = mntArg.GetFlags();
    string options = mntArg.OptionsToString();
    int ret = mount(src.c_str(), dst.c_str(), "hmdfs", flags, options.c_str());
    if (ret == -1 && errno != EEXIST && errno != EBUSY) {
        auto cond = system_category().default_error_condition(errno);
        LOGE("Failed to mount: %{public}d %{public}s", cond.value(), cond.message().c_str());
        throw system_error(errno, system_category());
    }
    LOGI("mount sucess: src %{public}s --> dst %{public}s", src.c_str(), dst.c_str());

    if (mntArg.accountless_) {
        Utils::ForceCreateDirectory(dst + "/device_view/local/data/" + mntArg.packageName_,
                                    S_IRWXU | S_IRWXG | S_IXOTH);
    }
}

void MountPoint::Umount() const
{
    string dst = GetMountArgument().GetFullDst();
    if (umount2(dst.c_str(), MNT_DETACH) == -1) {
        auto cond = system_category().default_error_condition(errno);
        LOGE("Failed to umount: %{public}d %{public}s", cond.value(), cond.message().c_str());
        throw system_error(errno, system_category());
    }

    Utils::ForceRemoveDirectory(dst);
}

bool MountPoint::operator==(const MountPoint &rop) const
{
    return mountArg_.GetFullDst() == rop.mountArg_.GetFullDst();
}

Utils::MountArgument MountPoint::GetMountArgument() const
{
    return mountArg_;
}

string MountPoint::ToString() const
{
    return "";
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS