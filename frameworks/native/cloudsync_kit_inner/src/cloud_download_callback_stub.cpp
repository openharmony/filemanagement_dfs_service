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

#include "cloud_download_callback_stub.h"
#include "cloud_download_uri_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudDownloadCallbackStub::CloudDownloadCallbackStub()
{
    opToInterfaceMap_[SERVICE_CMD_ON_PROCESS] = [this](MessageParcel &data, MessageParcel &reply) {
            return this->HandleOnProcess(data, reply);
        };
}

int32_t CloudDownloadCallbackStub::OnRemoteRequest(uint32_t code,
                                                   MessageParcel &data,
                                                   MessageParcel &reply,
                                                   MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return E_SERVICE_DESCRIPTOR_IS_EMPTY;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    auto memberFunc = interfaceIndex->second;
    return memberFunc(data, reply);
}

int32_t CloudDownloadCallbackStub::HandleOnProcess(MessageParcel &data, MessageParcel &reply)
{
    sptr<DownloadProgressObj> progress = data.ReadParcelable<DownloadProgressObj>();
    if (!progress) {
        LOGE("object of DownloadProgressObj is nullptr");
        return E_INVAL_ARG;
    }

    CloudDownloadUriManager& uriMgr = CloudDownloadUriManager::GetInstance();
    std::string path = progress->path;
    std::string uri = uriMgr.GetUri(path);
    if (uri.empty()) {
        LOGI("CloudDownloadCallbackStub path %{public}s trans to uri error, skip.", GetAnonyString(path).c_str());
        return E_OK;
    } else {
        progress->path = uri;
        // if download finished, call need call RemoveUri and CheckDownloadIdPathMap later
    }

    OnDownloadProcess(*progress);
    return E_OK;
}

} // namespace OHOS::FileManagement::CloudSync
