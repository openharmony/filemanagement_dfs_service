/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cloud_disk_service_syncfolder.h"

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

#include "cloud_disk_service_logfile.h"
#include "cloud_disk_service_metafile.h"

namespace OHOS::FileManagement::CloudDiskService {

int32_t CloudDiskServiceSyncFolder::RegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex,
                                                       const std::string &path)
{
    return LogFileMgr::GetInstance().RegisterSyncFolder(userId, syncFolderIndex, path);
}

int32_t CloudDiskServiceSyncFolder::UnRegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex)
{
    return LogFileMgr::GetInstance().UnRegisterSyncFolder(userId, syncFolderIndex);
}

void CloudDiskServiceSyncFolder::RegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex)
{
    LogFileMgr::GetInstance().RegisterSyncFolderChanges(userId, syncFolderIndex);
}

void CloudDiskServiceSyncFolder::UnRegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex)
{
    LogFileMgr::GetInstance().UnRegisterSyncFolderChanges(userId, syncFolderIndex);
}

int32_t CloudDiskServiceSyncFolder::GetSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex,
                                                         const uint64_t start, const uint64_t count,
                                                         struct ChangesResult &changesResult)
{
    return LogFileMgr::GetInstance().PraseRequest(userId, syncFolderIndex, start, count, changesResult);
}

int32_t CloudDiskServiceSyncFolder::SetSyncFolderChanges(const struct EventInfo &eventInfo)
{
    return LogFileMgr::GetInstance().ProduceRequest(eventInfo);
}

void CloudDiskServiceSyncFolder::CloudDiskServiceClearAll()
{
    LogFileMgr::GetInstance().CloudDiskServiceClearAll();
    MetaFileMgr::GetInstance().CloudDiskServiceClearAll();
}

} // namespace OHOS::FileManagement::CloudDiskService