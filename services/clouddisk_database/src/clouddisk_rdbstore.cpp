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

#include "clouddisk_rdbstore.h"

#include <ctime>
#include <algorithm>
#include <cerrno>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "clouddisk_rdb_utils.h"
#include "rdb_sql_utils.h"
#include "utils_log.h"
#include "dfs_error.h"
#include "dk_database.h"
#include "drive_kit.h"
#include "mimetype_utils.h"
#include "media_file_utils.h"
#include "cloud_pref_impl.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace OHOS::NativeRdb;
using namespace DriveKit;
using namespace CloudSync;
using namespace OHOS::Media;

CloudDiskRdbStore::CloudDiskRdbStore(const std::string &bundleName, const int32_t &userId)
    : bundleName_(bundleName), userId_(userId)
{
    RdbInit();
    MimeTypeUtils::InitMimeTypeMap();
    InitRootId();
}

CloudDiskRdbStore::~CloudDiskRdbStore()
{
    Stop();
}

int32_t CloudDiskRdbStore::RdbInit()
{
    return E_OK;
}

void CloudDiskRdbStore::InitRootId()
{
    return;
}

void CloudDiskRdbStore::Stop()
{
    return;
}
shared_ptr<RdbStore> CloudDiskRdbStore::GetRaw()
{
    return rdbStore_;
}

int32_t CloudDiskRdbStore::FillFileType(const std::string &fileName, ValuesBucket &fileInfo)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::UpdateParentFolder(const std::string &parentCloudId)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::Create(const string &cloudId, const std::string &parentCloudId,
    const std::string &fileName)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::GetXAttr(const std::string &cloudId, const std::string &position, std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::SetXAttr(const std::string &cloudId, const std::string &position, const std::string &value)
{
    return E_OK;
}

int32_t CloudDiskDataCallBack::OnCreate(RdbStore &store)
{
    return NativeRdb::E_OK;
}

int32_t CloudDiskDataCallBack::OnUpgrade(RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    return NativeRdb::E_OK;
}
}