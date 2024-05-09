/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef I_DAEMON_H
#define I_DAEMON_H

#include "dm_device_info.h"
#include "iremote_broker.h"
#include "hmdfs_info.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class IDaemon : public IRemoteBroker {
public:
    enum {
        DFS_DAEMON_SUCCESS = 0,
        DFS_DAEMON_DESCRIPTOR_IS_EMPTY,
    };
    virtual int32_t OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) = 0;
    virtual int32_t CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) = 0;
    virtual int32_t PrepareSession(const std::string &srcUri,
                                   const std::string &dstUri,
                                   const std::string &srcDeviceId,
                                   const sptr<IRemoteObject> &listener,
                                   HmdfsInfo &info) = 0;
    virtual int32_t CancelCopyTask(const std::string &sessionName) = 0;
    virtual int32_t RequestSendFile(const std::string &srcUri,
                                    const std::string &dstPath,
                                    const std::string &remoteDeviceId,
                                    const std::string &sessionName) = 0;
    static inline const std::string SERVICE_NAME { "ohos.storage.distributedfile.daemon" };
    virtual int32_t GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.storage.distributedfile.daemon")
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // I_DAEMON_H