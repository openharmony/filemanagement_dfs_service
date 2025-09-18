/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "oh_cloud_disk_utils.h"

CloudDisk_ErrorCode ConvertToErrorCode(int32_t innerErrorCode)
{
    auto iter = innerToNErrTable.find(innerErrorCode);
    if (iter != innerToNErrTable.end()) {
        return iter->second;
    } else {
        return CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN;
    }
}