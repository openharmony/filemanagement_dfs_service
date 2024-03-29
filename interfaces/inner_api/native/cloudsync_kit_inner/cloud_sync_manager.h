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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_H

#include <memory>

#include "cloud_download_callback.h"
#include "cloud_sync_callback.h"
#include "cloud_sync_common.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncManager {
public:
    static CloudSyncManager &GetInstance();
    /**
     * @brief 注册
     *
     * @param callback 注册同步回调
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t RegisterCallback(const std::shared_ptr<CloudSyncCallback> callback,
                                     const std::string &bundleName = "") = 0;
    /**
     * @brief 注册
     *
     * @param callback 卸载同步回调
     * @return int32_t 返回执行结果
     */
    virtual int32_t UnRegisterCallback(const std::string &bundleName = "") = 0;
    /**
     * @brief 启动同步
     *
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t StartSync(const std::string &bundleName = "") = 0;
    /**
     * @brief 启动同步
     *
     * @param forceFlag 是否强制继续同步，当前仅支持低电量下暂停后继续上传
     * @param callback 注册同步回调
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t StartSync(bool forceFlag, const std::shared_ptr<CloudSyncCallback> callback) = 0;
    /**
     * @brief 触发同步
     *
     * @param bundleName 应用包名
     * @param userId 用户ID
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t TriggerSync(const std::string &bundleName, const int32_t &userId) = 0;
    /**
     * @brief 停止同步
     *
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t StopSync(const std::string &bundleName = "") = 0;
    /**
     * @brief 切换应用云同步开关
     *
     * @param accoutId 账号ID
     * @param bundleName 应用包名
     * @param status 应用开关状态
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status) = 0;
    /**
     * @brief 云数据变化触发同步
     *
     * @param accoutId 账号ID
     * @param bundleName 应用包名
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t NotifyDataChange(const std::string &accoutId, const std::string &bundleName) = 0;
    virtual int32_t NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData) = 0;
    virtual int32_t EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData) = 0;
    virtual int32_t DisableCloud(const std::string &accoutId) = 0;
    /**
     * @brief 云账号退出选择保留或清除云数据
     *
     * @param accoutId 账号ID
     * @param cleanOptions 说明云文件如何处理的方式
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t Clean(const std::string &accountId, const CleanOptions &cleanOptions) = 0;
    virtual int32_t StartDownloadFile(const std::string &path) = 0;
    virtual int32_t StartFileCache(const std::string &path) = 0;
    virtual int32_t StopDownloadFile(const std::string &path) = 0;
    virtual int32_t RegisterDownloadFileCallback(const std::shared_ptr<CloudDownloadCallback> downloadCallback) = 0;
    virtual int32_t UnregisterDownloadFileCallback() = 0;
    virtual int32_t GetSyncTime(int64_t &syncTime, const std::string &bundleName = "") = 0;
    virtual int32_t CleanCache(const std::string &uri) = 0;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_H
