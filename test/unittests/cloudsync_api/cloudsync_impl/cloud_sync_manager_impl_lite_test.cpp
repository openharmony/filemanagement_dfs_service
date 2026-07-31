/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cloud_sync_manager_impl_lite.h"
#include "dfs_error.h"
#include "service_proxy_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
using namespace testing::ext;
using namespace testing;
using namespace std;

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"mock") {}
    MOCK_METHOD1(AddDeathRecipient, bool(const sptr<DeathRecipient> &recipient));
    MOCK_METHOD1(RemoveDeathRecipient, bool(const sptr<DeathRecipient> &recipient));
    int32_t SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                        MessageOption &option) override { return 0; }
    sptr<IRemoteObject> AsObject() { return this; }
    bool IsProxyObject() const override { return false; }
    bool CheckObjectLegality() const override { return true; }
    int GetObjectRefCount() override { return 1; }
    int Dump(int fd, const std::vector<std::u16string> &args) override { return 0; }
};

class CloudSyncManagerImplLiteTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<MockServiceProxy> proxy_ = nullptr;
    static inline sptr<CloudSyncServiceMock> serviceProxy_ = nullptr;
};

void CloudSyncManagerImplLiteTest::SetUpTestCase(void)
{
    proxy_ = std::make_shared<MockServiceProxy>();
    IserviceProxy::proxy_ = proxy_;
    serviceProxy_ = sptr(new CloudSyncServiceMock());
    CloudSyncManagerImplLite::GetInstance().isFirstCall_.test_and_set();
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudSyncManagerImplLiteTest::TearDownTestCase(void)
{
    IserviceProxy::proxy_ = nullptr;
    proxy_ = nullptr;
    serviceProxy_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudSyncManagerImplLiteTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudSyncManagerImplLiteTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RemovedCleanTest001
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, RemovedCleanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest001 Start";
    try {
        string bundleName = "";
        int32_t userId = 1;
        auto res = CloudSyncManagerImplLite::GetInstance().RemovedClean(bundleName, userId);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest001 End";
}

/**
 * @tc.name: RemovedCleanTest002
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, RemovedCleanTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest002 Start";
    try {
        string bundleName = "test.bundle.name";
        int32_t userId = 1;
        auto res = CloudSyncManagerImplLite::GetInstance().RemovedClean(bundleName, userId);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest002 End";
}

/**
 * @tc.name: RemovedCleanTest003
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, RemovedCleanTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest003 Start";
    try {
        string bundleName = "test.bundle.name";
        int32_t userId = 101;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImplLite::GetInstance().RemovedClean(bundleName, userId);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest003 End";
}

/**
 * @tc.name: RemovedCleanTest004
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, RemovedCleanTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest004 Start";
    try {
        string bundleName = "test.bundle.name";
        int32_t userId = 101;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(serviceProxy_));
        CloudSyncManagerImplLite::GetInstance().RemovedClean(bundleName, userId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemovedCleanTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest004 End";
}
/**
 * @tc.name: SetDeathRecipient_NullRemoteObject_001
 * @tc.desc: Verify SetDeathRecipient returns early when remoteObject is nullptr
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, SetDeathRecipient_NullRemoteObject_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipient_NullRemoteObject_001 Start";
    try {
        auto &manager = CloudSyncManagerImplLite::GetInstance();
        manager.isFirstCall_.clear();
 
        manager.SetDeathRecipient(nullptr);
 
        // isFirstCall_ should NOT be set because we returned early before test_and_set
        EXPECT_FALSE(manager.isFirstCall_.test_and_set());
        manager.isFirstCall_.clear();
        // deathRecipient_ should remain null since we never created one
        EXPECT_EQ(manager.deathRecipient_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipient_NullRemoteObject_001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipient_NullRemoteObject_001 End";
}
 
/**
 * @tc.name: SetDeathRecipient_AddDeathRecipientFailed_001
 * @tc.desc: Verify isFirstCall_ is cleared when AddDeathRecipient returns false
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, SetDeathRecipient_AddDeathRecipientFailed_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipient_AddDeathRecipientFailed_001 Start";
    try {
        auto &manager = CloudSyncManagerImplLite::GetInstance();
        manager.isFirstCall_.clear();
        manager.deathRecipient_ = nullptr;
 
        // Create a mock IRemoteObject whose AddDeathRecipient returns false
        sptr<MockRemoteObject> mockRemote = sptr(new MockRemoteObject());
        EXPECT_CALL(*mockRemote, AddDeathRecipient(_)).WillOnce(Return(false));
 
        manager.SetDeathRecipient(mockRemote);
 
        // isFirstCall_ should be cleared so next call can retry
        EXPECT_FALSE(manager.isFirstCall_.test_and_set());
        manager.isFirstCall_.clear();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipient_AddDeathRecipientFailed_001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipient_AddDeathRecipientFailed_001 End";
}
 
/**
 * @tc.name: SetDeathRecipient_Success_001
 * @tc.desc: Verify SetDeathRecipient works normally when AddDeathRecipient returns true
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncManagerImplLiteTest, SetDeathRecipient_Success_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipient_Success_001 Start";
    try {
        auto &manager = CloudSyncManagerImplLite::GetInstance();
        manager.isFirstCall_.clear();
        manager.deathRecipient_ = nullptr;
 
        sptr<MockRemoteObject> mockRemote = sptr(new MockRemoteObject());
        EXPECT_CALL(*mockRemote, AddDeathRecipient(_)).WillOnce(Return(true));
 
        manager.SetDeathRecipient(mockRemote);
 
        // isFirstCall_ should remain set on success
        EXPECT_TRUE(manager.isFirstCall_.test_and_set());
        manager.isFirstCall_.clear();
        // deathRecipient_ should have been created
        EXPECT_NE(manager.deathRecipient_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipient_Success_001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipient_Success_001 End";
}

} // namespace FileManagement::CloudSync
} // namespace OHOS