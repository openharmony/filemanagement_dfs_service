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
EventAgent::EventAgent(napi_env env, napi_value thisVar)
    : env_(env), first_(nullptr), last_(nullptr)
{
    deviceList_.clear();
    eventList_.clear();
    napi_create_reference(env, thisVar, 1, &thisVarRef_);
    napi_get_uv_event_loop(env, &loop_);
    pthread_create(&threadId_, nullptr, ThreadProc, this);
}

EventAgent::~EventAgent()
{
    ClearDevice();
    {
        std::unique_lock<std::mutex> lock(eventListMut_);
        for (auto event : eventList_) {
            if (event != nullptr) {
                delete event;
            }
        }
        eventList_.clear();
    }

    EventListener* temp = nullptr;
    for (EventListener* iter = first_; iter != nullptr; iter = temp) {
        temp = iter->next;
        if (iter == first_) {
            first_ = first_->next;
        } else if (iter == last_) {
            last_ = last_->back;
        } else {
            iter->next->back = iter->back;
            iter->back->next = iter->next;
        }
        napi_delete_reference(env_, iter->handlerRef);
        delete iter;
    }
    napi_delete_reference(env_, thisVarRef_);
    isExit_ = true;
    {
        std::unique_lock<std::mutex> lock(getEventCVMut_);
        getEventCV_.notify_one();
    }
    void* threadResult = nullptr;
    LOGD("~EventAgent: Deconstruction %{public}d, %{public}d", eventList_.size(), deviceList_.size());
    pthread_join(threadId_, &threadResult);
}

void EventAgent::On(const char* type, napi_value handler)
{
    auto tmp = new EventListener();

    LOGD("SendFile EventAgent::On: JS callback register.\n");
    if (strncpy_s(tmp->type, LISTENER_TYPTE_MAX_LENGTH, type, strlen(type)) == -1) {
        delete tmp;
        tmp = nullptr;
        return;
    }

    if (first_ == nullptr) {
        first_ = last_ = tmp;
    } else {
        last_->next = tmp;
        last_->next->back = last_;
        last_ = last_->next;
    }
    last_->isOnce = false;
    napi_create_reference(env_, handler, 1, &last_->handlerRef);
}

void EventAgent::Off(const char* type)
{
    EventListener* temp = nullptr;
    LOGD("SendFile EventAgent::Off: JS callback unregister.\n");
    for (EventListener* eventListener = first_; eventListener != nullptr; eventListener = temp) {
        temp = eventListener->next;
        if (strcmp(eventListener->type, type) == 0) {
            if (eventListener == first_) {
                first_ = first_->next;
            } else if (eventListener == last_) {
                last_ = last_->back;
            } else {
                eventListener->next->back = eventListener->back;
                eventListener->back->next = eventListener->next;
            }
            napi_delete_reference(env_, eventListener->handlerRef);
            delete eventListener;
            eventListener = nullptr;
        }
    }
}

void EventAgent::InsertEvent(TransEvent* event)
{
    LOGD("SendFile EventAgent::InsertEvent");
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
        uv_sleep(1000);
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
    LOGD("SendFile EventAgent::Callback thread tid = %{public}lu\n", (unsigned long)pthread_self());
}

void EventAgent::AfterCallback(uv_work_t *work, int status)
{
    auto agent = (EventAgent*)(work->data);
    if (agent == nullptr) {
        LOGE("SendFile return #1].");
        return;
    }
    LOGD("AfterCallback agent %{public}p.", agent);
    if (agent->eventList_.empty()) {
        LOGE("SendFile return #2].");
        return;
    }

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(agent->env_, &scope);

    napi_value thisVar = nullptr;
    napi_get_reference_value(agent->env_, agent->thisVarRef_, &thisVar);

    auto event = agent->eventList_.front();
    if (event == nullptr) {
        LOGE("AfterCallback event pointer is empty.");
        return;
    }
    EventListener* eventListener = agent->first_;
    for (; eventListener != nullptr; eventListener = eventListener->next) {
        if (strcmp(eventListener->type, event->GetName().c_str()) == 0) {
            napi_value jsEvent = event ? event->ToJsObject(agent->env_) : nullptr;
            napi_value handler = nullptr;
            napi_value callbackResult = nullptr;
            napi_value result = nullptr;
            napi_get_undefined(agent->env_, &result);
            napi_value callbackValues[2] = {0};
            callbackValues[0] = result;
            callbackValues[1] = jsEvent;
            LOGD("SendFile EventAgent::AfterCallback: listener type[%{public}s].", eventListener->type);
            LOGD("SendFile EventAgent::AfterCallback thread tid = %{public}lu\n", (unsigned long)pthread_self());
            napi_get_reference_value(agent->env_, eventListener->handlerRef, &handler);
            napi_call_function(agent->env_, thisVar, handler, std::size(callbackValues),
                callbackValues, &callbackResult);
        }
    }

    napi_close_handle_scope(agent->env_, scope);
    agent->eventList_.pop_front();
    delete event;
    delete work;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS