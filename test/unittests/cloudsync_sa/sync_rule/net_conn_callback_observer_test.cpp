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

#include "dfs_error.h"
#include "sync_rule/net_conn_callback_observer.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class NetConnCallbackObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<NetConnCallbackObserver> oberverPtr_;
};
void NetConnCallbackObserverTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void NetConnCallbackObserverTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void NetConnCallbackObserverTest::SetUp(void)
{
    if (oberverPtr_ == nullptr) {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        oberverPtr_ = make_shared<NetConnCallbackObserver>(dataSyncManager);
        ASSERT_TRUE(oberverPtr_ != nullptr) << "CallbackObserver failed";
    }
    GTEST_LOG_(INFO) << "SetUp";
}

void NetConnCallbackObserverTest::TearDown(void)
{
    oberverPtr_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: NetAvailableTest
 * @tc.desc: Verify the NetAvailable function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetConnCallbackObserverTest, NetAvailable, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetAvailable Start";
    sptr<NetManagerStandard::NetHandle> netHandle = sptr(new NetManagerStandard::NetHandle());
    int res = oberverPtr_->NetAvailable(netHandle);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "NetAvailable End";
}

/**
 * @tc.name: NetCapabilitiesChangeTest
 * @tc.desc: Verify the NetCapabilitiesChange function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetConnCallbackObserverTest, NetCapabilitiesChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetCapabilitiesChange Start";
    sptr<NetManagerStandard::NetHandle> netHandle = sptr(new NetManagerStandard::NetHandle());
    sptr<NetManagerStandard::NetAllCapabilities> netAllCap = sptr(new NetManagerStandard::NetAllCapabilities());
    int res = oberverPtr_->NetCapabilitiesChange(netHandle, netAllCap);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "NetCapabilitiesChange End";
}

/**
 * @tc.name: NetConnectionPropertiesChangeTest
 * @tc.desc: Verify the NetConnectionPropertiesChange function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetConnCallbackObserverTest, NetConnectionPropertiesChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetConnectionPropertiesChange Start";
    sptr<NetManagerStandard::NetHandle> netHandle = sptr(new NetManagerStandard::NetHandle());
    sptr<NetManagerStandard::NetLinkInfo> info = sptr(new NetManagerStandard::NetLinkInfo());
    int res = oberverPtr_->NetConnectionPropertiesChange(netHandle, info);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "NetConnectionPropertiesChange End";
}

/**
 * @tc.name: NetLostTest
 * @tc.desc: Verify the NetLost function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetConnCallbackObserverTest, NetLost, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetLost Start";
    sptr<NetManagerStandard::NetHandle> netHandle = sptr(new NetManagerStandard::NetHandle());
    int res = oberverPtr_->NetLost(netHandle);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "NetLost End";
}

/**
 * @tc.name: NetUnavailableTest
 * @tc.desc: Verify the NetUnavailable function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetConnCallbackObserverTest, NetUnavailable, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetUnavailable Start";
    int res = oberverPtr_->NetUnavailable();
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "NetUnavailable End";
}

/**
 * @tc.name: NetBlockStatusChangeTest
 * @tc.desc: Verify the NetBlockStatusChange function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(NetConnCallbackObserverTest, NetBlockStatusChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetBlockStatusChange Start";
    sptr<NetManagerStandard::NetHandle> netHandle = sptr(new NetManagerStandard::NetHandle());
    bool blocked = true;
    int res = oberverPtr_->NetBlockStatusChange(netHandle, blocked);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "NetBlockStatusChange End";
}
}