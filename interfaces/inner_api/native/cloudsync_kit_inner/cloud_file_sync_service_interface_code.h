/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_SYNC_SERVICE_INTERFACE_CODE_H
#define OHOS_FILEMGMT_CLOUD_FILE_SYNC_SERVICE_INTERFACE_CODE_H

/* SAID:5204 */
namespace OHOS::FileManagement::CloudSync {
enum class CloudFileSyncServiceInterfaceCode {
    SERVICE_CMD_UNREGISTER_CALLBACK,
    SERVICE_CMD_REGISTER_CALLBACK,
    SERVICE_CMD_START_SYNC,
    SERVICE_CMD_TRIGGER_SYNC,
    SERVICE_CMD_STOP_SYNC,
    SERVICE_CMD_CHANGE_APP_SWITCH,
    SERVICE_CMD_NOTIFY_DATA_CHANGE,
    SERVICE_CMD_NOTIFY_EVENT_CHANGE,
    SERVICE_CMD_ENABLE_CLOUD,
    SERVICE_CMD_DISABLE_CLOUD,
    SERVICE_CMD_CLEAN,
    SERVICE_CMD_START_DOWNLOAD_FILE,
    SERVICE_CMD_STOP_DOWNLOAD_FILE,
    SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK,
    SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK,
    SERVICE_CMD_UPLOAD_ASSET,
    SERVICE_CMD_DOWNLOAD_FILE,
    SERVICE_CMD_DOWNLOAD_FILES,
    SERVICE_CMD_DELETE_ASSET,
    SERVICE_CMD_GET_SYNC_TIME,
    SERVICE_CMD_CLEAN_CACHE,
    SERVICE_CMD_START_FILE_CACHE,
    SERVICE_CMD_DOWNLOAD_ASSET,
    SERVICE_CMD_REGISTER_DOWNLOAD_ASSET_CALLBACK
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_FILE_SYNC_SERVICE_INTERFACE_CODE_H