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
#include "network/softbus/softbus_agent.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <memory>
#include <unistd.h>

#include "dm_constants.h"

#include "device_manager_agent_mock.h"
#include "device_manager_impl_mock.h"
#include "dm_constants.h"
#include "network/softbus/softbus_session_dispatcher.h"
#include "network/softbus/softbus_session.h"
#include "utils_log.h"
#include "connect_count/connect_count.h"

using namespace std;

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace testing;

const std::string NETWORKID_ONE = "45656596896323231";
const std::string NETWORKID_TWO = "45656596896323232";
constexpr int USER_ID = 100;
static const string SAME_ACCOUNT = "account";

class SoftbusAgentSupTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
    static inline std::shared_ptr<DeviceManagerAgentMock> deviceManagerAgentMock_ = nullptr;
    static inline shared_ptr<DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
};

void SoftbusAgentSupTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    deviceManagerAgentMock_ = std::make_shared<DeviceManagerAgentMock>();
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = deviceManagerAgentMock_;
    deviceManagerImplMock_ = make_shared<DeviceManagerImplMock>();
    DeviceManagerImplMock::dfsDeviceManagerImpl = deviceManagerImplMock_;
}

void SoftbusAgentSupTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = nullptr;
    deviceManagerAgentMock_ = nullptr;
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
}

/**
 * @tc.name: SoftbusAgentSupTest_GetSessionProcessInner_0100
 * @tc.desc: Verify the GetSessionProcessInner function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftbusAgentSupTest, SoftbusAgentSupTest_GetSessionProcessInner_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusAgentSupTest_GetSessionProcessInner_0100 start";
    auto mp = make_unique<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    shared_ptr<MountPoint> smp = move(mp);
    weak_ptr<MountPoint> wmp(smp);
    std::shared_ptr<SoftbusAgent> agent = std::make_shared<SoftbusAgent>(wmp);

    NotifyParam param {.fd = 1, .remoteCid = "testNetworkId"};
    agent->sessionPool_.ReleaseAllSession();
    agent->GetSessionProcessInner(param);
    EXPECT_FALSE(agent->sessionPool_.FindCid("testNetworkId"));

    param.fd = -1;
    auto session = make_shared<SoftbusSession>(1, "testNetworkId");
    session->SetFromServer(false);
    agent->sessionPool_.AddSessionToPool(session);
    ConnectCount::GetInstance().RemoveAllConnect();
    agent->GetSessionProcessInner(param);
    EXPECT_FALSE(agent->sessionPool_.FindCid("testNetworkId"));

    sptr<IFileDfsListener> listener = nullptr;
    agent->sessionPool_.AddSessionToPool(session);
    ConnectCount::GetInstance().AddConnect(1, "testNetworkId", listener);
    agent->GetSessionProcessInner(param);
    EXPECT_FALSE(agent->sessionPool_.FindCid("testNetworkId"));

    GTEST_LOG_(INFO) << "SoftbusAgentSupTest_GetSessionProcessInner_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
