/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_ASSET_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_SYNC_ASSET_MANAGER_H

#include <functional>
#include <string>

namespace OHOS::FileManagement::CloudSync {
struct AssetInfo {
    std::string uri;
    std::string recordType;
    std::string recordId;
    std::string fieldKey;
    std::string assetName;
};

class CloudSyncAssetManager {
public:
    using ResultCallback = std::function<void(const std::string &, int32_t)>;
    static CloudSyncAssetManager &GetInstance();
    /**
     * @brief 接口触发文件上传
     *
     * @param userId 用户Id
     * @param request 请求
     * @param result 结果
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t UploadAsset(const int32_t userId, const std::string &request, std::string &result) = 0;
    /**
     * @brief 接口触发文件下载
     *
     * @param userId 用户Id
     * @param bundleName 应用包名
     * @param assetInfo 下载文件信息
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfo &assetInfo) = 0;
    /**
     * @brief 接口触发文件下载
     *
     * @param userId 用户Id
     * @param bundleName 应用包名
     * @param assetInfo 下载文件信息
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t DownloadFile(const int32_t userId,
                                 const std::string &bundleName,
                                 const std::string &networkId,
                                 AssetInfo &assetInfo,
                                 ResultCallback resultCallback) = 0;
    /**
     * @brief 接口触发批量文件下载
     *
     * @param userId 用户Id
     * @param bundleName 应用包名
     * @param assetInfo 下载批量文件信息
     * @param assetResultMap 同步返回执行结果
     * @return int32_t 同步返回执行总结果
     */
    virtual int32_t DownloadFiles(const int32_t userId,
                                  const std::string &bundleName,
                                  const std::vector<AssetInfo> &assetInfo,
                                  std::vector<bool> &assetResultMap) = 0;
    /**
     * @brief 接口触发附件删除
     *
     * @param userId 用户Id
     * @param uri 用户uri
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t DeleteAsset(const int32_t userId, const std::string &uri) = 0;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_ASSET_MANAGER_H
