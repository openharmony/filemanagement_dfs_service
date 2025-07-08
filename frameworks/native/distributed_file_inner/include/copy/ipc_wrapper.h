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

#ifndef DISTRIBUTED_FILE_IPC_WRAPPER_H
#define DISTRIBUTED_FILE_IPC_WRAPPER_H

#include <dirent.h>
#include <memory>
#include <string>

#include "message_parcel.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class IpcWrapper {
public:
    static bool WriteUriByRawData(MessageParcel &data, const std::vector<std::string> &uriVec);
    static bool WriteBatchUris(MessageParcel &data, const std::vector<std::string> &uriVec);
    static bool GetData(void *&buffer, size_t size, const void *data);
    static bool ReadBatchUriByRawData(MessageParcel &data, std::vector<std::string> &uriVec);
    static int32_t ReadBatchUris(MessageParcel &data, std::vector<std::string> &uriVec);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DISTRIBUTED_FILE_IPC_WRAPPER_H
