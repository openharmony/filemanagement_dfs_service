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

#include <mutex>
#include <unordered_map>
#include <vector>
#include "event_agent.h"
#include "trans_event.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SendFile {
public:
    enum {
        NAPI_SENDFILE_NO_ERROR = 0,
        NAPI_SENDFILE_SA_ERROR = -1,
        NAPI_SENDFILE_IPC_ERROR = -2,
        NAPI_SENDFILE_DEVICE_ID_ERROR = -3,
        NAPI_SENDFILE_GET_AGENT_ERROR = -4,
        NAPI_SENDFILE_APP_AGENT_ERROR = -5,
        NAPI_SENDFILE_PARA_ERROR = -6,
        NAPI_SENDFILE_AGENT_ERROR = -7,
        NAPI_SENDFILE_SEND_ERROR = -8,
        NAPI_SENDFILE_FD_ERROR = -9,
        NAPI_SENDFILE_UNKNOWN_ERROR = -99,
    };

    SendFile() {}
    virtual ~SendFile() {}
    static int32_t JoinCidToAppId(const std::string &cid, const std::string &AppId = BUNDLE_ID_);
    static int32_t DisjoinCidToAppId(const std::string &cid, const std::string &AppId = BUNDLE_ID_);
    static int32_t EmitTransEvent(std::shared_ptr<TransEvent> event, const std::string &cid,
                                const std::string &AppId = BUNDLE_ID_);
    static int32_t WriteFile(int32_t fd, const std::string &fileName);
    static int32_t RegisterCallback();
    static int32_t ExecSendFile(const std::string &deviceId, const std::vector<std::string>& srcList,
        const std::vector<std::string>& dstList, uint32_t num);

    static std::mutex g_uidMutex;
    static std::unordered_map<std::string, std::shared_ptr<EventAgent>> mapUidToEventAgent_;
    static inline const std::string BUNDLE_ID_ {"SendFileHapUid"};
    static inline const std::string APP_PATH {"/data/accounts/account_0/appdata/"};
    static inline const int32_t FILE_BLOCK_SIZE {1024};
    static inline const int32_t MAX_SEND_FILE_HAP_NUMBER {50};
    static inline const int32_t SENDFILE_NAPI_BUF_LENGTH {64};
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // SENDFILE_H