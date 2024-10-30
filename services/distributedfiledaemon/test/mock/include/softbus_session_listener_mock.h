/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_LISTENER_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_LISTENER_MOCK_H

#include <gmock/gmock.h>
#include "network/softbus/softbus_session_listener.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class ISoftBusSessionListenerMock {
public:
    ISoftBusSessionListenerMock() = default;
    virtual ~ISoftBusSessionListenerMock() = default;
    virtual std::string GetBundleName(const std::string &uri);
    virtual std::string GetRealPath(const std::string &srcUri);
    virtual std::vector<std::string> GetFileName(const std::vector<std::string> &fileList, const std::string &path,
                                                 const std::string &dstPath);
public:
    static inline std::shared_ptr<ISoftBusSessionListenerMock> iSoftBusSessionListenerMock_ = nullptr;
};

class SoftBusSessionListenerMock : public ISoftBusSessionListenerMock {
public:
    MOCK_METHOD1(GetBundleName, std::string(const std::string &uri));
    MOCK_METHOD1(GetRealPath, std::string(const std::string &srcUri));
    MOCK_METHOD3(GetFileName, std::vector<std::string>(const std::vector<std::string> &fileList,
                                                       const std::string &path, const std::string &dstPath));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_LISTENER_MOCK_H
