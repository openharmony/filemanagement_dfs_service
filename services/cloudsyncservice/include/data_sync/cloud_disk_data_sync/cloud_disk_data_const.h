/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_CONST_H
#define OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_CONST_H

#include "data_sync_const.h"
#include "gallery_file_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

const std::string DK_FILE_NAME = FILE_FILE_NAME;
const std::string DK_PARENT_CLOUD_ID = "parentFolder";
const std::string DK_FILE_SIZE = FILE_SIZE;
const std::string DK_FILE_SHA256 = "sha256";
const std::string DK_IS_DIRECTORY = "type";
const std::string DK_FILE_TIME_ADDED = "createTime";
const std::string DK_FILE_TIME_EDITED = "fileEditedTime";
const std::string DK_META_TIME_EDITED = "metaDataEditedTime";
const std::string DK_FILE_TIME_RECYCLED = FILE_RECYCLE_TIME;
const std::string DK_DIRECTLY_RECYCLED = "directlyRecycled";
const std::string DK_MIME_TYPE = FILE_MIME_TYPE;
const std::string DK_IS_RECYCLED = FILE_RECYCLED;
const std::string DK_FILE_ATTRIBUTES = FILE_ATTRIBUTES;
const std::string DK_FILE_CONTENT = "content";
const std::string DK_FILE_OPERATE_TYPE = "operateType";
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_CLOUD_DISK_CONST_H
