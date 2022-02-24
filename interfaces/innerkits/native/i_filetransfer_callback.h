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
#ifndef I_FILETRANSFER_CALLBACK_H
#define I_FILETRANSFER_CALLBACK_H

#include <string>

#include "iremote_broker.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class IFileTransferCallback : public IRemoteBroker {
public:
    virtual ~IFileTransferCallback() = default;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Storage.DistributedFile.IFileTransferCallback");
    enum {
        ON_SEND_FINISHED = 0,
        ON_SEND_ERROR,
        ON_RECEIVE_FINISHED,
        ON_RECEIVE_ERROR,
        ON_SESSION_OPENED,
        ON_SESSION_CLOSED,
        ON_WRITE_FILE
    };
    // define the error code
    enum {
        DFS_CALLBACK_SUCCESS = 0,
        DFS_CALLBACK_REMOTE_ADDRESS_IS_NULL,
        DFS_CALLBACK_DESCRIPTOR_IS_EMPTY,
        DFS_CALLBACK_WRITE_REPLY_FAIL,
        DFS_CALLBACK_WRITE_REMOTE_OBJECT_FAIL,
        DFS_CALLBACK_WRITE_DESCRIPTOR_TOKEN_FAIL,
        DFS_CALLBACK_ERR_SET_FD_FAIL
    };
public:
    virtual int32_t SendFinished(const std::string &cid, std::string fileName) = 0;
    virtual int32_t SendError(const std::string &cid) = 0;
    virtual int32_t ReceiveFinished(const std::string &cid, const std::string &fileName, uint32_t num) = 0;
    virtual int32_t ReceiveError(const std::string &cid) = 0;
    virtual int32_t SessionOpened(const std::string &cid) = 0;
    virtual int32_t SessionClosed(const std::string &cid) = 0;
    virtual int32_t WriteFile(int32_t fd, const std::string &fileName) = 0;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // I_FILETRANSFER_CALLBACK_H