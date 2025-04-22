/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "copy/file_copy_listener.h"
#include "dfs_error.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

/* Test Constants */
const std::string TEST_DIR = "/data/test_file_copy_listener/";
const std::string SRC_FILE = TEST_DIR + "test_src.txt";
const std::string DEST_DIR = TEST_DIR + "dest/";
constexpr uint64_t TEST_FILE_SIZE = 1024; // 1KB test file

/* Test Fixture */
class FileCopyLocalListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void CreateTestFile(const std::string &path, size_t size);
    bool CheckFileExists(const std::string &path);
};

/* Static Setup/TearDown */
void FileCopyLocalListenerTest::SetUpTestCase()
{
    // Create test directories
    mkdir(TEST_DIR.c_str(), 0777);
    mkdir(DEST_DIR.c_str(), 0777);
}

void FileCopyLocalListenerTest::TearDownTestCase()
{
    // Cleanup test directories
    rmdir(TEST_DIR.c_str());
}

/* Per-test Setup */
void FileCopyLocalListenerTest::SetUp()
{
    // Create standard test file before each test
    CreateTestFile(SRC_FILE, TEST_FILE_SIZE);
}

/* Per-test Cleanup */
void FileCopyLocalListenerTest::TearDown()
{
    // Remove test file after each test
    unlink(SRC_FILE.c_str());
}

/* Helper Functions */
void FileCopyLocalListenerTest::CreateTestFile(const std::string &path, size_t size)
{
    std::ofstream ofs(path.c_str(), std::ios::binary);
    if (!ofs) {
        FAIL() << "Failed to create test file: " << path;
    }
    std::vector<char> buffer(size, 'A');
    ofs.write(buffer.data(), buffer.size());
}

bool FileCopyLocalListenerTest::CheckFileExists(const std::string &path)
{
    return access(path.c_str(), F_OK) == 0;
}

/**
 * @tc.name: FileCopyLocalListener_Constructor_0001
 * @tc.desc: Verify construction with valid callback
 * @tc.type: FUNC
 * @tc.require: AR000H0E5H
 */
HWTEST_F(FileCopyLocalListenerTest, FileCopyLocalListener_Constructor_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyLocalListener_Constructor_0001 Start";

    /* Use explicit std::function type to match header */
    std::function<void(uint64_t, uint64_t)> callback = [](uint64_t, uint64_t) {};
    auto listener = FileCopyLocalListener::GetLocalListener(SRC_FILE, true, callback);

    /* Verify successful construction */
    EXPECT_NE(listener, nullptr);
    EXPECT_EQ(listener->GetResult(), E_OK);

    GTEST_LOG_(INFO) << "FileCopyLocalListener_Constructor_0001 End";
}

/**
 * @tc.name: FileCopyLocalListener_Constructor_0002
 * @tc.desc: Verify construction with nullptr callback
 * @tc.type: FUNC
 * @tc.require: AR000H0E5H
 */
HWTEST_F(FileCopyLocalListenerTest, FileCopyLocalListener_Constructor_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyLocalListener_Constructor_0002 Start";

    /* Pass nullptr explicitly */
    auto listener = FileCopyLocalListener::GetLocalListener(SRC_FILE, true, nullptr);

    /* Should still construct successfully */
    EXPECT_NE(listener, nullptr);

    GTEST_LOG_(INFO) << "FileCopyLocalListener_Constructor_0002 End";
}

/**
 * @tc.name: FileCopyLocalListener_StartStop_0001
 * @tc.desc: Verify StartListener and StopListener behavior
 * @tc.type: FUNC
 * @tc.require: AR000H0E5H
 */
HWTEST_F(FileCopyLocalListenerTest, FileCopyLocalListener_StartStop_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyLocalListener_StartStop_0001 Start";

    std::string testPath = "/data/test_file_listener.txt";
    std::function<void(uint64_t, uint64_t)> callback = [](uint64_t current, uint64_t total) {
        GTEST_LOG_(INFO) << "Progress: " << current << "/" << total;
    };

    // 准备测试文件
    std::ofstream out(testPath);
    out << "init";
    out.close();

    auto listener = FileCopyLocalListener::GetLocalListener(testPath, true, callback);
    ASSERT_NE(listener, nullptr);

    // 添加监听路径
    listener->AddListenerFile(testPath, IN_MODIFY | IN_CLOSE_WRITE);
    listener->AddFile(testPath); // 使其通过 CheckFileValid 校验

    listener->StartListener();

    // 模拟事件：修改监听文件
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::ofstream modify(testPath);
    modify << "trigger event";
    modify.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // 等待事件处理

    listener->StopListener();

    // 清理
    remove(testPath.c_str());

    GTEST_LOG_(INFO) << "FileCopyLocalListener_StartStop_0001 End";
}

/**
 * @tc.name: FileCopyLocalListener_FileProgress_0001
 * @tc.desc: Verify single file progress reporting
 * @tc.type: FUNC
 * @tc.require: AR000H0E5H
 */
HWTEST_F(FileCopyLocalListenerTest, FileCopyLocalListener_FileProgress_0001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "FileCopyLocalListener_FileProgress_0001 Start";

    /* Track progress updates */
    uint64_t lastProgress = 0;
    std::function<void(uint64_t, uint64_t)> callback = [&](uint64_t progress, uint64_t total) {
        EXPECT_GE(progress, lastProgress);
        lastProgress = progress;
    };

    /* Configure listener */
    auto listener = FileCopyLocalListener::GetLocalListener(SRC_FILE, true, callback);
    listener->StartListener();
    listener->AddListenerFile(SRC_FILE, IN_MODIFY);

    /* Simulate 5 file modifications */
    constexpr int MODIFY_COUNT = 5;
    for (int i = 0; i < MODIFY_COUNT; i++) {
        CreateTestFile(SRC_FILE, TEST_FILE_SIZE * (i + 1));
        usleep(50000); // Allow time for event processing
    }

    /* Verify final progress */
    listener->StopListener();
    EXPECT_EQ(lastProgress, TEST_FILE_SIZE * MODIFY_COUNT);

    GTEST_LOG_(INFO) << "FileCopyLocalListener_FileProgress_0001 End";
}

/**
 * @tc.name: FileCopyLocalListener_ThreadSafety_0001
 * @tc.desc: Verify thread-safe file additions
 * @tc.type: FUNC
 * @tc.require: AR000H0E5H
 */
HWTEST_F(FileCopyLocalListenerTest, FileCopyLocalListener_ThreadSafety_0001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "FileCopyLocalListener_ThreadSafety_0001 Start";

    /* Create listener without callback */
    auto listener = FileCopyLocalListener::GetLocalListener(SRC_FILE, false, nullptr);

    /* Configure concurrent test */
    constexpr int THREAD_COUNT = 5;
    constexpr int FILES_PER_THREAD = 10;
    std::vector<std::thread> workers;

    /* Launch concurrent adders */
    for (int i = 0; i < THREAD_COUNT; i++) {
        workers.emplace_back([&, i] {
            for (int j = 0; j < FILES_PER_THREAD; j++) {
                std::string filename = SRC_FILE + "_t" + std::to_string(i) + "_f" + std::to_string(j);
                listener->AddFile(filename);
            }
        });
    }

    /* Wait for completion */
    for (auto &t : workers) {
        t.join();
    }

    /* Verify all files registered */
    EXPECT_EQ(listener->GetFilePath().size(), THREAD_COUNT * FILES_PER_THREAD);

    GTEST_LOG_(INFO) << "FileCopyLocalListener_ThreadSafety_0001 End";
}

} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS