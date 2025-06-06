/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef UTILS_LOG_H
#define UTILS_LOG_H

#include <string>

#include "hilog/log_c.h"
#include "hilog/log_cpp.h"

namespace OHOS {
#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD004315
#endif

#ifndef LOG_TAG
#define LOG_TAG "distributedfile"
#endif

std::string GetFileNameFromFullPath(const char *str);

#define LOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, "[%{public}s:%{public}d->%{public}s] " fmt, \
                            GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOGI(fmt, ...) HILOG_INFO(LOG_CORE, "[%{public}s:%{public}d->%{public}s] " fmt, \
                            GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define LOGW(fmt, ...) HILOG_WARN(LOG_CORE, "[%{public}s:%{public}d->%{public}s] " fmt, \
                            GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOGE(fmt, ...) HILOG_ERROR(LOG_CORE, "[%{public}s:%{public}d->%{public}s] " fmt, \
                            GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define LOGF(fmt, ...) HILOG_FATAL(LOG_CORE, "[%{public}s:%{public}d->%{public}s] " fmt, \
                            GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, __FUNCTION__,  ##__VA_ARGS__)

std::string GetAnonyString(const std::string &value);
std::string GetAnonyInt32(const int32_t value);
} // namespace OHOS
#endif // UTILS_LOG_H