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

#ifndef MOCK_I_CLOUD_SYNC_SERVICE_H
#define MOCK_I_CLOUD_SYNC_SERVICE_H

#include "i_cloud_sync_service.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "iremote_broker.h"
#include "cloud_sync_constants.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncServiceMock : public IRemoteStub<ICloudSyncService> {
public:
    int code_;
    CloudSyncServiceMock() : code_(0) {}
    virtual ~CloudSyncServiceMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));

    int32_t InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        reply.WriteInt32(E_OK);
        return E_OK;
    }

    int32_t UnRegisterCallbackInner() override
    {
        return E_OK;
    }

    int32_t RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject) override
    {
        return E_OK;
    }

    int32_t StartSyncInner(bool forceFlag) override
    {
        return E_OK;
    }

    int32_t StopSyncInner() override
    {
        return E_OK;
    }

    int32_t ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status) override
    {
        return E_OK;
    }

    int32_t Clean(const std::string &accountId, const CleanOptions &cleanOptions) override
    {
        return E_OK;
    }

    int32_t NotifyDataChange(const std::string &accoutId, const std::string &bundleName) override
    {
        return E_OK;
    }
    int32_t NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData) override
    {
        return E_OK;
    }
    int32_t EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData) override
    {
        return E_OK;
    }
    int32_t DisableCloud(const std::string &accoutId) override
    {
        return E_OK;
    }
    int32_t StartDownloadFile(const std::string &path)
    {
        return E_OK;
    }
    int32_t StartFileCache(const std::string &path)
    {
        return E_OK;
    }
    int32_t StopDownloadFile(const std::string &path)
    {
        return E_OK;
    }
    int32_t RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback)
    {
        return E_OK;
    }
    int32_t UnregisterDownloadFileCallback()
    {
        return E_OK;
    }
    int32_t UploadAsset(const int32_t userId, const std::string &request, std::string &result)
    {
        return E_OK;
    }
    int32_t DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj)
    {
        return E_OK;
    }
    int32_t DeleteAsset(const int32_t userId, const std::string &uri)
    {
        return E_OK;
    }
    int32_t GetSyncTimeInner(int64_t &syncTime)
    {
        return E_OK;
    }
    int32_t CleanCacheInner(const std::string &uri)
    {
        return E_OK;
    }
};

} // namespace OHOS::FileManagement::CloudSync

#endif // MOCK_I_CLOUD_SYNC_SERVICE_H
