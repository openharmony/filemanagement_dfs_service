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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <unistd.h>

#include "dfs_error.h"
#include "net_conn_client.h"
#include "parameter.h"
#include "sync_rule/net_conn_callback_observer.h"
#include "sync_rule/network_status.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class NetworkStatusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void NetworkStatusTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void NetworkStatusTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void NetworkStatusTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void NetworkStatusTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RegisterNetConnCallbackTest
 * @tc.desc: Verify the RegisterNetConnCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, RegisterNetConnCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterNetConnCallbackTest Start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        int32_t ret = NetworkStatus::RegisterNetConnCallback(dataSyncManager);
        EXPECT_EQ(ret, E_GET_NETWORK_MANAGER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RegisterNetConnCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterNetConnCallbackTest End";
}

/**
 * @tc.name: GetDefaultNetTest
 * @tc.desc: Verify the GetDefaultNet function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, GetDefaultNetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDefaultNetTest Start";
    try {
        int32_t ret = NetworkStatus::GetDefaultNet();
        EXPECT_EQ(ret, E_GET_NETWORK_MANAGER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDefaultNetTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetDefaultNetTest End";
}

/**
 * @tc.name: SetNetConnStatusTest001
 * @tc.desc: Verify the SetNetConnStatus function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, SetNetConnStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 Start";
    try {
        NetManagerStandard::NetAllCapabilities netAllCap;
        NetworkStatus::SetNetConnStatus(netAllCap);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetNetConnStatusTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetNetConnStatusTest001 End";
}

/**
 * @tc.name: GetAndRegisterNetworkTest
 * @tc.desc: Verify the GetAndRegisterNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, GetAndRegisterNetworkTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAndRegisterNetworkTest Start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        int32_t ret = NetworkStatus::GetAndRegisterNetwork(dataSyncManager);
        EXPECT_EQ(ret, E_GET_NETWORK_MANAGER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAndRegisterNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetAndRegisterNetworkTest End";
}

/**
 * @tc.name: InitNetworkTest
 * @tc.desc: Verify the InitNetwork function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, InitNetworkTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitNetworkTest Start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        NetworkStatus::InitNetwork(dataSyncManager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " InitNetworkTest FAILED";
    }
    GTEST_LOG_(INFO) << "InitNetworkTest End";
}

/**
 * @tc.name: SetNetConnStatusTest002
 * @tc.desc: Verify the SetNetConnStatus function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, SetNetConnStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetNetConnStatusTest002 Start";
    try {
        NetworkStatus::SetNetConnStatus(NetworkStatus::NO_NETWORK);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetNetConnStatusTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "SetNetConnStatusTest002 End";
}

/**
 * @tc.name: GetNetConnStatusTest
 * @tc.desc: Verify the GetNetConnStatus function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetworkStatusTest, GetNetConnStatusTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetConnStatusTest Start";
    try {
        auto ret = NetworkStatus::GetNetConnStatus();
        EXPECT_EQ(ret, NetworkStatus::NO_NETWORK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetNetConnStatusTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetNetConnStatusTest End";
}
} // namespace OHOS::FileManagement::CloudSync::Test
