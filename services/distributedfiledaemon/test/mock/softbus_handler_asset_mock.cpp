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

#include "softbus_handler_asset_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
SoftBusHandlerAsset::SoftBusHandlerAsset() = default;
SoftBusHandlerAsset::~SoftBusHandlerAsset() = default;

SoftBusHandlerAsset &SoftBusHandlerAsset::GetInstance()
{
    static SoftBusHandlerAsset assetHandle;
    return assetHandle;
}

void SoftBusHandlerAsset::CreateAssetLocalSessionServer()
{
    return;
}

void SoftBusHandlerAsset::DeleteAssetLocalSessionServer()
{
    return;
}

int32_t SoftBusHandlerAsset::AssetBind(const std::string &dstNetworkId, int32_t &socketId)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return -1;
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->AssetBind(dstNetworkId, socketId);
}

int32_t SoftBusHandlerAsset::AssetSendFile(int32_t socketId, const std::string& sendFile, bool isSingleFile)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return -1;
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->AssetSendFile(socketId, sendFile, isSingleFile);
}

void SoftBusHandlerAsset::closeAssetBind(int32_t socketId)
{
    return;
}

void SoftBusHandlerAsset::OnAssetRecvBind(int32_t socketId, const std::string &srcNetWorkId)
{
    return;
}

std::string SoftBusHandlerAsset::GetClientInfo(int32_t socketId)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return "";
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->GetClientInfo(socketId);
}

void SoftBusHandlerAsset::RemoveClientInfo(int32_t socketId)
{
    return;
}

void SoftBusHandlerAsset::AddAssetObj(int32_t socketId, const sptr<AssetObj> &assetObj)
{
    return;
}

sptr<AssetObj> SoftBusHandlerAsset::GetAssetObj(int32_t socketId)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return nullptr;
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->GetAssetObj(socketId);
}

void SoftBusHandlerAsset::RemoveAssetObj(int32_t socketId)
{
    return;
}

int32_t SoftBusHandlerAsset::GenerateAssetObjInfo(int32_t socketId,
                                                  const std::string &fileName,
                                                  const sptr<AssetObj> &assetObj)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return -1;
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->GenerateAssetObjInfo(socketId, fileName, assetObj);
}

bool SoftBusHandlerAsset::IsSameAccount(const std::string &networkId)
{
    return true;
}

std::string SoftBusHandlerAsset::GetDstFile(const std::string &file,
                                            const std::string &srcBundleName,
                                            const std::string &dstBundleName,
                                            const std::string &sessionId,
                                            bool isSingleFile)
{
    return "test";
}

std::vector<std::string> SoftBusHandlerAsset::GenerateUris(const std::vector<std::string> &fileList,
                                                           const std::string &dstBundleName,
                                                           bool isSingleFile)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return {};
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->GenerateUris(fileList, dstBundleName, isSingleFile);
}

int32_t SoftBusHandlerAsset::CompressFile(const std::vector<std::string> &fileList,
                                          const std::string &relativePath,
                                          const std::string &zipFileName)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return -1;
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->CompressFile(fileList, relativePath, zipFileName);
}

std::vector<std::string> SoftBusHandlerAsset::DecompressFile(const std::string &unZipFileName,
                                                             const std::string &relativePath)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return {};
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->DecompressFile(unZipFileName, relativePath);
}

std::string SoftBusHandlerAsset::GetLocalNetworkId()
{
    return "test";
}

int32_t SoftBusHandlerAsset::MkDir(const std::string &path, mode_t mode)
{
    return 0;
}

bool SoftBusHandlerAsset::MkDirRecurse(const std::string& path, mode_t mode)
{
    if (ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_ == nullptr) {
        return -1;
    }
    return ISoftBusHandlerAssetMock::iSoftBusHandlerAssetMock_->MkDirRecurse(path, mode);
}

bool SoftBusHandlerAsset::IsDir(const std::string &path)
{
    return true;
}

std::string SoftBusHandlerAsset::ExtractFile(unzFile unZipFile, const std::string &dir)
{
    return "test";
}

void SoftBusHandlerAsset::RemoveFile(const std::string &path, bool isRemove)
{
    return;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS