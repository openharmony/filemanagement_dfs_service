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

#include "send_file.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
DfsFileTransferCallback::DfsFileTransferCallback() {}
DfsFileTransferCallback::~DfsFileTransferCallback() {}

int32_t DfsFileTransferCallback::SessionOpened(const std::string &cid)
{
    SendFile::JoinCidToAppId(cid);
    return DFS_FILE_TRANS_NO_ERROR;
}

int32_t DfsFileTransferCallback::SessionClosed(const std::string &cid)
{
    SendFile::DisjoinCidToAppId(cid);
    return DFS_FILE_TRANS_NO_ERROR;
}

int32_t DfsFileTransferCallback::SendFinished(const std::string &cid, std::string fileName)
{
    TransEvent event(TransEvent::TRANS_SUCCESS, fileName);
    std::string eventName("sendFinished");
    event.SetName(eventName);
    SendFile::EmitTransEvent(event, cid);
    LOGI("DfsFileTransferCallback::SendFinished: [%{public}s].", cid.c_str());
    return DFS_FILE_TRANS_NO_ERROR;
}

int32_t DfsFileTransferCallback::SendError(const std::string &cid)
{
    TransEvent event(TransEvent::TRANS_FAILURE);
    std::string eventName("sendFinished");
    event.SetName(eventName);
    SendFile::EmitTransEvent(event, cid);
    LOGI("DfsFileTransferCallback::SendError: [%{public}s].", cid.c_str());
    return DFS_FILE_TRANS_NO_ERROR;
}

int32_t DfsFileTransferCallback::ReceiveFinished(const std::string &cid, const std::string &fileName, uint32_t num)
{
    TransEvent event(TransEvent::TRANS_SUCCESS, fileName, num);
    std::string eventName("receiveFinished");
    event.SetName(eventName);
    SendFile::EmitTransEvent(event, cid);
    LOGI("DfsFileTransferCallback::ReceiveFinished: [%{public}s].", cid.c_str());
    return DFS_FILE_TRANS_NO_ERROR;
}

int32_t DfsFileTransferCallback::ReceiveError(const std::string &cid)
{
    TransEvent event(TransEvent::TRANS_FAILURE);
    std::string eventName("receiveFinished");
    event.SetName(eventName);
    SendFile::EmitTransEvent(event, cid);
    LOGI("DfsFileTransferCallback::ReceiveError: [%{public}s].", cid.c_str());
    return DFS_FILE_TRANS_NO_ERROR;
}

int32_t DfsFileTransferCallback::WriteFile(int32_t fd, const std::string &fileName)
{
    SendFile::WriteFile(fd, fileName);
    return DFS_FILE_TRANS_NO_ERROR;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS