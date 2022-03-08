/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef I_DISTRIBUTEDFILE_SERVICE_H
#define I_DISTRIBUTEDFILE_SERVICE_H

#include "iremote_broker.h"
#include "i_filetransfer_callback.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class IDistributedFileService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedFile.IDistributedFileService");
    // define the message code
    enum DistributedFileSurfaceCode {
        INTERFACE1 = 0,
        SEND_FILE_DISTRIBUTED,
        OPEN_FILE_FD,
        REGISTER_NOTIFY_CALLBACK,
        UN_REGISTER_NOTIFY_CALLBACK,
        IS_DEVICE_ONLINE
    };
    // define the error code
    enum {
        DFS_SUCCESS = 0,
        DFS_NO_ERROR = 0,
        DFS_SENDFILE_SUCCESS = 0,
        DFS_REMOTE_ADDRESS_IS_NULL = -100,
        DFS_DESCRIPTOR_IS_EMPTY,
        DFS_SESSION_ID_IS_EMPTY,
        DFS_WRITE_REPLY_FAIL,
        DFS_WRITE_REMOTE_OBJECT_FAIL,
        DFS_WRITE_DESCRIPTOR_TOKEN_FAIL,
        DFS_CALLBACK_PARAM_ERROR,
        DFS_NO_SUCH_FILE,
        DFS_NO_DEVICE_ONLINE,
        DFS_PARAM_FILE_COUNT_ERROR,
        DFS_PARAM_DEVICE_ID_ERROR,
        DFS_MEM_ERROR,
        DFS_SOFTBUS_SEND_ERROR,
        DFS_FILE_OP_ERROR,
        DFS_NOTIFY_CALLBACK_EMPTY,
    };
    enum {
        DEVICE_NOT_ONLINE,
        DEVICE_ONLINE,
    };
    virtual int32_t SendFile(const std::string &cid,
                             const std::vector<std::string> &sourceFileList,
                             const std::vector<std::string> &destinationFileList,
                             const uint32_t fileCount) = 0;
    virtual int32_t OpenFile(int32_t fd, const std::string &fileName, int32_t mode) = 0;
    virtual int32_t RegisterNotifyCallback(sptr<IFileTransferCallback> &callback) = 0;
    virtual int32_t UnRegisterNotifyCallback() = 0;
    virtual int32_t IsDeviceOnline(const std::string &cid) = 0;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // I_DISTRIBUTEDFILE_SERVICE_H