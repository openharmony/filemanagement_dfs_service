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

#include "dfsu_memory_guard.h"
#include "sdk_helper.h"
#include "xcollie_helper.h"

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
    unique_lock<mutex> lock(mutex_);

    /* insert task */
    t->SetId(GenerateTaskId());
    taskList_.emplace_back(t);

    return E_OK;
}

int32_t TaskRunner::StartTask(shared_ptr<Task> t, TaskAction action)
{
    /*
     * Try to execute the previous callback even in stop process.
     * Yet no new task could be added in the callback.
     */
    LOGI("StartTask begin, Task id: %{public}d", t->GetId());
    t->SetAction(action);
    int32_t ret = commitFunc_(shared_from_this(), t);
    if (ret != E_OK) {
        LOGE("commit task err %{public}d", ret);
        return ret;
    }
    LOGI("StartTask end, Task id: %{public}d", t->GetId());
    return E_OK;
}

int32_t TaskRunner::CommitTask(shared_ptr<Task> t)
{
    /* add task */
    int32_t ret = AddTask(t);
    if (ret != E_OK) {
        LOGE("add task err %{public}d", ret);
        return ret;
    }

    LOGI("start CommitTask, Task id: %{public}d", t->GetId());
    /* launch */
    ret = commitFunc_(shared_from_this(), t);
    if (ret != E_OK) {
        LOGE("commit task err %{public}d", ret);
        return ret;
    }

    LOGI("CommitTask success,Task id: %{public}d", t->GetId());
    return E_OK;
}

void TaskRunner::CompleteTask(int32_t id)
{
    /* remove task */
    unique_lock<mutex> lock(mutex_);
    const int32_t DFX_DELAY_S = 60;
    int32_t xcollieId = XCollieHelper::SetTimer("CloudSyncService_CompleteTask", DFX_DELAY_S, nullptr, nullptr, true);
    for (auto entry = taskList_.begin(); entry != taskList_.end();) {
        if (entry->get()->GetId() == id) {
            (void)taskList_.erase(entry);
            break;
        } else {
            entry++;
        }
    }

    LOGI("taskList size: %{public}zu, Task id: %{public}d", taskList_.size(), id);
    if (taskList_.empty()) {
        if (!(*stopFlag_)) {
            lock.unlock();
            /* otherwise just run its callback */
            callback_();
        }
    }
    XCollieHelper::CancelTimer(xcollieId);
}

bool TaskRunner::ReleaseTask()
{
    unique_lock<mutex> lock(mutex_);
    LOGI("task manager stop");
    taskList_.clear();
    return taskList_.empty();
}

void TaskRunner::Reset()
{
    currentId_.store(0);
}

void TaskRunner::SetCommitFunc(function<int32_t(shared_ptr<TaskRunner>, shared_ptr<Task>)> func)
{
    commitFunc_ = func;
}

void TaskRunner::CommitDummyTask()
{
    auto task = make_shared<Task>(nullptr, nullptr);
    task->SetId(INVALID_ID);

    unique_lock<mutex> lock(mutex_);
    taskList_.emplace_back(task);
}

void TaskRunner::CompleteDummyTask()
{
    CompleteTask(INVALID_ID);
}

std::shared_ptr<bool> TaskRunner::GetStopFlag()
{
    return stopFlag_;
}

void TaskRunner::SetStopFlag(std::shared_ptr<bool> stopFlag)
{
    stopFlag_ = stopFlag;
}

bool TaskRunner::NeedRun(shared_ptr<Task> t)
{
    unique_lock<mutex> lock(mutex_);
    for (auto it = taskList_.begin(); it != taskList_.end();) {
        if (it->get()->GetId() == t->GetId()) {
            return true;
        } else {
            it++;
        }
    }
    return false;
}

/* TaskManager */
TaskManager::TaskManager()
{
    pool_.SetMaxTaskNum(MAX_THREAD_NUM);
    pool_.Start(MAX_THREAD_NUM);
}

TaskManager::~TaskManager()
{
    pool_.Stop();
}

shared_ptr<TaskRunner> TaskManager::AllocRunner(int32_t userId, const std::string &bundleName,
                                                function<void()> callback)
{
    string key = GetKey(userId, bundleName);
    unique_lock wlock(mapMutex_);
    if (map_.find(key) == map_.end()) {
        auto runner = make_shared<TaskRunner>(callback);
        InitRunner(*runner);
        map_.insert({ key, runner });
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

string TaskManager::GetKey(int32_t userId, const string &bundleName)
{
    return to_string(userId) + bundleName;
}

int32_t TaskManager::CommitTask(shared_ptr<TaskRunner> runner, shared_ptr<Task> t)
{
    LOGI("CommitTask begin, Task id: %{public}d", t->GetId());
    if (runner->NeedRun(t)) {
        LOGD("task need run");
        pool_.AddTask([t, runner]() {
            DfsuMemoryGuard cacheGuard;
            t->Run();
            runner->CompleteTask(t->GetId());
        });
    }
    LOGI("CommitTask end, Task id: %{public}d", t->GetId());
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
