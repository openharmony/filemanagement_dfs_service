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

#include "data_handler_mock.h"
#include "dfs_error.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"
#include "result_set_mock.h"
#include "sync_rule/cloud_status.h"
#include "task.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
const int USER_ID = 100;
const std::string BUND_NAME = "com.ohos.photos";
const std::string TABLE_NAME = "test";
void CallBack()
{
    return;
}

void Action(std::shared_ptr<TaskContext> context)
{
    return;
}

int32_t CommitFuncSuccess(std::shared_ptr<TaskRunner> runner, std::shared_ptr<Task> task)
{
    return 0;
}

int32_t CommitFuncFail(std::shared_ptr<TaskRunner> runner, std::shared_ptr<Task> task)
{
    return 1;
}

class TaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TaskTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void TaskTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TaskTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TaskTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AddTask
 * @tc.desc: Verify the AddTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, AddTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddTask Begin";
    try {
        auto handler = std::make_shared<DataHandlerMock>(USER_ID, BUND_NAME, TABLE_NAME);
        auto context = std::make_shared<TaskContext>(handler);
        TaskAction action = Action;
        auto task = make_shared<Task>(context, action);
        std::function<void()> callBack = CallBack;
        TaskRunner taskRunner(callBack);
        auto ret = taskRunner.AddTask(task);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddTask ERROR";
    }

    GTEST_LOG_(INFO) << "AddTask End";
}

/**
 * @tc.name: StartTask001
 * @tc.desc: Verify the StartTask001 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, StartTask001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTask001 Begin";
    try {
        auto handler = std::make_shared<DataHandlerMock>(USER_ID, BUND_NAME, TABLE_NAME);
        auto context = std::make_shared<TaskContext>(handler);
        TaskAction action = Action;
        auto task = make_shared<Task>(context, action);
        std::function<void()> callBack = CallBack;
        auto taskRunner = make_shared<TaskRunner>(callBack);
        taskRunner->stopFlag_ = make_shared<bool>(false);
        taskRunner->SetCommitFunc(CommitFuncSuccess);
        auto ret = taskRunner->StartTask(task, action);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartTask001 ERROR";
    }

    GTEST_LOG_(INFO) << "StartTask001 End";
}

/**
 * @tc.name: StartTask002
 * @tc.desc: Verify the StartTask002 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, StartTask002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTask002 Begin";
    try {
        auto handler = std::make_shared<DataHandlerMock>(USER_ID, BUND_NAME, TABLE_NAME);
        auto context = std::make_shared<TaskContext>(handler);
        TaskAction action = Action;
        auto task = make_shared<Task>(context, action);
        std::function<void()> callBack = CallBack;
        auto taskRunner = make_shared<TaskRunner>(callBack);
        taskRunner->stopFlag_ = make_shared<bool>(false);
        taskRunner->SetCommitFunc(CommitFuncFail);
        auto ret = taskRunner->StartTask(task, action);
        EXPECT_EQ(1, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartTask002 ERROR";
    }

    GTEST_LOG_(INFO) << "StartTask002 End";
}

/**
 * @tc.name: CommitTask001
 * @tc.desc: Verify the CommitTask001 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, CommitTask001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommitTask001 Begin";
    try {
        auto handler = std::make_shared<DataHandlerMock>(USER_ID, BUND_NAME, TABLE_NAME);
        auto context = std::make_shared<TaskContext>(handler);
        TaskAction action = Action;
        auto task = make_shared<Task>(context, action);
        std::function<void()> callBack = CallBack;
        auto taskRunner = make_shared<TaskRunner>(callBack);
        taskRunner->stopFlag_ = make_shared<bool>(false);
        taskRunner->AddTask(task);
        taskRunner->SetCommitFunc(CommitFuncFail);
        auto ret = taskRunner->CommitTask(task);
        EXPECT_EQ(1, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CommitTask001 ERROR";
    }

    GTEST_LOG_(INFO) << "CommitTask001 End";
}

/**
 * @tc.name: CommitTask002
 * @tc.desc: Verify the CommitTask002 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, CommitTask002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommitTask002 Begin";
    try {
        auto handler = std::make_shared<DataHandlerMock>(USER_ID, BUND_NAME, TABLE_NAME);
        auto context = std::make_shared<TaskContext>(handler);
        TaskAction action = Action;
        auto task = make_shared<Task>(context, action);
        std::function<void()> callBack = CallBack;
        auto taskRunner = make_shared<TaskRunner>(callBack);
        taskRunner->stopFlag_ = make_shared<bool>(false);
        taskRunner->AddTask(task);
        taskRunner->SetCommitFunc(CommitFuncSuccess);
        auto ret = taskRunner->CommitTask(task);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CommitTask002 ERROR";
    }

    GTEST_LOG_(INFO) << "CommitTask002 End";
}

/**
 * @tc.name: CompleteTask
 * @tc.desc: Verify the CompleteTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, CompleteTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompleteTask Begin";
    try {
        auto handler = std::make_shared<DataHandlerMock>(USER_ID, BUND_NAME, TABLE_NAME);
        auto context = std::make_shared<TaskContext>(handler);
        TaskAction action = Action;
        auto task = make_shared<Task>(context, action);
        task->SetId(0);

        std::function<void()> callBack = CallBack;
        TaskRunner taskRunner(callBack);
        taskRunner.stopFlag_ = make_shared<bool>(true);
        taskRunner.CompleteTask(0);
        EXPECT_TRUE(true);
        *taskRunner.stopFlag_ = false;
        taskRunner.CompleteTask(0);
        EXPECT_TRUE(true);
        taskRunner.taskList_.push_back(task);
        taskRunner.CompleteTask(0);
        EXPECT_TRUE(true);
        taskRunner.CompleteTask(1);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CompleteTask ERROR";
    }

    GTEST_LOG_(INFO) << "CompleteTask End";
}

/**
 * @tc.name: ReleaseTask
 * @tc.desc: Verify the ReleaseTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, ReleaseTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTask Begin";
    try {
        auto handler = std::make_shared<DataHandlerMock>(USER_ID, BUND_NAME, TABLE_NAME);
        auto context = std::make_shared<TaskContext>(handler);
        TaskAction action = Action;
        auto task = make_shared<Task>(context, action);

        std::function<void()> callBack = CallBack;
        TaskRunner taskRunner(callBack);
        taskRunner.taskList_.push_back(task);
        auto ret = taskRunner.ReleaseTask();
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReleaseTask ERROR";
    }

    GTEST_LOG_(INFO) << "ReleaseTask End";
}

/**
 * @tc.name: Reset
 * @tc.desc: Verify the Reset function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, Reset, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Reset Begin";
    try {
        std::function<void()> callBack = CallBack;
        TaskRunner taskRunner(callBack);
        taskRunner.Reset();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Reset ERROR";
    }

    GTEST_LOG_(INFO) << "Reset End";
}

/**
 * @tc.name: CommitDummyTask
 * @tc.desc: Verify the CommitDummyTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, CommitDummyTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CommitDummyTask Begin";
    try {
        std::function<void()> callBack = CallBack;
        TaskRunner taskRunner(callBack);
        taskRunner.CommitDummyTask();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CommitDummyTask ERROR";
    }

    GTEST_LOG_(INFO) << "CommitDummyTask End";
}

/**
 * @tc.name: CompleteDummyTask
 * @tc.desc: Verify the CompleteDummyTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, CompleteDummyTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompleteDummyTask Begin";
    try {
        std::function<void()> callBack = CallBack;
        TaskRunner taskRunner(callBack);
        taskRunner.stopFlag_ = make_shared<bool>(false);
        taskRunner.CompleteDummyTask();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CompleteDummyTask ERROR";
    }

    GTEST_LOG_(INFO) << "CompleteDummyTask End";
}

/**
 * @tc.name: AllocRunner
 * @tc.desc: Verify the AllocRunner function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, AllocRunner, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocRunner Begin";
    try {
        TaskManager taskManager;
        int32_t userId = 0;
        std::string bundleName = "bundleName";
        function<void()> callBack = CallBack;
        auto taskRunner = taskManager.AllocRunner(userId, bundleName, callBack);
        EXPECT_TRUE(taskRunner != nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AllocRunner ERROR";
    }

    GTEST_LOG_(INFO) << "AllocRunner End";
}

/**
 * @tc.name: ReleaseRunner
 * @tc.desc: Verify the ReleaseRunner function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, ReleaseRunner, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseRunner Begin";
    try {
        TaskManager taskManager;
        int32_t userId = 0;
        std::string bundleName = "bundleName";
        taskManager.ReleaseRunner(userId, bundleName);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReleaseRunner ERROR";
    }

    GTEST_LOG_(INFO) << "ReleaseRunner End";
}

/**
 * @tc.name: GetRunner
 * @tc.desc: Verify the GetRunner function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, GetRunner, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRunner Begin";
    try {
        TaskManager taskManager;
        int32_t userId = 0;
        std::string bundleName = "bundleName";
        auto taskRunner = taskManager.GetRunner(userId, bundleName);
        EXPECT_TRUE(taskRunner == nullptr);

        std::string key = "key";
        std::function<void()> callBack = CallBack;
        auto value = make_shared<TaskRunner>(callBack);
        taskManager.map_.insert({key, value});
        taskRunner = taskManager.GetRunner(userId, bundleName);
        EXPECT_TRUE(taskRunner == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetRunner ERROR";
    }

    GTEST_LOG_(INFO) << "GetRunner End";
}

/**
 * @tc.name: InitRunner
 * @tc.desc: Verify the InitRunner function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, InitRunner, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitRunner Begin";
    try {
        TaskManager taskManager;
        std::function<void()> callBack = CallBack;
        TaskRunner taskRunner(callBack);
        taskManager.InitRunner(taskRunner);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " InitRunner ERROR";
    }

    GTEST_LOG_(INFO) << "InitRunner End";
}

/**
 * @tc.name: TaskManager_CommitTask
 * @tc.desc: Verify the TaskManager_CommitTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, TaskManager_CommitTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskManager_CommitTask Begin";
    try {
        auto handler = std::make_shared<DataHandlerMock>(USER_ID, BUND_NAME, TABLE_NAME);
        auto context = std::make_shared<TaskContext>(handler);
        TaskAction action = Action;
        auto task = make_shared<Task>(context, action);
        std::function<void()> callBack = CallBack;
        auto taskRunner = make_shared<TaskRunner>(callBack);
        TaskManager taskManager;
        taskManager.CommitTask(taskRunner, task);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " TaskManager_CommitTask ERROR";
    }

    GTEST_LOG_(INFO) << "TaskManager_CommitTask End";
}

/**
 * @tc.name: GetKey
 * @tc.desc: Verify the GetKey function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(TaskTest, GetKey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetKey Begin";
    try {
        TaskManager taskManager;
        int32_t userId = 0;
        string bundleName = "bundleName";
        auto ret = taskManager.GetKey(userId, bundleName);
        EXPECT_EQ(ret, "0bundleName");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetKey ERROR";
    }

    GTEST_LOG_(INFO) << "GetKey End";
}
} // namespace OHOS::FileManagement::CloudSync::Test
