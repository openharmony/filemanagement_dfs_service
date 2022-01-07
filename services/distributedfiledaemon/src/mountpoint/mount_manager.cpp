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

#include "mountpoint/mount_manager.h"

#include <algorithm>
#include <sstream>

#include "device/device_manager_agent.h"
#include "utils_log.h"
#include "utils_mount_argument.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

MountManager::MountManager()
{
    Mount(make_unique<MountPoint>(Utils::MountArgumentDescriptors::Alpha()));
}

MountManager::~MountManager()
{
    try {
        // Umount mountpoints in reverse order to eliminate dependencies
        for_each(mountPoints_.rbegin(), mountPoints_.rend(), [this](auto &cur_mp) { Umount(cur_mp); });
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
}

void MountManager::Mount(unique_ptr<MountPoint> mp)
{
    lock_guard<mutex> lock(serializer_);

    shared_ptr<MountPoint> smp = move(mp);

    auto isExisted = [smp](const auto &cur_mp) { return *smp == *cur_mp; };
    if (!smp || any_of(mountPoints_.begin(), mountPoints_.end(), isExisted)) {
        stringstream ss;
        ss << "Mount an empty/existent mountpoint" << (smp ? smp->GetMountArgument().GetFullDst() : "");
        LOGE("%{public}s", ss.str().c_str());
        throw runtime_error(ss.str());
    }
    try {
        smp->Umount(); // try umount one time
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    smp->Mount();
    auto dm = DeviceManagerAgent::GetInstance();
    dm->Recv(make_unique<Cmd<DeviceManagerAgent, weak_ptr<MountPoint>>>(&DeviceManagerAgent::JoinGroup, smp));
    mountPoints_.push_back(smp);
}

void MountManager::Umount(weak_ptr<MountPoint> wmp)
{
    auto smp = wmp.lock();
    lock_guard<mutex> lock(serializer_);

    decltype(mountPoints_.begin()) it;
    auto isExisted = [smp](const auto &cur_mp) { return *smp == *cur_mp; };
    if (!smp || ((it = find_if(mountPoints_.begin(), mountPoints_.end(), isExisted)) == mountPoints_.end())) {
        stringstream ss;
        ss << "Umount an empty/non-existent mountpoint" << (smp ? smp->GetMountArgument().GetFullDst() : "");
        LOGE("%{public}s", ss.str().c_str());
        throw runtime_error(ss.str());
    }
    LOGI("Umount begin");
    smp->Umount();
    auto dm = DeviceManagerAgent::GetInstance();
    dm->Recv(make_unique<Cmd<DeviceManagerAgent, weak_ptr<MountPoint>>>(&DeviceManagerAgent::QuitGroup, smp));
    mountPoints_.erase(it);
    LOGI("Umount end");
}

void MountManager::Umount(const std::string &groupId)
{
    if (groupId == "") {
        LOGE("groupId is null, no auth group to unmount");
        return;
    }

    decltype(mountPoints_.begin()) iter =
        find_if(mountPoints_.begin(), mountPoints_.end(),
                [groupId](const auto &cur_mp) { return cur_mp->authGroupId_ == groupId; });
    if (iter == mountPoints_.end()) {
        stringstream ss;
        ss << "Umount not find this auth group id" << groupId;
        LOGE("Umount not find this auth group id %{public}s", groupId.c_str());
        throw runtime_error(ss.str());
    }
    Umount(*iter);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS