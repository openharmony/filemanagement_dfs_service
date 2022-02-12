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
    : env_(env), thisVarRef_(nullptr), first_(nullptr), last_(nullptr)
{
    napi_create_reference(env, thisVar, 1, &thisVarRef_);
}

EventAgent::~EventAgent()
{
    EventListener* temp = nullptr;
    for (EventListener* i = first_; i != nullptr; i = temp) {
        temp = i->next;
        if (i == first_) {
            first_ = first_->next;
        } else if (i == last_) {
            last_ = last_->back;
        } else {
            i->next->back = i->back;
            i->back->next = i->next;
        }
        napi_delete_reference(env_, i->handlerRef);
        delete i;
    }
    napi_delete_reference(env_, thisVarRef_);
}

void EventAgent::On(const char* type, napi_value handler)
{
    auto tmp = new EventListener();

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

void EventAgent::Emit(const char* type, Event* event)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);

    napi_value thisVar = nullptr;
    napi_get_reference_value(env_, thisVarRef_, &thisVar);
    for (EventListener* eventListener = first_; eventListener != nullptr; eventListener = eventListener->next) {
        if (strcmp(eventListener->type, type) == 0) {
            napi_value jsEvent = event ? event->ToJsObject(env_) : nullptr;
            napi_value handler = nullptr;
            napi_value callbackResult = nullptr;
            napi_value result = nullptr;
            napi_get_undefined(env_, &result);
            napi_value callbackValues[2] = {0};
            callbackValues[0] = result;
            callbackValues[1] = jsEvent;
            napi_get_reference_value(env_, eventListener->handlerRef, &handler);
            napi_call_function(env_, thisVar, handler, std::size(callbackValues), callbackValues, &callbackResult);
            if (eventListener->isOnce) {
                Off(type);
            }
        }
    }

    napi_close_handle_scope(env_, scope);
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
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS