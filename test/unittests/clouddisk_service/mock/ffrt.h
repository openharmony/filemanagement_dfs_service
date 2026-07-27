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

#ifndef FFRT_API_FFRT_H
#define FFRT_API_FFRT_H

#include <cinttypes>
#include <functional>
#include <string>
#include <thread>
#include <mutex>

typedef void (*ffrt_timer_cb)(void* data);
typedef int ffrt_timer_t;
typedef int ffrt_qos_t;

typedef enum {
    ffrt_qos_inherit = -1,
    ffrt_qos_background,
    ffrt_qos_utility,
    ffrt_qos_default,
    ffrt_qos_user_initiated,
    ffrt_qos_deadline_request,
    ffrt_qos_user_interactive,
    ffrt_qos_max = ffrt_qos_user_interactive,
} ffrt_qos_default_t;

ffrt_timer_t ffrt_timer_start(ffrt_qos_t qos, uint64_t timeout, void *data, ffrt_timer_cb cb, bool repeat);
int ffrt_timer_stop(ffrt_qos_t qos, ffrt_timer_t handle);

namespace ffrt {
class mutex {
public:
    mutex() = default;
    ~mutex() = default;

    mutex(const mutex&) = delete;
    void operator=(const mutex&) = delete;

    inline bool try_lock()
    {
        return mutex_.try_lock();
    }

    inline void lock()
    {
        mutex_.lock();
    }

    inline void unlock()
    {
        mutex_.unlock();
    }

private:
    std::mutex mutex_;
};

struct task_attr {
    inline task_attr &qos(int32_t) { return *this;}
    inline task_attr &name(const std::string &) { return *this; }
    inline task_attr &delay(int64_t) { return *this; }
    inline task_attr &timeout(int64_t) { return *this; }
};
struct task_handle {};

inline void submit(std::function<void()> &&task, task_attr attr = {})
{
    if (task) {
    }
}

inline task_handle submit_h(std::function<void()> &&task)
{
    return task_handle{};
}

inline void wait(std::vector<task_handle> taskHandles) {}
}  // namespace ffrt
#endif // FFRT_API_FFRT_H