/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dfs_service_fuzz_utils.h"
#include "daemon.h"
#include "ipc/daemon_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::Storage::DistributedFile;
namespace OHOS {
constexpr int32_t SAID = 5203;

void DfsServiceFuzzUtils::FuzzTestRemoteRequest(const uint8_t *rawData, size_t size, uint32_t code)
{
    sptr daemon = sptr(new Daemon(SAID));
    MessageParcel data;
    data.WriteInterfaceToken(DaemonStub::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    daemon->OnRemoteRequest(code, data, reply, option);
    daemon = nullptr;
}
} // namespace OHOS