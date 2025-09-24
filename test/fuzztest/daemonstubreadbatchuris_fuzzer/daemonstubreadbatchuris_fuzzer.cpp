/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "daemonstubreadbatchuris_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#include "copy/ipc_wrapper.h"
#include "daemon_stub.h"
#include "distributed_file_daemon_ipc_interface_code.h"
#include "ipc_skeleton.h"
#include "message_option.h"
#include "message_parcel.h"
#include "securec.h"
#include "accesstoken_kit.h"
#include "utils_log.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
constexpr pid_t UID = 1009;
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
pid_t IPCSkeleton::GetCallingUid()
{
    return UID;
}
}

namespace OHOS {
using namespace OHOS::Storage::DistributedFile;
void ReadBatchUriByRawDataFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    std::vector<std::string> uriVec;

    datas.WriteInt32(0);
    IpcWrapper::ReadBatchUriByRawData(datas, uriVec);

    datas.WriteInt32(1);
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);

    IpcWrapper::ReadBatchUriByRawData(datas, uriVec);
}

void SetAccessTokenPermission()
{
    uint64_t tokenId;
    const char *perms[1];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "distributdFileDaemonstubFuzzer",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    if (tokenId == 0) {
        LOGE("Get Access Token Id Failed");
        return;
    }
    int ret = SetSelfTokenID(tokenId);
    if (ret != 0) {
        LOGE("Set Access Token Id Failed");
        return;
    }
    ret = Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    if (ret < 0) {
        LOGE("Reload Native Token Info Failed");
        return;
    }
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::SetAccessTokenPermission();
    OHOS::ReadBatchUriByRawDataFuzzTest(data, size);
    return 0;
}
