/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef CORE_SERVICE_CLIENT_MOCK_H
#define CORE_SERVICE_CLIENT_MOCK_H

#include <gmock/gmock.h>
#include "core_service_client_mock.h"
#include "signal_information.h"
#include "cellular_data_client.h"
#include "core_service_client.h"

namespace OHOS::FileManagement::CloudSync {

class DfsCoreServiceClient {
public:
    static Telephony::CoreServiceClient &GetInstance()
    {
        static Telephony::CoreServiceClient client;
        return client;
    }
    virtual int32_t GetSignalInfoList(int32_t slotId, std::vector<sptr<Telephony::SignalInformation>> &signals) = 0;
public:
    DfsCoreServiceClient() = default;
    virtual ~DfsCoreServiceClient() = default;
    static inline std::shared_ptr<DfsCoreServiceClient> dfsCoreServiceClient = nullptr;
};

class CoreServiceClientMock : public DfsCoreServiceClient {
public:
    MOCK_METHOD2(GetSignalInfoList, int32_t(int32_t slotId, std::vector<sptr<Telephony::SignalInformation>> &signals));
};
}
#endif