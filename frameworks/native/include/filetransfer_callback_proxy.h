/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef FILETRANSFER_CALLBACK_PROXY_H
#define FILETRANSFER_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "i_filetransfer_callback.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class FileTransferCallbackProxy : public IRemoteProxy<IFileTransferCallback> {
public:
    explicit FileTransferCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IFileTransferCallback>(impl) {}
    virtual ~FileTransferCallbackProxy() {}

public:
    int32_t SessionOpened(const std::string &cid) override;
    int32_t SessionClosed(const std::string &cid) override;
    int32_t SendFinished(const std::string &cid, std::string fileName) override;
    int32_t SendError(const std::string &cid) override;
    int32_t ReceiveFinished(const std::string &cid, const std::string &fileName, uint32_t num) override;
    int32_t ReceiveError(const std::string &cid) override;
    int32_t WriteFile(int32_t fd, const std::string &fileName) override;
private:
    static inline BrokerDelegator<FileTransferCallbackProxy> delegator_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILETRANSFER_CALLBACK_PROXY_H