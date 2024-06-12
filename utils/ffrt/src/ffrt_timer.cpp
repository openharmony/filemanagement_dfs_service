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

#include "ffrt_timer.h"

#include "cloud_file_log.h"

namespace OHOS::FileManagement::CloudFile {
static constexpr uint32_t TIMER_MAX_INTERVAL_MS = 200;
using namespace std;

FfrtTimer::FfrtTimer(const std::string &name) : name_(name) {}

FfrtTimer ::~FfrtTimer()
{
    Stop();
}

void FfrtTimer::Start(const TimerCallback &callback, uint32_t interval, uint32_t repatTimes)
{
    unique_lock<ffrt::mutex> lock(taskMutex_);
    if (running_ == true) {
        LOGE("timer may be is running, timerName:%{public}s", name_.c_str());
        return;
    }
    LOGD("start timer, timerName:%{public}s", name_.c_str());
    running_ = true;
    auto task = [this, interval, callback, repatTimes]() {
        LOGD("task entering loop");
        uint32_t times = repatTimes;
        while (times > 0) {
            callback();
            times--;
            unique_lock<ffrt::mutex> lock(sleepMutex_);
            bool stop =
                sleepCv_.wait_for(lock, std::chrono::milliseconds(interval), [this]() { return !this->running_; });
            if (stop) { // is stopped
                break;
            }
        }
        LOGD("task leaving loop");
    };

    isJoinable_ = std::make_unique<bool>(true);
    ffrt::submit(task, {}, {isJoinable_.get()}, ffrt::task_attr().name(name_.c_str()));
}

void FfrtTimer::Stop()
{
    unique_lock<ffrt::mutex> lock(taskMutex_);
    if (running_ == false) {
        LOGE("timer may be is stopped, timerName:%{public}s", name_.c_str());
        return;
    }

    LOGD("stop timer, timerName:%{public}s", name_.c_str());
    {
        std::unique_lock<ffrt::mutex> lock(sleepMutex_);
        running_ = false;
        sleepCv_.notify_one();
    }

    if (isJoinable_.get() && *isJoinable_) {
        ffrt::wait({isJoinable_.get()});
        *isJoinable_ = false;
    }
    LOGD("timer is stoped, timerName:%{public}s", name_.c_str());
}
} // namespace OHOS::FileManagement::CloudFile