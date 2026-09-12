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

#include <gtest/gtest.h>

#include "cloud_disk_service_error.h"
#include "iremote_stub.h"
#include "placeholder_callback_manager.h"
#include "placeholder_task_manager.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;

namespace {
constexpr uint32_t TEST_SYNC_FOLDER_INDEX = 100;
const std::string TEST_BUNDLE_NAME = "com.example.cloud.disk";

class CallbackTableStub final : public IRemoteStub<ICloudDiskServiceCallbackTable> {
public:
    void OnCallback(const CloudDiskCallbackReqHead &reqHead, CloudDiskCallbackContext &reqContext) override
    {
        ++callbackCount;
        callbackType = reqHead.callbackType;
        switch (reqHead.callbackType) {
            case CloudDiskCallbackType::FETCH_DATA:
                if (reqContext.fetchData != nullptr) {
                    filePath = std::string(reqContext.fetchData->filePath.value, reqContext.fetchData->filePath.length);
                    priority = reqContext.fetchData->priority;
                }
                break;
            case CloudDiskCallbackType::CANCEL_FETCH_DATA:
                if (reqContext.cancelFetchData != nullptr) {
                    filePath = std::string(reqContext.cancelFetchData->value, reqContext.cancelFetchData->length);
                }
                break;
            case CloudDiskCallbackType::DEHYDRATE:
                if (reqContext.dehydrateData != nullptr) {
                    filePath = std::string(reqContext.dehydrateData->filePath.value,
                                           reqContext.dehydrateData->filePath.length);
                    reqContext.dehydrateData->allow = allowDehydrate;
                }
                break;
        }
    }

    int32_t OnRemoteRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return E_OK;
    }

    uint32_t callbackCount = 0;
    CloudDiskCallbackType callbackType = CloudDiskCallbackType::FETCH_DATA;
    std::string filePath;
    CloudDiskHydratePriority priority = CLOUD_DISK_HYDRATE_PRIORITY_LOW;
    bool allowDehydrate = false;
};

class ErrorCallbackTableStub final : public IRemoteStub<ICloudDiskServiceCallbackTable> {
public:
    explicit ErrorCallbackTableStub(int32_t result) : result_(result) {}

    void OnCallback(const CloudDiskCallbackReqHead &, CloudDiskCallbackContext &) override {}

    int32_t SendCallback(const CloudDiskCallbackReqHead &, CloudDiskCallbackContext &) override
    {
        ++sendCount_;
        return result_;
    }

    int32_t OnRemoteRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return E_OK;
    }

    int32_t result_;
    uint32_t sendCount_ = 0;
};

class NullRemoteCallbackTable final : public ICloudDiskServiceCallbackTable {
public:
    void OnCallback(const CloudDiskCallbackReqHead &, CloudDiskCallbackContext &) override {}

    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

class ControlledCallbackRemote final : public IRemoteObject {
public:
    ControlledCallbackRemote() : IRemoteObject(u"controlled_callback_proxy") {}

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
        ++removeCount;
        return recipient != nullptr;
    }

    int Dump(int, const std::vector<std::u16string> &) override
    {
        return E_OK;
    }

    bool addResult = false;
    int32_t addCount = 0;
    int32_t removeCount = 0;
};

class ProxyBackedCallbackTable final : public ICloudDiskServiceCallbackTable {
public:
    explicit ProxyBackedCallbackTable(const sptr<IRemoteObject> &remote) : remote_(remote) {}

    void OnCallback(const CloudDiskCallbackReqHead &, CloudDiskCallbackContext &) override {}

    sptr<IRemoteObject> AsObject() override
    {
        return remote_;
    }

private:
    sptr<IRemoteObject> remote_;
};
} // namespace

class PlaceholderCallbackManagerTest : public testing::Test {
public:
    void TearDown() override
    {
        PlaceholderTaskManager::GetInstance().StopScheduler();
        PlaceholderCallbackManager::GetInstance().ClearAll();
    }
};

/**
 * @tc.name: RegisterDispatchAndUnregisterTest001
 * @tc.desc: Verify registration, duplicate detection, dispatch, and unregister error codes.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, RegisterDispatchAndUnregisterTest001, TestSize.Level1)
{
    auto callback = sptr(new CallbackTableStub());
    auto &manager = PlaceholderCallbackManager::GetInstance();
    EXPECT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);
    EXPECT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback),
              E_CALLBACK_ALREADY_REGISTERED);

    std::string path = "dir/file.txt";
    CloudDiskPathInfo pathInfo{path.data(), path.length()};
    CloudDiskCallbackReqHead reqHead{};
    EXPECT_EQ(manager.DispatchFetchData(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo), E_OK);
    EXPECT_EQ(callback->callbackCount, 1U);
    EXPECT_EQ(callback->callbackType, CloudDiskCallbackType::FETCH_DATA);
    EXPECT_EQ(callback->filePath, path);
    EXPECT_EQ(callback->priority, CLOUD_DISK_HYDRATE_PRIORITY_NORMAL);

    EXPECT_EQ(manager.DispatchFetchData(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo,
                                        CLOUD_DISK_HYDRATE_PRIORITY_HIGH),
              E_OK);
    EXPECT_EQ(callback->priority, CLOUD_DISK_HYDRATE_PRIORITY_HIGH);

    PlaceholderTaskManager::RequestKey taskKey;
    ASSERT_EQ(PlaceholderTaskManager::GetInstance().CreateHydrateTask(
                  "/storage/Users/currentUser/sync", "dir/file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                  CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, UniqueFd(dup(STDOUT_FILENO)), taskKey),
              E_OK);
    EXPECT_EQ(manager.UnregisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX), E_OK);
    PlaceholderTaskState taskState;
    EXPECT_FALSE(PlaceholderTaskManager::GetInstance().GetTaskState(taskKey, taskState));
    CallbackExecuteRequest executeRequest;
    executeRequest.reqKey = taskKey;
    executeRequest.syncFolder = "/storage/Users/currentUser/sync";
    executeRequest.filePath = "dir/file.txt";
    executeRequest.isComplete = true;
    EXPECT_EQ(PlaceholderTaskManager::GetInstance().Execute(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, executeRequest),
              E_NO_HYDRATION_IN_PROGRESS);
    EXPECT_EQ(manager.UnregisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX), E_CALLBACK_NOT_REGISTERED);
}

/**
 * @tc.name: DispatchWithoutRegistrationTest001
 * @tc.desc: Verify dispatch fails with the callback-specific not-registered error.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, DispatchWithoutRegistrationTest001, TestSize.Level1)
{
    std::string path = "dir/file.txt";
    CloudDiskPathInfo pathInfo{path.data(), path.length()};
    CloudDiskCallbackReqHead reqHead{};
    EXPECT_EQ(PlaceholderCallbackManager::GetInstance().DispatchFetchData(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                          reqHead, pathInfo),
              E_CALLBACK_NOT_REGISTERED);
}

/**
 * @tc.name: DispatchDehydrateAuthorizationTest001
 * @tc.desc: Verify dehydrate callbacks return the application's synchronous authorization decision.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, DispatchDehydrateAuthorizationTest001, TestSize.Level1)
{
    auto callback = sptr(new CallbackTableStub());
    auto &manager = PlaceholderCallbackManager::GetInstance();
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);

    std::string path = "dir/file.txt";
    CloudDiskPathInfo pathInfo{path.data(), path.length()};
    CloudDiskCallbackReqHead reqHead{};
    callback->allowDehydrate = false;
    EXPECT_EQ(manager.DispatchDehydrate(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo),
              E_DEHYDRATE_DENIED);
    EXPECT_EQ(callback->callbackType, CloudDiskCallbackType::DEHYDRATE);
    EXPECT_EQ(callback->filePath, path);

    callback->allowDehydrate = true;
    EXPECT_EQ(manager.DispatchDehydrate(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo), E_OK);
}
/**
 * @tc.name: OnRemoteDied_001
 * @tc.desc: Death clears all roots associated with the remote and cancels only their pending or active tasks.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, OnRemoteDied_001, TestSize.Level2)
{
    auto callback = sptr(new CallbackTableStub());
    auto &callbacks = PlaceholderCallbackManager::GetInstance();
    auto &tasks = PlaceholderTaskManager::GetInstance();
    ASSERT_EQ(callbacks.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);
    ASSERT_EQ(callbacks.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1, callback), E_OK);
    std::vector<uint8_t> firstKey;
    std::vector<uint8_t> secondKey;
    std::vector<uint8_t> otherKey;
    ASSERT_EQ(tasks.CreateHydrateTask("sync", "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                      CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, UniqueFd(dup(STDOUT_FILENO)), firstKey),
              E_OK);
    ASSERT_EQ(tasks.CreateHydrateTask("sync", "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1,
                                      CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, UniqueFd(dup(STDOUT_FILENO)), secondKey),
              E_OK);
    ASSERT_EQ(tasks.CreateHydrateTask("sync", "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 2,
                                      CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, UniqueFd(dup(STDOUT_FILENO)), otherKey),
              E_OK);
    tasks.taskMap_.at(secondKey)->state = PlaceholderTaskState::IN_PROGRESS;
    const void *remoteKey = callback->AsObject().GetRefPtr();
    callbacks.OnRemoteDied(remoteKey);
    EXPECT_FALSE(callbacks.IsCallbackRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX));
    EXPECT_FALSE(callbacks.IsCallbackRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1));
    EXPECT_EQ(callbacks.remoteCallbackMap_.count(remoteKey), 0U);
    EXPECT_EQ(callbacks.deathRecipientMap_.count(remoteKey), 0U);
    PlaceholderTaskState state;
    EXPECT_FALSE(tasks.GetTaskState(firstKey, state));
    EXPECT_FALSE(tasks.GetTaskState(secondKey, state));
    ASSERT_TRUE(tasks.GetTaskState(otherKey, state));
    EXPECT_EQ(state, PlaceholderTaskState::PENDING);
}

/**
 * @tc.name: InvalidArguments_001
 * @tc.desc: Verify registration, unregistration and dispatch reject invalid callback-table identities.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, InvalidArguments_001, TestSize.Level2)
{
    auto &manager = PlaceholderCallbackManager::GetInstance();
    auto callback = sptr(new CallbackTableStub());
    sptr<ICloudDiskServiceCallbackTable> nullCallback;
    EXPECT_EQ(manager.RegisterCallbackTable("", TEST_SYNC_FOLDER_INDEX, callback), E_INVALID_ARG);
    EXPECT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, nullCallback), E_INVALID_ARG);
    EXPECT_EQ(manager.UnregisterCallbackTable("", TEST_SYNC_FOLDER_INDEX), E_INVALID_ARG);

    std::string path = "file.txt";
    CloudDiskPathInfo pathInfo{path.data(), path.length()};
    CloudDiskCallbackReqHead reqHead{};
    EXPECT_EQ(manager.DispatchFetchData("", TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo), E_INVALID_ARG);
    manager.ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX);
    manager.OnRemoteDied(nullptr);
    EXPECT_TRUE(manager.callbackMap_.empty());
}

/**
 * @tc.name: DispatchCancelAndSharedRemoteCleanup_001
 * @tc.desc: Verify cancel callbacks preserve their context and shared-remote cleanup is correct.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, DispatchCancelAndSharedRemoteCleanup_001, TestSize.Level1)
{
    auto &manager = PlaceholderCallbackManager::GetInstance();
    auto callback = sptr(new CallbackTableStub());
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1, callback), E_OK);
    const void *remoteKey = callback->AsObject().GetRefPtr();

    std::string path = "dir/file.txt";
    CloudDiskPathInfo pathInfo{path.data(), path.length()};
    CloudDiskCallbackReqHead reqHead{};
    EXPECT_EQ(manager.DispatchCancelFetchData(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo), E_OK);
    EXPECT_EQ(callback->callbackType, CloudDiskCallbackType::CANCEL_FETCH_DATA);
    EXPECT_EQ(callback->filePath, path);

    manager.ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX);
    EXPECT_FALSE(manager.IsCallbackRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX));
    EXPECT_TRUE(manager.IsCallbackRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1));
    EXPECT_EQ(manager.remoteCallbackMap_.at(remoteKey).size(), 1U);
    manager.ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1);
    EXPECT_EQ(manager.remoteCallbackMap_.count(remoteKey), 0U);
    EXPECT_EQ(manager.deathRecipientMap_.count(remoteKey), 0U);
}

/**
 * @tc.name: RegisterCallbackTable_001
 * @tc.desc: Reject a callback table whose remote object cannot be obtained.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, RegisterCallbackTable_001, TestSize.Level2)
{
    auto callback = sptr<ICloudDiskServiceCallbackTable>(new NullRemoteCallbackTable());
    EXPECT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                                                              callback),
              E_INVALID_ARG);
}

/**
 * @tc.name: RegisterCallbackTable_002
 * @tc.desc: Retry death-recipient registration after the remote rejects the first attempt.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, RegisterCallbackTable_002, TestSize.Level2)
{
    auto &manager = PlaceholderCallbackManager::GetInstance();
    auto remote = sptr(new ControlledCallbackRemote());
    auto callback = sptr<ICloudDiskServiceCallbackTable>(new ProxyBackedCallbackTable(remote));
    const void *remoteKey = remote.GetRefPtr();

    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);
    EXPECT_EQ(remote->addCount, 1);
    EXPECT_EQ(manager.deathRecipientMap_.count(remoteKey), 0U);

    remote->addResult = true;
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1, callback), E_OK);
    EXPECT_EQ(remote->addCount, 2);
    EXPECT_EQ(manager.deathRecipientMap_.count(remoteKey), 1U);

    manager.ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX);
    EXPECT_EQ(remote->removeCount, 0);
    manager.ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1);
    EXPECT_EQ(remote->removeCount, 1);
}

/**
 * @tc.name: GetCallback_001
 * @tc.desc: Return the exact registered callback and nullptr after it is removed.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, GetCallback_001, TestSize.Level1)
{
    auto &manager = PlaceholderCallbackManager::GetInstance();
    auto callback = sptr(new CallbackTableStub());
    sptr<ICloudDiskServiceCallbackTable> callbackBase = callback;
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);
    EXPECT_EQ(manager.GetCallback(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX), callbackBase);
    EXPECT_EQ(manager.GetCallback(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1), nullptr);

    ASSERT_EQ(manager.UnregisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX), E_OK);
    EXPECT_EQ(manager.GetCallback(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX), nullptr);
}

/**
 * @tc.name: RunIfRegistered_001
 * @tc.desc: Cover argument validation, missing registration and operation result forwarding.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, RunIfRegistered_001, TestSize.Level1)
{
    auto &manager = PlaceholderCallbackManager::GetInstance();
    std::function<int32_t()> emptyOperation;
    EXPECT_EQ(manager.RunIfRegistered("", TEST_SYNC_FOLDER_INDEX, []() { return E_OK; }), E_INVALID_ARG);
    EXPECT_EQ(manager.RunIfRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, emptyOperation), E_INVALID_ARG);
    EXPECT_EQ(manager.RunIfRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, []() { return E_OK; }),
              E_CALLBACK_NOT_REGISTERED);

    auto callback = sptr(new CallbackTableStub());
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);
    uint32_t callCount = 0;
    EXPECT_EQ(manager.RunIfRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX,
                                      [&callCount]() {
                                          ++callCount;
                                          return E_TRY_AGAIN;
                                      }),
              E_TRY_AGAIN);
    EXPECT_EQ(callCount, 1U);
}

/**
 * @tc.name: DispatchCallbackFailure_001
 * @tc.desc: Propagate callback transport failures for each callback request type.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, DispatchCallbackFailure_001, TestSize.Level2)
{
    auto &manager = PlaceholderCallbackManager::GetInstance();
    auto callback = sptr(new ErrorCallbackTableStub(E_IPC_FAILED));
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, callback), E_OK);
    std::string path = "dir/file.txt";
    CloudDiskPathInfo pathInfo{path.data(), path.length()};
    CloudDiskCallbackReqHead reqHead{};

    EXPECT_EQ(manager.DispatchFetchData(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo), E_IPC_FAILED);
    EXPECT_EQ(manager.DispatchCancelFetchData(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo),
              E_IPC_FAILED);
    EXPECT_EQ(manager.DispatchDehydrate(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, pathInfo), E_IPC_FAILED);
    EXPECT_EQ(callback->sendCount_, 3U);
}

/**
 * @tc.name: ClearAll_001
 * @tc.desc: Clear entries for shared and distinct remotes, including death-recipient bookkeeping.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, ClearAll_001, TestSize.Level2)
{
    auto &manager = PlaceholderCallbackManager::GetInstance();
    auto sharedCallback = sptr(new CallbackTableStub());
    auto otherCallback = sptr(new CallbackTableStub());
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, sharedCallback), E_OK);
    ASSERT_EQ(manager.RegisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1, sharedCallback), E_OK);
    ASSERT_EQ(manager.RegisterCallbackTable("com.example.other", TEST_SYNC_FOLDER_INDEX, otherCallback), E_OK);

    manager.ClearAll();
    EXPECT_TRUE(manager.callbackMap_.empty());
    EXPECT_TRUE(manager.remoteCallbackMap_.empty());
    EXPECT_TRUE(manager.deathRecipientMap_.empty());
    manager.ClearAll();
    EXPECT_TRUE(manager.callbackMap_.empty());
}

/**
 * @tc.name: RemoveRemoteKeyLocked_001
 * @tc.desc: Removing a null or untracked remote is a safe no-op.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, RemoveRemoteKeyLocked_001, TestSize.Level2)
{
    auto &manager = PlaceholderCallbackManager::GetInstance();
    PlaceholderCallbackManager::CallbackKey key{TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX};
    sptr<ICloudDiskServiceCallbackTable> nullCallback;
    manager.RemoveRemoteKeyLocked(nullCallback, key);

    auto callback = sptr(new CallbackTableStub());
    manager.RemoveRemoteKeyLocked(callback, key);
    EXPECT_TRUE(manager.remoteCallbackMap_.empty());
    EXPECT_TRUE(manager.deathRecipientMap_.empty());
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
