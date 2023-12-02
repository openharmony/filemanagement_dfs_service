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

#ifndef FILE_TRANS_LISTENER_PROXY_H
#define FILE_TRANS_LISTENER_PROXY_H

#include <string>

#include "dm_device_info.h"
#include "i_file_trans_listener.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class FileTransListenerProxy : public IRemoteProxy<IFileTransListener> {
public:
    explicit FileTransListenerProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IFileTransListener>(object) {}
    ~FileTransListenerProxy() override {}
    int32_t OnFileReceive(uint64_t totalBytes, uint64_t processedBytes) override;
    int32_t OnFailed(const std::string &sessionName) override;
    int32_t OnFinished(const std::string &sessionName) override;

private:
    static inline BrokerDelegator<FileTransListenerProxy> delegator_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILE_TRANS_LISTENER_PROXY_H
