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

#ifndef OHOS_FILEMGMT_DFSU_TIMER_H
#define OHOS_FILEMGMT_DFSU_TIMER_H

#include "nocopyable.h"
#include "timer.h"

namespace OHOS::FileManagement {
class DfsuTimer final : public NoCopyable {
public:
    static DfsuTimer &GetInstance();
    void Register(const Utils::Timer::TimerCallback &callback, uint32_t &outTimerId, uint32_t interval);
    void Unregister(uint32_t timerId);

private:
    DfsuTimer();
    ~DfsuTimer();
    std::unique_ptr<Utils::Timer> timer_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_DFSU_TIMER_H