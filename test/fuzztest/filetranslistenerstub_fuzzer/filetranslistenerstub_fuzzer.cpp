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
#include "filetranslistenerstub_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "message_option.h"
#include "message_parcel.h"
#include "file_trans_listener_stub.h"
#include "securec.h"

namespace OHOS {

constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;

using namespace OHOS::Storage::DistributedFile;


namespace Storage {
namespace DistributedFile {
class FileTransListenerStubImpl : public FileTransListenerStub {
public:
    FileTransListenerStubImpl() = default;
    ~FileTransListenerStubImpl() override {}
    int32_t OnFileReceive(uint64_t totalBytes, uint64_t processedBytes) override;
    int32_t OnFailed(const std::string &sessionName) override;
    int32_t OnFinished(const std::string &sessionName) override;
};
}
}

int32_t FileTransListenerStubImpl::OnFileReceive(uint64_t totalBytes, uint64_t processedBytes)
{
    return 0;
}

int32_t FileTransListenerStubImpl::OnFailed(const std::string &sessionName)
{
    return 0;
}

int32_t FileTransListenerStubImpl::OnFinished(const std::string &sessionName)
{
    return 0;
}


uint32_t GetU32Data(const char *ptr)
{
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

bool FileTransListenerStubFuzzTest(
    std::shared_ptr<Storage::DistributedFile::FileTransListenerStub> transListenerStubPtr,
    std::unique_ptr<char[]> data, size_t size)
{
    uint32_t code = GetU32Data(data.get());
    if (code == 0) {
        return true;
    }
    MessageParcel datas;
    datas.WriteInterfaceToken(FileTransListenerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    transListenerStubPtr->OnRemoteRequest(code, datas, reply, option);

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
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    auto str = std::make_unique<char[]>(size + 1);
    if (memcpy_s(str.get(), size, data, size) != EOK) {
        return 0;
    }

    auto transListenerStubPtr = std::make_shared<OHOS::Storage::DistributedFile::FileTransListenerStubImpl>();
    OHOS::FileTransListenerStubFuzzTest(transListenerStubPtr, move(str), size);
    return 0;
}
