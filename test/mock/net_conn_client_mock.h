/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DFS_NET_CONN_CLIENT_MOCK_H
#define OHOS_DFS_NET_CONN_CLIENT_MOCK_H
#include <gmock/gmock.h>
#include "net_conn_client.h"
#include "net_handle.h"

namespace OHOS::FileManagement::CloudSync {

class DfsNetConnClient {
public:
    static NetManagerStandard::NetConnClient &GetInstance()
    {
        static NetManagerStandard::NetConnClient client;
        return client;
    }
    virtual int32_t GetDefaultNet(NetManagerStandard::NetHandle &netHandle) = 0;
public:
    DfsNetConnClient() = default;
    virtual ~DfsNetConnClient() = default;
    static inline std::shared_ptr<DfsNetConnClient> dfsNetConnClient = nullptr;
};

class NetConnClientMock : public DfsNetConnClient {
public:
    MOCK_METHOD1(GetDefaultNet, int32_t(NetManagerStandard::NetHandle &netHandle));
};
}

#endif