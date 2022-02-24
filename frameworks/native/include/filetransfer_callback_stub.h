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
#ifndef  FILETRANSFER_CALLBACK_STUB_H
#define  FILETRANSFER_CALLBACK_STUB_H

#include <map>

#include "iremote_stub.h"
#include "i_filetransfer_callback.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class FileTransferCallbackStub : public IRemoteStub<IFileTransferCallback> {
public:
    FileTransferCallbackStub();
    virtual ~FileTransferCallbackStub();

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using NotifyCallbackFunc = int32_t (FileTransferCallbackStub::*)(MessageParcel &, MessageParcel &);

private:
    int32_t CmdOnSessionOpened(MessageParcel &data, MessageParcel &reply);
    int32_t CmdOnSessionClosed(MessageParcel &data, MessageParcel &reply);
    int32_t CmdOnSendFinished(MessageParcel &data, MessageParcel &reply);
    int32_t CmdOnSendError(MessageParcel &data, MessageParcel &reply);
    int32_t CmdOnReceiveFinished(MessageParcel &data, MessageParcel &reply);
    int32_t CmdOnReceiveError(MessageParcel &data, MessageParcel &reply);
    int32_t CmdWriteFile(MessageParcel &data, MessageParcel &reply);
private:
    std::map<uint32_t, NotifyCallbackFunc> memberFuncMap_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILETRANSFER_CALLBACK_STUB_H