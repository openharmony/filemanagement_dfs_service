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

#include "cloud_disk_xattr_mock.h"

#include <algorithm>
#include <cerrno>
#include <limits.h>
#include <securec.h>
#include <sys/stat.h>

#include "dfsu_access_token_helper.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
namespace {
XattrMockState g_xattrMockState;
}

XattrMockState &GetXattrMockState()
{
    return g_xattrMockState;
}

void ResetXattrMock()
{
    g_xattrMockState = XattrMockState();
}
} // namespace OHOS::FileManagement::CloudDiskService::Test

namespace OHOS::FileManagement {
bool DfsuAccessTokenHelper::CheckCallerPermission(const std::string &permissionName)
{
    auto &mockState = CloudDiskService::Test::GetXattrMockState();
    mockState.permissionChecked = true;
    mockState.permissionName = permissionName;
    return mockState.permissionGranted;
}

bool DfsuAccessTokenHelper::IsSystemApp()
{
    auto &mockState = CloudDiskService::Test::GetXattrMockState();
    mockState.systemAppChecked = true;
    return mockState.systemApp;
}

int32_t DfsuAccessTokenHelper::GetUserId()
{
    return CloudDiskService::Test::GetXattrMockState().userId;
}
} // namespace OHOS::FileManagement

extern "C" int Lstat(const char *path, struct stat *buf) __asm__("lstat");
extern "C" int Lstat(const char *path, struct stat *buf)
{
    auto &mockState = OHOS::FileManagement::CloudDiskService::Test::GetXattrMockState();
    mockState.lstatCalled = true;
    mockState.lstatPath = path == nullptr ? "" : path;
    if (mockState.lstatRet != 0) {
        errno = mockState.lstatErrno;
        return mockState.lstatRet;
    }
    if (buf != nullptr) {
        *buf = {};
        buf->st_mode = mockState.lstatMode;
    }
    return 0;
}

extern "C" char *Realpath(const char *path, char *resolvedPath) __asm__("realpath");
extern "C" char *Realpath(const char *path, char *resolvedPath)
{
    auto &mockState = OHOS::FileManagement::CloudDiskService::Test::GetXattrMockState();
    mockState.realpathCalled = true;
    std::string inputPath = path == nullptr ? "" : path;
    mockState.realpathPaths.push_back(inputPath);
    bool shouldFail = mockState.realpathFail &&
        (mockState.realpathFailPath.empty() || mockState.realpathFailPath == inputPath);
    if (shouldFail || path == nullptr || resolvedPath == nullptr) {
        errno = ENOENT;
        return nullptr;
    }

    auto item = mockState.realpathResults.find(inputPath);
    const std::string &resolved = item == mockState.realpathResults.end() ? inputPath : item->second;
    errno_t ret = strcpy_s(resolvedPath, PATH_MAX, resolved.c_str());
    if (ret != EOK) {
        errno = EINVAL;
        return nullptr;
    }
    return resolvedPath;
}

extern "C" ssize_t Getxattr(const char *path, const char *name, void *value, size_t size) __asm__("getxattr");
extern "C" ssize_t Getxattr(const char *path, const char *name, void *value, size_t size)
{
    auto &mockState = OHOS::FileManagement::CloudDiskService::Test::GetXattrMockState();
    mockState.getxattrCalled = true;
    mockState.getxattrPath = path == nullptr ? "" : path;
    mockState.getxattrName = name == nullptr ? "" : name;
    if (mockState.getxattrRet < 0) {
        errno = mockState.getxattrErrno;
        return mockState.getxattrRet;
    }
    if (value != nullptr && size > 0 && !mockState.getxattrValue.empty()) {
        size_t copySize = std::min(size, mockState.getxattrValue.size());
        errno_t ret = memcpy_s(value, size, mockState.getxattrValue.data(), copySize);
        if (ret != EOK) {
            errno = EINVAL;
            return OHOS::FileManagement::CloudDiskService::Test::MOCK_XATTR_RET_FAIL;
        }
    }
    return mockState.getxattrRet;
}
