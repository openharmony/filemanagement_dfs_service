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

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

void SessionPool::OccupySession(int32_t sessionId, uint8_t linkType)
{
    lock_guard lock(sessionPoolLock_);
    occupySession_.insert(std::pair<int32_t, uint8_t>(sessionId, linkType));
}

bool SessionPool::FindSession(int32_t sessionId)
{
    lock_guard lock(sessionPoolLock_);
    auto linkTypeIter = occupySession_.find(sessionId);
    if (linkTypeIter != occupySession_.end()) {
        return true;
    } else {
        return false;
    }
}

void SessionPool::HoldSession(shared_ptr<BaseSession> session, const std::string backStage)
{
    lock_guard lock(sessionPoolLock_);
    talker_->SinkSessionTokernel(session, backStage);
    AddSessionToPool(session);
}

uint8_t SessionPool::ReleaseSession(const int32_t fd)
{
    uint8_t linkType = 0;
    lock_guard lock(sessionPoolLock_);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end();) {
        if ((*iter)->GetHandle() == fd) {
            auto linkTypeIter = occupySession_.find((*iter)->GetSessionId());
            if (linkTypeIter != occupySession_.end()) {
                linkType = linkTypeIter->second;
                occupySession_.erase(linkTypeIter);
                (*iter)->Release();
                iter = usrSpaceSessionPool_.erase(iter);
                break;
            }
        }
        ++iter;
    }
    return linkType;
}

void SessionPool::ReleaseSession(const string &cid, const uint8_t linkType)
{
    uint8_t mlinkType = 0;
    lock_guard lock(sessionPoolLock_);
    for (auto iter = usrSpaceSessionPool_.begin(); iter != usrSpaceSessionPool_.end();) {
        if ((*iter)->GetCid() == cid) {
            auto linkTypeIter = occupySession_.find((*iter)->GetSessionId());
            if (linkTypeIter != occupySession_.end()) {
                mlinkType = (linkTypeIter->second == 0) ? linkType : linkTypeIter->second;
            }
            if (mlinkType == linkType) {
                (*iter)->Release();
                occupySession_.erase(linkTypeIter);
                iter = usrSpaceSessionPool_.erase(iter);
                mlinkType = 0;
                continue;
            }
        }
        ++iter;
    }
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
    usrSpaceSessionPool_.push_back(session);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
