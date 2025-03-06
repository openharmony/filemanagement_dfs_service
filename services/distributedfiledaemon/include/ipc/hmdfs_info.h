/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef FILEMANAGEMENT_DFS_SERVICE_HMDFS_INFO_H
#define FILEMANAGEMENT_DFS_SERVICE_HMDFS_INFO_H

#include <string>
#include "string_ex.h"
#include "parcel.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
struct HmdfsInfo {
    std::string copyPath;
    bool dirExistFlag = false;
    std::string authority;
    std::string sandboxPath;
    std::string sessionName;
    std::string dstPhysicalPath;
};

struct HmdfsInfoExt : public HmdfsInfo, public Parcelable {
        using HmdfsInfo::HmdfsInfo;
        HmdfsInfoExt(const HmdfsInfo& hmdfsInfo) : HmdfsInfo(hmdfsInfo) {}

        virtual bool Marshalling(Parcel &parcel) const override;

        static HmdfsInfoExt *Unmarshalling(Parcel &parcel);
};

HmdfsInfo convertExttoInfo(HmdfsInfoExt &infoExt);
} //DistributedFile
} //Storage
} //OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_HMDFS_INFO_H
