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
#include "filetranslistenerstub_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "message_option.h"
#include "message_parcel.h"
#include "file_trans_listener_stub.h"
#include "file_trans_listener_interface_code.h"
#include "ipc/distributed_file_daemon_manager.h"
#include "distributed_file_daemon_manager_impl.h"


using namespace OHOS::Storage::DistributedFile;

namespace OHOS {
namespace Storage {
namespace DistributedFile {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;

std::shared_ptr<Storage::DistributedFile::FileTransListenerStub> FileTransListenerStubPtr =
    std::make_shared<Storage::DistributedFile::FileTransListenerStub>();

uint32_t GetU32Data(const char *ptr) {
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

bool FileTransListenerStubFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    uint32_t code = static_cast<int32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_PROGRESS);
    if (code == 0) {
        return true;
    }
    MessageParcel datas;
    datas.WriteInterfaceToken(FileTransListenerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    FileTransListenerStubPtr->OnRemoteRequest(code, datas, reply, option);

    return true;
}
} // namespace OHOS::Storage::DistributedFile

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::Storage::DistributedFile::U32_AT_SIZE || size > OHOS::Storage::DistributedFile::FOO_MAX_LEN) {
        return 0;
    }

    auto str = std::make_unique<char[]>(size + 1);
    (void)memset_s(str.get(), size + 1, 0x00, size + 1);
    if (memcpy_s(str.get(), size, data, size) != EOK) {
        return 0;
    }
    OHOS::Storage::DistributedFile::FileTransListenerStubFuzzTest(move(str), size);
    return 0;
}
