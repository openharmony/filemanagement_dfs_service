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

#ifndef TEST_UNITTESTS_CLOUD_FILE_FUSEASSISTANT_H
#define TEST_UNITTESTS_CLOUD_FILE_FUSEASSISTANT_H
#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 30
#endif

#include <fuse_lowlevel.h>
#include <fuse_opt.h>
#include <gmock/gmock.h>
#include <memory>

namespace OHOS::FileManagement::CloudFile {
class FuseAssistant {
public:
    virtual ~FuseAssistant() = default;
    virtual int fuse_opt_add_arg(struct fuse_args *args, const char *arg) = 0;
    virtual struct fuse_session* fuse_session_new(struct fuse_args *args, const struct fuse_lowlevel_ops *op,
        size_t opSize, void *userData) = 0;
    virtual void fuse_session_destroy(struct fuse_session *se) = 0;
    virtual int mkdir(const char* path, mode_t mode) = 0;
    virtual int chown(const char *path, uid_t owner, gid_t group) = 0;
    virtual int chmod(const char *path, mode_t mode) = 0;
    virtual void HandleStartMove(int32_t userId) = 0;
    static inline std::shared_ptr<FuseAssistant> ins = nullptr;
};

class FuseAssistantMock : public FuseAssistant {
public:
    MOCK_METHOD2(fuse_opt_add_arg, int(struct fuse_args *args, const char *arg));
    MOCK_METHOD4(fuse_session_new, struct fuse_session*(struct fuse_args *args, const struct fuse_lowlevel_ops *op,
        size_t opSize, void *userData));
    MOCK_METHOD1(fuse_session_destroy, void(struct fuse_session *se));
    MOCK_METHOD2(mkdir, int(const char* path, mode_t mode));
    MOCK_METHOD3(chown, int(const char * path, uid_t owner, gid_t group));
    MOCK_METHOD2(chmod, int(const char * path, mode_t mode));
    MOCK_METHOD1(HandleStartMove, void(int32_t userId));
};
} // namespace OHOS::FileManagement::CloudDisk
#endif // TEST_UNITTESTS_CLOUD_FILE_FUSEASSISTANT_H