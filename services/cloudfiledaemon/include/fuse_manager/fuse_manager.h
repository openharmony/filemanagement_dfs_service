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

#ifndef FUSE_MANAGER_H
#define FUSE_MANAGER_H

#include <string>
#include <map>

struct fuse_session;

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

class FuseManager final {
public:
    static FuseManager &GetInstance();
    int32_t StartFuse(int32_t userId, int32_t devFd, const string &path);
    FuseManager(const FuseManager&) = delete;
    FuseManager& operator=(const FuseManager&) = delete;
private:
    FuseManager() = default;
    ~FuseManager() = default;
private:
    map<int32_t, struct fuse_session *> sessions_;
};
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
#endif // FUSE_MANAGER_H
