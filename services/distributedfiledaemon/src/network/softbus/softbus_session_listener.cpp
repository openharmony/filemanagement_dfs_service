/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "network/softbus/softbus_session_listener.h"

#include "dfs_error.h"
#include "network/softbus/softbus_session_pool.h"
#include "os_account_manager.h"
#include "sandbox_helper.h"
#include "string"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr size_t MAX_SIZE = 500;
constexpr int32_t SESSION_NAME_SIZE = 128;
constexpr int32_t DEFAULT_USER_ID = 100;
using namespace OHOS::AppFileService;
using namespace OHOS::FileManagement;

int32_t SoftBusSessionListener::QueryActiveUserId()
{
    std::vector<int32_t> ids;
    ErrCode errCode = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (errCode != ERR_OK || ids.empty()) {
        LOGE("Query active userid failed, errCode: %{public}d, ", errCode);
        return DEFAULT_USER_ID;
    }
    return ids[0];
}

std::vector<std::string> SoftBusSessionListener::GetFileName(const std::vector<std::string> &fileList,
                                                             const std::string &path)
{
    std::vector<std::string> tmp;
    for (const auto &it : fileList) {
        tmp.push_back(it.substr(path.size() + 1));
    }
    return tmp;
}

int SoftBusSessionListener::OnSessionOpened(int sessionId, int result)
{
    int side = GetSessionSide(sessionId);
    if (side != 1) {
        LOGI("GetSessionSide, sessionId = %{public}d,  side = %{public}d", sessionId, side);
        return E_OK;
    }

    char sessionName[SESSION_NAME_SIZE] = {};
    auto ret = ::GetMySessionName(sessionId, sessionName, SESSION_NAME_SIZE);
    if (ret != E_OK) {
        LOGE("GetMySessionName failed, ret = %{public}d", ret);
        return E_SOFTBUS_SESSION_FAILED;
    }

    SoftBusSessionPool::SessionInfo sessionInfo;
    ret = SoftBusSessionPool::GetInstance().GetSessionInfo(sessionName, sessionInfo);
    if (!ret) {
        LOGE("GetSessionInfo failed");
        return E_SOFTBUS_SESSION_FAILED;
    }

    std::string physicalPath;
    ret = SandboxHelper::GetPhysicalPath(sessionInfo.srcUri, std::to_string(QueryActiveUserId()), physicalPath);
    if (ret != E_OK) {
        LOGE("GetPhysicalPath failed, invalid uri. ret = %{public}d", ret);
        return ret;
    }
    if (!SandboxHelper::CheckValidPath(physicalPath)) {
        LOGE("invalid path, ret = %{public}d", ret);
        return E_OK;
    }
    auto fileList = OHOS::Storage::DistributedFile::Utils::GetFilePath(physicalPath);
    if (fileList.empty()) {
        LOGE("GetFilePath failed or file is empty");
        return E_OK;
    }

    const char *src[MAX_SIZE] = {};
    for (size_t i = 0; i < fileList.size() && fileList.size() < MAX_SIZE; i++) {
        src[i] = fileList.at(i).c_str();
    }

    auto fileNameList = GetFileName(fileList, physicalPath);
    if (fileNameList.empty()) {
        LOGE("GetFileName failed or file is empty");
        return E_OK;
    }
    const char *dst[MAX_SIZE] = {};
    for (size_t i = 0; i < fileNameList.size() && fileList.size() < MAX_SIZE; i++) {
        dst[i] = fileNameList.at(i).c_str();
    }

    ret = ::SendFile(sessionId, src, dst, static_cast<uint32_t>(fileList.size()));
    if (ret != E_OK) {
        LOGE("SendFile failed, sessionId = %{public}d", sessionId);
        SoftBusSessionPool::GetInstance().DeleteSessionInfo(sessionName);
    }
    return ret;
}

void SoftBusSessionListener::OnSessionClosed(int sessionId)
{
    LOGI("OnSessionClosed, sessionId = %{public}d", sessionId);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS