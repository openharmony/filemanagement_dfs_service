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

#include "data_sync_manager.h"
#include "data_syncer_rdb_col.h"
#include "data_syncer_rdb_store.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "sync_rule/system_load.h"
#include "res_sched_client.h"

namespace OHOS::FileManagement::CloudSync {

void SystemLoadStatus::RegisterSystemloadCallback()
{
    sptr<SystemLoadListener> loadListener = new (std::nothrow) SystemLoadListener();
    if (loadListener == nullptr) {
        return;
    }
    ResourceSchedule::ResSchedClient::GetInstance().RegisterSystemloadNotifier(loadListener);
}

void SystemLoadListener::OnSystemloadLevel(int32_t level)
{
    SystemLoadStatus::Setload(level);
    if (level >= SYSTEMLOADLEVEL) {
        LOGI("systemloadlevel over temperature");
    }
}

void SystemLoadStatus::GetSystemloadLevel()
{
    loadstatus_ = ResourceSchedule::ResSchedClient::GetInstance().GetSystemloadLevel();
    LOGI("GetSystemloadLevel finish, loadstatus:%{public}d", loadstatus_);
}

void SystemLoadStatus::Setload(int32_t load)
{
    loadstatus_ = load;
}

void SystemLoadStatus::InitSystemload()
{
    GetSystemloadLevel();
    RegisterSystemloadCallback();
}

bool SystemLoadStatus::IsLoadStatusOkay()
{
    if (loadstatus_ > SYSTEMLOADLEVEL) {
        return false;
    }
    return true;
}
}