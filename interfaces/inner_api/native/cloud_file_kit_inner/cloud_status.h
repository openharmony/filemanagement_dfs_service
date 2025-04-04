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

#ifndef OHOS_FILEMGMT_CLOUD_STATUS_H
#define OHOS_FILEMGMT_CLOUD_STATUS_H

#include <map>
#include <mutex>
#include <string>

#include "cloud_info.h"

namespace OHOS::FileManagement::CloudSync {
class CloudStatus {
public:
    static int32_t GetCurrentCloudInfo(const std::string &bundleName, const int32_t userId);
    static std::pair<uint64_t, uint64_t> GetCurrentSpaceInfo(const int32_t userId, const std::string &bundleName);
    static bool IsCloudStatusOkay(const std::string &bundleName, const int32_t userId);
    static int32_t ChangeAppSwitch(const std::string &bundleName, const int32_t userId, bool appSwitchStatus);
    static bool IsAccountIdChanged(const std::string &accountId);

private:
    static inline CloudFile::CloudUserInfo userInfo_;
    static inline std::map<std::string, bool> appSwitches_;
    static inline int32_t userId_{-1};
    static inline std::mutex mutex_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_STATUS_H