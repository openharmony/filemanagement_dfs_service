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

#include <functional>
#include <chrono>

#include "dfs_error.h"
#include "utils_log.h"
#include "sdk_helper.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

TaskManager::TaskManager(function<void()> callback) : callback_(callback)
{
    pool_.SetMaxTaskNum(MAX_THREAD_NUM);
    pool_.Start(MAX_THREAD_NUM);
}

TaskManager::~TaskManager()
{
    pool_.Stop();
}

int32_t TaskManager::GenerateTaskId()
{
    return currentId_.fetch_add(1);
}

int32_t TaskManager::AddTask(shared_ptr<Task> t)
{
    unique_lock<mutex> lock(mutex_);

    /* If stopped, no more tasks can be added */
    if (stopFlag_) {
        LOGI("add task fail since stop");
        return E_STOP;
    }

    /* insert task */
    t->SetId(GenerateTaskId());
    taskList_.emplace_back(t);

    return E_OK;
}

int32_t TaskManager::StartTask(shared_ptr<Task> t, TaskAction action)
{
    /* If stopped, no more tasks can be executed */
    if (stopFlag_) {
        LOGI("start task fail since stop");
        CompleteTask(t->GetId());
        return E_STOP;
    }

    t->SetAction(action);
    pool_.AddTask([t, this]() {
        t->Run();
        this->CompleteTask(t->GetId());
    });
    return E_OK;
}

int32_t TaskManager::CommitTask(shared_ptr<Task> t)
{
    /* add task */
    int32_t ret = AddTask(t);
    if (ret != E_OK) {
        LOGE("add task err %{public}d", ret);
        return ret;
    }

    /* launch */
    pool_.AddTask([t, this]() {
        t->Run();
        this->CompleteTask(t->GetId());
    });

    return E_OK;
}

void TaskManager::CompleteTask(int32_t id)
{
    /* remove task */
    unique_lock<mutex> lock(mutex_);
    for (auto entry = taskList_.begin(); entry != taskList_.end();) {
        if (entry->get()->GetId() == id) {
            (void)taskList_.erase(entry);
            break;
        } else {
            entry++;
        }
    }

    if (taskList_.empty()) {
        if (stopFlag_) {
            /* if it has been stopped, notify the blocking caller */
            cv_.notify_all();
        } else {
            lock.unlock();
            Reset();
            /* otherwise just run its callback */
            callback_();
        }
    }
}

bool TaskManager::StopAndWaitFor()
{
    unique_lock<mutex> lock(mutex_);
    LOGI("task manager stop");
    stopFlag_ = true;

    return cv_.wait_for(lock, chrono::seconds(WAIT_FOR_SECOND), [this] {
        return taskList_.empty();
    });
}

void TaskManager::Reset()
{
    currentId_.store(0);
}

void TaskManager::CommitDummyTask()
{
    auto task = make_shared<Task>(nullptr, nullptr);
    task->SetId(INVALID_ID);

    unique_lock<mutex> lock(mutex_);
    taskList_.emplace_back(task);
}

void TaskManager::CompleteDummyTask()
{
    CompleteTask(INVALID_ID);
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
