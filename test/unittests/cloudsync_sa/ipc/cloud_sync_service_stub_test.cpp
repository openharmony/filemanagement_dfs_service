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

#include "cloud_sync_service_stub.h"
#include "i_cloud_sync_service.h"
#include "service_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockService final : public CloudSyncServiceStub {
public:
    MOCK_METHOD1(RegisterCallbackInner, int32_t(const sptr<IRemoteObject> &remoteObject));
    MOCK_METHOD1(StartSyncInner, int32_t(bool forceFlag));
    MOCK_METHOD0(StopSyncInner, int32_t());
    MOCK_METHOD3(ChangeAppSwitch, int32_t(const std::string &accoutId, const std::string &bundleName, bool status));
    MOCK_METHOD2(Clean, int32_t(const std::string &accountId, const CleanOptions &cleanOptions));
    MOCK_METHOD2(NotifyDataChange, int32_t(const std::string &accoutId, const std::string &bundleName));
    MOCK_METHOD2(EnableCloud, int32_t(const std::string &accoutId, const SwitchDataObj &switchData));
    MOCK_METHOD1(DisableCloud, int32_t(const std::string &accoutId));
    MOCK_METHOD1(StartDownloadFile, int32_t(const std::string &path));
    MOCK_METHOD1(StopDownloadFile, int32_t(const std::string &path));
    MOCK_METHOD1(RegisterDownloadFileCallback, int32_t(const sptr<IRemoteObject> &downloadCallback));
    MOCK_METHOD0(UnregisterDownloadFileCallback, int32_t());
    MOCK_METHOD3(DownloadFile, int32_t(const int32_t userId, const std::string &bundleName, AssetInfo &assetInfo));
};

class CloudSyncServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncServiceStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncServiceStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncServiceStubTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncServiceStubTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: HandleRegisterCallbackInnerTest
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleRegisterCallbackInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRegisterCallbackInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, RegisterCallbackInner(_)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        sptr<CloudSyncCallbackMock> remote = sptr(new CloudSyncCallbackMock());
        EXPECT_TRUE(data.WriteRemoteObject(remote->AsObject().GetRefPtr()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_REGISTER_CALLBACK, data, reply, option));
        remote = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleRegisterCallbackInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleRegisterCallbackInner End";
}

/**
 * @tc.name: HandleStartSyncInnerTest
 * @tc.desc: Verify the HandleStartSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStartSyncInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStartSyncInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, StartSyncInner(_)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        bool forceFlag = true;
        EXPECT_TRUE(data.WriteBool(forceFlag));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_START_SYNC, data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStartSyncInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStartSyncInner End";
}

/**
 * @tc.name: HandleStopSyncInnerTest
 * @tc.desc: Verify the HandleStopSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStopSyncInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStopSyncInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, StopSyncInner()).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_STOP_SYNC, data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStopSyncInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStopSyncInner End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
