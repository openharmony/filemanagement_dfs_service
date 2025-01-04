/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "network/session_pool.h"
#include "dm_device_info.h"
#include "device/device_manager_agent.h"
#include "dfs_error.h"
#include "ipc/i_daemon.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

void SessionPool::HoldSession(shared_ptr<BaseSession> session, const std::string backStage)
{
    lock_guard lock(sessionPoolLock_);
    talker_->SinkSessionTokernel(session, backStage);
    AddSessionToPool(session);
}

void SessionPool::ReleaseSession(const int32_t fd)
{
    lock_guard lock(sessionPoolLock_);
    LOGI("ReleaseSession start, fd=%{public}d", fd);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end(); ++iter) {
        if ((*iter)->GetHandle() == fd) {
            (*iter)->Release();
            iter = usrSpaceSessionPool_.erase(iter);
            break;
        }
    }
}

bool SessionPool::CheckIfGetSession(const int32_t fd)
{
    lock_guard lock(sessionPoolLock_);
    std::shared_ptr<BaseSession> session = nullptr;
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end(); ++iter) {
        if ((*iter)->GetHandle() == fd) {
            session = *iter;
            break;
        }
    }
    if (session == nullptr) {
        return false;
    }
    return !session->IsFromServer();
}

void SessionPool::SinkOffline(const std::string &cid)
{
    lock_guard lock(sessionPoolLock_);
    if (!FindCid(cid)) {
        talker_->SinkOfflineCmdToKernel(cid);
    }
}

bool SessionPool::FindSocketId(int32_t socketId)
{
    lock_guard lock(sessionPoolLock_);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end(); ++iter) {
        if ((*iter)->GetSessionId() == socketId) {
            return true;
        }
    }
    return false;
}

void SessionPool::ReleaseSession(const std::string &cid, bool releaseServer)
{
    lock_guard lock(sessionPoolLock_);
    std::vector<std::shared_ptr<BaseSession>> sessions;
    LOGI("ReleaseSession, cid:%{public}s", Utils::GetAnonyString(cid).c_str());
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end(); ++iter) {
        if ((*iter)->GetCid() != cid || ((*iter)->IsFromServer() && !releaseServer)) {
            continue;
        }
        sessions.push_back(*iter);
        iter = usrSpaceSessionPool_.erase(iter);
    }
    for (auto session : sessions) {
        session->Release();
    }

    if (!FindCid(cid)) {
        talker_->SinkOfflineCmdToKernel(cid);
    }
}

bool SessionPool::FindCid(const std::string &cid)
{
    lock_guard lock(sessionPoolLock_);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end(); ++iter) {
        if ((*iter)->GetCid() == cid) {
            return true;
        }
    }
    return false;
}

void SessionPool::ReleaseAllSession()
{
    lock_guard lock(sessionPoolLock_);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end();) {
        talker_->SinkOfflineCmdToKernel((*iter)->GetCid());
        /* device offline, session release by softbus */
        iter = usrSpaceSessionPool_.erase(iter);
    }
}

void SessionPool::AddSessionToPool(shared_ptr<BaseSession> session)
{
    lock_guard lock(sessionPoolLock_);
    usrSpaceSessionPool_.push_back(session);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
