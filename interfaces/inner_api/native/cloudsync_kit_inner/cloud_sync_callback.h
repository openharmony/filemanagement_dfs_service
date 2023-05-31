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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_CALLBACK_H
#define OHOS_FILEMGMT_CLOUD_SYNC_CALLBACK_H

#include <functional>

#include "cloud_sync_constants.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncCallback {
public:
    virtual ~CloudSyncCallback() = default;
    /**
     * @brief 同步状态变更通知回调
     *
     * @param type 0:upload 1:download 2:all
     * @param state 同步状态
     * SYNC_STATE_DEFAULT                   同步完成
     * SYNC_STATE_SYNCING                   同步中
     * SYNC_STATE_FAILED                    同步失败
     * SYNC_STATE_PAUSED_NO_NETWORK         未连接网络
     * SYNC_STATE_PAUSED_FOR_WIFI           未连接wifi
     * SYNC_STATE_PAUSED_FOR_BATTERY        低电量
     * SYNC_STATE_BATTERY_TOO_LOW           电量过低
     * SYNC_STATE_PAUSED_FOR_SPACE_TOO_LOW  云空间满
     */
    virtual void OnSyncStateChanged(SyncType type, SyncPromptState state) {};
    /**
     * @brief 同步状态变更通知回调
     *
     * @param state 同步状态类型
     * UPLOADING                            正在上传
     * UPLOAD_FAILED                        同步上传失败
     * DOWNLOADING                          正在下载
     * DOWNLOAD_FAILED                      同步下载失败
     * COMPLETED                            同步完成
     * STOPPED                              同步停止
     * @param error 同步错误类型
     * NO_ERROR                             无错误
     * NETWORK_UNAVAILABLE                  网络不可用
     * WIFI_UNAVAILABLE                     wifi不可用
     * BATTERY_LEVEL_LOW                    低电量
     * BATTERY_LEVEL_WARNING                电量警告
     * CLOUD_STORAGE_FULL                   云空间满
     * LOCAL_STORAGE_FULL                   本地空间满
     */
    virtual void OnSyncStateChanged(CloudSyncState state, ErrorType error) {};
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_CALLBACK_H