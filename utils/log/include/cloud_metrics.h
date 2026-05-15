/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef CLOUD_METRICS_H
#define CLOUD_METRICS_H

#ifdef ENABLE_API_METRICS
#include "dfs_error.h"
#include "histogram_plugin_macros.h"

inline void MetricsCount(const std::string &name)
{
    HISTOGRAM_BOOLEAN(name, 1);
}

inline void MetricsError(const std::string &name, int32_t errCode)
{
    HISTOGRAM_ENUMERATION(
        name, OHOS::FileManagement::Convert2MetricsIndex(errCode), OHOS::FileManagement::CLOUD_INDEX_MAX);
}

#else
inline void MetricsCount(const std::string &name) {}
inline void MetricsError(const std::string &name, int32_t errCode) {}
#endif

#endif