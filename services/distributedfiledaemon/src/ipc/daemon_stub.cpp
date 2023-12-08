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

#include "ipc/daemon_stub.h"

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "dfs_error.h"
#include "dm_device_info.h"
#include "ipc/distributed_file_daemon_ipc_interface_code.h"
#include "os_account_manager.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace OHOS::FileManagement;
using namespace std;
const std::string HMDFS_PATH = "/mnt/hmdfs/<currentUserId>/account";
const std::string SYS_HMDFS_PATH = "/sys/fs/hmdfs/";
const std::string CONNECTION_STATUS_FILE_NAME = "status";
const std::string CONNECTION_STATUS = "2";
const std::string CURRENT_USER_ID_FLAG = "<currentUserId>";
constexpr int32_t MAX_RETRY = 25;
constexpr int32_t CHECK_SESSION_DELAY_TIME = 200000;
constexpr int32_t NO_ERROR = 0;
constexpr int32_t INVALID_USER_ID = -1;
constexpr int DISMATCH = 0;
constexpr int MATCH = 1;

DaemonStub::DaemonStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION)] =
        &DaemonStub::HandleOpenP2PConnection;
    opToInterfaceMap_[static_cast<uint32_t>(
        DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION)] =
        &DaemonStub::HandleCloseP2PConnection;
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PREPARE_SESSION)] =
        &DaemonStub::HandlePrepareSession;
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REQUEST_SEND_FILE)] =
        &DaemonStub::HandleRequestSendFile;
}

int32_t DaemonStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return DFS_DAEMON_DESCRIPTOR_IS_EMPTY;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

static std::string GetLineFromFile(const std::string &fileName, int targetIndex)
{
    ifstream statusFile(fileName);
    std::string str;
    int lineIndex = 0;
    while (lineIndex++ < targetIndex) {
        getline(statusFile, str);
        if (str.empty()) {
            statusFile.close();
            return "";
        }
    }
    statusFile.close();
    return str;
}

static std::string ParseConnectionStatus(const std::string &fileName)
{
    auto line = GetLineFromFile(fileName, 4);
    int count = 0;
    std::regex regex("\\s+");
    std::sregex_token_iterator iter(line.begin(), line.end(), regex, -1);
    std::sregex_token_iterator end{};

    while (iter != end) {
        ++iter;
        ++count;
        if (count == 2) {
            auto result = iter->str();
            if (result == CONNECTION_STATUS) {
                LOGI("get connect status success, status = %{public}s", result.c_str());
                return result;
            }
        }
    }
    return "";
}

static int FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return DISMATCH;
    }
    return MATCH;
}

struct NameList {
    struct dirent **namelist = { nullptr };
    int direntNum = 0;
};

static void Deleter(struct NameList *arg)
{
    for (int i = 0; i < arg->direntNum; i++) {
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
}

static bool CheckValidDir(const std::string &path)
{
    struct stat buf{};
    auto ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        LOGE("stat failed, errno = %{public}d", errno);
        return false;
    }
    if ((buf.st_mode & S_IFMT) != S_IFDIR) {
        LOGE("It is not a dir.");
        return false;
    }
    return true;
}

static bool GetConnectionStatus(const std::string &targetDir)
{
    if (!CheckValidDir(SYS_HMDFS_PATH)) {
        return false;
    }
    unique_ptr<struct NameList, decltype(Deleter) *> pNameList = { new (nothrow) struct NameList, Deleter };
    if (pNameList == nullptr) {
        LOGE("Failed to request heap memory.");
        return false;
    }
    int num = scandir(SYS_HMDFS_PATH.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
    pNameList->direntNum = num;
    for (int i = 0; i < num; i++) {
        string dest = SYS_HMDFS_PATH + '/' + string((pNameList->namelist[i])->d_name);
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            dest = dest + '/' + CONNECTION_STATUS_FILE_NAME;
            if (ParseConnectionStatus(dest) == CONNECTION_STATUS) {
                return true;
            }
        }
    }
    return false;
}

static uint64_t MocklispHash(const string &str)
{
    uint64_t res = 0;
    constexpr int mocklispHashPos = 5;
    /* Mocklisp hash function. */
    for (auto ch : str) {
        res = (res << mocklispHashPos) - res + (uint64_t)ch;
    }
    return res;
}

static int32_t RepeatGetConnectionStatus(const std::string &targetDir)
{
    int retryCount = 0;
    while (retryCount++ < MAX_RETRY && !GetConnectionStatus(targetDir)) {
        usleep(CHECK_SESSION_DELAY_TIME);
    }
    return retryCount == MAX_RETRY ? -1 : NO_ERROR;
}

static int32_t GetCurrentUserId()
{
    std::vector<int32_t> userIds{};
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != NO_ERROR || userIds.empty()) {
        LOGE("query active os account id failed");
        return INVALID_USER_ID;
    }
    return userIds[0];
}

static std::string ParseHmdfsPath()
{
    auto userId = GetCurrentUserId();
    if (userId == INVALID_USER_ID) {
        return "";
    }
    std::string path = HMDFS_PATH;
    size_t pos = path.find(CURRENT_USER_ID_FLAG);
    if (pos == std::string::npos) {
        return "";
    }
    return path.replace(pos, CURRENT_USER_ID_FLAG.length(), std::to_string(userId));
}

int32_t DaemonStub::HandleOpenP2PConnection(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin OpenP2PConnection");
    DistributedHardware::DmDeviceInfo deviceInfo;
    auto ret = strcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source device id failed, ret is %{public}d", ret);
        return -1;
    }
    ret = strcpy_s(deviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source device name failed, ret is %{public}d", ret);
        return -1;
    }
    ret = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source network id failed, ret is %{public}d", ret);
        return -1;
    }
    deviceInfo.deviceTypeId = data.ReadUint16();
    deviceInfo.range = static_cast<int32_t>(data.ReadUint32());
    deviceInfo.authForm = static_cast<DistributedHardware::DmAuthForm>(data.ReadInt32());
    int32_t res = OpenP2PConnection(deviceInfo);
    if (res != NO_ERROR) {
        LOGE("OpenP2PConnection failed, ret = %{public}d", res);
        return res;
    }
    auto path = ParseHmdfsPath();
    auto hashedPath = MocklispHash(path);
    stringstream ss;
    ss << hashedPath;
    auto targetDir = ss.str();
    if (!GetConnectionStatus(targetDir)) {
        ret = OpenP2PConnection(deviceInfo);
        if (ret != NO_ERROR) {
            LOGE("OpenP2PConnection failed, ret = %{public}d", ret);
        } else {
            ret = RepeatGetConnectionStatus(targetDir);
        }
    }
    reply.WriteInt32(ret);
    LOGI("OpenP2PConnection end, res = %{public}d.", ret);
    return ret;
}

int32_t DaemonStub::HandleCloseP2PConnection(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin CloseP2PConnection");
    DistributedHardware::DmDeviceInfo deviceInfo;
    auto ret = strcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source device id failed, ret is %{public}d", ret);
        return -1;
    }
    ret = strcpy_s(deviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source device name failed, ret is %{public}d", ret);
        return -1;
    }
    ret = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source network id failed, ret is %{public}d", ret);
        return -1;
    }
    deviceInfo.deviceTypeId = data.ReadUint16();
    deviceInfo.range = static_cast<int32_t>(data.ReadUint32());
    deviceInfo.authForm = static_cast<DistributedHardware::DmAuthForm>(data.ReadInt32());

    int32_t res = CloseP2PConnection(deviceInfo);
    reply.WriteInt32(res);
    LOGI("End CloseP2PConnection");
    return res;
}

int32_t DaemonStub::HandlePrepareSession(MessageParcel &data, MessageParcel &reply)
{
    std::string srcUri;
    if (!data.ReadString(srcUri)) {
        LOGE("read srcUri failed");
        return E_IPC_READ_FAILED;
    }
    std::string dstUri;
    if (!data.ReadString(dstUri)) {
        LOGE("read dstUri failed");
        return E_IPC_READ_FAILED;
    }
    std::string srcDeviceId;
    if (!data.ReadString(srcDeviceId)) {
        LOGE("read srcDeviceId failed");
        return E_IPC_READ_FAILED;
    }
    auto listener = data.ReadRemoteObject();
    if (listener == nullptr) {
        LOGE("read listener failed");
        return E_IPC_READ_FAILED;
    }

    int32_t res = PrepareSession(srcUri, dstUri, srcDeviceId, listener);
    reply.WriteInt32(res);
    LOGD("End PrepareSession, ret = %{public}d.", res);
    return res;
}

int32_t DaemonStub::HandleRequestSendFile(MessageParcel &data, MessageParcel &reply)
{
    std::string srcUri;
    if (!data.ReadString(srcUri)) {
        LOGE("read srcUri failed");
        return E_IPC_READ_FAILED;
    }
    std::string dstPath;
    if (!data.ReadString(dstPath)) {
        LOGE("read dstPath failed");
        return E_IPC_READ_FAILED;
    }
    std::string dstDeviceId;
    if (!data.ReadString(dstDeviceId)) {
        LOGE("read remoteDeviceId failed");
        return E_IPC_READ_FAILED;
    }
    std::string sessionName;
    if (!data.ReadString(sessionName)) {
        LOGE("read sessionName failed");
        return E_IPC_READ_FAILED;
    }
    auto res = RequestSendFile(srcUri, dstPath, dstDeviceId, sessionName);
    reply.WriteInt32(res);
    LOGD("End RequestSendFile, ret = %{public}d.", res);
    return res;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS