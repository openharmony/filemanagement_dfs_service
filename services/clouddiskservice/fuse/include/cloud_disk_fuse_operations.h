/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_DISK_FUSE_OPERATIONS_H
#define OHOS_CLOUD_DISK_FUSE_OPERATIONS_H

#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 317
#endif

#include <cstdint>

#include <fuse_lowlevel.h>

namespace OHOS::FileManagement::CloudDiskService {

class CloudDiskFuseOperations final {
public:
    static void Lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
    static void Forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup);
    static void GetAttr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
    static void Open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
    static void Release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_CLOUD_DISK_FUSE_OPERATIONS_H
