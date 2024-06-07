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

#ifndef FILEMANAGEMENT_DFS_SERVICE_EVENTHANDLE_DAEMON_EXECUTE_H
#define FILEMANAGEMENT_DFS_SERVICE_EVENTHANDLE_DAEMON_EXECUTE_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>

#include "daemon_event.h"
#include "daemon_eventhandler.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DaemonExecute {
public:
    explicit DaemonExecute();
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event);

private:
    void ExecutePushAsset(const AppExecFwk::InnerEvent::Pointer &event);
    std::vector<std::string> GetFileList(const std::vector<std::string> &uris,
                                         int32_t userId,
                                         const std::string &srcBundleName);
    int32_t HandleZip(const std::vector<std::string> &fileList,
                      const std::string &srcBundleName,
                      std::string &sendFileName,
                      bool &isSingleFile);
    void HandlePushAssetFail(int32_t socketId, sptr<AssetObj> &assetObj);

    using ExecuteFunc = void (DaemonExecute::*)(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<uint32_t, ExecuteFunc> executeFuncMap_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_EVENTHANDLE_DAEMON_EXECUTE_H
