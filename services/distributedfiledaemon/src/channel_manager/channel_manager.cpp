/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "channel_manager.h"
#include "dfs_error.h"
#include "network/softbus/softbus_permission_check.h"
#include "nlohmann/json.hpp"
#include "securec.h"
#include "softbus_error_code.h"
#include "utils_log.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <sys/prctl.h>
#include <unordered_set>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
IMPLEMENT_SINGLE_INSTANCE(ChannelManager);
using namespace OHOS::FileManagement;

int32_t ChannelManager::SendRequest(const std::string &networkId,
                                    ControlCmd &request,
                                    ControlCmd &response,
                                    bool needResponse)
{
    LOGI("start sendRequest, networkId: %{public}.6s", networkId.c_str());

    // implement later, now for temp ut test
    return E_OK;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
