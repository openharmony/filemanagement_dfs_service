/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef DEVSL_DISPATCHER_H
#define DEVSL_DISPATCHER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "dev_slinfo_mgr.h"
#include "network/kernel_talker.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
enum class SecurityLabel : int32_t {
    S1 = 1,
    S2,
    S3,
    S4,
};

class DevslDispatcher final {
public:
    DevslDispatcher() = delete;
    ~DevslDispatcher() = delete;
    static int Start();
    static void Stop();
    static void DevslGottonCallback(DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo);
    static uint32_t DevslGetRegister(const std::string &cid, std::weak_ptr<KernelTalker>);
    static bool CompareDevslWithLocal(const std::string &peerNetworkId, const std::vector<std::string> &paths);
    
private:
    static DEVSLQueryParams MakeDevslQueryParams(const std::string &cid);
    static void DevslGottonCallbackAsync(const std::string udid, uint32_t devsl);
    static int32_t GetDeviceDevsl(const std::string &networkId);
    static int32_t GetSecurityLabel(const std::string &path);
    static std::map<std::string, std::vector<std::weak_ptr<KernelTalker>>> talkersMap_;
    static std::map<std::string, std::string> idMap_;
    static std::mutex mutex;
    static std::mutex devslMapMutex_;
    static std::map<std::string, int32_t> devslMap_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DSL_DISPATCHER_H