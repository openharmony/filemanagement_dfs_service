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

#ifndef DFS_FILETRANSFER_CALLBACK_H
#define DFS_FILETRANSFER_CALLBACK_H

#include "filetransfer_callback_stub.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DfsFileTransferCallback : public FileTransferCallbackStub {
public:
    enum {
        DFS_FILE_TRANS_NO_ERROR = 0,
        DFS_FILE_TRANS_ERROR = -1,
    };

    DfsFileTransferCallback();
    virtual ~DfsFileTransferCallback() override;
    int32_t SessionOpened(const std::string &cid) override;
    int32_t SessionClosed(const std::string &cid) override;
    int32_t SendFinished(const std::string &cid, std::string fileName) override;
    int32_t SendError(const std::string &cid) override;
    int32_t ReceiveFinished(const std::string &cid, const std::string &fileName, uint32_t num) override;
    int32_t ReceiveError(const std::string &cid) override;
    int32_t WriteFile(int32_t fd, const std::string &fileName) override;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFS_FILETRANSFER_CALLBACK_H