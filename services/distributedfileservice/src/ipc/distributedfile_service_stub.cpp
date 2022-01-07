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

#include "distributedfile_service_stub.h"

#include <ipc_skeleton.h>
#include <sys/stat.h>

#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
DistributedFileServiceStub::DistributedFileServiceStub()
{
}

DistributedFileServiceStub::~DistributedFileServiceStub()
{
    memberFuncMap_.clear();
}

int DistributedFileServiceStub::OnRemoteRequest(uint32_t code,
                                                MessageParcel &data,
                                                MessageParcel &reply,
                                                MessageOption &option)
{
    std::u16string myDescriptor = DistributedFileServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        return DISTRIBUTEDFILE_BAD_TYPE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS