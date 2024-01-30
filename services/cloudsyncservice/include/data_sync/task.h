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
#include <nocopyable.h>
#include <singleton.h>
#include <shared_mutex>
#include <vector>

#include "thread_pool.h"

#include "data_handler.h"
#include "dfs_error.h"
#include "sdk_helper.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class TaskContext : public DriveKit::DKContext {
public:
    TaskContext(std::shared_ptr<DataHandler> handler) : handler_(handler),
        assets_(0)
    {
    }

    std::shared_ptr<DataHandler> GetHandler()
    {
        return handler_;
    }

    void SetAssets(std::vector<DriveKit::DKDownloadAsset> &assets)
    {
        assets_ = std::move(assets);
    }

    std::vector<DriveKit::DKDownloadAsset> GetAssets()
    {
        return std::move(assets_);
    }

private:
    std::shared_ptr<DataHandler> handler_;
    /* not final, might put this in its derived class */
    std::vector<DriveKit::DKDownloadAsset> assets_;
};

class DownloadTaskContext : public TaskContext {
public:
    DownloadTaskContext(std::shared_ptr<DataHandler> handler, int32_t batchNo)
        : TaskContext(std::move(handler)), batchNo_(batchNo) {}

    int32_t GetBatchNo() const
    {
        return batchNo_;
    }

private:
    int32_t batchNo_;
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

class TaskRunner : public std::enable_shared_from_this<TaskRunner> {
public:
    TaskRunner(std::function<void()> callback);
    virtual ~TaskRunner();

    /* async */
    template<typename T>
    int32_t AsyncRun(std::shared_ptr<TaskContext> context, void(T::*f)(std::shared_ptr<TaskContext>),
        T *ptr);
    template<typename T, typename RET, typename... ARGS>
    std::function<RET(ARGS...)> AsyncCallback(RET(T::*f)(ARGS...), T *ptr);

    /* dummy */
    void CommitDummyTask();
    void CompleteDummyTask();

    /* reset and stop */
    void Reset();
    bool ReleaseTask();

    bool NeedRun(std::shared_ptr<Task> t);

    std::shared_ptr<bool> GetStopFlag();
    void SetStopFlag(std::shared_ptr<bool> stopFlag);

private:
    int32_t GenerateTaskId();

    /* task operations */
    int32_t AddTask(std::shared_ptr<Task> t);
    int32_t StartTask(std::shared_ptr<Task> t, TaskAction action);
    int32_t CommitTask(std::shared_ptr<Task> t);
    void CompleteTask(int32_t id);

    /* set commit func */
    friend class TaskManager;
    void SetCommitFunc(std::function<int32_t(std::shared_ptr<TaskRunner>,
        std::shared_ptr<Task>)> func);
    std::function<int32_t(std::shared_ptr<TaskRunner>, std::shared_ptr<Task>)> commitFunc_;

    /* stop */
    std::shared_ptr<bool> stopFlag_ = nullptr;

    /* id */
    std::atomic<int32_t> currentId_ = 0;

    /* task list */
    std::mutex mutex_;
    std::list<std::shared_ptr<Task>> taskList_;

    /* data syncer callback */
    std::function<void()> callback_;
};

template<typename T>
inline int32_t TaskRunner::AsyncRun(std::shared_ptr<TaskContext> context,
    void(T::*f)(std::shared_ptr<TaskContext>), T *ptr)
{
    std::shared_ptr<Task> task = std::make_shared<Task>(context,
        [ptr, f](std::shared_ptr<TaskContext> ctx) {
            (ptr->*f)(ctx);
        }
    );

    int32_t ret = CommitTask(task);
    if (ret != E_OK) {
        LOGE("async run commit task err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

/*
 * About ARGS...
 * <1> async execute requires value-copy or shared_ptr like input parameters,
 *     but no reference for lifecycle consideration.
 * <2> In addition, [=] requires the wrapped function with const parameters.
 */
template<typename T, typename RET, typename... ARGS>
inline std::function<RET(ARGS...)> TaskRunner::AsyncCallback(RET(T::*f)(ARGS...), T *ptr)
{
    std::shared_ptr<Task> task = std::make_shared<Task>(nullptr, nullptr);

    int32_t ret = AddTask(task);
    if (ret != E_OK) {
        LOGE("async callback add task err %{public}d", ret);
        return nullptr;
    }

    return [ptr, f, task, this](ARGS... args) -> RET {
        int32_t ret = this->StartTask(task, [ptr, f, args...](std::shared_ptr<TaskContext>) {
            (ptr->*f)(args...);
        });
        if (ret != E_OK) {
            LOGE("async callback start task err %{public}d", ret);
        }
    };
}

class TaskManager : public NoCopyable {
DECLARE_DELAYED_SINGLETON(TaskManager);

public:
    std::shared_ptr<TaskRunner> AllocRunner(int32_t userId, const std::string &bundleName,
        std::function<void()> callback);
    std::shared_ptr<TaskRunner> GetRunner(int32_t userId, const std::string &bundleName);
    void ReleaseRunner(int32_t userId, const std::string &bundleName);

    int32_t CommitTask(std::shared_ptr<TaskRunner> runner, std::shared_ptr<Task> t);

private:
    std::string GetKey(int32_t userId, const std::string &bundleName);
    void InitRunner(TaskRunner &runner);

    /* runners */
    std::unordered_map<std::string, std::shared_ptr<TaskRunner>> map_;
    std::shared_mutex mapMutex_;

    /* thread pool */
    ThreadPool pool_ = ThreadPool("TaskManager");
    const int32_t MAX_THREAD_NUM = 12;
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_TASK_H
