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
#include "assistant.h"
#include <sys/fanotify.h>

using namespace OHOS::FileManagement::CloudDiskService;

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz)
{
    return Assistant::ins->readlink(pathname, buf, bufsiz);
}

int fanotify_init(unsigned flags, unsigned event_f_flags)
{
    return Assistant::ins->fanotify_init(flags, event_f_flags);
}

int fanotify_mark(int fanotify_fd,
                  unsigned flags,
                  unsigned long long mask,
                  int dfd,
                  const char *pathname)
{
    return Assistant::ins->fanotify_mark(fanotify_fd, flags, mask, dfd, pathname);
}

DIR* opendir(const char* path)
{
    return Assistant::ins->opendir(path);
}

int dirfd(DIR *d)
{
    return Assistant::ins->dirfd(d);
}
