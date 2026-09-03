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
        if (reqHead.callbackType == CloudDiskCallbackType::DEHYDRATE) {
            if (reqContext.dehydrateData != nullptr) {
                filePath = std::string(reqContext.dehydrateData->filePath.value,
                                       reqContext.dehydrateData->filePath.length);
                reqContext.dehydrateData->allow = allowDehydrate;
            }
            return;
        }
        if (reqContext.fetchData != nullptr) {
            filePath = std::string(reqContext.fetchData->filePath.value, reqContext.fetchData->filePath.length);
            priority = reqContext.fetchData->priority;
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

    EXPECT_EQ(manager.UnregisterCallbackTable(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX), E_OK);
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
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, UniqueFd(dup(STDOUT_FILENO)), firstKey), E_OK);
    ASSERT_EQ(tasks.CreateHydrateTask("sync", "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1,
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, UniqueFd(dup(STDOUT_FILENO)), secondKey), E_OK);
    ASSERT_EQ(tasks.CreateHydrateTask("sync", "file.txt", TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 2,
        CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, UniqueFd(dup(STDOUT_FILENO)), otherKey), E_OK);
    tasks.taskMap_.at(secondKey)->state = PlaceholderTaskState::IN_PROGRESS;
    const void *remoteKey = callback->AsObject().GetRefPtr();
    callbacks.OnRemoteDied(remoteKey);
    EXPECT_FALSE(callbacks.IsCallbackRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX));
    EXPECT_FALSE(callbacks.IsCallbackRegistered(TEST_BUNDLE_NAME, TEST_SYNC_FOLDER_INDEX + 1));
    EXPECT_EQ(callbacks.remoteCallbackMap_.count(remoteKey), 0U);
    EXPECT_EQ(callbacks.deathRecipientMap_.count(remoteKey), 0U);
    PlaceholderTaskState state;
    ASSERT_TRUE(tasks.GetTaskState(firstKey, state));
    EXPECT_EQ(state, PlaceholderTaskState::CANCELLED);
    ASSERT_TRUE(tasks.GetTaskState(secondKey, state));
    EXPECT_EQ(state, PlaceholderTaskState::CANCELLED);
    ASSERT_TRUE(tasks.GetTaskState(otherKey, state));
    EXPECT_EQ(state, PlaceholderTaskState::PENDING);
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
