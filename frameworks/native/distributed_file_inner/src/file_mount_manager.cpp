/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <file_mount_manager.h>
#include <distributed_file_daemon_proxy.h>
#include "dfs_error.h"
#include "ipc_skeleton.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD004315
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
std::shared_ptr<FileMountManager> FileMountManager::instance_ = nullptr;

std::shared_ptr<FileMountManager> FileMountManager::GetInstance()
{
    static std::once_flag once;
    std::call_once(once, []() {
        FileMountManager::instance_ = std::make_shared<FileMountManager>();
    });

    return instance_;
}

int32_t FileMountManager::GetDfsUrisDirFromLocal(const std::vector<std::string> &uriList,
                                                 const int32_t &userId,
                                                 std::unordered_map<std::string,
                                                 AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> &uriToDfsUriMaps)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);
}

}
}
}