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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_H

#include "transport/socket.h"
#include "transport/trans_type.h"
#include <map>
#include <mutex>
#include <cstdint>
#include <string>
#include "asset/asset_obj.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include "third_party/zlib/contrib/minizip/zip.h"
#include "third_party/zlib/contrib/minizip/unzip.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
typedef enum {
    DFS_ASSET_ROLE_SEND = 0,
    DFS_ASSET_ROLE_RECV = 1,
} DFS_ASSET_ROLE;

class SoftBusHandlerAsset {
public:
    SoftBusHandlerAsset();
    ~SoftBusHandlerAsset();
    static SoftBusHandlerAsset &GetInstance();
    void CreateAssetLocalSessionServer();
    void DeleteAssetLocalSessionServer();

    int32_t AssetBind(const std::string &dstNetworkId, int32_t &socketId);
    int32_t AssetSendFile(int32_t socketId, const std::string& sendFile, bool isSingleFile);
    void closeAssetBind(int32_t socketId);
    void OnAssetRecvBind(int32_t socketId, const std::string &srcNetWorkId);

    std::string GetClientInfo(int32_t socketId);
    void RemoveClientInfo(int32_t socketId);
    void AddAssetObj(int32_t socketId, const sptr<AssetObj> &assetObj);
    sptr<AssetObj> GetAssetObj(int32_t socketId);
    void RemoveAssetObj(int32_t socketId);

    int32_t GenerateAssetObjInfo(int32_t socketId,
                             const std::string &fileName,
                             const sptr<AssetObj> &assetObj);
    std::vector<std::string> GenerateUris(const std::vector<std::string> &fileList,
                                          const std::string &dstBundleName,
                                          bool isSingleFile);
    int32_t ZipFile(const std::vector<std::string> &fileList,
                    const std::string &relativePath,
                    const std::string &zipFileName);
    std::vector<std::string> UnzipFile(std::string zipFileName, std::string relativePath);
    bool MkDirRecurse(const std::string& path, mode_t mode);
private:
    static bool IsSameAccount(const std::string &networkId);
    std::string GetDstFile(const std::string &file,
                           const std::string &srcBundleName,
                           const std::string &dstBundleName,
                           const std::string &sessionId,
                           bool isSingleFile);
    std::string GetLocalNetworkId();
    int32_t MkDir(const std::string &path, mode_t mode);

    bool IsDir(const std::string &path);
    std::string ExtractFile(unzFile zipFile, std::string dir);
    std::mutex clientInfoMutex_;
    std::map<int32_t, std::string> clientInfoMap_;
    std::mutex serverIdMapMutex_;
    std::map<std::string, int32_t> serverIdMap_;
    std::mutex assetObjMapMutex_;
    std::map<int32_t, sptr<AssetObj>> assetObjMap_;
    std::map<DFS_ASSET_ROLE, ISocketListener> sessionListener_;
    static inline const std::string SERVICE_NAME {"ohos.storage.distributedfile.daemon"};
    static inline const std::string ASSET_LOCAL_SESSION_NAME {"DistributedFileService_assetListener"};
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_H
