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

#include "network/softbus/softbus_handler_asset.h"

#include <memory>
#include <regex>
#include <sstream>
#include <fstream>

#include "asset_callback_mananger.h"
#include "device_manager.h"
#include "dfs_error.h"
#include "dm_device_info.h"
#include "ipc_skeleton.h"
#include "network/softbus/softbus_asset_recv_listener.h"
#include "network/softbus/softbus_asset_send_listener.h"
#include "network/softbus/softbus_session_listener.h"
#include "network/softbus/softbus_session_pool.h"
#include "utils_log.h"
#include "refbase.h"
#include "softbus_bus_center.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace OHOS::FileManagement;
constexpr size_t MAX_SIZE = 500;
constexpr size_t BUFFER_SIZE = 512;
const int32_t DFS_QOS_TYPE_MIN_BW = 90 * 1024 * 1024;
const int32_t DFS_QOS_TYPE_MAX_LATENCY = 10000;
const int32_t DFS_QOS_TYPE_MIN_LATENCY = 2000;
const uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
const std::string RELATIVE_PATH_FLAG = "/account/device_view/local/data/";
const std::string DST_BUNDLE_NAME_FLAG = "/";
const std::string TEMP_DIR = "ASSET_TEMP";
const std::string ASSET_POSTFIX_SINGLE = ".asset_single?";
const std::string ASSET_POSTFIX_ZIP = ".asset_zip?";
SoftBusHandlerAsset::SoftBusHandlerAsset()
{
    ISocketListener fileSendListener;
    fileSendListener.OnBind = nullptr;
    fileSendListener.OnShutdown = DistributedFile::SoftBusSessionListener::OnSessionClosed;
    fileSendListener.OnFile = SoftBusAssetSendListener::OnFile;
    fileSendListener.OnBytes = nullptr;
    fileSendListener.OnMessage = nullptr;
    fileSendListener.OnQos = nullptr;
    sessionListener_[DFS_ASSET_ROLE_SEND] = fileSendListener;

    ISocketListener fileReceiveListener;
    fileReceiveListener.OnBind = SoftbusAssetRecvListener::OnAssetRecvBind;
    fileReceiveListener.OnShutdown = DistributedFile::SoftBusSessionListener::OnSessionClosed;
    fileReceiveListener.OnFile = SoftbusAssetRecvListener::OnFile;
    fileReceiveListener.OnBytes = nullptr;
    fileReceiveListener.OnMessage = nullptr;
    fileReceiveListener.OnQos = nullptr;
    sessionListener_[DFS_ASSET_ROLE_RECV] = fileReceiveListener;
}

SoftBusHandlerAsset::~SoftBusHandlerAsset() = default;

SoftBusHandlerAsset &SoftBusHandlerAsset::GetInstance()
{
    LOGD("SoftBusHandlerAsset::GetInstance");
    static SoftBusHandlerAsset assetHandle;
    return assetHandle;
}

void SoftBusHandlerAsset::CreateAssetLocalSessionServer()
{
    LOGI("CreateAssetLocalSessionServer Enter.");
    {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        if (serverIdMap_.find(ASSET_LOCAL_SESSION_NAME) != serverIdMap_.end()) {
            LOGI("%s: Session already create.", ASSET_LOCAL_SESSION_NAME.c_str());
            return;
        }
    }

    SocketInfo serverInfo = {
        .name = const_cast<char*>(ASSET_LOCAL_SESSION_NAME.c_str()),
        .pkgName = const_cast<char*>(SERVICE_NAME.c_str()),
        .dataType = DATA_TYPE_FILE,
    };
    int32_t socketId = Socket(serverInfo);
    if (socketId < E_OK) {
        LOGE("Create Socket fail socketId, socketId = %{public}d", socketId);
        return;
    }
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };
    int32_t ret = Listen(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener_[DFS_ASSET_ROLE_RECV]);
    if (ret != E_OK) {
        LOGE("Listen SocketClient error");
        Shutdown(socketId);
        return;
    }
    SoftBusSessionPool::SessionInfo sessionInfo{.uid = IPCSkeleton::GetCallingUid()};
    SoftBusSessionPool::GetInstance().AddSessionInfo(ASSET_LOCAL_SESSION_NAME, sessionInfo);
    {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        serverIdMap_.insert(std::make_pair(ASSET_LOCAL_SESSION_NAME, socketId));
    }
    LOGI("CreateAssetLocalSessionServer Success.");
}

void SoftBusHandlerAsset::DeleteAssetLocalSessionServer()
{
    LOGI("DeleteAssetLocalSessionServer Enter.");
    if (!serverIdMap_.empty()) {
        std::lock_guard<std::mutex> lock(serverIdMapMutex_);
        auto it = serverIdMap_.find(ASSET_LOCAL_SESSION_NAME);
        if (it == serverIdMap_.end()) {
            LOGI("%s: Session already delete.", ASSET_LOCAL_SESSION_NAME.c_str());
            return;
        }
        int32_t socketId = it->second;
        serverIdMap_.erase(it);
        Shutdown(socketId);
        LOGI("RemoveSessionServer success.");
    }
    SoftBusSessionPool::GetInstance().DeleteSessionInfo(ASSET_LOCAL_SESSION_NAME);
}

int32_t SoftBusHandlerAsset::AssetBind(const std::string &dstNetworkId, int32_t &socketId)
{
    if (dstNetworkId.empty()) {
        LOGI("The parameter is empty");
        return E_OPEN_SESSION;
    }
    LOGI("AssetBind Enter.");
    if (!IsSameAccount(dstNetworkId)) {
        LOGI("The source and sink device is not same account, not support.");
        return E_OPEN_SESSION;
    }
    QosTV qos[] = {
        {.qos = QOS_TYPE_MIN_BW,        .value = DFS_QOS_TYPE_MIN_BW},
        {.qos = QOS_TYPE_MAX_LATENCY,        .value = DFS_QOS_TYPE_MAX_LATENCY},
        {.qos = QOS_TYPE_MIN_LATENCY,        .value = DFS_QOS_TYPE_MIN_LATENCY},
    };
    SocketInfo clientInfo = {
        .name = const_cast<char*>((ASSET_LOCAL_SESSION_NAME.c_str())),
        .peerName = const_cast<char*>(ASSET_LOCAL_SESSION_NAME.c_str()),
        .peerNetworkId = const_cast<char*>(dstNetworkId.c_str()),
        .pkgName = const_cast<char*>(SERVICE_NAME.c_str()),
        .dataType = DATA_TYPE_FILE,
    };

    socketId = Socket(clientInfo);
    if (socketId < E_OK) {
        LOGE("Create OpenSoftbusChannel Socket error");
        return E_OPEN_SESSION;
    }

    int32_t ret = Bind(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessionListener_[DFS_ASSET_ROLE_SEND]);
    if (ret != E_OK) {
        LOGE("Bind SocketClient error");
        Shutdown(socketId);
        return ret;
    }
    LOGI("OpenSession success.");
    return E_OK;
}

int32_t SoftBusHandlerAsset::AssetSendFile(int32_t socketId, const std::string& sendFile, bool isSingleFile)
{
    auto assetObj = GetAssetObj(socketId);
    if (assetObj == nullptr) {
        LOGE("get assetObj fail.");
        return ERR_BAD_VALUE;
    }
    if (!IsSameAccount(assetObj->dstNetworkId_)) {
        LOGI("The source and sink device is not same account, not support.");
        return ERR_BAD_VALUE;
    }

    const char *src[MAX_SIZE] = {};
    src[0] = sendFile.c_str();

    auto dstFile = GetDstFile(sendFile, assetObj->srcBundleName_,
                              assetObj->dstBundleName_, assetObj->sessionId_, isSingleFile);
    if (dstFile.empty()) {
        LOGE("GetFileName failed or file is empty");
        return ERR_BAD_VALUE;
    }
    const char *dst[MAX_SIZE] = {};
    dst[0] = dstFile.c_str();

    LOGI("AssetSendFile Enter.");
    int32_t ret = ::SendFile(socketId, src, dst, 1);
    if (ret != E_OK) {
        LOGE("SendFile failed, sessionId = %{public}d", socketId);
        return ret;
    }
    return E_OK;
}

void SoftBusHandlerAsset::closeAssetBind(int32_t socketId)
{
    LOGI("closeAssetBind Enter.");
    RemoveAssetObj(socketId);
    Shutdown(socketId);
}

void SoftBusHandlerAsset::OnAssetRecvBind(int32_t socketId, const std::string &srcNetWorkId)
{
    if (!IsSameAccount(srcNetWorkId)) {
        LOGE("The source and sink device is not same account, not support.");
        Shutdown(socketId);
        return;
    }
    std::lock_guard<std::mutex> lock(clientInfoMutex_);
    clientInfoMap_.insert(std::make_pair(socketId, srcNetWorkId));
}

std::string SoftBusHandlerAsset::GetClientInfo(int32_t socketId)
{
    std::lock_guard<std::mutex> lock(clientInfoMutex_);
    auto iter = clientInfoMap_.find(socketId);
    if (iter == clientInfoMap_.end()) {
        LOGE("ClientInfo not registered");
    }
    return iter->second;
}

void SoftBusHandlerAsset::RemoveClientInfo(int32_t socketId)
{
    std::lock_guard<std::mutex> lock(clientInfoMutex_);
    auto it = clientInfoMap_.find(socketId);
    if (it != clientInfoMap_.end()) {
        clientInfoMap_.erase(it->first);
    }
}

void SoftBusHandlerAsset::AddAssetObj(int32_t socketId, const sptr<AssetObj> &assetObj)
{
    std::lock_guard<std::mutex> lock(assetObjMapMutex_);
    auto iter = assetObjMap_.find(socketId);
    if (iter != assetObjMap_.end()) {
        LOGI("assetObj exist.");
        return;
    }
    assetObjMap_.insert(std::pair<int32_t, sptr<AssetObj>>(socketId, assetObj));
}

sptr<AssetObj> SoftBusHandlerAsset::GetAssetObj(int32_t socketId)
{
    std::lock_guard<std::mutex> lock(assetObjMapMutex_);
    auto iter = assetObjMap_.find(socketId);
    return iter->second;
}

void SoftBusHandlerAsset::RemoveAssetObj(int32_t socketId)
{
    std::lock_guard<std::mutex> lock(assetObjMapMutex_);
    auto iter = assetObjMap_.find(socketId);
    if (iter != assetObjMap_.end()) {
        assetObjMap_.erase(iter);
    }
}

int32_t SoftBusHandlerAsset::GenerateAssetObjInfo(int32_t socketId,
                                                  const std::string &fileName,
                                                  const sptr<AssetObj> &assetObj)
{
    size_t pos = fileName.find(RELATIVE_PATH_FLAG);
    if (pos == std::string::npos) {
        LOGE("Generate dstBundleName fail, firstFile is %{public}s", fileName.c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    std::string relativeFileName = fileName.substr(pos + RELATIVE_PATH_FLAG.length());

    pos = fileName.find(DST_BUNDLE_NAME_FLAG);
    if (pos == std::string::npos) {
        LOGE("Generate dstBundleName fail, relativeFirstFile is %{public}s", fileName.c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    auto dstBundleName = relativeFileName.substr(0, pos);
    assetObj->dstBundleName_ = dstBundleName;

    std::smatch match;
    std::regex sessionIdRegex("sessionId=([^&]+)");
    if (!std::regex_search(fileName, match, sessionIdRegex)) {
        LOGE("Generate sessionId fail, relativeFirstFile is %{public}s", fileName.c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    assetObj->sessionId_ = match[1].str();

    std::regex srcBundleNameRegex("srcBundleName=([^&]+)");
    if (!std::regex_search(fileName, match, srcBundleNameRegex)) {
        LOGE("Generate srcBundleName fail, relativeFirstFile is %{public}s", fileName.c_str());
        return FileManagement::ERR_BAD_VALUE;
    }
    assetObj->srcBundleName_ = match[1].str();

    assetObj->dstNetworkId_ = GetLocalNetworkId();
    if (assetObj->dstNetworkId_.empty()) {
        LOGE("Failed to get info of local devices");
        return FileManagement::ERR_BAD_VALUE;
    }
    return FileManagement::ERR_OK;
}

bool SoftBusHandlerAsset::IsSameAccount(const std::string &networkId)
{
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    DistributedHardware::DeviceManager::GetInstance().GetTrustedDeviceList(SERVICE_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        LOGE("trust device list size is invalid, size=%zu", deviceList.size());
        return false;
    }
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            return (deviceInfo.authForm == DistributedHardware::DmAuthForm::IDENTICAL_ACCOUNT);
        }
    }
    return false;
}

std::string SoftBusHandlerAsset::GetDstFile(const std::string &file,
                                            const std::string &srcBundleName,
                                            const std::string &dstBundleName,
                                            const std::string &sessionId,
                                            bool isSingleFile)
{
    std::stringstream dstFile;
    size_t pos = file.find(srcBundleName);
    if (pos == std::string::npos) {
        return "";
    }
    std::string tempDir;
    if (isSingleFile) {
        tempDir = ASSET_POSTFIX_SINGLE;
    } else {
        tempDir = ASSET_POSTFIX_ZIP;
    }
    dstFile << dstBundleName << "/" << TEMP_DIR << file.substr(pos + srcBundleName.length())
            << tempDir << "srcBundleName=" << srcBundleName << "&sessionId=" << sessionId;

    return dstFile.str();
}

std::vector<std::string> SoftBusHandlerAsset::GenerateUris(const std::vector<std::string> &fileList,
                                                           const std::string &dstBundleName,
                                                           bool isSingleFile)
{
    auto tempDir = dstBundleName + "/" + TEMP_DIR;
    std::vector<std::string> uris;
    if (isSingleFile) {
        std::stringstream uri;
        std::string file = fileList[0];
        size_t posPrefix = file.find(tempDir);
        if (posPrefix == std::string::npos) {
            LOGE("not find tempDir in fileList.");
            return {};
        }
        size_t posPostfix = file.find(ASSET_POSTFIX_SINGLE);
        if (posPostfix == std::string::npos) {
            LOGE("not find asset postfix in fileList.");
            return {};
        }
        uri <<"file://" << dstBundleName << "/data/storage/el2/distributedfiles"
            << file.substr(posPrefix + tempDir.length(), posPostfix);
        uris.emplace_back(uri.str());
        return uris;
    }
    for (auto file : fileList) {
        std::stringstream uri;
        size_t posPrefix = file.find(tempDir);
        if (posPrefix == std::string::npos) {
            LOGE("not find tempDir in fileList.");
            return {};
        }
        uri <<"file://" << dstBundleName << "/data/storage/el2/distributedfiles"
            << file.substr(posPrefix + tempDir.length());
        uris.emplace_back(uri.str());
    }
    return uris;
}

int32_t SoftBusHandlerAsset::ZipFile(const std::vector<std::string> &fileList,
                                     const std::string &relativePath,
                                     const std::string &zipFileName)
{
    if (!IsDir(zipFileName)) {
        MkDirRecurse(zipFileName, S_IRWXU | S_IRWXG | S_IXOTH);
    }

    zipFile outputFile = zipOpen64(zipFileName.c_str(), APPEND_STATUS_CREATE);
    if (!outputFile) {
        LOGE("Minizip failed to zipOpen, zipFileName = %{public}s", zipFileName.c_str());
        return E_ZIP;
    }

    for (const std::string& rootFile : fileList) {
        size_t pos = rootFile.find(relativePath);
        if (pos == std::string::npos) {
            LOGE("rootFile not have relativePath");
            return E_ZIP;
        }
        auto file = rootFile.substr(pos + relativePath.length());

        int err = zipOpenNewFileInZip3_64(outputFile, file.c_str(), NULL, NULL, 0, NULL, 0, NULL,
                                          Z_DEFLATED, 9, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, NULL, 0, 0);
        if (err != ZIP_OK) {
            LOGE("Minizip failed to zipOpenNewFileInZip, file = %{public}s", file.c_str());
            zipClose(outputFile, NULL);
            return E_ZIP;
        }

        FILE* f = fopen(rootFile.c_str(), "rb");
        char buf[1024];
        int len;
        while ((len = fread(buf, 1, sizeof(buf), f)) > 0) {
            zipWriteInFileInZip(outputFile, buf, len);
        }
        int ret = fclose(f);
        if (!ret) {
            LOGE("Minizip failed to fclose");
        }
        zipCloseFileInZip(outputFile);
    }
    zipClose(outputFile, NULL);
    return E_OK;
}

std::vector<std::string> SoftBusHandlerAsset::UnzipFile(std::string zipFileName, std::string relativePath)
{
    if (!IsDir(relativePath)) {
        MkDirRecurse(relativePath, S_IRWXU | S_IRWXG | S_IXOTH);
    }

    unzFile zipFile = unzOpen64(zipFileName.c_str());
    if (!zipFile) {
        LOGE("Minizip failed to unzOpen, zipFileName = %{public}s", zipFileName.c_str());
        return {};
    }

    unz_global_info64 globalInfo;
    if (unzGetGlobalInfo64(zipFile, &globalInfo) != UNZ_OK) {
        unzClose(zipFile);
        LOGE("Minizip failed to unzGetGlobalInfo");
        return {};
    }

    std::vector<std::string> fileList;

    for (int i = 0; i < globalInfo.number_entry; ++i) {
        std::string filePath = ExtractFile(zipFile, relativePath.c_str());
        if (filePath.length() > 0) {
            fileList.push_back(filePath);
        }
        unzCloseCurrentFile(zipFile);
        unzGoToNextFile(zipFile);
    }
    unzClose(zipFile);
    return fileList;
}

std::string SoftBusHandlerAsset::GetLocalNetworkId()
{
    NodeBasicInfo tmpNodeInfo;
    int errCode = GetLocalNodeDeviceInfo(SERVICE_NAME.c_str(), &tmpNodeInfo);
    if (errCode != 0) {
        LOGE("Failed to get info of local devices");
        return "";
    }
    return tmpNodeInfo.networkId;
}

int32_t SoftBusHandlerAsset::MkDir(const std::string &path, mode_t mode)
{
    return TEMP_FAILURE_RETRY(mkdir(path.c_str(), mode));
}

bool SoftBusHandlerAsset::MkDirRecurse(const std::string& path, mode_t mode)
{
    size_t pos = path.rfind("/");
    if (pos == std::string::npos) {
        LOGE("is not a dir, path : %{public}s", path.c_str());
    }
    auto dirPath = path.substr(0, pos);

    std::string::size_type index = 0;
    do {
        std::string subPath = dirPath;
        index = path.find('/', index + 1);
        if (index != std::string::npos) {
            subPath = path.substr(0, index);
        }

        if (TEMP_FAILURE_RETRY(access(subPath.c_str(), F_OK)) != 0) {
            if (MkDir(subPath, mode) != 0 && errno != EEXIST) {
                return false;
            }
        }
    } while (index != std::string::npos);

    return TEMP_FAILURE_RETRY(access(path.c_str(), F_OK)) == 0;
}

bool SoftBusHandlerAsset::IsDir(const std::string &path)
{
    // check whether the path exists
    struct stat st;
    int ret = TEMP_FAILURE_RETRY(lstat(path.c_str(), &st));
    if (ret) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

std::string SoftBusHandlerAsset::ExtractFile(unzFile zipFile, std::string dir)
{
    char *filenameWithPath = new char[BUFFER_SIZE];
    char *p;
    char *filenameWithoutPath;
    unz_file_info64 fileInfo;
    p = filenameWithoutPath = filenameWithPath;
    if (unzGetCurrentFileInfo64(zipFile, &fileInfo, filenameWithPath, BUFFER_SIZE, NULL, 0, NULL, 0) != UNZ_OK) {
        LOGE("Minizip failed to unzGetCurrentFileInfo64");
        return "";
    }
    char *temp = new char[BUFFER_SIZE];
    int retCpy = strcpy_s(temp, BUFFER_SIZE, dir.c_str());
    if (!retCpy) {
        return "";
    }
    int retCat = strcat_s(temp, BUFFER_SIZE, filenameWithPath);
    if (!retCat) {
        return "";
    }
    filenameWithPath = temp;
    while ((*p) != '\0') {
        if ((*p) == '/') {
            filenameWithoutPath = p + 1;
        }
        p++;
    }
    LOGI("Minizip ExtractFile, filenameWithPath = %{public}s ", filenameWithPath);
    if (*filenameWithoutPath != '\0') {
        if (!IsDir(filenameWithPath)) {
            MkDirRecurse(filenameWithPath, S_IRWXU | S_IRWXG | S_IXOTH);
        }
        if (unzOpenCurrentFile(zipFile) != UNZ_OK) {
            LOGE("Minizip failed to unzOpenCurrentFile");
        }
        std::fstream file;
        file.open(filenameWithPath, std::ios_base::out | std::ios_base::binary);
        ZPOS64_T fileLength = fileInfo.uncompressed_size;
        char *fileData = new char[fileLength];
        ZPOS64_T err = unzReadCurrentFile(zipFile, (voidp)fileData, fileLength);
        if (err < 0) {
            LOGE("Minizip failed to unzReadCurrentFile");
        }
        file.write(fileData, fileLength);
        file.close();
        free(fileData);
    }
    std::string filenameWithPathStr = filenameWithPath;
    return filenameWithPathStr;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS