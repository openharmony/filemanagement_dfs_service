/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SENDFILE_H
#define SENDFILE_H

#include "event_agent.h"

#include <unordered_map>
#include <vector>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class TransEvent : public Event {
public:
    enum {
        TRANS_SUCCESS = 0,
        TRANS_FAILURE = 1
    };
public:
    TransEvent(int32_t err, std::string fname, int32_t num) : errorCode_(err), fileName_(fname), fileCount_(num) {}
    TransEvent(int32_t err, std::string fname) : errorCode_(err), fileName_(fname) {}
    TransEvent(int32_t err) : errorCode_(err) {}
    virtual ~TransEvent() {}
    napi_value ToJsObject(napi_env env);

private:
    int32_t errorCode_ = TRANS_SUCCESS;
    std::string fileName_ = "";
    int32_t fileCount_ = 0;
};

int32_t NapiDeviceOnline(const std::string &cid);
int32_t NapiDeviceOffline(const std::string &cid);
int32_t NapiSendFinished(const std::string &cid, const std::string &fileName);
int32_t NapiSendError(const std::string &cid);
int32_t NapiReceiveFinished(const std::string &cid, const std::string &fileName, uint32_t num);
int32_t NapiReceiveError(const std::string &cid);
int32_t NapiWriteFile(int32_t fd, const std::string &fileName);

napi_value RegisterSendFileNotifyCallback();

static std::unordered_map<std::string, EventAgent*> g_mapUidToEventAgent;

int32_t ExecSendFile(const std::string &deviceId, const std::vector<std::string>& srcList,
    const std::vector<std::string>& dstList, uint32_t num);
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // SENDFILE_H