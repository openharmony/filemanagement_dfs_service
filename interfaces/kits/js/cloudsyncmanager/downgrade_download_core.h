/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_CORE_H
#define OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_CORE_H

#include "cloud_sync_common.h"
#include "filemgmt_libfs.h"
#include "downgrade_callback_ani.h"

namespace OHOS::FileManagement::CloudSync {
using namespace ModuleFileIO;
using namespace std;

class DowngradeDownloadCore {
public:
    explicit DowngradeDownloadCore(const string &bundleName);
    ~DowngradeDownloadCore() = default;

    static FsResult<DowngradeDownloadCore *> Constructor(const string &bundleName);
    FsResult<CloudFileInfo> DoDowngradeDlGetCloudFileInfo();
    FsResult<void> DoDowngradeDlStartDownload(const shared_ptr<DowngradeCallbackAniImpl> callback);
    FsResult<void> DoDowngradeDlStopDownload();
    const string &GetBundleName() const;
private:
    shared_ptr<DowngradeCallbackAniImpl> callback_;
    unique_ptr<DowngradeEntity> bundleEntity;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_CORE_H