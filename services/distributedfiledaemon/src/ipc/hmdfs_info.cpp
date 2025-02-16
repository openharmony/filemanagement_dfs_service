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

#include "hmdfs_info.h"

#include "string_ex.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

bool HmdfsInfoExt::Marshalling(Parcel &data) const
{
    if (!data.WriteString16(Str8ToStr16(copyPath))) {
        return false;
    }

    if (!data.WriteInt32(dirExistFlag ? 1 : 0)) {
        return false;
    }

    if (!data.WriteString16(Str8ToStr16(authority))) {
        return false;
    }

    if (!data.WriteString16(Str8ToStr16(sandboxPath))) {
        return false;
    }

    if (!data.WriteString16(Str8ToStr16(sessionName))) {
        return false;
    }

    if (!data.WriteString16(Str8ToStr16(dstPhysicalPath))) {
        return false;
    }
    return true;
}

HmdfsInfoExt *HmdfsInfoExt::Unmarshalling(Parcel &parcel)
{
    HmdfsInfoExt *data = new (std::nothrow) HmdfsInfoExt();

    std::string copyPathCp = Str16ToStr8(parcel.ReadString16());
    data->copyPath = copyPathCp;

    data->dirExistFlag = parcel.ReadInt32() == 1 ? true : false;

    std::string authorityCp = Str16ToStr8(parcel.ReadString16());
    data->authority = authorityCp;

    std::string sandboxPathCp = Str16ToStr8(parcel.ReadString16());
    data->sandboxPath = sandboxPathCp;

    std::string sessionNameCp = Str16ToStr8(parcel.ReadString16());
    data->sessionName = sessionNameCp;

    std::string dstPhysicalPathCp = Str16ToStr8(parcel.ReadString16());
    data->dstPhysicalPath = dstPhysicalPathCp;

    return data;
}

HmdfsInfo convertExttoInfo(HmdfsInfoExt &infoExt)
{
    HmdfsInfo info;
    info.copyPath = infoExt.copyPath;
    info.dirExistFlag = infoExt.dirExistFlag;
    info.authority = infoExt.authority;
    info.sandboxPath = infoExt.sandboxPath;
    info.sessionName = infoExt.sessionName;
    info.dstPhysicalPath = infoExt.dstPhysicalPath;
    return info;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
