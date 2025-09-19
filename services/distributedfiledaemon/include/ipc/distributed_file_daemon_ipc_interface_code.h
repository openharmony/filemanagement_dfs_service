/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef IPC_DISTRIBUTED_FILE_DAEMON_IPC_INTERFACE_CODE_H
#define IPC_DISTRIBUTED_FILE_DAEMON_IPC_INTERFACE_CODE_H

/* SAID:5201 */
namespace OHOS {
namespace Storage {
namespace DistributedFile {
    enum class DistributedFileDaemonInterfaceCode {
        DISTRIBUTED_FILE_OPEN_P2P_CONNECTION = 0,
        DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION,
        DISTRIBUTED_FILE_PREPARE_SESSION,
        DISTRIBUTED_FILE_REQUEST_SEND_FILE,
        DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO,
        DISTRIBUTED_FILE_CANCEL_COPY_TASK,
        DISTRIBUTED_FILE_CANCEL_INNER_COPY_TASK,
        DISTRIBUTED_FILE_OPEN_P2P_CONNECTION_EX,
        DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION_EX,
        DISTRIBUTED_FILE_REGISTER_ASSET_CALLBACK,
        DISTRIBUTED_FILE_UN_REGISTER_ASSET_CALLBACK,
        DISTRIBUTED_FILE_PUSH_ASSET,
        GET_DFS_URI_IS_DIR_FROM_LOCAL,
        DISTRIBUTED_FILE_GET_DFS_SWITCH_STATUS,
        DISTRIBUTED_FILE_UPDATE_DFS_SWITCH_STATUS,
        DISTRIBUTED_FILE_GET_CONNECTED_DEVICE_LIST,
        DISTRIBUTED_FILE_REGISTER_FILE_DFS_LISTENER,
        DISTRIBUTED_FILE_UNREGISTER_FILE_DFS_LISTENER,
        DISTRIBUTED_FILE_IS_SAME_ACCOUNT_DEVICE,
        DISTRIBUTED_FILE_REQUEST_SEND_FILE_ACL,
        DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO_ACL,
    };
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif