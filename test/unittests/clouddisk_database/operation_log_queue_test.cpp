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
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "operation_log_const.h"
#include "operation_log_entry.h"
#include "operation_log_queue.h"

namespace OHOS {
namespace FileManagement::CloudDisk {
namespace Test {
using namespace testing::ext;

namespace {
OperationLogEntry CreateEntry(int64_t opTime)
{
    return OperationLogEntry{ opTime, OperationLogConst::OP_TYPE_DELETE, "/data/oplog/foo.txt",
        100, 200, "test_proc", 300, 400 };
}
} // namespace

class OperationLogQueueTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: PopBatchEmptyWaitsUntilTimeout
 * @tc.desc: Verify PopBatch blocks up to the timeout and returns an empty batch when the queue is empty.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogQueueTest, PopBatchEmptyWaitsUntilTimeout, TestSize.Level1)
{
    OperationLogQueue queue;
    auto start = std::chrono::steady_clock::now();
    auto batch = queue.PopBatch();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    EXPECT_TRUE(batch.empty());
    EXPECT_GE(elapsed, static_cast<int64_t>(900));
}

/**
 * @tc.name: PushAndPopSingle
 * @tc.desc: Verify a single pushed entry can be popped back with all fields intact.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogQueueTest, PushAndPopSingle, TestSize.Level1)
{
    OperationLogQueue queue;
    queue.Push(CreateEntry(42));
    auto batch = queue.PopBatch();
    ASSERT_EQ(batch.size(), 1);
    EXPECT_EQ(batch[0].opTime, 42);
    EXPECT_EQ(batch[0].opType, OperationLogConst::OP_TYPE_DELETE);
    EXPECT_EQ(batch[0].filePath, "/data/oplog/foo.txt");
    EXPECT_EQ(batch[0].fileInode, 100);
    EXPECT_EQ(batch[0].fileUid, 200);
    EXPECT_EQ(batch[0].processName, "test_proc");
    EXPECT_EQ(batch[0].processPid, 300);
    EXPECT_EQ(batch[0].processUid, 400);
}

/**
 * @tc.name: PushAndPopInOrder
 * @tc.desc: Verify PopBatch preserves the push order (FIFO).
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogQueueTest, PushAndPopInOrder, TestSize.Level1)
{
    OperationLogQueue queue;
    queue.Push(CreateEntry(1));
    queue.Push(CreateEntry(2));
    queue.Push(CreateEntry(3));
    auto batch = queue.PopBatch();
    ASSERT_EQ(batch.size(), 3);
    EXPECT_EQ(batch[0].opTime, 1);
    EXPECT_EQ(batch[1].opTime, 2);
    EXPECT_EQ(batch[2].opTime, 3);
}

/**
 * @tc.name: PopBatchCapsAtBatchSize
 * @tc.desc: Verify PopBatch returns at most BATCH_SIZE entries and the rest remain queued.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogQueueTest, PopBatchCapsAtBatchSize, TestSize.Level1)
{
    OperationLogQueue queue;
    constexpr uint32_t total = OperationLogQueue::BATCH_SIZE + 50;
    for (uint32_t i = 0; i < total; ++i) {
        queue.Push(CreateEntry(static_cast<int64_t>(i)));
    }
    auto first = queue.PopBatch();
    ASSERT_EQ(first.size(), static_cast<size_t>(OperationLogQueue::BATCH_SIZE));
    auto second = queue.PopBatch();
    ASSERT_EQ(second.size(), static_cast<size_t>(total - OperationLogQueue::BATCH_SIZE));
}

/**
 * @tc.name: PushWakesUpWaitingPop
 * @tc.desc: Verify a push from another thread wakes up a blocked PopBatch immediately.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogQueueTest, PushWakesUpWaitingPop, TestSize.Level1)
{
    OperationLogQueue queue;
    OperationLogEntry entry = CreateEntry(7);
    std::thread pusher([&queue, &entry]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        queue.Push(entry);
    });
    auto start = std::chrono::steady_clock::now();
    auto batch = queue.PopBatch();
    pusher.join();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    ASSERT_EQ(batch.size(), 1);
    EXPECT_EQ(batch[0].opTime, 7);
    EXPECT_LT(elapsed, static_cast<int64_t>(900));
}

/**
 * @tc.name: PushWhenStoppedReturnsDirectly
 * @tc.desc: Verify Push returns immediately without pushing entry when queue is stopped.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogQueueTest, PushWhenStoppedReturnsDirectly, TestSize.Level1)
{
    OperationLogQueue queue;
    queue.Shutdown();
    OperationLogEntry entry = CreateEntry(42);

    queue.Push(entry);

    EXPECT_TRUE(queue.IsEmpty());
}

/**
 * @tc.name: ResetClearsQueue
 * @tc.desc: Verify Reset clears all entries from the queue.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogQueueTest, ResetClearsQueue, TestSize.Level1)
{
    OperationLogQueue queue;

    queue.Push(CreateEntry(1));
    queue.Push(CreateEntry(2));
    queue.Push(CreateEntry(3));

    auto batch = queue.PopBatch();
    ASSERT_EQ(batch.size(), 3);
    EXPECT_EQ(batch[0].opTime, 1);
    EXPECT_EQ(batch[1].opTime, 2);
    EXPECT_EQ(batch[2].opTime, 3);

    queue.Reset();

    EXPECT_TRUE(queue.IsEmpty());

    auto batch2 = queue.PopBatch();
    EXPECT_EQ(batch2.size(), 0);
}
} // namespace Test
} // namespace FileManagement
} // namespace OHOS
