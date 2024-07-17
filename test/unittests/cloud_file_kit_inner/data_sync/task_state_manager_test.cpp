/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "task_state_manager.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "refbase.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockTaskStateManager : public TaskStateManager {
public:
    MOCK_METHOD0(GetInstance, TaskStateManager&());
    MOCK_METHOD0(DelayUnloadTask, void());
    MOCK_METHOD0(CancelUnloadTask, void());
    MOCK_METHOD0(StartTask, void());
    MOCK_METHOD2(StartTask, void(string bundleName, TaskType task));
    MOCK_METHOD2(CompleteTask, void(string bundleName, TaskType task));
    MOCK_METHOD2(HasTask, bool(string bundleName, TaskType task));
};

class TaskStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TaskStateManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void TaskStateManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TaskStateManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TaskStateManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_001 Start";
    try {
        TaskStateManager::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_001 Start ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_001 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_002 Start";
    try {
        TaskStateManager::GetInstance().HasTask("testBundleName", TaskType::UPLOAD_ASSET_TASK);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_002 Start ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_002 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_003 Start";
    try {
        TaskStateManager::GetInstance().HasTask("testBundleName", TaskType::DOWNLOAD_ASSET_TASK);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_003 Start ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_003 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_004 Start";
    try {
        MockTaskStateManager mockTaskStateManager;
        EXPECT_CALL(mockTaskStateManager, GetInstance())
            .WillOnce(ReturnRef(mockTaskStateManager));
        
        mockTaskStateManager.GetInstance();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_004 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_005 Start";
    try {
        MockTaskStateManager mockTaskStateManager;
        EXPECT_CALL(mockTaskStateManager, DelayUnloadTask());
        mockTaskStateManager.DelayUnloadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_005 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_006 Start";
    try {
        MockTaskStateManager mockTaskStateManager;
        EXPECT_CALL(mockTaskStateManager, CancelUnloadTask());
        mockTaskStateManager.CancelUnloadTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_006 ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_006 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_007 Start";
    try {
        MockTaskStateManager mockTaskStateManager;
        EXPECT_CALL(mockTaskStateManager, StartTask());
        mockTaskStateManager.StartTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_007 ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_007 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_008 Start";
    try {
        MockTaskStateManager mockTaskStateManager;
        string bundleName = "testbundleName";
        EXPECT_CALL(mockTaskStateManager, StartTask(bundleName, TaskType::SYNC_TASK));
        EXPECT_CALL(mockTaskStateManager, StartTask(bundleName, TaskType::DOWNLOAD_TASK));
        EXPECT_CALL(mockTaskStateManager, StartTask(bundleName, TaskType::UPLOAD_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, StartTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, StartTask(bundleName, TaskType::DOWNLOAD_REMOTE_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, StartTask(bundleName, TaskType::DOWNLOAD_THUMB_TASK));
        EXPECT_CALL(mockTaskStateManager, StartTask(bundleName, TaskType::DISABLE_CLOUD_TASK));

        mockTaskStateManager.StartTask(bundleName, TaskType::SYNC_TASK);
        mockTaskStateManager.StartTask(bundleName, TaskType::DOWNLOAD_TASK);
        mockTaskStateManager.StartTask(bundleName, TaskType::UPLOAD_ASSET_TASK);
        mockTaskStateManager.StartTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK);
        mockTaskStateManager.StartTask(bundleName, TaskType::DOWNLOAD_REMOTE_ASSET_TASK);
        mockTaskStateManager.StartTask(bundleName, TaskType::DOWNLOAD_THUMB_TASK);
        mockTaskStateManager.StartTask(bundleName, TaskType::DISABLE_CLOUD_TASK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_008 ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_008 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_009 Start";
    try {
        MockTaskStateManager mockTaskStateManager;
        string bundleName = "testbundleName";
        EXPECT_CALL(mockTaskStateManager, CompleteTask(bundleName, TaskType::SYNC_TASK));
        EXPECT_CALL(mockTaskStateManager, CompleteTask(bundleName, TaskType::DOWNLOAD_TASK));
        EXPECT_CALL(mockTaskStateManager, CompleteTask(bundleName, TaskType::UPLOAD_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, CompleteTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, CompleteTask(bundleName, TaskType::DOWNLOAD_REMOTE_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, CompleteTask(bundleName, TaskType::DOWNLOAD_THUMB_TASK));
        EXPECT_CALL(mockTaskStateManager, CompleteTask(bundleName, TaskType::DISABLE_CLOUD_TASK));

        mockTaskStateManager.CompleteTask(bundleName, TaskType::SYNC_TASK);
        mockTaskStateManager.CompleteTask(bundleName, TaskType::DOWNLOAD_TASK);
        mockTaskStateManager.CompleteTask(bundleName, TaskType::UPLOAD_ASSET_TASK);
        mockTaskStateManager.CompleteTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK);
        mockTaskStateManager.CompleteTask(bundleName, TaskType::DOWNLOAD_REMOTE_ASSET_TASK);
        mockTaskStateManager.CompleteTask(bundleName, TaskType::DOWNLOAD_THUMB_TASK);
        mockTaskStateManager.CompleteTask(bundleName, TaskType::DISABLE_CLOUD_TASK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_009 ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_009 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_010 Start";
    try {
        MockTaskStateManager mockTaskStateManager;
        string bundleName = "testbundleName";
        EXPECT_CALL(mockTaskStateManager, HasTask(bundleName, TaskType::SYNC_TASK));
        EXPECT_CALL(mockTaskStateManager, HasTask(bundleName, TaskType::DOWNLOAD_TASK));
        EXPECT_CALL(mockTaskStateManager, HasTask(bundleName, TaskType::UPLOAD_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, HasTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, HasTask(bundleName, TaskType::DOWNLOAD_REMOTE_ASSET_TASK));
        EXPECT_CALL(mockTaskStateManager, HasTask(bundleName, TaskType::DOWNLOAD_THUMB_TASK));
        EXPECT_CALL(mockTaskStateManager, HasTask(bundleName, TaskType::DISABLE_CLOUD_TASK));

        mockTaskStateManager.HasTask(bundleName, TaskType::SYNC_TASK);
        mockTaskStateManager.HasTask(bundleName, TaskType::DOWNLOAD_TASK);
        mockTaskStateManager.HasTask(bundleName, TaskType::UPLOAD_ASSET_TASK);
        mockTaskStateManager.HasTask(bundleName, TaskType::DOWNLOAD_ASSET_TASK);
        mockTaskStateManager.HasTask(bundleName, TaskType::DOWNLOAD_REMOTE_ASSET_TASK);
        mockTaskStateManager.HasTask(bundleName, TaskType::DOWNLOAD_THUMB_TASK);
        mockTaskStateManager.HasTask(bundleName, TaskType::DISABLE_CLOUD_TASK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_010 ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_010 End";
}

HWTEST_F(TaskStateManagerTest, TaskStateManagerTest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskStateManagerTest_011 Start";
    try {
        MockTaskStateManager mockTaskStateManager;
        string bundleName = "testbundleName";
        EXPECT_CALL(mockTaskStateManager, HasTask(bundleName, TaskType::SYNC_TASK))
            .WillOnce(Return(true))
            .WillOnce(Return(false));

        EXPECT_TRUE(mockTaskStateManager.HasTask(bundleName, TaskType::SYNC_TASK));
        EXPECT_FALSE(mockTaskStateManager.HasTask(bundleName, TaskType::SYNC_TASK));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TaskStateManagerTest_011 ERROR";
    }
    GTEST_LOG_(INFO) << "TaskStateManagerTest_011 End";
}
}