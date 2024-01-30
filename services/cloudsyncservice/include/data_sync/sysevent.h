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

#ifndef OHOS_CLOUD_SYNC_SERVICE_SYSEVENT_H
#define OHOS_CLOUD_SYNC_SERVICE_SYSEVENT_H

#include <atomic>

#include "hisysevent.h"

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
#define CLOUD_SYNC_SYS_EVENT(eventName, type, ...)    \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName,   \
                    type, ##__VA_ARGS__)    \

class SysEventData {
public:
    virtual void Report() = 0;
};

template <typename T>
static inline std::vector<T> vector_atomic_to_origin(std::vector<std::atomic<T>> &v)
{
    std::vector<T> ret;
    ret.reserve(v.size());
    for (auto &atomic_value : v) {
        ret.push_back(atomic_value.load());
    }
    return ret;
}

class SyncStat : public SysEventData {
public:
    SyncStat()
    {
    }

    virtual ~SyncStat() {}

    void AppendSyncInfo(std::string &info)
    {
        sync_info_.append(info);
    }

    bool IsFullSync()
    {
        return isFullSync_;
    }

    void SetFullSync()
    {
        isFullSync_ = true;
    }

    void SetSyncReason(uint32_t reason)
    {
        syncReason_ = reason;
    }

    void SetStopReason(uint32_t reason)
    {
        stopReason_ = reason;
    }

    void SetStartTime(uint64_t time)
    {
        startTime_ = time;
    }

    void SetDuration(uint64_t time)
    {
        duration_ = time - startTime_;
    }

protected:
    bool isFullSync_;

    uint32_t syncReason_;
    uint32_t stopReason_;
    uint64_t startTime_;
    uint64_t duration_;
    std::string sync_info_;
};

template<typename T>
class SyncStatContainer {
public:
    void SetSyncStat(std::shared_ptr<T> stat)
    {
        syncStat_ = stat;
    }

    void PutSyncStat()
    {
        syncStat_ = nullptr;
    }

    std::shared_ptr<T> GetSyncStat()
    {
        return syncStat_;
    }

private:
    std::shared_ptr<T> syncStat_;
};

template<typename T>
class CheckStatContainer {
public:
    void SetCheckStat(std::shared_ptr<T> stat)
    {
        checkStat_ = stat;
    }

    void PutCheckStat()
    {
        checkStat_ = nullptr;
    }

    std::shared_ptr<T> GetCheckStat()
    {
        return checkStat_;
    }

private:
    std::shared_ptr<T> checkStat_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_SYSEVENT_H
