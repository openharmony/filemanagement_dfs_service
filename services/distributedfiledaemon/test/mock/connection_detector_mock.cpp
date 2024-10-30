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

#include "connection_detector_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
std::string ConnectionDetector::GetCellByIndex(const std::string &str, int targetIndex)
{
    return "";
}

bool ConnectionDetector::MatchConnectionStatus(ifstream &inputFile)
{
    return true;
}

bool ConnectionDetector::MatchConnectionGroup(const std::string &fileName, const string &networkId)
{
    return true;
}

bool ConnectionDetector::CheckValidDir(const std::string &path)
{
    return true;
}

bool ConnectionDetector::GetConnectionStatus(const std::string &targetDir, const std::string &networkId)
{
    if (IConnectionDetectorMock::iConnectionDetectorMock_ == nullptr) {
        return false;
    }
    return IConnectionDetectorMock::iConnectionDetectorMock_->GetConnectionStatus(targetDir, networkId);
}

uint64_t ConnectionDetector::MocklispHash(const string &str)
{
    return 1;
}

int32_t ConnectionDetector::RepeatGetConnectionStatus(const std::string &targetDir, const std::string &networkId)
{
    if (IConnectionDetectorMock::iConnectionDetectorMock_ == nullptr) {
        return -1;
    }
    return IConnectionDetectorMock::iConnectionDetectorMock_->RepeatGetConnectionStatus(targetDir, networkId);
}

int32_t ConnectionDetector::GetCurrentUserId()
{
    return 0;
}

std::string ConnectionDetector::ParseHmdfsPath()
{
    return "";
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS