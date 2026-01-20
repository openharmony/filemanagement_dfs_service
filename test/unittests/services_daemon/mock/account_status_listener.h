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

#ifndef CLOUD_FILE_DAEMON_ACCOUNT_STATUS_LISTENER_H
#define CLOUD_FILE_DAEMON_ACCOUNT_STATUS_LISTENER_H

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
class AccountStatusListener {
public:
    AccountStatusListener();
    ~AccountStatusListener();
    void Start();
    void Stop();
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_FILE_DAEMON_ACCOUNT_STATUS_LISTENER_H