/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef NATIVE_MODULE_SEND_FILE_EVENT_AGENT_H
#define NATIVE_MODULE_SEND_FILE_EVENT_AGENT_H

#include <condition_variable>
#include <list>
#include <map>
#include <mutex>
#include <pthread.h>
#include <set>

#include "trans_event.h"
#include "uv.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
    constexpr int32_t LISTENER_TYPTE_MAX_LENGTH = 64;
}

class EventAgent {
public:
    EventAgent(napi_env env, napi_value thisVar);
    virtual ~EventAgent();

    void On(const char* type, napi_value handler);
    void Off(const char* type, napi_value handler);
    void Off(const char* type);
    void Emit(const char* type, Event* event);
    void InsertEvent(std::shared_ptr<TransEvent>);
    bool IsEventListEmpty() { return eventList_.empty(); }
    void WaitEvent();
    void InsertDevice(const std::string&);
    void RemoveDevice(const std::string&);
    bool FindDevice(const std::string&);
    void ClearDevice();

protected:
    static void* ThreadProc(void* arg);
    static void Callback(uv_work_t *work);
    static void AfterCallback(uv_work_t *work, int status);

    std::mutex deviceListMut_;
    std::set<std::string> deviceList_;
    std::mutex eventListMut_;
    std::list<std::shared_ptr<TransEvent>> eventList_;
    std::mutex listenerMapMut_;
    std::map<std::string, napi_ref> listenerMap_;
    napi_env env_;
    napi_ref thisVarRef_;
    std::mutex getEventCVMut_;
    std::condition_variable getEventCV_;
    pthread_t threadId_ {0};
    uv_loop_t *loop_ {nullptr};
    bool isExit_ {false};
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif /* NATIVE_MODULE_SEND_FILE_EVENT_AGENT_H */