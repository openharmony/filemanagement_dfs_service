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

#ifndef OHOS_CLOUD_SYNC_SERVICE_TASK_H
#define OHOS_CLOUD_SYNC_SERVICE_TASK_H

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "thread_pool.h"

#include "data_handler.h"
#include "sdk_helper.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class TaskContext : public DriveKit::DKContext {
public:
    TaskContext(std::shared_ptr<DataHandler> handler) : handler_(handler) {}

    std::shared_ptr<DataHandler> GetHandler()
    {
        return handler_;
    }

private:
    std::shared_ptr<DataHandler> handler_;
};

constexpr int32_t INVALID_ID = -1;

using TaskAction = std::function<void(std::shared_ptr<TaskContext>)>;

class Task {
public:
    Task(std::shared_ptr<TaskContext> context, TaskAction action) : id_(INVALID_ID),
        context_(context), action_(action) {}
    virtual ~Task() = default;

    virtual void Run()
    {
        action_(context_);
    }

    void SetAction(TaskAction action)
    {
        action_ = action;
    }

    void SetId(int32_t id)
    {
        id_ = id;
    }

    int32_t GetId()
    {
        return id_;
    }

private:
    int32_t id_;
    std::shared_ptr<TaskContext> context_;
    TaskAction action_;
};

class TaskManager {
public:
    TaskManager(std::function<void()> callback);
    virtual ~TaskManager();

    int32_t AddTask(std::shared_ptr<Task> t);
    int32_t StartTask(std::shared_ptr<Task> t, TaskAction action);
    int32_t CommitTask(std::shared_ptr<Task> t);
    void CompleteTask(int32_t id);

    /* dummy */
    void CommitDummyTask();
    void CompleteDummyTask();

    /* reset and stop */
    void Reset();
    bool StopAndWaitFor();

private:
    int32_t GenerateTaskId();

    /* thread pool */
    ThreadPool pool_ = ThreadPool("TaskManager");
    const int32_t MAX_THREAD_NUM = 4;
    const int32_t WAIT_FOR_SECOND = 30;

    /* stop */
    bool stopFlag_ = false;

    /* id */
    std::atomic<int32_t> currentId_ = 0;

    /* task list */
    std::mutex mutex_;
    std::list<std::shared_ptr<Task>> taskList_;
    /* specific cv for shared_mutex */
    std::condition_variable cv_;

    /* callback */
    std::function<void()> callback_;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_TASK_H
