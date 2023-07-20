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

#ifndef OHOS_FILEMGMT_DATA_SYNC_NOTIFIER_H
#define OHOS_FILEMGMT_DATA_SYNC_NOTIFIER_H

#include <list>
#include <memory>

#include "dataobs_mgr_client.h"
#include "uri.h"

namespace OHOS::FileManagement::CloudSync {
class DataSyncNotifier : public NoCopyable {
using ChangeType = AAFwk::ChangeInfo::ChangeType;
public:
    static std::unordered_map<ChangeType, std::list<Uri>> notifyListMap_;
    static std::mutex mtx_;
    static DataSyncNotifier &GetInstance();
    ~DataSyncNotifier() = default;
    int32_t TryNotify(const std::string &uri, const ChangeType changeType, const std::string &fileAssetId);
    int32_t FinalNotify();
private:
    DataSyncNotifier() = default;
};
}
#endif // OHOS_FILEMGMT_DATA_SYNC_MANAGER_H