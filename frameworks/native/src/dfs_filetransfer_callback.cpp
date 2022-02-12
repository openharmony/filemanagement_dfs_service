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

#include "dfs_filetransfer_callback.h"

#include "sendfile.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
DfsFileTransferCallback::DfsFileTransferCallback() {}

DfsFileTransferCallback::~DfsFileTransferCallback() {}

int32_t DfsFileTransferCallback::DeviceOnline(const std::string &cid)
{
    NapiDeviceOnline(cid);
    return 0;
}

int32_t DfsFileTransferCallback::DeviceOffline(const std::string &cid)
{
    NapiDeviceOffline(cid);
    return 0;
}

int32_t DfsFileTransferCallback::SendFinished(const std::string &cid, std::string fileName)
{
    NapiSendFinished(cid, fileName);
    return 0;
}

int32_t DfsFileTransferCallback::SendError(const std::string &cid)
{
    NapiSendError(cid);
    return 0;
}

int32_t DfsFileTransferCallback::ReceiveFinished(const std::string &cid, const std::string &fileName, uint32_t num)
{
    NapiReceiveFinished(cid, fileName, num);
    return 0;
}

int32_t DfsFileTransferCallback::ReceiveError(const std::string &cid)
{
    NapiReceiveError(cid);
    return 0;
}

int32_t DfsFileTransferCallback::WriteFile(int32_t fd, const std::string &fileName)
{
    NapiWriteFile(fd, fileName);
    return 0;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS