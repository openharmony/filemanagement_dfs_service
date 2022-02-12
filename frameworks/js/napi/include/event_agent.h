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

#include <set>
#include <mutex>

#include "napi/native_api.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
struct EventListener;
class Event {
public:
    virtual napi_value ToJsObject(napi_env env) = 0;
};

class EventAgent {
public:
    EventAgent(napi_env env, napi_value thisVar);
    virtual ~EventAgent();

    void On(const char* type, napi_value handler);
    void Off(const char* type, napi_value handler);
    void Off(const char* type);
    void Emit(const char* type, Event* event);
    void InsertDevice(const std::string&);
    void RemoveDevice(const std::string&);
    bool FindDevice(const std::string&);
    void ClearDevice();

protected:
    std::mutex deviceListMut_;
    std::set<std::string> deviceList_;
    napi_env env_;
    napi_ref thisVarRef_;
    EventListener* first_;
    EventListener* last_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif /* NATIVE_MODULE_SEND_FILE_EVENT_AGENT_H */