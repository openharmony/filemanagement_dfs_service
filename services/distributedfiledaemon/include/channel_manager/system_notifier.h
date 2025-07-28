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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SYSTEM_NOTIFIER_H
#define FILEMANAGEMENT_DFS_SERVICE_SYSTEM_NOTIFIER_H

#include "pixel_map.h"
#include <iostream>
#include <mutex>
#include <utility>
#include <shared_mutex>
#include <string>
#include <unistd.h>
#include <vector>

namespace OHOS {
namespace Storage {
namespace DistributedFile {

class SystemNotifier final{
    DECLARE_SINGLE_INSTANCE_BASE(SystemNotifier);
public:
    ~SystemNotifier() = default;

    int32_t CreateLocalLiveView(const std::string &networkId);
    int32_t CreateNotification(const std::string &networkId);
    int32_t DestroyNotifyByNetworkId(const std::string &networkId);
    int32_t DestroyNotifyByNotificationId(int32_t notifyId);

private:
    SystemNotifier();
    bool GetPixelMap(const std::string &path, std::shared_ptr<Media::PixelMap> &pixelMap);
    int32_t GenerateNextNotificationId();

    int32_t DisconnectByNetworkId(const std::string &networkId);
    std::string GetRemoteDeviceName(const std::string &networkId);

    void UpdateResourceMap(const std::string &resourcePath);
    void UpdateResourceMapByLanguage();

    template <typename... Args>
    std::string GetKeyValue(const std::string &key, Args &&...args);

private:
    std::shared_ptr<Media::PixelMap> capsuleIconPixelMap_{};
    std::shared_ptr<Media::PixelMap> notificationIconPixelMap_{};
    std::shared_ptr<Media::PixelMap> buttonIconPixelMap_{};

    // networkId -> notificationId
    std::shared_mutex mutex_;
    std::map<std::string, int32_t> notificationMap_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SYSTEM_NOTIFIER_H