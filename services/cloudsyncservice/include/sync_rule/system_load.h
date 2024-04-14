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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SYSTEM_LOAD_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SYSTEM_LOAD_H

#include "data_sync/data_sync_manager.h"
#include <functional>
#include "res_sched_systemload_notifier_client.h"
#include "res_type.h"
#include "sync_state_manager.h"

namespace OHOS::FileManagement::CloudSync {
constexpr int32_t SYSTEMLOADLEVEL = ResourceSchedule::ResType::SystemloadLevel::WARNING;

class SystemLoadStatus {
public:
    SystemLoadStatus() = default;
    virtual ~SystemLoadStatus() = default;
    static void RegisterSystemloadCallback();
    static void GetSystemloadLevel();
    static bool IsLoadStatusOkay();
    static void InitSystemload();
    static void Setload(int32_t load);
    static inline int32_t loadstatus_ = 0;
};

class SystemLoadListener : public ResourceSchedule::ResSchedSystemloadNotifierClient {
public:
    SystemLoadListener() = default;
    void OnSystemloadLevel(int32_t level) override;
};
} // OHOS

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SYSTEM_LOAD_H