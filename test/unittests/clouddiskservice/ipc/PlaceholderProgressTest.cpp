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
#include <chrono>
#include <mutex>
#include <vector>

#include "cloud_disk_progress_callback_client.h"
#include "cloud_disk_service_error.h"
#include "placeholder_progress_manager.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing::ext;

class RecordingProgress final : public CloudDiskProgressCallbackStub {
public:
    void OnProgress(const HydrateProgress &progress) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        events.push_back(progress);
    }
    std::mutex mutex;
    std::vector<HydrateProgress> events;
};

class PlaceholderProgressTest : public testing::Test {
public:
    void SetUp() override
    {
        PlaceholderProgressManager::GetInstance().Drain();
        PlaceholderProgressManager::GetInstance().Clear();
    }
    void TearDown() override
    {
        PlaceholderProgressManager::GetInstance().Drain();
        PlaceholderProgressManager::GetInstance().Clear();
    }
};
/**
 * @tc.name: Subscription_001
 * @tc.desc: Preserve caller isolation, idempotence and stale death-recipient safety.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, Subscription_001, TestSize.Level2)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    auto first = sptr(new RecordingProgress());
    auto second = sptr(new RecordingProgress());
    EXPECT_EQ(manager.Register({1, 10}, 100, nullptr), E_INVALID_ARG);
    ASSERT_EQ(manager.Register({1, 10}, 100, first), E_OK);
    EXPECT_EQ(manager.Register({1, 10}, 100, first), E_OK);
    EXPECT_EQ(manager.Register({1, 10}, 101, first), E_CALLBACK_ALREADY_REGISTERED);
    EXPECT_EQ(manager.Register({1, 10}, 100, second), E_CALLBACK_ALREADY_REGISTERED);
    ASSERT_EQ(manager.Register({2, 20}, 101, second), E_OK);
    HydrateProgress event;
    event.filePath = "/storage/Users/currentUser/sync/file";
    manager.OnTaskProgress({1}, 100, event);
    manager.Drain();
    ASSERT_EQ(first->events.size(), 1u);
    EXPECT_TRUE(second->events.empty());
    manager.OnRemoteDied({1, 10}, wptr<IRemoteObject>(second->AsObject()));
    EXPECT_EQ(manager.subscribers_.size(), 2u);
    manager.OnRemoteDied({1, 10}, wptr<IRemoteObject>(first->AsObject()));
    EXPECT_EQ(manager.Unregister({1, 10}), E_CALLBACK_NOT_REGISTERED);
    EXPECT_EQ(manager.Unregister({2, 20}), E_OK);
    EXPECT_TRUE(manager.subscribers_.empty());
}
/**
 * @tc.name: Throttle_001
 * @tc.desc: Send every state transition and terminal event, but throttle repeated byte updates.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, Throttle_001, TestSize.Level2)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    auto callback = sptr(new RecordingProgress());
    ASSERT_EQ(manager.Register({1, 10}, 100, callback), E_OK);
    HydrateProgress event;
    event.filePath = "/path";
    event.totalSize = 100;
    manager.OnTaskProgress({1}, 100, event);
    event.state = 1;
    manager.OnTaskProgress({1}, 100, event);
    event.processedSize = 10;
    manager.OnTaskProgress({1}, 100, event);
    manager.Drain();
    ASSERT_EQ(callback->events.size(), 2u);
    {
        std::lock_guard<std::mutex> lock(manager.mutex_);
        manager.lastProgress_.at({1}).time -= std::chrono::milliseconds(501);
    }
    manager.OnTaskProgress({1}, 100, event);
    event.state = 2;
    event.processedSize = 100;
    manager.OnTaskProgress({1}, 100, event);
    manager.Drain();
    ASSERT_EQ(callback->events.size(), 4u);
    EXPECT_EQ(callback->events[0].state, 0);
    EXPECT_EQ(callback->events[1].state, 1);
    EXPECT_EQ(callback->events[2].processedSize, 10u);
    EXPECT_EQ(callback->events[3].state, 2);
    EXPECT_EQ(callback->events[3].processedSize, 100u);
    EXPECT_TRUE(manager.lastProgress_.empty());
}
/**
 * @tc.name: Multiplex_001
 * @tc.desc: Removing one local accessor retains the other subscriber and duplicate adds are idempotent.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, Multiplex_001, TestSize.Level2)
{
    auto client = sptr(new CloudDiskProgressCallbackClient());
    auto first = sptr(new RecordingProgress());
    auto second = sptr(new RecordingProgress());
    EXPECT_TRUE(client->Add(first));
    EXPECT_FALSE(client->Add(first));
    EXPECT_TRUE(client->Add(second));
    HydrateProgress event;
    client->OnProgress(event);
    EXPECT_EQ(first->events.size(), 1u);
    EXPECT_EQ(second->events.size(), 1u);
    EXPECT_FALSE(client->Remove(first));
    client->OnProgress(event);
    EXPECT_EQ(first->events.size(), 1u);
    EXPECT_EQ(second->events.size(), 2u);
    EXPECT_TRUE(client->Remove(nullptr));
    client->OnProgress(event);
    EXPECT_EQ(second->events.size(), 2u);
}
/**
 * @tc.name: Unregister_001
 * @tc.desc: Unregistered subscribers receive no later terminal events.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, Unregister_001, TestSize.Level2)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    auto callback = sptr(new RecordingProgress());
    ASSERT_EQ(manager.Register({1, 10}, 100, callback), E_OK);
    EXPECT_EQ(manager.Unregister({1, 10}), E_OK);
    HydrateProgress event;
    event.state = 3;
    manager.OnTaskProgress({1}, 100, event);
    manager.Drain();
    EXPECT_TRUE(callback->events.empty());
    EXPECT_TRUE(manager.lastProgress_.empty());
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
