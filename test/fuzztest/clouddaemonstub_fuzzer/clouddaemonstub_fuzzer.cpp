/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "clouddaemonstub_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <iostream>

#include "cloud_daemon_stub.h"
#include "securec.h"
#include "message_parcel.h"
#include "cloud_file_daemon_interface_code.h"
#include "dfs_error.h"

namespace OHOS {

constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;

using namespace OHOS::FileManagement::CloudFile;
using namespace std;
using namespace OHOS::FileManagement;

const std::u16string CLOUD_DEAMON_TOKEN = u"ohos.filemanagement.distributedfile.clouddaemon";

class CloudDaemonStubImpl : public CloudDaemonStub {
public:
    CloudDaemonStubImpl() = default;
    ~CloudDaemonStubImpl() override {}
    int32_t StartFuse(int32_t userId, int32_t deviceFd, const std::string &path) override
    {
        return E_OK;
    }
};

void HandleStartFuseInnerFuzzTest(
    std::shared_ptr<CloudDaemonStub> cloudDaemonStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // CLOUD_DAEMON_CMD_START_FUSE
    uint32_t code = static_cast<uint32_t>(CloudFileDaemonInterfaceCode::CLOUD_DAEMON_CMD_START_FUSE);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_DEAMON_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudDaemonStubStr->OnRemoteRequest(code, datas, reply, option);
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    auto str = std::make_unique<char[]>(size + 1);
    (void)memset_s(str.get(), size + 1, 0x00, size + 1);
    if (memcpy_s(str.get(), size, data, size) != EOK) {
        return 0;
    }

    auto cloudDaemonStubStr = std::make_shared<OHOS::CloudDaemonStubImpl>();

    OHOS::HandleStartFuseInnerFuzzTest(cloudDaemonStubStr, move(str), size);
    return 0;
}
