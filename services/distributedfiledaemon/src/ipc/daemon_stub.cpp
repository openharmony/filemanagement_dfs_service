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
constexpr int CID_MAX_LEN = 64;

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

static std::string ParseWorkFromString(const std::string &str, int targetIndex)
{
    size_t begin = 0;
    size_t end = 0;
    int count = 0;
    do {
        size_t i;
        for (i = begin; i < str.size(); ++i) {
            if (str[i] != '\t' && str[i] != ' ') {
                break;
            }
        }
        begin = i;
        for (i = begin; i < str.size(); ++i) {
            if (str[i] != '\t' && str[i] != ' ') {
                break;
            }
        }
        end = i;
        if (end == str.size()) {
            break;
        }
        ++count;
        if (count == targetIndex) {
            break;
        }
        begin = end;
    } while (true);
    return str.substr(begin, end - begin);
}

static std::string ParseConnectionStatus(const std::string &fileName, const std::string &networkId)
{
    auto line = GetLineFromFile(fileName, 2);
    auto cid = ParseWorkFromString(line, 1);
    if (cid != networkId) {
        LOGI("parse networkId = %{public}s", cid.c_str());
        return "";
    }
    line = GetLineFromFile(fileName, 4);
    return ParseWorkFromString(line, 2);
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

static bool GetConnectionStatus(const std::string &targetDir, const std::string &networkId)
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
        if ((pNameList->namelist[i])->d_name != targetDir) {
            continue;
        }
        string dest = SYS_HMDFS_PATH + '/' + string((pNameList->namelist[i])->d_name);
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            dest = dest + '/' + CONNECTION_STATUS_FILE_NAME;
            if (ParseConnectionStatus(dest, networkId) == CONNECTION_STATUS) {
                LOGI("ParseConnectionStatus success.");
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

static int32_t RepeatGetConnectionStatus(const std::string &targetDir, const std::string &networkId)
{
    int retryCount = 0;
    while (retryCount++ < MAX_RETRY && !GetConnectionStatus(targetDir, networkId)) {
        usleep(CHECK_SESSION_DELAY_TIME);
    }
    return retryCount == MAX_RETRY ? -1 : NO_ERROR;
}

static int32_t GetCurrentUserId()
{
    std::vector<int32_t> userIds{};
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != NO_ERROR || userIds.empty()) {
        LOGE("query active os account id failed, ret = %{public}d", ret);
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

static std::string GetKernelNetworkId(const std::string &networkId)
{
    uint8_t cid[CID_MAX_LEN];
    if (memcpy_s(cid, CID_MAX_LEN, networkId.c_str(), CID_MAX_LEN)) {
        return "";
    }
    std::ostringstream oss;
    for (int i = 0; i < CID_MAX_LEN; ++i) {
        oss << cid[i];
    }
    return oss.str();
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
    auto kernelCid = GetKernelNetworkId(std::string(deviceInfo.networkId));
    auto path = ParseHmdfsPath();
    auto hashedPath = MocklispHash(path);
    stringstream ss;
    ss << hashedPath;
    auto targetDir = ss.str();
    if (!GetConnectionStatus(targetDir, kernelCid)) {
        ret = OpenP2PConnection(deviceInfo);
        if (ret != NO_ERROR) {
            LOGE("OpenP2PConnection failed, ret = %{public}d", ret);
        } else {
            ret = RepeatGetConnectionStatus(targetDir, kernelCid);
        }
    }
    reply.WriteInt32(ret);
    LOGI("OpenP2PConnection end, ret = %{public}d.", ret);
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