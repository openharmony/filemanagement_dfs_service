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

#include "event_agent.h"

#include "securec.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
EventAgent::EventAgent(napi_env env, napi_value thisVar) : env_(env)
{
    napi_create_reference(env, thisVar, 1, &thisVarRef_);
    napi_get_uv_event_loop(env, &loop_);
    pthread_create(&threadId_, nullptr, ThreadProc, this);
}

EventAgent::~EventAgent()
{
    ClearDevice();
    eventList_.clear();
    LOGD("~EventAgent: Deconstruction ---- 1.");

    {
        std::unique_lock<std::mutex> lock(listenerMapMut_);
        for (auto it = listenerMap_.begin(); it != listenerMap_.end();) {
            napi_delete_reference(env_, it->second);
            it = listenerMap_.erase(it);
        }
    }
    napi_delete_reference(env_, thisVarRef_);

    isExit_ = true;
    {
        std::unique_lock<std::mutex> lock(getEventCVMut_);
        getEventCV_.notify_one();
    }
    void* threadResult = nullptr;
    LOGD("~EventAgent: Deconstruction ---- 2. eventList_ count %{public}d, %{public}d", eventList_.size(), deviceList_.size());
    pthread_join(threadId_, &threadResult);
    // if (loop_ != nullptr) {
    //     uv_stop(loop_);
    // }
    LOGD("~EventAgent: Deconstruction ---- 3.");
}

void EventAgent::On(const char* type, napi_value handler)
{
    LOGD("SendFile EventAgent::On thread tid = %{public}lu\n", (unsigned long)pthread_self());
    if (type == nullptr || LISTENER_TYPTE_MAX_LENGTH == strnlen(type, LISTENER_TYPTE_MAX_LENGTH)) {
        LOGE("SendFile EventAgent::On: event type exceed 64 byte or empty.");
        return;
    }

    std::string eventName(type);
    napi_ref handlerRef;
    napi_create_reference(env_, handler, 1, &handlerRef);
    LOGD("SendFile EventAgent::On: handlerRef[1] = %{public}p.", handlerRef);
    {
        std::unique_lock<std::mutex> lock(listenerMapMut_);
        listenerMap_.insert(make_pair(eventName, handlerRef));
        LOGD("SendFile EventAgent::On: handlerRef[2] = %{public}p.", listenerMap_[eventName]);
    }
}

void EventAgent::Off(const char* type)
{
    LOGD("SendFile EventAgent::Off: JS callback unregister.\n");
    if (type == nullptr || LISTENER_TYPTE_MAX_LENGTH == strnlen(type, LISTENER_TYPTE_MAX_LENGTH)) {
        LOGE("SendFile EventAgent::Off: event type exceed 64 byte or empty.");
        return;
    }

    std::string eventName(type);
    napi_delete_reference(env_, listenerMap_[eventName]);
    {
        std::unique_lock<std::mutex> lock(listenerMapMut_);
        listenerMap_.erase(eventName);
    }
}

void EventAgent::InsertEvent(std::shared_ptr<TransEvent> event)
{
    LOGD("SendFile: EventAgent::InsertEvent.");
    {
        std::unique_lock<std::mutex> lock(eventListMut_);
        eventList_.push_back(event);
    }

    std::unique_lock<std::mutex> lock(getEventCVMut_);
    getEventCV_.notify_one();
}

void EventAgent::WaitEvent()
{
    std::unique_lock<std::mutex> lock(getEventCVMut_);
    getEventCV_.wait(lock, [this]() { return (!this->IsEventListEmpty() || this->isExit_); });
}

void EventAgent::InsertDevice(const std::string& deviceId)
{
    if (deviceList_.find(deviceId) == deviceList_.end()) {
        std::unique_lock<std::mutex> lock(deviceListMut_);
        deviceList_.insert(deviceId);
    }
}

void EventAgent::RemoveDevice(const std::string& deviceId)
{
    if (deviceList_.find(deviceId) != deviceList_.end()) {
        std::unique_lock<std::mutex> lock(deviceListMut_);
        deviceList_.erase(deviceId);
    }
}

bool EventAgent::FindDevice(const std::string& deviceId)
{
    return (deviceList_.find(deviceId) != deviceList_.end());
}

void EventAgent::ClearDevice()
{
    deviceList_.clear();
}

void* EventAgent::ThreadProc(void* arg)
{
    // auto thisVar = reinterpret_cast<EventAgent*>(arg);
    auto thisVar = (EventAgent*)(arg);
    LOGD("ThreadProc thisVar %{public}p.", thisVar);
    do {
        thisVar->WaitEvent();
        LOGD("SendFile event daemon thread loop.");
        uv_work_t* work = new uv_work_t();
        LOGD("daemon ThreadProc thisVar %{public}p.", thisVar);
        work->data = thisVar;
        uv_queue_work(thisVar->loop_, work, Callback, AfterCallback);
    } while (!thisVar->isExit_);
    LOGD("SendFile event daemon thread exit.");
    return nullptr;
}

void EventAgent::Callback(uv_work_t *work)
{
    //auto agent = reinterpret_cast<EventAgent*>(work->data);
    //agent->WaitEvent();
    LOGD("SendFile EventAgent::Callback event daemon thread receive an event.");
    //LOGD("SendFile EventAgent::Callback thread lwpid = %{public}u\n", syscall(SYS_gettid));
    LOGD("SendFile EventAgent::Callback thread tid = %{public}lu\n", (unsigned long)pthread_self());
}

void EventAgent::AfterCallback(uv_work_t *work, int status)
{
    LOGD("SendFile EventAgent::AfterCallback: entered.");
    auto agent = (EventAgent*)(work->data);
    if (agent == nullptr) {
        LOGE("SendFile return #1].");
        return;
    }

    LOGD("SendFile EventAgent::AfterCallback: event num %{public}d", agent->eventList_.size());
    if (agent->eventList_.empty()) {
        LOGE("SendFile return #2].");
        return;
    }

    auto event = agent->eventList_.front();
    if (event.get() == nullptr) {
        LOGE("AfterCallback event pointer is empty.");
        return;
    }

    napi_ref handlerRef;
    if (agent->listenerMap_[event.get()->GetName()] == nullptr) {
        return ;
    } else {
        handlerRef = agent->listenerMap_[event.get()->GetName()];
    }

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(agent->env_, &scope);

    napi_value thisVar = nullptr;
    napi_get_reference_value(agent->env_, agent->thisVarRef_, &thisVar);

    napi_value jsEvent = event.get()->ToJsObject(agent->env_);
    napi_value handler = nullptr;
    napi_value callbackResult = nullptr;
    napi_value result = nullptr;
    napi_get_undefined(agent->env_, &result);
    napi_value callbackValues[2] = {0};
    callbackValues[0] = result;
    callbackValues[1] = jsEvent;

    napi_get_reference_value(agent->env_, handlerRef, &handler);
    napi_call_function(agent->env_, thisVar, handler, std::size(callbackValues),
        callbackValues, &callbackResult);
    LOGD("SendFile EventAgent::AfterCallback: listener type[%{public}s].", event.get()->GetName().c_str());
    LOGD("SendFile EventAgent::AfterCallback thread tid = %{public}lu\n", (unsigned long)pthread_self());

    napi_close_handle_scope(agent->env_, scope);
    agent->eventList_.pop_front();
    delete work;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS