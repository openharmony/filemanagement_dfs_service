/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "ipc_skeleton.h"

#include <cstdint>
#include <sys/types.h>

#include "access_token_adapter.h"
#include "accesstoken_kit.h"
#include "iosfwd"
#include "ipc_process_skeleton.h"
#include "ipc_thread_skeleton.h"
#include "ipc_types.h"
#include "iremote_invoker.h"
#include "iremote_object.h"
#include "refbase.h"
#include "string"
#include "tokenInfo.h"
#include "unistd.h"
#include "utils_log.h"

int32_t TokenInfo::tokenid;

namespace OHOS {
using namespace Security::AccessToken;
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif

pid_t IPCSkeleton::GetCallingUid()
{
    return 0;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    int ret = 0;
    switch (TokenInfo::GetTokenId()) {
        case TOKEN_HAP: {
            ret = TOKEN_HAP;
            break;
        }
        case TOKEN_NATIVE: {
            ret = TOKEN_NATIVE;
            break;
        }
        case TOKEN_SHELL: {
            ret = TOKEN_NATIVE;
            break;
        }
        default: {
            ret = -1;
            break;
        }
    }
    return ret;
}

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    IRemoteInvoker *invoker = IPCThreadSkeleton::GetActiveInvoker();
    if (invoker != nullptr) {
        return invoker->GetCallerTokenID();
    }
    return GetSelfTokenID();
}

uint64_t IPCSkeleton::GetSelfTokenID()
{
    IRemoteInvoker *invoker = IPCThreadSkeleton::GetDefaultInvoker();
    if (invoker != nullptr) {
        return invoker->GetSelfTokenID();
    }
    return 0;
}
} // namespace OHOS
