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

#ifndef OHOS_STORAGE_CLOUD_DAEMON_MANAGER_H
#define OHOS_STORAGE_CLOUD_DAEMON_MANAGER_H

#include <memory>

namespace OHOS::FileManagement::CloudFile {
using namespace std;
class CloudDaemonManager {
public:
    static CloudDaemonManager &GetInstance();

    virtual int32_t StartFuse(int32_t userId, int32_t devFd, const string &path) = 0;
};
} // namespace OHOS::FileManagement::CloudFile

#endif // OHOS_STORAGE_CLOUD_DAEMON_MANAGER_H
