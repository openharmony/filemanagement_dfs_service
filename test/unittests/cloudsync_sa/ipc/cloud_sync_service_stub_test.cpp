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
#include "dfsu_access_token_helper_mock.h"
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
    MOCK_METHOD2(RegisterFileSyncCallbackInner,
                 int32_t(const sptr<IRemoteObject> &remoteObject, const std::string &bundleName));
    MOCK_METHOD1(UnRegisterCallbackInner, int32_t(const std::string &bundleName));
    MOCK_METHOD1(UnRegisterFileSyncCallbackInner, int32_t(const std::string &bundleName));
    MOCK_METHOD2(StartSyncInner, int32_t(bool forceFlag, const std::string &bundleName));
    MOCK_METHOD2(StartFileSyncInner, int32_t(bool forceFlag, const std::string &bundleName));
    MOCK_METHOD2(TriggerSyncInner, int32_t(const std::string &bundleName, const int32_t &userId));
    MOCK_METHOD2(StopSyncInner, int32_t(const std::string &bundleName, bool forceFlag));
    MOCK_METHOD2(StopFileSyncInner, int32_t(const std::string &bundleName, bool forceFlag));
    MOCK_METHOD1(ResetCursor, int32_t(const std::string &bundleName));
    MOCK_METHOD3(ChangeAppSwitch, int32_t(const std::string &accoutId, const std::string &bundleName, bool status));
    MOCK_METHOD2(Clean, int32_t(const std::string &accountId, const CleanOptions &cleanOptions));
    MOCK_METHOD2(NotifyDataChange, int32_t(const std::string &accoutId, const std::string &bundleName));
    MOCK_METHOD3(NotifyEventChange, int32_t(int32_t userId, const std::string &eventId, const std::string &extraData));
    MOCK_METHOD2(EnableCloud, int32_t(const std::string &accoutId, const SwitchDataObj &switchData));
    MOCK_METHOD1(DisableCloud, int32_t(const std::string &accoutId));
    MOCK_METHOD1(StartDownloadFile, int32_t(const std::string &path));
    MOCK_METHOD6(StartFileCache, int32_t(const std::vector<std::string> &pathVec, int64_t &downloadId,
                                         std::bitset<FIELD_KEY_MAX_SIZE> fieldkey,
                                         bool isCallbackValid,
                                         const sptr<IRemoteObject> &downloadCallback,
                                         int32_t timeout));
    MOCK_METHOD2(StopDownloadFile, int32_t(const std::string &path, bool needClean));
    MOCK_METHOD3(StopFileCache, int32_t(int64_t downloadId, bool needClean, int32_t timeout));
    MOCK_METHOD1(RegisterDownloadFileCallback, int32_t(const sptr<IRemoteObject> &downloadCallback));
    MOCK_METHOD1(RegisterFileCacheCallback, int32_t(const sptr<IRemoteObject> &downloadCallback));
    MOCK_METHOD0(UnregisterDownloadFileCallback, int32_t());
    MOCK_METHOD0(UnregisterFileCacheCallback, int32_t());
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
    MOCK_METHOD0(DownloadThumb, int32_t());
    MOCK_METHOD2(BatchCleanFile, int32_t(const std::vector<CleanFileInfoObj> &fileInfo,
        std::vector<std::string> &failCloudId));
    MOCK_METHOD3(OptimizeStorage, int32_t(const OptimizeSpaceOptions &optimizeOptions, bool isCallbackValid,
            const sptr<IRemoteObject> &optimizeCallback));
    MOCK_METHOD2(BatchDentryFileInsert, int32_t(const std::vector<DentryFileInfoObj> &fileInfo,
        std::vector<std::string> &failCloudId));
    MOCK_METHOD0(StopOptimizeStorage, int32_t());
};

class CloudSyncServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DfsuAccessTokenMock> dfsuAccessToken_ = nullptr;
};

void CloudSyncServiceStubTest::SetUpTestCase(void)
{
    dfsuAccessToken_ = make_shared<DfsuAccessTokenMock>();
    DfsuAccessTokenMock::dfsuAccessToken = dfsuAccessToken_;
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncServiceStubTest::TearDownTestCase(void)
{
    DfsuAccessTokenMock::dfsuAccessToken = nullptr;
    dfsuAccessToken_ = nullptr;
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
 * @tc.name: HandleOptimizeStorageTest001
 * @tc.desc: Verify the HandleOptimizeStorage function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleOptimizeStorageTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOptimizeStorage Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;

        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int32_t res = service.HandleOptimizeStorage(data, reply);
        EXPECT_EQ(res, E_PERMISSION_DENIED);

    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOptimizeStorage ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOptimizeStorage End";
}

/**
 * @tc.name: HandleOptimizeStorageTest002
 * @tc.desc: Verify the HandleOptimizeStorage function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleOptimizeStorageTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOptimizeStorage Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;

        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
        int32_t res = service.HandleOptimizeStorage(data, reply);
        EXPECT_EQ(res, E_PERMISSION_SYSTEM);

    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOptimizeStorage ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOptimizeStorage End";
}

/**
 * @tc.name: HandleOptimizeStorageTest003
 * @tc.desc: Verify the HandleOptimizeStorage function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleOptimizeStorageTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOptimizeStorage Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;

        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        int32_t res = service.HandleOptimizeStorage(data, reply);
        EXPECT_EQ(res, E_INVAL_ARG);

    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOptimizeStorage ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOptimizeStorage End";
}

/**
 * @tc.name: HandleEnableCloudTest003
 * @tc.desc: Verify the HandleEnableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleEnableCloudTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleEnableCloud Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;

        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        int32_t res = service.HandleEnableCloud(data, reply);
        EXPECT_EQ(res, E_INVAL_ARG);

    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleEnableCloud ERROR";
    }
    GTEST_LOG_(INFO) << "HandleEnableCloud End";
}

/**
 * @tc.name: HandleStartFileCacheTest003
 * @tc.desc: Verify the HandleStartFileCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStartFileCacheTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStartFileCache Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;
        data.writable_ = true;

        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        EXPECT_CALL(*dfsuAccessToken_, CheckUriPermission(_)).WillOnce(Return(false));
        int32_t res = service.HandleStartFileCache(data, reply);
        EXPECT_EQ(res, E_OK);

    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleStartFileCache ERROR";
    }
    GTEST_LOG_(INFO) << "HandleStartFileCache End";
}

/**
 * @tc.name: HandleDownloadThumbTest001
 * @tc.desc: Verify the HandleDownloadThumb function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDownloadThumbTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDownloadThumb Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;

        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
        int32_t res = service.HandleDownloadThumb(data, reply);
        EXPECT_EQ(res, E_PERMISSION_SYSTEM);

    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDownloadThumb ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDownloadThumb End";
}

/**
 * @tc.name: HandleDownloadThumbTest002
 * @tc.desc: Verify the HandleDownloadThumb function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDownloadThumbTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDownloadThumb Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;

        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int32_t res = service.HandleDownloadThumb(data, reply);
        EXPECT_EQ(res, E_PERMISSION_DENIED);

    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDownloadThumb ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDownloadThumb End";
}

/**
 * @tc.name: HandleDownloadThumbTest003
 * @tc.desc: Verify the HandleDownloadThumb function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDownloadThumbTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDownloadThumb Start";
    try {
        MockService service;
        MessageParcel data;
        MessageParcel reply;

        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        int32_t res = service.HandleDownloadThumb(data, reply);
        EXPECT_EQ(res, E_OK);

    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleDownloadThumb ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDownloadThumb End";
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        bool forceFlag = false;
        MockService service;
        EXPECT_CALL(service, StopSyncInner(bundleName, forceFlag)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncService::GetDescriptor()));

        EXPECT_TRUE(data.WriteString(bundleName));

        EXPECT_TRUE(data.WriteBool(forceFlag));

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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(service, StopDownloadFile(_, _)).WillOnce(Return(E_OK));
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
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

HWTEST_F(CloudSyncServiceStubTest, HandleDownloadAssetTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleDownloadAsset(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDownloadAssetTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleDownloadAsset(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleUnRegisterCallbackInnerTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleUnRegisterCallbackInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleUnRegisterCallbackInnerTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleUnRegisterCallbackInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleRegisterCallbackInnerTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleRegisterCallbackInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleRegisterCallbackInnerTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleRegisterCallbackInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartSyncInnerTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStartSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartSyncInnerTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStartSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleTriggerSyncInnerTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleTriggerSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleTriggerSyncInnerTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleTriggerSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopSyncInnerTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStopSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopSyncInnerTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStopSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleResetCursorTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleResetCursor(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleResetCursorTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleResetCursor(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleResetCursorTest003, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillRepeatedly(Return(true));
    int32_t ret = service.HandleResetCursor(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleChangeAppSwitchTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleChangeAppSwitch(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleChangeAppSwitchTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleChangeAppSwitch(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleCleanTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleClean(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleCleanTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillRepeatedly (Return(false));
    int32_t ret = service.HandleClean(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleNotifyDataChangeTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleNotifyDataChange(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleNotifyDataChangeTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleNotifyDataChange(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleNotifyEventChangeTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleNotifyEventChange(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleNotifyEventChangeTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleNotifyEventChange(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleEnableCloudTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleEnableCloud(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleEnableCloudTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleEnableCloud(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDisableCloudTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleDisableCloud(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDisableCloudTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleDisableCloud(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartDownloadFileTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStartDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartDownloadFileTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStartDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartFileCacheTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStartFileCache(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartFileCacheTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
	EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStartFileCache(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopDownloadFileTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStopDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopDownloadFileTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStopDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopFileCacheTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStopFileCache(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopFileCacheTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStopFileCache(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopFileCacheTest003, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillRepeatedly(Return(true));
    int32_t ret = service.HandleStopFileCache(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleRegisterDownloadFileCallbackTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleRegisterDownloadFileCallback(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleUnregisterDownloadFileCallbackTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleUnregisterDownloadFileCallback(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleUploadAssetTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleUploadAsset(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleUploadAssetTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleUploadAsset(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDownloadFileTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDownloadFileTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDownloadFilesTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleDownloadFiles(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDownloadFilesTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleDownloadFiles(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDownloadFilesTest003, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
    int32_t ret = service.HandleDownloadFiles(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleRegisterDownloadAssetCallbackTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleRegisterDownloadAssetCallback(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleRegisterDownloadAssetCallbackTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleRegisterDownloadAssetCallback(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDeleteAssetTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleDeleteAsset(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDeleteAssetTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleDeleteAsset(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleGetSyncTimeTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleGetSyncTime(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleGetSyncTimeTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleGetSyncTime(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleBatchDentryFileInsert001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleBatchDentryFileInsert(data, reply);\
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleBatchDentryFileInsert002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleBatchDentryFileInsert(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleBatchDentryFileInsert003, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
    int32_t ret = service.HandleBatchDentryFileInsert(data, reply);
    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleCleanCacheTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleCleanCache(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleCleanCacheTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleCleanCache(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleBatchCleanFileTest001, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleBatchCleanFile(data, reply);\
    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleBatchCleanFileTest002, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleBatchCleanFile(data, reply);
    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleBatchCleanFileTest003, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
    int32_t ret = service.HandleBatchCleanFile(data, reply);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: HandleDownloadAssetTest1
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDownloadAssetTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleDownloadAsset(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

/**
 * @tc.name: HandleDownloadAssetTest2
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleDownloadAssetTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleDownloadAsset(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

/**
 * @tc.name: HandleUnRegisterCallbackInnerTest1
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleUnRegisterCallbackInnerTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleUnRegisterCallbackInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

/**
 * @tc.name: HandleUnRegisterCallbackInnerTest2
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleUnRegisterCallbackInnerTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleUnRegisterCallbackInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

/**
 * @tc.name: HandleRegisterCallbackInnerTest1
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleRegisterCallbackInnerTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleRegisterCallbackInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

/**
 * @tc.name: HandleRegisterCallbackInnerTest2
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleRegisterCallbackInnerTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleRegisterCallbackInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

/**
 * @tc.name: HandleStartSyncInnerTest1
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStartSyncInnerTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStartSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

/**
 * @tc.name: HandleStartSyncInnerTest2
 * @tc.desc: Verify the HandleRegisterCallbackInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceStubTest, HandleStartSyncInnerTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStartSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleTriggerSyncInnerTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleTriggerSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleTriggerSyncInnerTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleTriggerSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopSyncInnerTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStopSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopSyncInnerTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStopSyncInner(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleResetCursorTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleResetCursor(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleResetCursorTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleResetCursor(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleResetCursorTest3, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillRepeatedly(Return(true));
    int32_t ret = service.HandleResetCursor(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleChangeAppSwitchTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleChangeAppSwitch(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleChangeAppSwitchTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleChangeAppSwitch(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleCleanTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleClean(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleCleanTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillRepeatedly(Return(false));
    int32_t ret = service.HandleClean(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleNotifyDataChangeTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleNotifyDataChange(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleNotifyDataChangeTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleNotifyDataChange(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleNotifyEventChangeTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleNotifyEventChange(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleNotifyEventChangeTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleNotifyEventChange(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleEnableCloudTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleEnableCloud(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleEnableCloudTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleEnableCloud(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDisableCloudTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleDisableCloud(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleDisableCloudTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleDisableCloud(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartDownloadFileTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStartDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartDownloadFileTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStartDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartFileCacheTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStartFileCache(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStartFileCacheTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
	EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStartFileCache(data, reply);

    EXPECT_EQ(ret, E_OK);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopDownloadFileTest1, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
    int32_t ret = service.HandleStopDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncServiceStubTest, HandleStopDownloadFileTest2, TestSize.Level1)
{
    MockService service;
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = service.HandleStopDownloadFile(data, reply);

    EXPECT_EQ(ret, E_PERMISSION_SYSTEM);
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
