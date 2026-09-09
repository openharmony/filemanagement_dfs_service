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
#ifndef OHOS_DFS_CELLULAR_DATA_CLIENT_MOCK_H
#define OHOS_DFS_CELLULAR_DATA_CLIENT_MOCK_H

#include <gmock/gmock.h>
#include "cellular_data_client.h"

namespace OHOS::FileManagement::CloudSync {

class DfsCellularDataClient {
public:
    static Telephony::CellularDataClient &GetInstance()
    {
        static Telephony::CellularDataClient client;
        return client;
    }
    virtual int32_t GetDefaultCellularDataSlotId() = 0;
public:
    DfsCellularDataClient() = default;
    virtual ~DfsCellularDataClient() = default;
    static inline std::shared_ptr<DfsCellularDataClient> dfsCellularDataClient = nullptr;
};

class CellularDataClientMock : public DfsCellularDataClient {
public:
    MOCK_METHOD0(GetDefaultCellularDataSlotId, int32_t());
};
}
#endif