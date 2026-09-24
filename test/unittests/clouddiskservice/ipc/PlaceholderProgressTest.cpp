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

#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <vector>

#include "cloud_disk_progress_callback_client.h"
#include "cloud_disk_progress_callback_proxy.h"
#include "cloud_disk_service_callback_mock.h"
#include "cloud_disk_service_error.h"
#include "placeholder_progress_manager.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
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

class NullRemoteProgress final : public ICloudDiskProgressCallback {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    void OnProgress(const HydrateProgress &) override {}
};

class ControlledProxyRemote final : public IRemoteObject {
public:
    ControlledProxyRemote() : IRemoteObject(u"controlled_progress_proxy") {}

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

    bool addResult = true;
    int32_t addCount = 0;
    int32_t removeCount = 0;
};

class ProxyBackedProgress final : public ICloudDiskProgressCallback {
public:
    explicit ProxyBackedProgress(const sptr<IRemoteObject> &remote) : remote_(remote) {}

    sptr<IRemoteObject> AsObject() override
    {
        return remote_;
    }

    void OnProgress(const HydrateProgress &) override {}

private:
    sptr<IRemoteObject> remote_;
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
    EXPECT_EQ(manager.Register({1, 10, 11}, 100, nullptr), E_INVALID_ARG);
    ASSERT_EQ(manager.Register({1, 10, 11}, 100, first), E_OK);
    EXPECT_EQ(manager.Register({1, 10, 11}, 100, first), E_OK);
    EXPECT_EQ(manager.Register({1, 10, 11}, 101, first), E_CALLBACK_ALREADY_REGISTERED);
    EXPECT_EQ(manager.Register({1, 10, 11}, 100, second), E_CALLBACK_ALREADY_REGISTERED);
    ASSERT_EQ(manager.Register({2, 20, 22}, 101, second), E_OK);
    HydrateProgress event;
    event.filePath = "/storage/Users/currentUser/sync/file";
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    manager.Drain();
    ASSERT_EQ(first->events.size(), 1u);
    EXPECT_TRUE(second->events.empty());
    manager.OnRemoteDied({1, 10, 11}, wptr<IRemoteObject>(second->AsObject()));
    EXPECT_EQ(manager.subscribers_.size(), 2u);
    manager.OnRemoteDied({1, 10, 11}, wptr<IRemoteObject>(first->AsObject()));
    EXPECT_EQ(manager.Unregister({1, 10, 11}), E_CALLBACK_NOT_REGISTERED);
    EXPECT_EQ(manager.Unregister({2, 20, 22}), E_OK);
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
    ASSERT_EQ(manager.Register({1, 10, 11}, 100, callback), E_OK);
    HydrateProgress event;
    event.filePath = "/path";
    event.totalSize = 100;
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    event.state = 1;
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    event.processedSize = 10;
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    manager.Drain();
    ASSERT_EQ(callback->events.size(), 2u);
    {
        std::lock_guard<std::mutex> lock(manager.mutex_);
        manager.lastProgress_.at({1}).time -= std::chrono::milliseconds(501);
    }
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    event.state = 2;
    event.processedSize = 100;
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
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
    bool added = false;
    ASSERT_EQ(client->Add(11, first, added), E_OK);
    EXPECT_TRUE(added);
    EXPECT_EQ(client->Add(11, first, added), E_OK);
    EXPECT_FALSE(added);
    ASSERT_EQ(client->Add(22, second, added), E_OK);
    HydrateProgress event;
    event.accessorId = 11;
    client->OnProgress(event);
    event.accessorId = 22;
    client->OnProgress(event);
    EXPECT_EQ(first->events.size(), 1u);
    EXPECT_EQ(second->events.size(), 1u);
    EXPECT_TRUE(client->Remove(11, first));
    event.accessorId = 11;
    client->OnProgress(event);
    event.accessorId = 22;
    client->OnProgress(event);
    EXPECT_EQ(first->events.size(), 1u);
    EXPECT_EQ(second->events.size(), 2u);
    EXPECT_TRUE(client->Remove(22, second));
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
    ASSERT_EQ(manager.Register({1, 10, 11}, 100, callback), E_OK);
    EXPECT_EQ(manager.Unregister({1, 10, 11}), E_OK);
    HydrateProgress event;
    event.state = 3;
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    manager.Drain();
    EXPECT_TRUE(callback->events.empty());
    EXPECT_TRUE(manager.lastProgress_.empty());
}

/**
 * @tc.name: Register_001
 * @tc.desc: Reject callbacks without a remote object and negative user identifiers.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, Register_001, TestSize.Level2)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    sptr<ICloudDiskProgressCallback> nullRemote = sptr(new NullRemoteProgress());
    auto callback = sptr(new RecordingProgress());
    EXPECT_EQ(manager.Register({1, 1, 11}, 100, nullRemote), E_INVALID_ARG);
    EXPECT_EQ(manager.Register({1, 1, 11}, -1, callback), E_INVALID_ARG);
    EXPECT_TRUE(manager.subscribers_.empty());
}

/**
 * @tc.name: ProxyDeathRecipient_001
 * @tc.desc: Cover proxy death-recipient add success/failure and removal by unregister and clear.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, ProxyDeathRecipient_001, TestSize.Level2)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    auto remote = sptr(new ControlledProxyRemote());
    auto callback = sptr(new ProxyBackedProgress(remote));

    ASSERT_EQ(manager.Register({1, 1, 11}, 100, callback), E_OK);
    EXPECT_EQ(remote->addCount, 1);
    EXPECT_EQ(manager.Unregister({1, 1, 11}), E_OK);
    EXPECT_EQ(remote->removeCount, 1);

    remote->addResult = false;
    EXPECT_EQ(manager.Register({2, 2, 22}, 100, callback), E_IPC_FAILED);
    EXPECT_EQ(remote->addCount, 2);
    EXPECT_TRUE(manager.subscribers_.empty());

    remote->addResult = true;
    ASSERT_EQ(manager.Register({3, 3, 33}, 100, callback), E_OK);
    manager.Clear();
    EXPECT_EQ(remote->addCount, 3);
    EXPECT_EQ(remote->removeCount, 2);
}

/**
 * @tc.name: OnTaskProgress_001
 * @tc.desc: Skip inactive subscribers retained by an already queued notification snapshot.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, OnTaskProgress_001, TestSize.Level1)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    auto callback = sptr(new RecordingProgress());
    PlaceholderProgressManager::SubscriberKey key{1, 10, 11};
    ASSERT_EQ(manager.Register(key, 100, callback), E_OK);
    {
        std::lock_guard<std::mutex> lock(manager.mutex_);
        manager.subscribers_.at(key)->active = false;
    }
    HydrateProgress progress;
    manager.OnTaskProgress({1}, 100, progress, key);
    manager.Drain();
    EXPECT_TRUE(callback->events.empty());
}

/**
 * @tc.name: OnTaskProgress_002
 * @tc.desc: Queued events are discarded after their subscriber is replaced without affecting other accessors.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, OnTaskProgress_002, TestSize.Level2)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    auto original = sptr(new RecordingProgress());
    auto replacement = sptr(new RecordingProgress());
    auto other = sptr(new RecordingProgress());
    ASSERT_EQ(manager.Register({1, 10, 11}, 100, original), E_OK);
    ASSERT_EQ(manager.Register({1, 10, 22}, 100, other), E_OK);
    auto gate = std::make_shared<std::mutex>();
    HydrateProgress progress;
    {
        std::unique_lock<std::mutex> lock(*gate);
        manager.queue_.submit([gate] { std::lock_guard<std::mutex> lock(*gate); });
        manager.OnTaskProgress({1}, 100, progress, {1, 10, 11});
        ASSERT_EQ(manager.Unregister({1, 10, 11}), E_OK);
        ASSERT_EQ(manager.Register({1, 10, 11}, 100, replacement), E_OK);
        manager.OnTaskProgress({2}, 100, progress, {1, 10, 22});
    }
    manager.Drain();
    EXPECT_TRUE(original->events.empty());
    EXPECT_TRUE(replacement->events.empty());
    ASSERT_EQ(other->events.size(), 1U);
    EXPECT_EQ(other->events.front().accessorId, 22U);
    progress.state = static_cast<int32_t>(HydrateProgressState::COMPLETED);
    manager.OnTaskProgress({1}, 100, progress, {1, 10, 11});
    manager.Drain();
    ASSERT_EQ(replacement->events.size(), 1U);
    EXPECT_EQ(replacement->events.front().state, progress.state);
    EXPECT_EQ(other->events.size(), 1U);
}

/**
 * @tc.name: Remove_001
 * @tc.desc: Verify null add, absent remove, last remove, and empty-client dispatch branches.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, Remove_001, TestSize.Level1)
{
    auto client = sptr(new CloudDiskProgressCallbackClient());
    auto first = sptr(new RecordingProgress());
    auto absent = sptr(new RecordingProgress());
    bool added = false;
    EXPECT_EQ(client->Add(11, nullptr, added), E_INVALID_ARG);
    ASSERT_EQ(client->Add(11, first, added), E_OK);
    EXPECT_FALSE(client->Remove(11, absent));
    EXPECT_TRUE(client->Remove(11, first));
    HydrateProgress progress;
    progress.accessorId = 11;
    client->OnProgress(progress);
    EXPECT_TRUE(first->events.empty());
}

/**
 * @tc.name: ProgressStubOnRemoteRequest_001
 * @tc.desc: Verify progress stub token, transaction, parcel, and success branches.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, ProgressStubOnRemoteRequest_001, TestSize.Level2)
{
    auto callback = sptr(new RecordingProgress());
    MessageParcel reply;
    MessageOption option;

    MessageParcel invalidToken;
    ASSERT_TRUE(invalidToken.WriteInterfaceToken(u"invalid"));
    EXPECT_EQ(callback->OnRemoteRequest(ICloudDiskProgressCallback::ON_PROGRESS, invalidToken, reply, option),
              E_INVALID_ARG);

    MessageParcel unsupported;
    ASSERT_TRUE(unsupported.WriteInterfaceToken(callback->GetDescriptor()));
    EXPECT_NE(callback->OnRemoteRequest(ICloudDiskProgressCallback::ON_PROGRESS + 1, unsupported, reply, option), E_OK);

    MessageParcel malformed;
    ASSERT_TRUE(malformed.WriteInterfaceToken(callback->GetDescriptor()));
    EXPECT_EQ(callback->OnRemoteRequest(ICloudDiskProgressCallback::ON_PROGRESS, malformed, reply, option),
              E_INVALID_ARG);

    HydrateProgress progress;
    progress.filePath = "file.txt";
    progress.state = static_cast<int32_t>(HydrateProgressState::IN_PROGRESS);
    progress.processedSize = 1;
    progress.totalSize = 2;
    MessageParcel valid;
    ASSERT_TRUE(valid.WriteInterfaceToken(callback->GetDescriptor()));
    ASSERT_TRUE(valid.WriteParcelable(&progress));
    EXPECT_EQ(callback->OnRemoteRequest(ICloudDiskProgressCallback::ON_PROGRESS, valid, reply, option), E_OK);
    ASSERT_EQ(callback->events.size(), 1U);
    EXPECT_EQ(callback->events.front().filePath, progress.filePath);
}

/**
 * @tc.name: ProgressProxyOnProgress_001
 * @tc.desc: Verify progress proxy null-remote, serialization, send-success, and send-failure branches.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, ProgressProxyOnProgress_001, TestSize.Level2)
{
    HydrateProgress progress;
    progress.filePath = "file.txt";
    sptr<IRemoteObject> nullRemote;
    CloudDiskProgressCallbackProxy nullProxy(nullRemote);
    nullProxy.OnProgress(progress);

    auto remote = sptr(new CloudDiskServiceCallbackMock());
    CloudDiskProgressCallbackProxy proxy(remote);
    progress.state = -1;
    EXPECT_CALL(*remote, SendRequest(_, _, _, _)).Times(0);
    proxy.OnProgress(progress);

    Mock::VerifyAndClearExpectations(remote.GetRefPtr());
    progress.state = static_cast<int32_t>(HydrateProgressState::PENDING);
    EXPECT_CALL(*remote, SendRequest(ICloudDiskProgressCallback::ON_PROGRESS, _, _, _))
        .WillOnce(Invoke([](uint32_t, MessageParcel &, MessageParcel &, MessageOption &option) {
            EXPECT_EQ(option.GetFlags(), MessageOption::TF_ASYNC);
            return E_OK;
        }))
        .WillOnce(Return(E_IPC_FAILED));
    proxy.OnProgress(progress);
    proxy.OnProgress(progress);
}
/**
 * @tc.name: AccessorIsolation_001
 * @tc.desc: Route equal paths by accessor, token and pid; anonymous tasks never notify accessors.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, AccessorIsolation_001, TestSize.Level2)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    auto first = sptr(new RecordingProgress());
    auto second = sptr(new RecordingProgress());
    auto otherPid = sptr(new RecordingProgress());
    auto otherToken = sptr(new RecordingProgress());
    ASSERT_EQ(manager.Register({1, 10, 11}, 100, first), E_OK);
    ASSERT_EQ(manager.Register({1, 10, 22}, 100, second), E_OK);
    ASSERT_EQ(manager.Register({1, 20, 11}, 100, otherPid), E_OK);
    ASSERT_EQ(manager.Register({2, 10, 11}, 100, otherToken), E_OK);
    HydrateProgress event;
    event.filePath = "/same/path";
    event.accessorId = 22;
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    manager.OnTaskProgress({2}, 100, event, {1, 10, 22});
    manager.OnTaskProgress({3}, 101, event, {1, 10, 11});
    manager.Drain();
    ASSERT_EQ(first->events.size(), 1U);
    ASSERT_EQ(second->events.size(), 1U);
    EXPECT_EQ(first->events.front().accessorId, 11U);
    EXPECT_EQ(second->events.front().accessorId, 22U);
    EXPECT_TRUE(otherPid->events.empty());
    EXPECT_TRUE(otherToken->events.empty());
    manager.OnTaskProgress({4}, 100, event, {});
    manager.Drain();
    EXPECT_EQ(manager.lastProgress_.count({4}), 0U);
    EXPECT_EQ(first->events.size(), 1U);
    EXPECT_EQ(second->events.size(), 1U);
    EXPECT_TRUE(otherPid->events.empty());
    EXPECT_TRUE(otherToken->events.empty());
}

/**
 * @tc.name: AccessorSubscription_001
 * @tc.desc: Off/on resumes the owner's future progress and stale death does not remove its replacement.
 * @tc.type: RELI
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, AccessorSubscription_001, TestSize.Level2)
{
    auto &manager = PlaceholderProgressManager::GetInstance();
    auto oldCallback = sptr(new RecordingProgress());
    auto replacement = sptr(new RecordingProgress());
    auto other = sptr(new RecordingProgress());
    ASSERT_EQ(manager.Register({1, 10, 11}, 100, oldCallback), E_OK);
    ASSERT_EQ(manager.Register({1, 10, 22}, 100, other), E_OK);
    HydrateProgress event;
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    manager.Drain();
    ASSERT_EQ(oldCallback->events.size(), 1U);
    ASSERT_EQ(manager.Unregister({1, 10, 11}), E_OK);
    event.state = static_cast<int32_t>(HydrateProgressState::IN_PROGRESS);
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    manager.OnTaskProgress({2}, 100, event, {1, 10, 22});
    manager.Drain();
    ASSERT_EQ(manager.Register({1, 10, 11}, 100, replacement), E_OK);
    manager.OnRemoteDied({1, 10, 11}, wptr<IRemoteObject>(oldCallback->AsObject()));
    event.state = static_cast<int32_t>(HydrateProgressState::COMPLETED);
    manager.OnTaskProgress({1}, 100, event, {1, 10, 11});
    manager.Drain();
    EXPECT_EQ(oldCallback->events.size(), 1U);
    ASSERT_EQ(replacement->events.size(), 1U);
    EXPECT_EQ(replacement->events.front().state, event.state);
    EXPECT_EQ(other->events.size(), 1U);
    EXPECT_EQ(manager.Unregister({1, 10, 22}), E_OK);
}

/**
 * @tc.name: AccessorClient_001
 * @tc.desc: A client routes nonzero ids to exactly one callback and ignores zero or unknown ids.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, AccessorClient_001, TestSize.Level2)
{
    auto client = sptr(new CloudDiskProgressCallbackClient());
    auto first = sptr(new RecordingProgress());
    auto second = sptr(new RecordingProgress());
    bool added = false;
    EXPECT_EQ(client->Add(11, nullptr, added), E_INVALID_ARG);
    ASSERT_EQ(client->Add(11, first, added), E_OK);
    EXPECT_TRUE(added);
    ASSERT_EQ(client->Add(11, first, added), E_OK);
    EXPECT_FALSE(added);
    EXPECT_EQ(client->Add(11, second, added), E_CALLBACK_ALREADY_REGISTERED);
    ASSERT_EQ(client->Add(22, second, added), E_OK);
    HydrateProgress event;
    event.accessorId = 11;
    client->OnProgress(event);
    EXPECT_EQ(first->events.size(), 1U);
    EXPECT_TRUE(second->events.empty());
    event.accessorId = 22;
    client->OnProgress(event);
    event.accessorId = 0;
    client->OnProgress(event);
    event.accessorId = 99;
    client->OnProgress(event);
    EXPECT_EQ(first->events.size(), 1U);
    EXPECT_EQ(second->events.size(), 1U);
}

/**
 * @tc.name: AccessorClient_002
 * @tc.desc: Removing an accessor or requesting removal for id zero leaves other registrations intact.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(PlaceholderProgressTest, AccessorClient_002, TestSize.Level2)
{
    auto client = sptr(new CloudDiskProgressCallbackClient());
    auto first = sptr(new RecordingProgress());
    auto second = sptr(new RecordingProgress());
    bool added = false;
    ASSERT_EQ(client->Add(11, first, added), E_OK);
    ASSERT_EQ(client->Add(22, second, added), E_OK);
    EXPECT_FALSE(client->Remove(11, second));
    EXPECT_FALSE(client->Remove(11, nullptr));
    EXPECT_TRUE(client->Remove(11, first));
    EXPECT_FALSE(client->Remove(11, first));
    EXPECT_FALSE(client->Remove(0, second));
    HydrateProgress event;
    event.accessorId = 11;
    client->OnProgress(event);
    event.accessorId = 22;
    client->OnProgress(event);
    EXPECT_TRUE(first->events.empty());
    EXPECT_EQ(second->events.size(), 1U);
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
