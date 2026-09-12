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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_disk_progress_callback_stub.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_manager_impl.h"
#include "iremote_stub.h"
#include "service_proxy.h"

namespace OHOS::FileManagement::CloudDiskService {
sptr<ICloudDiskService> ServiceProxy::GetInstance()
{
    return serviceProxy_;
}

void ServiceProxy::InvalidInstance()
{
    serviceProxy_ = nullptr;
}

namespace Test {
using namespace testing;
using namespace testing::ext;

class MockCloudDiskRemote : public IRemoteStub<ICloudDiskService> {
public:
    MOCK_METHOD(ErrCode,
                RegisterSyncFolderChangesInner,
                (const std::string &, const sptr<IRemoteObject> &),
                (override));
    MOCK_METHOD(ErrCode, UnregisterSyncFolderChangesInner, (const std::string &), (override));
    MOCK_METHOD(ErrCode, RegisterCallbackTableInner, (const std::string &, const sptr<IRemoteObject> &), (override));
    MOCK_METHOD(ErrCode, UnregisterCallbackTableInner, (const std::string &), (override));
    MOCK_METHOD(ErrCode,
                GetSyncFolderChangesInner,
                (const std::string &, uint64_t, uint64_t, ChangesResult &),
                (override));
    MOCK_METHOD(ErrCode,
                SetFileSyncStatesInner,
                (const std::string &, const std::vector<FileSyncState> &, std::vector<FailedList> &),
                (override));
    MOCK_METHOD(ErrCode,
                GetFileSyncStatesInner,
                (const std::string &, const std::vector<std::string> &, std::vector<ResultList> &),
                (override));
    MOCK_METHOD(ErrCode,
                CreatePlaceholderFileInner,
                (const std::string &, const std::string &, const PlaceholderInfo &, const PlaceholderCustomInfo &),
                (override));
    MOCK_METHOD(ErrCode, IsPlaceholderFileInner, (const std::string &, const std::string &, bool &), (override));
    MOCK_METHOD(ErrCode, RegisterSyncFolderInner, (int32_t, const std::string &, const std::string &), (override));
    MOCK_METHOD(ErrCode, UnregisterSyncFolderInner, (int32_t, const std::string &, const std::string &), (override));
    MOCK_METHOD(ErrCode, UnregisterForSaInner, (const std::string &), (override));
    MOCK_METHOD(ErrCode, ConvertPlaceholderToFileInner, (const std::string &, const std::string &), (override));
    MOCK_METHOD(ErrCode, MarkFileAsPlaceholderInner, (const std::string &, const std::string &), (override));
    MOCK_METHOD(ErrCode, UnmarkPlaceholderFileInner, (const std::string &, const std::string &), (override));
    MOCK_METHOD(ErrCode, StartHydrationInner, (const std::string &, const std::string &, int32_t), (override));
    MOCK_METHOD(ErrCode, CancelHydrationInner, (const std::string &, const std::string &), (override));
    MOCK_METHOD(ErrCode, ExecuteInner, (const CallbackExecuteRequest &), (override));
    MOCK_METHOD(ErrCode, StartHydrationByPathInner, (const std::string &, int32_t, int32_t), (override));
    MOCK_METHOD(ErrCode, DehydrateFileByPathInner, (const std::string &), (override));
    MOCK_METHOD(ErrCode, RegisterProgressCallbackInner, (const sptr<IRemoteObject> &), (override));
    MOCK_METHOD(ErrCode, UnregisterProgressCallbackInner, (), (override));
    MOCK_METHOD(ErrCode, DehydrateInner, (const std::string &, const std::string &), (override));
    MOCK_METHOD(ErrCode,
                UpdatePlaceholderInner,
                (const std::string &, const std::string &, const PlaceholderInfo &, const PlaceholderCustomInfo &),
                (override));
    MOCK_METHOD(ErrCode,
                GetPlaceholderCustomInfoInner,
                (const std::string &, const std::string &, PlaceholderCustomInfo &),
                (override));
    MOCK_METHOD(ErrCode, GetPlaceholderStateInner, (const std::string &, const std::string &, int32_t &), (override));
};

class RecordingCallbackTable final : public CloudDiskServiceCallbackTable {
public:
    void OnCallback(const CloudDiskCallbackReqHead &, CloudDiskCallbackContext &) override {}

    void OnDeathRecipient() override
    {
        ++deathCount;
    }

    int32_t deathCount = 0;
};

class RecordingProgressCallback final : public CloudDiskProgressCallbackStub {
public:
    void OnProgress(const HydrateProgress &) override {}
};

class RecordingLegacyCallback final : public CloudDiskServiceCallback {
public:
    void OnChangeData(const std::string &, const std::vector<ChangeData> &) override {}

    void OnDeathRecipient() override
    {
        ++deathCount;
    }

    int32_t deathCount = 0;
};

class ControlledManagerRemote final : public IRemoteObject {
public:
    ControlledManagerRemote() : IRemoteObject(u"controlled_manager_proxy") {}

    int32_t GetObjectRefCount() override
    {
        return 1;
    }

    int SendRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return E_OK;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        ++addCount;
        return recipient != nullptr && addResult;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return recipient != nullptr;
    }

    int Dump(int, const std::vector<std::u16string> &) override
    {
        return E_OK;
    }

    bool addResult = false;
    int32_t addCount = 0;
};

class CloudDiskServiceManagerImplTest : public testing::Test {
public:
    void SetUp() override
    {
        remote_ = sptr(new NiceMock<MockCloudDiskRemote>());
        ResetManager();
        ServiceProxy::serviceProxy_ = remote_;
    }

    void TearDown() override
    {
        Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
        ResetManager();
        ServiceProxy::serviceProxy_ = nullptr;
        remote_ = nullptr;
    }

    void ResetManager()
    {
        auto &manager = CloudDiskServiceManagerImpl::GetInstance();
        std::lock_guard<std::mutex> callbackLock(manager.callbackMutex_);
        std::lock_guard<std::mutex> progressLock(manager.progressMutex_);
        manager.callback_ = nullptr;
        manager.callbackTables_.clear();
        manager.callbackTableClients_.clear();
        manager.progressClient_ = nullptr;
        manager.deathRecipient_ = nullptr;
        manager.isFirstCall_.clear();
    }

    void RestoreRemote()
    {
        ServiceProxy::serviceProxy_ = remote_;
    }

    sptr<NiceMock<MockCloudDiskRemote>> remote_;
};

/**
 * @tc.name: RegisterCallbackTable_001
 * @tc.desc: Cover invalid callback, missing proxy, proxy error, and successful retention.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, RegisterCallbackTable_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_EQ(manager.RegisterCallbackTable("/sync", nullptr), E_INVALID_ARG);
    ServiceProxy::serviceProxy_ = nullptr;
    auto callback = std::make_shared<RecordingCallbackTable>();
    EXPECT_EQ(manager.RegisterCallbackTable("/sync", callback), E_IPC_FAILED);

    RestoreRemote();
    EXPECT_CALL(*remote_, RegisterCallbackTableInner("/sync", _)).WillOnce(Return(E_IPC_FAILED));
    EXPECT_EQ(manager.RegisterCallbackTable("/sync", callback), E_IPC_FAILED);
    EXPECT_TRUE(manager.callbackTables_.empty());

    Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
    EXPECT_CALL(*remote_, RegisterCallbackTableInner("/sync", _)).WillOnce(Return(E_OK));
    EXPECT_EQ(manager.RegisterCallbackTable("/sync", callback), E_OK);
    EXPECT_EQ(manager.callbackTables_.at("/sync"), callback);
    EXPECT_TRUE(manager.callbackTableClients_.at("/sync")->active_.load());
#else
    EXPECT_EQ(manager.RegisterCallbackTable("/sync", nullptr), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: UnregisterCallbackTable_001
 * @tc.desc: Reactivate on service failure, erase on success, and reject a missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, UnregisterCallbackTable_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto callback = std::make_shared<RecordingCallbackTable>();
    EXPECT_CALL(*remote_, RegisterCallbackTableInner("/sync", _)).WillOnce(Return(E_OK));
    ASSERT_EQ(manager.RegisterCallbackTable("/sync", callback), E_OK);

    Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
    EXPECT_CALL(*remote_, UnregisterCallbackTableInner("/sync")).WillOnce(Return(E_IPC_FAILED));
    EXPECT_EQ(manager.UnregisterCallbackTable("/sync"), E_IPC_FAILED);
    EXPECT_TRUE(manager.callbackTableClients_.at("/sync")->active_.load());

    Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
    EXPECT_CALL(*remote_, UnregisterCallbackTableInner("/sync")).WillOnce(Return(E_OK));
    EXPECT_EQ(manager.UnregisterCallbackTable("/sync"), E_OK);
    EXPECT_TRUE(manager.callbackTables_.empty());
    EXPECT_TRUE(manager.callbackTableClients_.empty());

    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.UnregisterCallbackTable("/sync"), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.UnregisterCallbackTable("/sync"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: CreatePlaceholderFile_001
 * @tc.desc: Forward metadata and custom information and handle a missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, CreatePlaceholderFile_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
    PlaceholderInfo info;
    info.logicalSize = 10;
    info.atimeMs = 20;
    info.mtimeMs = 30;
    PlaceholderCustomInfo customInfo;
    customInfo.data = {1, 2};
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, CreatePlaceholderFileInner("/sync", "file", _, _))
        .WillOnce(Invoke([&](const std::string &, const std::string &, const PlaceholderInfo &actualInfo,
                             const PlaceholderCustomInfo &actualCustom) {
            EXPECT_EQ(actualInfo.logicalSize, info.logicalSize);
            EXPECT_EQ(actualCustom.data, customInfo.data);
            return E_OK;
        }));
    EXPECT_EQ(manager.CreatePlaceholderFile("/sync", "file", info, customInfo), E_OK);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.CreatePlaceholderFile("/sync", "file", info, customInfo), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.CreatePlaceholderFile("/sync", "file", info, customInfo), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: IsPlaceholderFile_001
 * @tc.desc: Preserve the service result and cover a missing service proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, IsPlaceholderFile_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
    bool isPlaceholder = false;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, IsPlaceholderFileInner("/sync", "file", _))
        .WillOnce(DoAll(SetArgReferee<2>(true), Return(E_OK)))
        .WillOnce(Return(E_FILE_NOT_EXIST));
    EXPECT_EQ(manager.IsPlaceholderFile("/sync", "file", isPlaceholder), E_OK);
    EXPECT_TRUE(isPlaceholder);
    EXPECT_EQ(manager.IsPlaceholderFile("/sync", "file", isPlaceholder), E_FILE_NOT_EXIST);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.IsPlaceholderFile("/sync", "file", isPlaceholder), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.IsPlaceholderFile("/sync", "file", isPlaceholder), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: MarkFileAsPlaceholder_001
 * @tc.desc: Forward mark requests and handle a missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, MarkFileAsPlaceholder_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, MarkFileAsPlaceholderInner("/sync", "file"))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_IS_A_PLACEHOLDER));
    EXPECT_EQ(manager.MarkFileAsPlaceholder("/sync", "file"), E_OK);
    EXPECT_EQ(manager.MarkFileAsPlaceholder("/sync", "file"), E_IS_A_PLACEHOLDER);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.MarkFileAsPlaceholder("/sync", "file"), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.MarkFileAsPlaceholder("/sync", "file"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: UnmarkPlaceholderFile_001
 * @tc.desc: Forward unmark requests and handle a missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, UnmarkPlaceholderFile_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, UnmarkPlaceholderFileInner("/sync", "file"))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_NOT_A_PLACEHOLDER));
    EXPECT_EQ(manager.UnmarkPlaceholderFile("/sync", "file"), E_OK);
    EXPECT_EQ(manager.UnmarkPlaceholderFile("/sync", "file"), E_NOT_A_PLACEHOLDER);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.UnmarkPlaceholderFile("/sync", "file"), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.UnmarkPlaceholderFile("/sync", "file"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: StartHydration_001
 * @tc.desc: Forward hydration priority and cover success, service failure, and missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, StartHydration_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, StartHydrationInner("/sync", "file", CLOUD_DISK_HYDRATE_PRIORITY_HIGH))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_TRY_AGAIN));
    EXPECT_EQ(manager.StartHydration("/sync", "file", CLOUD_DISK_HYDRATE_PRIORITY_HIGH), E_OK);
    EXPECT_EQ(manager.StartHydration("/sync", "file", CLOUD_DISK_HYDRATE_PRIORITY_HIGH), E_TRY_AGAIN);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.StartHydration("/sync", "file", CLOUD_DISK_HYDRATE_PRIORITY_HIGH), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.StartHydration("/sync", "file", CLOUD_DISK_HYDRATE_PRIORITY_HIGH), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: CancelHydration_001
 * @tc.desc: Forward cancellation and cover success, service failure, and missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, CancelHydration_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, CancelHydrationInner("/sync", "file"))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_NO_HYDRATION_IN_PROGRESS));
    EXPECT_EQ(manager.CancelHydration("/sync", "file"), E_OK);
    EXPECT_EQ(manager.CancelHydration("/sync", "file"), E_NO_HYDRATION_IN_PROGRESS);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.CancelHydration("/sync", "file"), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.CancelHydration("/sync", "file"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: Execute_001
 * @tc.desc: Forward Execute data and cover success, service failure, and missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, Execute_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
    CallbackExecuteRequest request;
    request.reqKey = {1};
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, ExecuteInner(_)).WillOnce(Return(E_OK)).WillOnce(Return(E_CANCELLED));
    EXPECT_EQ(manager.Execute(request), E_OK);
    EXPECT_EQ(manager.Execute(request), E_CANCELLED);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.Execute(request), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.Execute(request), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: DehydrateFile_001
 * @tc.desc: Forward dehydration and handle a missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, DehydrateFile_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, DehydrateInner("/sync", "file")).WillOnce(Return(E_OK)).WillOnce(Return(E_DEHYDRATE_DENIED));
    EXPECT_EQ(manager.DehydrateFile("/sync", "file"), E_OK);
    EXPECT_EQ(manager.DehydrateFile("/sync", "file"), E_DEHYDRATE_DENIED);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.DehydrateFile("/sync", "file"), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.DehydrateFile("/sync", "file"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: UpdatePlaceholder_001
 * @tc.desc: Forward placeholder metadata and custom information and handle a missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, UpdatePlaceholder_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
    PlaceholderInfo info;
    info.logicalSize = 10;
    info.atimeMs = 20;
    info.mtimeMs = 30;
    PlaceholderCustomInfo customInfo;
    customInfo.data = {1, 2};
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, UpdatePlaceholderInner("/sync", "file", _, _))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_FILE_NOT_EXIST));
    EXPECT_EQ(manager.UpdatePlaceholder("/sync", "file", info, customInfo), E_OK);
    EXPECT_EQ(manager.UpdatePlaceholder("/sync", "file", info, customInfo), E_FILE_NOT_EXIST);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.UpdatePlaceholder("/sync", "file", info, customInfo), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.UpdatePlaceholder("/sync", "file", info, customInfo), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: GetPlaceholderCustomInfo_001
 * @tc.desc: Forward custom-information output and handle a missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, GetPlaceholderCustomInfo_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
    PlaceholderCustomInfo customInfo;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, GetPlaceholderCustomInfoInner("/sync", "file", _))
        .WillOnce(Invoke([](const std::string &, const std::string &, PlaceholderCustomInfo &output) {
            output.data = {1, 2};
            return E_OK;
        }))
        .WillOnce(Return(E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND));
    EXPECT_EQ(manager.GetPlaceholderCustomInfo("/sync", "file", customInfo), E_OK);
    EXPECT_THAT(customInfo.data, ElementsAre(1, 2));
    EXPECT_EQ(manager.GetPlaceholderCustomInfo("/sync", "file", customInfo), E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.GetPlaceholderCustomInfo("/sync", "file", customInfo), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.GetPlaceholderCustomInfo("/sync", "file", customInfo), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: StartHydrationByPath_001
 * @tc.desc: Forward system-accessor hydration and cover service failure and missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, StartHydrationByPath_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, StartHydrationByPathInner("/path", 0, 2))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_TRY_AGAIN));
    EXPECT_EQ(manager.StartHydrationByPath("/path", 0, 2), E_OK);
    EXPECT_EQ(manager.StartHydrationByPath("/path", 0, 2), E_TRY_AGAIN);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.StartHydrationByPath("/path", 0, 2), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.StartHydrationByPath("/path", 0, 2), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: DehydrateFileByPath_001
 * @tc.desc: Forward system-accessor dehydration and cover service failure and missing proxy.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, DehydrateFileByPath_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*remote_, DehydrateFileByPathInner("/path")).WillOnce(Return(E_OK)).WillOnce(Return(E_TRY_AGAIN));
    EXPECT_EQ(manager.DehydrateFileByPath("/path"), E_OK);
    EXPECT_EQ(manager.DehydrateFileByPath("/path"), E_TRY_AGAIN);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.DehydrateFileByPath("/path"), E_IPC_FAILED);
#else
    EXPECT_EQ(manager.DehydrateFileByPath("/path"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: RegisterProgressCallback_001
 * @tc.desc: Cover validation, multiplexing, duplicate registration, and failed-add rollback.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, RegisterProgressCallback_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto first = sptr(new RecordingProgressCallback());
    auto second = sptr(new RecordingProgressCallback());
    EXPECT_EQ(manager.RegisterProgressCallback(nullptr), E_INVALID_ARG);
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.RegisterProgressCallback(first), E_IPC_FAILED);

    RestoreRemote();
    EXPECT_CALL(*remote_, RegisterProgressCallbackInner(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(manager.RegisterProgressCallback(first), E_OK);
    ASSERT_NE(manager.progressClient_, nullptr);
    EXPECT_EQ(manager.progressClient_->callbacks_.size(), 1U);

    Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
    EXPECT_CALL(*remote_, RegisterProgressCallbackInner(_)).WillOnce(Return(E_IPC_FAILED));
    EXPECT_EQ(manager.RegisterProgressCallback(first), E_IPC_FAILED);
    EXPECT_EQ(manager.progressClient_->callbacks_.size(), 1U);

    Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
    EXPECT_CALL(*remote_, RegisterProgressCallbackInner(_)).WillOnce(Return(E_IPC_FAILED));
    EXPECT_EQ(manager.RegisterProgressCallback(second), E_IPC_FAILED);
    EXPECT_EQ(manager.progressClient_->callbacks_.size(), 1U);
#else
    EXPECT_EQ(manager.RegisterProgressCallback(nullptr), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: UnregisterProgressCallback_001
 * @tc.desc: Cover empty, non-last, missing-proxy, service error pass-through, failure, and success branches.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, UnregisterProgressCallback_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto first = sptr(new RecordingProgressCallback());
    auto second = sptr(new RecordingProgressCallback());
    EXPECT_EQ(manager.UnregisterProgressCallback(first), E_OK);
    manager.progressClient_ = sptr(new CloudDiskProgressCallbackClient());
    ASSERT_TRUE(manager.progressClient_->Add(first));
    ASSERT_TRUE(manager.progressClient_->Add(second));
    EXPECT_EQ(manager.UnregisterProgressCallback(first), E_OK);
    EXPECT_EQ(manager.progressClient_->callbacks_.size(), 1U);

    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.UnregisterProgressCallback(second), E_IPC_FAILED);
    RestoreRemote();
    ASSERT_TRUE(manager.progressClient_->Add(second));
    EXPECT_CALL(*remote_, UnregisterProgressCallbackInner()).WillOnce(Return(E_CALLBACK_NOT_REGISTERED));
    EXPECT_EQ(manager.UnregisterProgressCallback(second), E_CALLBACK_NOT_REGISTERED);
    EXPECT_TRUE(manager.progressClient_->callbacks_.empty());

    Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
    ASSERT_TRUE(manager.progressClient_->Add(second));
    EXPECT_CALL(*remote_, UnregisterProgressCallbackInner()).WillOnce(Return(E_IPC_FAILED));
    EXPECT_EQ(manager.UnregisterProgressCallback(second), E_IPC_FAILED);

    Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
    ASSERT_TRUE(manager.progressClient_->Add(second));
    EXPECT_CALL(*remote_, UnregisterProgressCallbackInner()).WillOnce(Return(E_OK));
    EXPECT_EQ(manager.UnregisterProgressCallback(second), E_OK);
#else
    EXPECT_EQ(manager.UnregisterProgressCallback(nullptr), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: GetPlaceholderState_001
 * @tc.desc: Initialize output deterministically and cover missing proxy, success, and service failure.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, GetPlaceholderState_001, TestSize.Level1)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
    int32_t state = 3;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    ServiceProxy::serviceProxy_ = nullptr;
    EXPECT_EQ(manager.GetPlaceholderState("/sync", "file", state), E_IPC_FAILED);
    EXPECT_EQ(state, 0);

    RestoreRemote();
    EXPECT_CALL(*remote_, GetPlaceholderStateInner("/sync", "file", _))
        .WillOnce(DoAll(SetArgReferee<2>(3), Return(E_OK)));
    EXPECT_EQ(manager.GetPlaceholderState("/sync", "file", state), E_OK);
    EXPECT_EQ(state, 3);

    Mock::VerifyAndClearExpectations(remote_.GetRefPtr());
    EXPECT_CALL(*remote_, GetPlaceholderStateInner("/sync", "file", _))
        .WillOnce(DoAll(SetArgReferee<2>(3), Return(E_INVALID_PLACEHOLDER_STATE)));
    EXPECT_EQ(manager.GetPlaceholderState("/sync", "file", state), E_INVALID_PLACEHOLDER_STATE);
    EXPECT_EQ(state, 0);
#else
    EXPECT_EQ(manager.GetPlaceholderState("/sync", "file", state), E_NOT_SUPPORTED);
    EXPECT_EQ(state, 0);
#endif
}

/**
 * @tc.name: SetDeathRecipient_001
 * @tc.desc: Deactivate retained callback tables and notify owners when the service dies.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, SetDeathRecipient_001, TestSize.Level2)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto callback = std::make_shared<RecordingCallbackTable>();
    EXPECT_CALL(*remote_, RegisterCallbackTableInner("/sync", _)).WillOnce(Return(E_OK));
    ASSERT_EQ(manager.RegisterCallbackTable("/sync", callback), E_OK);
    ASSERT_NE(manager.deathRecipient_, nullptr);
    auto client = manager.callbackTableClients_.at("/sync");
    manager.deathRecipient_->OnRemoteDied(wptr<IRemoteObject>(remote_->AsObject()));
    EXPECT_EQ(callback->deathCount, 1);
    EXPECT_FALSE(client->active_.load());
    EXPECT_EQ(ServiceProxy::serviceProxy_, nullptr);
#else
    SUCCEED();
#endif
}

/**
 * @tc.name: SetDeathRecipient_002
 * @tc.desc: Retry failed registration, avoid duplicate registration, and notify legacy callbacks on death.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceManagerImplTest, SetDeathRecipient_002, TestSize.Level2)
{
    auto &manager = CloudDiskServiceManagerImpl::GetInstance();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto remote = sptr(new ControlledManagerRemote());
    manager.SetDeathRecipient(remote);
    EXPECT_EQ(remote->addCount, 1);
    EXPECT_FALSE(manager.isFirstCall_.test_and_set());
    manager.isFirstCall_.clear();

    remote->addResult = true;
    manager.SetDeathRecipient(remote);
    EXPECT_EQ(remote->addCount, 2);
    EXPECT_TRUE(manager.isFirstCall_.test_and_set());

    manager.SetDeathRecipient(remote);
    EXPECT_EQ(remote->addCount, 2);

    auto callback = std::make_shared<RecordingLegacyCallback>();
    {
        std::lock_guard<std::mutex> lock(manager.callbackMutex_);
        manager.callback_ = callback;
        manager.callbackTables_["null"] = nullptr;
    }
    ASSERT_NE(manager.deathRecipient_, nullptr);
    manager.deathRecipient_->OnRemoteDied(wptr<IRemoteObject>(remote));
    EXPECT_EQ(callback->deathCount, 1);
    EXPECT_FALSE(manager.isFirstCall_.test_and_set());
    manager.isFirstCall_.clear();
    EXPECT_EQ(ServiceProxy::serviceProxy_, nullptr);
#else
    SUCCEED();
#endif
}
} // namespace Test
} // namespace OHOS::FileManagement::CloudDiskService
