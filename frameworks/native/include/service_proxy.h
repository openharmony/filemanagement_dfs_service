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

#ifndef DISTRIBUTEDFILE_SERVICE_PROXY_H
#define DISTRIBUTEDFILE_SERVICE_PROXY_H

#include "iremote_proxy.h"
#include "i_distributedfile_service.h"
#include "message_parcel.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class ServiceProxy : public IRemoteProxy<IDistributedFileService> {
public:
    /**
     * ServiceProxy
     *
     * @param impl
     */
    explicit ServiceProxy(const sptr<IRemoteObject> &impl);

    virtual ~ServiceProxy();

    int32_t SendFile(const std::string &cid,
                     const std::vector<std::string> &sourceFileList,
                     const std::vector<std::string> &destinationFileList,
                     const uint32_t fileCount) override;
    int32_t OpenFile(int32_t fd, const std::string &fileName, int32_t mode) override;
    int32_t RegisterNotifyCallback(sptr<IFileTransferCallback> &callback) override;
    int32_t UnRegisterNotifyCallback() override;
    int32_t IsDeviceOnline(const std::string &cid) override;
private:
    static inline BrokerDelegator<ServiceProxy> delegator_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DISTRIBUTEDFILE_SERVICE_PROXY_H