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

#ifndef OHOS_CLOUD_DISK_SERVICE_NOTIFY_H
#define OHOS_CLOUD_DISK_SERVICE_NOTIFY_H

#include <list>
#include <string>

#include "clouddisk_notify_const.h"
#include "dataobs_mgr_client.h"
#include "timer.h"


namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using ChangeInfo = OHOS::AAFwk::ChangeInfo;

struct CacheNotifyInfo {
    NotifyType notifyType;
    mutable list<Uri> uriList;
    mutable list<bool> isDirList;
};

class CloudDiskNotify final {
public:
    static CloudDiskNotify &GetInstance();
    void TryNotify(const NotifyParamDisk &paramDisk, const ParamDiskOthers &paramOthers = {});
    void TryNotifyService(const NotifyParamService &paramService, const ParamServiceOther &paramOthers = {});
    int32_t GetDeleteNotifyData(const vector<NativeRdb::ValueObject> &deleteIds,
                                vector<NotifyData> &notifyDataList,
                                const ParamServiceOther &paramOthers);
    void AddNotify(const NotifyData notifyData);
    void NotifyChangeOuter();

private:
    CloudDiskNotify();
    ~CloudDiskNotify();
    list<CacheNotifyInfo> nfList_;
    mutex mutex_;
    uint32_t timerId_;
    unique_ptr<Utils::Timer> timer_;
    Utils::Timer::TimerCallback timerCallback_;
    uint32_t notifyCount_;
};
} // namespace OHOS::FileManagement::CloudDisk

#endif // OHOS_CLOUD_DISK_SERVICE_NOTIFY_H