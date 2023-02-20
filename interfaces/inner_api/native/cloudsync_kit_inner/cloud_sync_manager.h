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

#include "cloud_sync_callback.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncManager {
public:
    static CloudSyncManager &GetInstance();
    /**
     * @brief 启动同步
     *
     * @param forceFlag 是否强制继续同步，当前仅支持低电量下暂停后继续上传
     * @param callback 注册同步回调
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t StartSync(bool forceFlag, const std::shared_ptr<CloudSyncCallback> callback) = 0;
    /**
     * @brief 停止同步
     *
     * @return int32_t 同步返回执行结果
     */
    virtual int32_t StopSync() = 0;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_H