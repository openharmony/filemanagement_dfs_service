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

#include "cloud_file_sync_service_interface_code.h"
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
    MOCK_METHOD2(RegisterCallbackInner,
                 int32_t(const sptr<IRemoteObject> &remoteObject, const std::string &bundleName));
    MOCK_METHOD1(UnRegisterCallbackInner, int32_t(const std::string &bundleName));
    MOCK_METHOD2(StartSyncInner, int32_t(bool forceFlag, const std::string &bundleName));
    MOCK_METHOD2(TriggerSyncInner, int32_t(const std::string &bundleName, const int32_t &userId));
    MOCK_METHOD1(StopSyncInner, int32_t(const std::string &bundleName));
    MOCK_METHOD3(ChangeAppSwitch, int32_t(const std::string &accoutId, const std::string &bundleName, bool status));
    MOCK_METHOD2(Clean, int32_t(const std::string &accountId, const CleanOptions &cleanOptions));
    MOCK_METHOD2(NotifyDataChange, int32_t(const std::string &accoutId, const std::string &bundleName));
    MOCK_METHOD3(NotifyEventChange, int32_t(int32_t userId, const std::string &eventId, const std::string &extraData));
    MOCK_METHOD2(EnableCloud, int32_t(const std::string &accoutId, const SwitchDataObj &switchData));
    MOCK_METHOD1(DisableCloud, int32_t(const std::string &accoutId));
    MOCK_METHOD1(StartDownloadFile, int32_t(const std::string &path));
    MOCK_METHOD1(StartFileCache, int32_t(const std::string &path));
    MOCK_METHOD1(StopDownloadFile, int32_t(const std::string &path));
    MOCK_METHOD1(RegisterDownloadFileCallback, int32_t(const sptr<IRemoteObject> &downloadCallback));
    MOCK_METHOD0(UnregisterDownloadFileCallback, int32_t());
    MOCK_METHOD3(UploadAsset, int32_t(const int32_t userId, const std::string &request, std::string &result));
    MOCK_METHOD3(DownloadFile,
                 int32_t(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj));
    MOCK_METHOD4(DownloadFiles,
                 int32_t(const int32_t userId,
                         const std::string &bundleName,
                         const std::vector<AssetInfoObj> &assetInfoObj,
                         std::vector<bool> &assetResultMap));
    MOCK_METHOD5(DownloadAsset,
                 int32_t(const uint64_t taskId,
                         const int32_t userId,
                         const std::string &bundleName,
                         const std::string &networkId,
                         AssetInfoObj &assetInfoObj));
    MOCK_METHOD1(RegisterDownloadAssetCallback, int32_t(const sptr<IRemoteObject> &remoteObject));
    MOCK_METHOD2(DeleteAsset, int32_t(const int32_t userId, const std::string &uri));
    MOCK_METHOD2(GetSyncTimeInner, int32_t(int64_t &syncTime, const std::string &bundleName));
    MOCK_METHOD1(CleanCacheInner, int32_t(const std::string &uri));
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
        string bundleName = "";
        MockService service;
        EXPECT_CALL(service, UnRegisterCallbackInner(bundleName)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        sptr<CloudSyncCallbackMock> remote = sptr(new CloudSyncCallbackMock());
        EXPECT_TRUE(data.WriteRemoteObject(remote->AsObject().GetRefPtr()));

        EXPECT_TRUE(data.WriteString(bundleName));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_CALLBACK),
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
        string bundleName = "";
        MockService service;
        EXPECT_CALL(service, RegisterCallbackInner(_, bundleName)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        sptr<CloudSyncCallbackMock> remote = sptr(new CloudSyncCallbackMock());
        EXPECT_TRUE(data.WriteRemoteObject(remote->AsObject().GetRefPtr()));

        EXPECT_TRUE(data.WriteString(bundleName));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_CALLBACK),
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
        string bundleName = "";
        MockService service;
        EXPECT_CALL(service, StartSyncInner(_, bundleName)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        bool forceFlag = true;
        EXPECT_TRUE(data.WriteBool(forceFlag));

        EXPECT_TRUE(data.WriteString(bundleName));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_SYNC), data,
                            reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStartSyncInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStartSyncInner End";
}

/**
 * @tc.name: HandleTriggerSyncInnerTest
 * @tc.desc: Verify the HandleStartSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleTriggerSyncInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleTriggerSyncInner Start";
    try {
        MockService service;
        EXPECT_CALL(service, TriggerSyncInner(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_TRIGGER_SYNC), data,
                            reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleTriggerSyncInner ERROR";
    }
    GTEST_LOG_(INFO) << "HandleTriggerSyncInner End";
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
        string bundleName = "";
        MockService service;
        EXPECT_CALL(service, StopSyncInner(bundleName)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_TRUE(data.WriteString(bundleName));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_SYNC), data,
                            reply, option));
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CHANGE_APP_SWITCH),
                            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleChangeAppSwitch ERROR";
    }
    GTEST_LOG_(INFO) << "HandleChangeAppSwitch End";
}

/**
 * @tc.name: HandleCleanTest
 * @tc.desc: Verify the HandleClean function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleCleandTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleClean Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_INVAL_ARG, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN), data,
                            reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleClean ERROR";
    }
    GTEST_LOG_(INFO) << "HandleClean End";
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_DATA_CHANGE),
                            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleNotifyDataChange ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNotifyDataChange End";
}

/**
 * @tc.name: HandleNotifyEventChangeTest
 * @tc.desc: Verify the HandleNotifyEventChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleNotifyEventChangeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNotifyEventChange Start";
    try {
        MockService service;
        EXPECT_CALL(service, NotifyEventChange(_, _, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_EVENT_CHANGE),
                            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleNotifyEventChange ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNotifyEventChange End";
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DISABLE_CLOUD), data,
                            reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDisableCloud ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDisableCloud End";
}

/**
 * @tc.name: HandleEnableCloudTest
 * @tc.desc: Verify the HandleEnableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleEnableCloudTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleEnableCloudFile Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_INVAL_ARG, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_ENABLE_CLOUD), data,
                            reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleEnableCloud ERROR";
    }
    GTEST_LOG_(INFO) << "HandleEnableCloud End";
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_DOWNLOAD_FILE),
                            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStartDownloadFile ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStartDownloadFile End";
}

/**
 * @tc.name: HandleStartFileCacheTest
 * @tc.desc: Verify the HandleStartFileCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStartFileCacheTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStartFileCache Start";
    try {
        MockService service;
        EXPECT_CALL(service, StartDownloadFile(_)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_FILE_CACHE),
                            data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStartFileCache ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStartFileCache End";
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_DOWNLOAD_FILE),
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(
                                CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK),
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(
                                CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK),
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

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UPLOAD_ASSET), data,
                            reply, option));
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
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_INVAL_ARG, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILE), data,
                            reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDownloadFile ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDownloadFile End";
}

/**
 * @tc.name: HandleDownloadAssetTest
 * @tc.desc: Verify the HandleDownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDownloadAssetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDownloadAsset Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_INVAL_ARG, service.OnRemoteRequest(
                            static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_ASSET), data,
                            reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDownloadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDownloadAsset End";
}

/**
 * @tc.name: HandleRegisterDownloadAssetCallbackTest
 * @tc.desc: Verify the HandleRegisterDownloadAssetCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleRegisterDownloadAssetCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRegisterDownloadAssetCallback Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
              static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_ASSET_CALLBACK),
              data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleRegisterDownloadAssetCallback ERROR";
    }
    GTEST_LOG_(INFO) << "HandleRegisterDownloadAssetCallback End";
}

/**
 * @tc.name: HandleDeleteAssetTest
 * @tc.desc: Verify the HandleDeleteAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDeleteAssetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDeleteAsset Start";
    try {
        MockService service;
        EXPECT_CALL(service, DeleteAsset(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
              static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DELETE_ASSET),
              data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleDeleteAsset ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDeleteAsset End";
}

/**
 * @tc.name: HandleGetSyncTimeTest
 * @tc.desc: Verify the HandleGetSyncTime function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleGetSyncTimeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleGetSyncTime Start";
    try {
        string bundleName = "";
        MockService service;
        EXPECT_CALL(service, GetSyncTimeInner(_, bundleName)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_TRUE(data.WriteString(bundleName));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
              static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_GET_SYNC_TIME),
              data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleGetSyncTime ERROR";
    }
    GTEST_LOG_(INFO) << "HandleGetSyncTime End";
}

/**
 * @tc.name: HandleCleanCacheTest
 * @tc.desc: Verify the HandleCleanCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleCleanCacheTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCleanCache Start";
    try {
        MockService service;
        EXPECT_CALL(service, CleanCacheInner(_)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_EQ(E_OK, service.OnRemoteRequest(
              static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN_CACHE),
              data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCleanCache ERROR";
    }
    GTEST_LOG_(INFO) << "HandleCleanCache End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
