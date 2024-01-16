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

#ifndef CLOUD_DISK_DATA_SYNCER_H
#define CLOUD_DISK_DATA_SYNCER_H

#include "cloud_disk_data_handler.h"
#include "functional"
#include "file_column.h"
#include "data_syncer.h"
#include "data_sync_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class CloudDiskDataSyncer : public DataSyncer, std::enable_shared_from_this<CloudDiskDataSyncer> {
public:
    CloudDiskDataSyncer(const std::string bundleName, const int32_t userId);
    virtual ~CloudDiskDataSyncer() = default;
    virtual void Schedule() override;
    virtual void ScheduleByType(SyncTriggerType syncTriggerType) override;
    virtual void Reset() override;

    virtual int32_t StartDownloadFile(const std::string path, const int32_t userId) override;
    virtual int32_t StopDownloadFile(const std::string path, const int32_t userId) override;
    virtual int32_t CleanCache(const std::string &uri) override;
    virtual int32_t Init(const std::string bundleName, const int32_t userId) override;
    virtual int32_t Clean(const int action) override;

private:
    enum {
        BEGIN,
        DOWNLOADFILE,
        COMPLETEPULL,
        UPLOADFILE,
        COMPLETEPUSH,
        END
    };

    int32_t DownloadFile();
    int32_t UploadFile();
    int32_t Complete(bool isNeedNotify = true) override;
    void ChangesNotify();

    int32_t stage_ = BEGIN;
    std::shared_ptr<CloudDiskDataHandler> cloudDiskHandler_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_DISK_DATA_SYNCER_H
