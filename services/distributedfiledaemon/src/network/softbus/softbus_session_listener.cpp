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
const std::string FILE_SCHEMA = "file://";
const std::string DOCS = "file://docs";
const std::string NETWORK_ID = "?networkid=";
const std::string FILE_SEPARATOR = "/";
const std::string DISTRIBUTED_PATH = "distributedfiles/.remote_share/";
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
    if (!Utils::IsFolder(path)) {
        int pos = path.rfind("/");
        tmp.push_back(path.substr(pos + 1));
    } else {
        for (const auto &it : fileList) {
            tmp.push_back(it.substr(path.size() + 1));
        }
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

    std::string physicalPath = GetRealPath(sessionInfo.srcUri);
    if (physicalPath.empty()) {
        LOGE("GetRealPath failed");
        return E_GET_PHYSICAL_PATH_FAILED;
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

std::string SoftBusSessionListener::GetLocalUri(const std::string &uri)
{
    int pos = uri.find(NETWORK_ID);
    if (pos <= 0) {
        return "";
    }
    return uri.substr(0, pos);
}

std::string SoftBusSessionListener::GetBundleName(const std::string &uri)
{
    int pos = uri.find(FILE_SCHEMA);
    if (pos < 0) {
        return "";
    }
    auto tmpUri = uri.substr(pos + FILE_SCHEMA.size());
    if (tmpUri.empty()) {
        return "";
    }
    int bundleNamePos = tmpUri.find(FILE_SEPARATOR);
    if (bundleNamePos <= 0) {
        return "";
    }
    return tmpUri.substr(0, bundleNamePos);
}

std::string SoftBusSessionListener::GetSandboxPath(const std::string &uri)
{
    int pos = uri.find(DISTRIBUTED_PATH);
    if (pos <= 0) {
        return "";
    }
    return uri.substr(pos + DISTRIBUTED_PATH.size());
}

std::string SoftBusSessionListener::GetRealPath(const std::string &srcUri)
{
    std::string localUri = GetLocalUri(srcUri);
    if (localUri.empty()) {
        LOGE("get local uri failed.");
        return localUri;
    }
    if (localUri.rfind(DOCS, 0) != 0) {
        std::string bundleName = GetBundleName(localUri);
        if (bundleName.empty()) {
            LOGI("get bundleName failed.");
            return "";
        }

        auto sandboxPath = GetSandboxPath(localUri);
        if (sandboxPath.empty()) {
            LOGI("get sandboxPath failed.");
            return "";
        }
        localUri = FILE_SCHEMA + bundleName + FILE_SEPARATOR + sandboxPath;
    }
    std::string physicalPath;
    if (SandboxHelper::GetPhysicalPath(localUri, std::to_string(QueryActiveUserId()), physicalPath) != E_OK) {
        LOGE("GetPhysicalPath failed, invalid uri");
        return "";
    }
    if (!SandboxHelper::CheckValidPath(physicalPath)) {
        LOGE("invalid path.");
        return "";
    }
    return physicalPath;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS