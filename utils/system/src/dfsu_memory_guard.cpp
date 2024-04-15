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

#include "dfsu_memory_guard.h"

#include "malloc.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
DfsuMemoryGuard::DfsuMemoryGuard()
{
#ifdef CONFIG_USE_JEMALLOC_DFX_INTF
    int setCacheRet = mallopt(M_SET_THREAD_CACHE, M_THREAD_CACHE_DISABLE);
    int setFreeRet = mallopt(M_DELAYED_FREE, M_DELAYED_FREE_DISABLE);
    if (setCacheRet != 1 || setFreeRet != 1) {
        LOGE("dfs disable tcache and delay free, result[%{public}d, %{public}d]", setCacheRet, setFreeRet);
    }
#endif
}

DfsuMemoryGuard::~DfsuMemoryGuard()
{
#ifdef CONFIG_USE_JEMALLOC_DFX_INTF
    int err = mallopt(M_FLUSH_THREAD_CACHE, 0);
    if (err != 1) {
        LOGE("dfs flush cache, result: %{public}d", err);
    }
#endif
}
} // namespace OHOS::FileManagement::CloudSync