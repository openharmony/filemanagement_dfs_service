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

#ifndef OHOS_STORAGE_FILE_TRANS_LISTENER_MOCK_H
#define OHOS_STORAGE_FILE_TRANS_LISTENER_MOCK_H

#include <gmock/gmock.h>

#include "dfs_error.h"
#include "i_file_trans_listener.h"
#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class FileTransListenerMock : public IRemoteStub<IFileTransListener> {
public:
    int code_ = 0;
    FileTransListenerMock() : code_(0) {}
    virtual ~FileTransListenerMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    MOCK_METHOD2(OnFileReceive, int32_t(uint64_t totalBytes, uint64_t processedBytes));
    MOCK_METHOD2(OnFailed, int32_t(const std::string &sessionName, int32_t errorCode));
    MOCK_METHOD1(OnFinished, int32_t(const std::string &sessionName));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // OHOS_STORAGE_FILE_TRANS_LISTENER_MOCK_H
