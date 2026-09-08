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
            case CloudDiskCallbackType::FETCH_RANGE_DATA:
                if (reqContext.fetchRangeData != nullptr) {
                    filePath = std::string(reqContext.fetchRangeData->filePath.value,
                                           reqContext.fetchRangeData->filePath.length);
                    rangeOffset = reqContext.fetchRangeData->offset;
                    rangeSize = reqContext.fetchRangeData->size;
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
    uint64_t rangeOffset = 0;
    uint64_t rangeSize = 0;
    bool allowDehydrate = false;
};
} // namespace

class PlaceholderCallbackManagerTest : public testing::Test {
public:
    void TearDown() override
    {
        PlaceholderTaskManager::GetInstance().StopWorkerPool();
        PlaceholderCallbackManager::GetInstance().ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX);
        PlaceholderCallbackManager::GetInstance().ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1);
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
              E_CANCELLED);
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
 * @tc.name: DispatchCallbackKinds_001
 * @tc.desc: Verify cancel and range callbacks preserve their discriminated context and shared-remote cleanup.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderCallbackManagerTest, DispatchCallbackKinds_001, TestSize.Level1)
{
    constexpr uint64_t RANGE_OFFSET = 8;
    constexpr uint64_t RANGE_SIZE = 16;
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

    CloudDiskRangeInfo rangeInfo{pathInfo, RANGE_OFFSET, RANGE_SIZE, {nullptr, 0}};
    EXPECT_EQ(manager.DispatchFetchRangeData(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX, reqHead, rangeInfo), E_OK);
    EXPECT_EQ(callback->callbackType, CloudDiskCallbackType::FETCH_RANGE_DATA);
    EXPECT_EQ(callback->rangeOffset, RANGE_OFFSET);
    EXPECT_EQ(callback->rangeSize, RANGE_SIZE);

    manager.ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX);
    EXPECT_FALSE(manager.IsCallbackRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX));
    EXPECT_TRUE(manager.IsCallbackRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1));
    EXPECT_EQ(manager.remoteCallbackMap_.at(remoteKey).size(), 1U);
    manager.ClearBySyncFolder(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1);
    EXPECT_EQ(manager.remoteCallbackMap_.count(remoteKey), 0U);
    EXPECT_EQ(manager.deathRecipientMap_.count(remoteKey), 0U);
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
