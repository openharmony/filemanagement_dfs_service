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
#ifndef FILEMANAGEMENT_DFS_SERVICE_CONNECTION_DETECTOR_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_CONNECTION_DETECTOR_MOCK_H

#include <gmock/gmock.h>
#include <memory>
#include "connection_detector.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class IConnectionDetectorMock {
public:
    IConnectionDetectorMock() = default;
    virtual ~IConnectionDetectorMock() = default;
    virtual bool GetConnectionStatus(const std::string &targetDir, const std::string &networkId) = 0;
    virtual int32_t RepeatGetConnectionStatus(const std::string &targetDir, const std::string &networkId) = 0;

public:
    static inline std::shared_ptr<IConnectionDetectorMock> iConnectionDetectorMock_ = nullptr;
};

class ConnectionDetectorMock : public IConnectionDetectorMock {
public:
    MOCK_METHOD2(GetConnectionStatus, bool(const std::string &targetDir, const std::string &networkId));
    MOCK_METHOD2(RepeatGetConnectionStatus, int32_t(const std::string &targetDir, const std::string &networkId));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_CONNECTION_DETECTOR_MOCK_H
