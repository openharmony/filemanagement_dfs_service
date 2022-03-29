/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
    void Emit(std::unique_ptr<TransEvent> event);
    void InsertDevice(const std::string&);
    void RemoveDevice(const std::string&);
    bool FindDevice(const std::string&);
    void ClearDevice();

protected:
    static void Callback(uv_work_t *work);
    static void AfterCallback(uv_work_t *work, int status);

    std::mutex deviceListMut_;
    std::set<std::string> deviceList_;
    napi_env env_;
    napi_ref thisVarRef_;
    napi_ref sendListenerRef_;
    napi_ref recvListenerRef_;
    uv_loop_t *loop_ {nullptr};

private:
    struct CallbackContext {
        napi_ref handlerRef;
        napi_ref thisVarRef;
        napi_env env;
        std::unique_ptr<TransEvent> event;
    };
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif /* NATIVE_MODULE_SEND_FILE_EVENT_AGENT_H */