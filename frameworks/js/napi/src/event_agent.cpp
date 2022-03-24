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
}

EventAgent::~EventAgent()
{
    LOGD("SendFile EventAgent::~EventAgent().");
    ClearDevice();
    napi_delete_reference(env_, sendListenerRef_);
    napi_delete_reference(env_, recvListenerRef_);
    napi_delete_reference(env_, thisVarRef_);
}

void EventAgent::On(const char* type, napi_value handler)
{
    LOGD("SendFile EventAgent::On thread tid = %{public}lu\n", (unsigned long)pthread_self());
    if (type == nullptr || strnlen(type, LISTENER_TYPTE_MAX_LENGTH) == LISTENER_TYPTE_MAX_LENGTH) {
        LOGE("SendFile EventAgent::On: event type exceed 64 byte or empty.");
        return;
    }

    std::string eventName(type);
    if (eventName == "sendFinished") {
        napi_create_reference(env_, handler, 1, &sendListenerRef_);
    } else if (eventName == "receiveFinished") {
        napi_create_reference(env_, handler, 1, &recvListenerRef_);
    }
}

void EventAgent::Off(const char* type)
{
    LOGD("SendFile EventAgent::Off: JS callback unregister.\n");
    if (type == nullptr || strnlen(type, LISTENER_TYPTE_MAX_LENGTH) == LISTENER_TYPTE_MAX_LENGTH) {
        LOGE("SendFile EventAgent::Off: event type exceed 64 byte or empty.");
        return;
    }

    std::string eventName(type);
    if (eventName == "sendFinished") {
        napi_delete_reference(env_, sendListenerRef_);
    } else if (eventName == "receiveFinished") {
        napi_delete_reference(env_, recvListenerRef_);
    }
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

void EventAgent::Emit(std::unique_ptr<TransEvent> event)
{
    uv_work_t* work = new uv_work_t();
    auto context = new CallbackContext();
    napi_value handler = nullptr;
    if (event.get()->GetName() == "sendFinished") {
        napi_get_reference_value(env_, sendListenerRef_, &handler);
    } else if (event.get()->GetName() == "receiveFinished") {
        napi_get_reference_value(env_, recvListenerRef_, &handler);
    }
    context->handler = handler;
    context->env = env_;
    context->thisVarRef = thisVarRef_;
    context->event = std::move(event);
    work->data = context;
    uv_queue_work(loop_, work, Callback, AfterCallback);
}

void EventAgent::Callback(uv_work_t *work)
{
    LOGD("SendFile EventAgent::Callback thread tid = %{public}lu\n", (unsigned long)pthread_self());
}

void EventAgent::AfterCallback(uv_work_t *work, int status)
{
    LOGD("SendFile EventAgent::AfterCallback: entered.");
    auto context = static_cast<CallbackContext*>(work->data);
    napi_value handler = context->handler;
    if (handler == nullptr) {
        LOGE("SendFile EventAgent::AfterCallback: null handler.");
        return;
    }

    TransEvent *event = context->event.get();
    if (event == nullptr) {
        LOGE("SendFile EventAgent::AfterCallback: emit null event.");
        return;
    }

    napi_env env = context->env;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);

    napi_value thisVar = nullptr;
    napi_ref thisVarRef = context->thisVarRef;
    napi_get_reference_value(env, thisVarRef, &thisVar);

    napi_value jsEvent = event->ToJsObject(env);
    napi_value callbackResult = nullptr;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    napi_value callbackValues[2] = {0};
    callbackValues[0] = result;
    callbackValues[1] = jsEvent;

    napi_call_function(env, thisVar, handler, std::size(callbackValues),
        callbackValues, &callbackResult);
    LOGD("SendFile EventAgent::AfterCallback: event type[%{public}s].", event->GetName().c_str());
    LOGD("SendFile EventAgent::AfterCallback thread tid = %{public}lu\n", (unsigned long)pthread_self());

    napi_close_handle_scope(env, scope);
    delete context;
    delete work;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS