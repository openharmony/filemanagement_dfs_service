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

#include "multiuser/os_account_observer.h"

#include <algorithm>
#include <sstream>

#include "device/device_manager_agent.h"
#include "utils_log.h"
#include "utils_mount_argument.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
namespace {
static const std::string SAME_ACCOUNT = "account";
static const std::string ACCOUNT_LESS = "no_account";
} // namespace

osAccountChangeObserver::osAccountChangeObserver(const AccountSA::OsAccountSubscribeInfo &subscribeInfo) : OsAccountSubscriber(subscribeInfo)
{
    LOGI("init first to create network of user 100");
    // lock_guard<mutex> lock(serializer_);
    curUsrId = 100;
    AddMPInfo(curUsrId, SAME_ACCOUNT);
    AddMPInfo(curUsrId, ACCOUNT_LESS);
    LOGI("init first to create network of user 100, done");
}

osAccountChangeObserver::~osAccountChangeObserver()
{
    // OHOS::AccountSA::OsAccountManager::UnsubscribeOsAccount(shared_from_this());
}

void osAccountChangeObserver::AddMPInfo(const int id, const std::string &relativePath)
{
    auto smp = make_shared<MountPoint>(Utils::MountArgumentDescriptors::Alpha(id, relativePath));
    auto dm = DeviceManagerAgent::GetInstance();
    dm->Recv(make_unique<Cmd<DeviceManagerAgent, weak_ptr<MountPoint>>>(&DeviceManagerAgent::JoinGroup, smp));
    mountPoints_[id].emplace_back(smp);
}

void osAccountChangeObserver::OnAccountsChanged(const int &id)
{
    LOGI("user id changed to %{public}d", id);
    lock_guard<mutex> lock(serializer_);
    if (curUsrId != -1) { // todo: 仅仅是切断当前用户，是否需要删除之前的用户信息？ 
        // first stop curUsrId network
        RemoveMPInfo(curUsrId);
    }

    // then start new network
    curUsrId = id;
    AddMPInfo(id, SAME_ACCOUNT);
    AddMPInfo(id, ACCOUNT_LESS);
    LOGI("user id %{public}d, add network done", curUsrId);
}

void osAccountChangeObserver::RemoveMPInfo(const int id)
{
    auto iter = mountPoints_.find(id);
    if (iter == mountPoints_.end()) {
        LOGE("user id %{public}d not find in map", curUsrId);
        return;
    }

    auto dm = DeviceManagerAgent::GetInstance();
    for (auto smp : iter->second) {  
        dm->Recv(make_unique<Cmd<DeviceManagerAgent, weak_ptr<MountPoint>>>(&DeviceManagerAgent::QuitGroup, smp));
    }
    mountPoints_.erase(iter);
    
    LOGE("remove mount info of user id %{public}d", id);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS