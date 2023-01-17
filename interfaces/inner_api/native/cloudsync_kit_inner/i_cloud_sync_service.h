/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_I_CLOUD_SYNC_SERVICE_H
#define OHOS_FILEMGMT_I_CLOUD_SYNC_SERVICE_H

#include "iremote_broker.h"

namespace OHOS::FileManagement::CloudSync {
class ICloudSyncService : public IRemoteBroker {
public:
    enum {
        SERVICE_CMD_REGISTER_CALLBACK = 0,
    };

    enum {
        CLOUD_SYNC_SERVICE_SUCCESS = 0,
        CLOUD_SYNC_SERVICE_DESCRIPTOR_IS_EMPTY,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Filemanagement.Dfs.ICloudSyncService")

protected:
    virtual int32_t RegisterCallbackInner(const std::string &appPackageName, const sptr<IRemoteObject> &callback) = 0;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_I_CLOUD_SYNC_SERVICE_H