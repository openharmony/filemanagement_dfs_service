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
    MOCK_METHOD0(UnRegisterCallbackInner, int32_t());
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
    MOCK_METHOD3(UploadAsset, int32_t(const int32_t userId, const std::string &request, std::string &result));
    MOCK_METHOD3(
        DownloadFile, int32_t(const int32_t userId,const std::string &bundleName, AssetInfoObj &assetInfoObj));
    MOCK_METHOD2(HandleUnRegisterCallbackInner, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleRegisterCallbackInner, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleStartSyncInner, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleStopSyncInner, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleChangeAppSwitch, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleClean, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleNotifyDataChange, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleEnableCloud, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleDisableCloud, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleStartDownloadFile, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleStopDownloadFile, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleRegisterDownloadFileCallback, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleUnregisterDownloadFileCallback, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleUploadAsset, int32_t(MessageParcel &data, MessageParcel &reply));
    MOCK_METHOD2(HandleDownloadFile, int32_t(MessageParcel &data, MessageParcel &reply));
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
 * @tc.name: HandleUnRegisterCallbackInnerTest
 * @tc.desc: Verify the HandleUnRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleUnRegisterCallbackInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUnRegisterCallbackInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, UnRegisterCallbackInner()).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        sptr<CloudSyncCallbackMock> remote = sptr(new CloudSyncCallbackMock());
        EXPECT_TRUE(data.WriteRemoteObject(remote->AsObject().GetRefPtr()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_UNREGISTER_CALLBACK,
            data, reply, option));
        remote = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleUnRegisterCallbackInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUnRegisterCallbackInner End";
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_REGISTER_CALLBACK,
            data, reply, option));
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

/**
 * @tc.name: HandleChangeAppSwitchTest
 * @tc.desc: Verify the HandleChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleChangeAppSwitchTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleChangeAppSwitch Start";
    try {
        MockService service;
        EXPECT_CALL(service, ChangeAppSwitch(_, _, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_CHANGE_APP_SWITCH,
            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleChangeAppSwitch ERROR";
    }
    GTEST_LOG_(INFO) << "HandleChangeAppSwitch End";
}

/**
 * @tc.name: HandleNotifyDataChangeTest
 * @tc.desc: Verify the HandleNotifyDataChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleNotifyDataChangeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNotifyDataChange Start";
    try {
        MockService service;
        EXPECT_CALL(service, NotifyDataChange(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_NOTIFY_DATA_CHANGE,
            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleNotifyDataChange ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNotifyDataChange End";
}

/**
 * @tc.name: HandleDisableCloudTest
 * @tc.desc: Verify the HandleDisableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDisableCloudTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStopSyncInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, DisableCloud(_)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_DISABLE_CLOUD, data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDisableCloud ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDisableCloud End";
}

/**
 * @tc.name: HandleStartDownloadFileTest
 * @tc.desc: Verify the HandleStartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStartDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStartDownloadFile Start";
    try {
        MockService service;
        EXPECT_CALL(service, StartDownloadFile(_)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_START_DOWNLOAD_FILE,
            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStartDownloadFile ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStartDownloadFile End";
}

/**
 * @tc.name: HandleStopDownloadFileTest
 * @tc.desc: Verify the HandleStopDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStopDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStopDownloadFile Start";
    try {
        MockService service;
        EXPECT_CALL(service, StopDownloadFile(_)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_STOP_DOWNLOAD_FILE,
            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStopDownloadFile ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStopDownloadFile End";
}

/**
 * @tc.name: HandleRegisterDownloadFileCallbackTest
 * @tc.desc: Verify the HandleRegisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleRegisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRegisterDownloadFileCallback Start";
    try {
        MockService service;
        EXPECT_CALL(service, RegisterDownloadFileCallback(_)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK,
            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleRegisterDownloadFileCallback ERROR";
    }
    GTEST_LOG_(INFO) << "HandleRegisterDownloadFileCallback End";
}

/**
 * @tc.name: HandleUnregisterDownloadFileCallbackTest
 * @tc.desc: Verify the HandleUnregisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleUnregisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUnregisterDownloadFileCallback Start";
    try {
        MockService service;
        EXPECT_CALL(service, UnregisterDownloadFileCallback()).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK,
            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleUnregisterDownloadFileCallback ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUnregisterDownloadFileCallback End";
}

/**
 * @tc.name: HandleUploadAssetTest
 * @tc.desc: Verify the HandleUploadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleUploadAssetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUploadAsset Start";
    try {
        MockService service;
        EXPECT_CALL(service, UploadAsset(_, _, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_UPLOAD_ASSET, data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleUploadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUploadAsset End";
}

/**
 * @tc.name: HandleDownloadFileTest
 * @tc.desc: Verify the HandleDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDownloadFile Start";
    try {
        MockService service;
        EXPECT_CALL(service, DownloadFile(_, _, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(ICloudSyncService::SERVICE_CMD_DOWNLOAD_FILE, data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDownloadFile ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDownloadFile End";
}

/**
 * @tc.name: HandleUnRegisterCallbackInnerTest002
 * @tc.desc: Verify the HandleUnRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleUnRegisterCallbackInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUnRegisterCallbackInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleUnRegisterCallbackInner(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleUnRegisterCallbackInner(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleUnRegisterCallbackInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUnRegisterCallbackInner End";
}

/**
 * @tc.name: HandleRegisterCallbackInnerTest002
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleRegisterCallbackInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRegisterCallbackInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleRegisterCallbackInner(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleRegisterCallbackInner(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleRegisterCallbackInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleRegisterCallbackInner End";
}

/**
 * @tc.name: HandleStartSyncInnerTest002
 * @tc.desc: Verify the HandleStartSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStartSyncInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRegisterCallbackInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleStartSyncInner(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleStartSyncInner(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStartSyncInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStartSyncInner End";
}

/**
 * @tc.name: HandleStopSyncInnerTest002
 * @tc.desc: Verify the HandleStopSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStopSyncInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStopSyncInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleStopSyncInner(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleStopSyncInner(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStopSyncInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStopSyncInner End";
}

/**
 * @tc.name: HandleChangeAppSwitchTest002
 * @tc.desc: Verify the HandleChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleChangeAppSwitchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleChangeAppSwitch Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleChangeAppSwitch(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleChangeAppSwitch(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleChangeAppSwitch ERROR";
    }
    GTEST_LOG_(INFO) << "HandleChangeAppSwitch End";
}

/**
 * @tc.name: HandleNotifyDataChangeTest002
 * @tc.desc: Verify the HandleNotifyDataChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleNotifyDataChangeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNotifyDataChange Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleNotifyDataChange(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleNotifyDataChange(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleNotifyDataChange ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNotifyDataChange End";
}

/**
 * @tc.name: HandleDisableCloudTest002
 * @tc.desc: Verify the HandleDisableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDisableCloudTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDisableCloud Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleDisableCloud(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleDisableCloud(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDisableCloud ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDisableCloud End";
}

/**
 * @tc.name: HandleStartDownloadFileTest002
 * @tc.desc: Verify the HandleStartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStartDownloadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStartDownloadFile Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleStartDownloadFile(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleStartDownloadFile(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStartDownloadFile ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStartDownloadFile End";
}

/**
 * @tc.name: HandleStopDownloadFileTest002
 * @tc.desc: Verify the HandleStopDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStopDownloadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStopDownloadFile Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleStopDownloadFile(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleStopDownloadFile(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStopDownloadFile ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStopDownloadFile End";
}

/**
 * @tc.name: HandleRegisterDownloadFileCallbackTest002
 * @tc.desc: Verify the HandleRegisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleRegisterDownloadFileCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRegisterDownloadFileCallback Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleRegisterDownloadFileCallback(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleRegisterDownloadFileCallback(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleRegisterDownloadFileCallback ERROR";
    }
    GTEST_LOG_(INFO) << "HandleRegisterDownloadFileCallback End";
}

/**
 * @tc.name: HandleUnregisterDownloadFileCallbackTest002
 * @tc.desc: Verify the HandleUnregisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleUnregisterDownloadFileCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUnregisterDownloadFileCallback Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleUnregisterDownloadFileCallback(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleUnregisterDownloadFileCallback(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleUnregisterDownloadFileCallback ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUnregisterDownloadFileCallback End";
}

/**
 * @tc.name: HandleUploadAssetTest002
 * @tc.desc: Verify the HandleUploadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleUploadAssetTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUploadAsset Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleUploadAsset(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleUploadAsset(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleUploadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUploadAsset End";
}

/**
 * @tc.name: HandleDownloadFileTest002
 * @tc.desc: Verify the HandleDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDownloadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDownloadFile Start";
    try {
        MockService service;
        EXPECT_CALL(service, HandleDownloadFile(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(E_OK, service.HandleDownloadFile(data, reply));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDownloadFile ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDownloadFile End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS