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

#include "file_operations_base.h"
#include "fuse_assistant.h"

using namespace OHOS::FileManagement::CloudFile;
int fuse_opt_add_arg(struct fuse_args *args, const char *arg)
{
    return FuseAssistant::ins->fuse_opt_add_arg(args, arg);
}

bool CheckPathForStartFuse(const std::string &path)
{
    return FuseAssistant::ins->CheckPathForStartFuse(path);
}

struct fuse_session* fuse_session_new(struct fuse_args *args, const struct fuse_lowlevel_ops *op,
    size_t opSize, void *userData)
{
    return FuseAssistant::ins->fuse_session_new(args, op, opSize, userData);
}

void fuse_session_destroy(struct fuse_session *se)
{
    FuseAssistant::ins->fuse_session_destroy(se);
}

void HandleStartMove(int32_t userId)
{
    FuseAssistant::ins->HandleStartMove(userId);
}
