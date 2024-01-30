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

#include "task.h"

#include <chrono>
#include <functional>
#include <gtest/gtest.h>

#include "sdk_helper.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

/* task runner */
TaskRunner::TaskRunner(function<void()> callback) : callback_(callback) {}

TaskRunner::~TaskRunner() {}

int32_t TaskRunner::GenerateTaskId()
{
    return currentId_.fetch_add(1);
}

int32_t TaskRunner::AddTask(shared_ptr<Task> t)
{
    return E_OK;
}

int32_t TaskRunner::StartTask(shared_ptr<Task> t, TaskAction action)
{
    GTEST_LOG_(INFO) << "StartTask Start";
    return E_OK;
}

int32_t TaskRunner::CommitTask(shared_ptr<Task> t)
{
    GTEST_LOG_(INFO) << "CommitTask Start";
    return E_OK;
}

void TaskRunner::CompleteTask(int32_t id) {}

bool TaskRunner::ReleaseTask()
{
    return false;
}

void TaskRunner::SetStopFlag(std::shared_ptr<bool> stopFlag)
{
}

void TaskRunner::Reset()
{
    currentId_.store(0);
}

void TaskRunner::SetCommitFunc(function<int32_t(shared_ptr<TaskRunner>, shared_ptr<Task>)> func) {}

void TaskRunner::CommitDummyTask() {}

void TaskRunner::CompleteDummyTask() {}

/* TaskManager */
TaskManager::TaskManager() {}

TaskManager::~TaskManager() {}

shared_ptr<TaskRunner> TaskManager::AllocRunner(int32_t userId,
                                                const std::string &bundleName,
                                                function<void()> callback)
{
    string key = GetKey(userId, bundleName);
    unique_lock wlock(mapMutex_);
    if (map_.find(key) == map_.end()) {
        auto runner = make_shared<TaskRunner>(callback);
        InitRunner(*runner);
        map_.insert({key, runner});
    }
    return map_[key];
}

void TaskManager::ReleaseRunner(int32_t userId, const std::string &bundleName)
{
    string key = GetKey(userId, bundleName);
    unique_lock wlock(mapMutex_);
    map_.erase(key);
}

shared_ptr<TaskRunner> TaskManager::GetRunner(int32_t userId, const std::string &bundleName)
{
    string key = GetKey(userId, bundleName);
    shared_lock rlock(mapMutex_);
    if (map_.find(key) == map_.end()) {
        return nullptr;
    }
    return map_[key];
}

void TaskManager::InitRunner(TaskRunner &runner)
{
    runner.SetCommitFunc(bind(&TaskManager::CommitTask, this, placeholders::_1, placeholders::_2));
}

int32_t TaskManager::CommitTask(shared_ptr<TaskRunner> runner, shared_ptr<Task> t)
{
    GTEST_LOG_(INFO) << "CommitTask Start";
    return E_OK;
}

string TaskManager::GetKey(int32_t userId, const string &bundleName)
{
    return to_string(userId) + bundleName;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
