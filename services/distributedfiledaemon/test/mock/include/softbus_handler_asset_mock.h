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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_MOCK_H

#include <gmock/gmock.h>
#include "network/softbus/softbus_handler_asset.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class ISoftBusHandlerAssetMock {
public:
    virtual ~ISoftBusHandlerAssetMock() = default;

    virtual int32_t AssetBind(const std::string &dstNetworkId, int32_t &socketId) = 0;
    virtual int32_t AssetSendFile(int32_t socketId, const std::string& sendFile, bool isSingleFile) = 0;
    virtual std::string GetClientInfo(int32_t socketId) = 0;
    virtual sptr<AssetObj> GetAssetObj(int32_t socketId) = 0;
    virtual int32_t GenerateAssetObjInfo(int32_t socketId,
                                         const std::string &fileName,
                                         const sptr<AssetObj> &assetObj) = 0;
    virtual std::vector<std::string> GenerateUris(const std::vector<std::string> &fileList,
                                                  const std::string &dstBundleName,
                                                  bool isSingleFile) = 0;
    virtual int32_t CompressFile(const std::vector<std::string> &fileList,
                                 const std::string &relativePath,
                                 const std::string &zipFileName) = 0;
    virtual std::vector<std::string> DecompressFile(const std::string &unZipFileName,
                                                    const std::string &relativePath) = 0;
    virtual bool MkDirRecurse(const std::string& path, mode_t mode) = 0;

public:
    static inline std::shared_ptr<ISoftBusHandlerAssetMock> iSoftBusHandlerAssetMock_ = nullptr;
};

class SoftBusHandlerAssetMock : public ISoftBusHandlerAssetMock {
public:
    MOCK_METHOD2(AssetBind, int32_t(const std::string &dstNetworkId, int32_t &socketId));
    MOCK_METHOD3(AssetSendFile, int32_t(int32_t socketId, const std::string& sendFile, bool isSingleFile));
    MOCK_METHOD1(GetClientInfo, std::string(int32_t socketId));
    MOCK_METHOD1(GetAssetObj, sptr<AssetObj>(int32_t socketId));
    MOCK_METHOD3(GenerateAssetObjInfo, int32_t(int32_t socketId,
                                               const std::string &fileName,
                                               const sptr<AssetObj> &assetObj));
    MOCK_METHOD3(GenerateUris, std::vector<std::string>(const std::vector<std::string> &fileList,
                                                        const std::string &dstBundleName,
                                                        bool isSingleFile));
    MOCK_METHOD3(CompressFile, int32_t(const std::vector<std::string> &fileList,
                                       const std::string &relativePath,
                                       const std::string &zipFileName));
    MOCK_METHOD2(DecompressFile, std::vector<std::string>(const std::string &unZipFileName,
                                                          const std::string &relativePath));
    MOCK_METHOD2(MkDirRecurse, bool(const std::string& path, mode_t mode));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_MOCK_H
