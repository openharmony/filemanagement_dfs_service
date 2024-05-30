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

#include "asset/asset_obj.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile{
bool AssetObj::ReadFromParcel(Parcel &parcel)
{
    parcel.ReadString(sessionId_);
    parcel.ReadString(dstNetworkId_);
    parcel.ReadString(srcBundleName_);
    parcel.ReadString(dstBundleName_);
    if (!parcel.ReadStringVector(&uris_)) {
        LOGE("failed to read uris_.");
        return false;
    }
    return true;
}

bool AssetObj::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(sessionId_);
    parcel.WriteString(dstNetworkId_);
    parcel.WriteString(srcBundleName_);
    parcel.WriteString(dstBundleName_);
    if (!parcel.WriteStringVector(uris_)) {
        LOGE("failed to write uris_.");
        return false;
    }
    return true;
}
AssetObj *AssetObj::Unmarshalling(Parcel &parcel)
{
    AssetObj *info = new (std::nothrow) AssetObj();
    if ((info != nullptr) && (!info->ReadFromParcel(parcel))) {
        LOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS