/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dfsu_timer.h"

#include "common_timer_errors.h"
#include "utils_log.h"

namespace OHOS::FileManagement {
using namespace std;

DfsuTimer &DfsuTimer::GetInstance()
{
    static DfsuTimer instance;
    return instance;
}

DfsuTimer::DfsuTimer() : timer_(std::make_unique<Utils::Timer>("OH_DfsTimer"))
{
    timer_->Setup();
}

DfsuTimer::~DfsuTimer()
{
    if (timer_) {
        timer_->Shutdown(true);
        timer_ = nullptr;
    }
}

void DfsuTimer::Register(const Utils::Timer::TimerCallback &callback, uint32_t &outTimerId, uint32_t interval)
{
    if (timer_ == nullptr) {
        LOGE("timer is nullptr");
        return;
    }

    uint32_t ret = timer_->Register(callback, interval);
    if (ret == Utils::TIMER_ERR_DEAL_FAILED) {
        LOGE("Register timer failed");
        return;
    }
    outTimerId = ret;
}

void DfsuTimer::Unregister(uint32_t timerId)
{
    if ((timerId == 0) || (!timer_)) {
        LOGE("timerId is 0 or timer_ is nullptr");
        return;
    }
    timer_->Unregister(timerId);
}
} // namespace OHOS::FileManagement::CloudSync