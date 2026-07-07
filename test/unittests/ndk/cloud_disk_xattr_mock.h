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

#ifndef OHOS_FILEMGMT_CLOUD_DISK_XATTR_MOCK_H
#define OHOS_FILEMGMT_CLOUD_DISK_XATTR_MOCK_H

#include <cerrno>
#include <map>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

namespace OHOS::FileManagement::CloudDiskService::Test {
constexpr int MOCK_STAT_RET_SUCCESS = 0;
constexpr ssize_t MOCK_XATTR_RET_FAIL = -1;
constexpr int DEFAULT_TEST_USER_ID = 100;
constexpr mode_t MOCK_REGULAR_FILE_MODE = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

struct XattrMockState {
    int lstatRet = MOCK_STAT_RET_SUCCESS;
    int lstatErrno = 0;
    int userId = DEFAULT_TEST_USER_ID;
    mode_t lstatMode = MOCK_REGULAR_FILE_MODE;
    ssize_t getxattrSizeRet = MOCK_XATTR_RET_FAIL;
    int getxattrSizeErrno = ENODATA;
    ssize_t getxattrValueRet = MOCK_XATTR_RET_FAIL;
    int getxattrValueErrno = ENODATA;
    std::vector<char> getxattrValue;
    bool permissionGranted = true;
    bool systemApp = true;
    bool lstatCalled = false;
    bool getxattrCalled = false;
    bool realpathCalled = false;
    bool permissionChecked = false;
    bool systemAppChecked = false;
    bool realpathFail = false;
    std::string lstatPath;
    std::string getxattrPath;
    std::string getxattrName;
    std::vector<size_t> getxattrSizes;
    std::string permissionName;
    std::string realpathFailPath;
    std::vector<std::string> realpathPaths;
    std::map<std::string, std::string> realpathResults;
};

XattrMockState &GetXattrMockState();
void ResetXattrMock();
} // namespace OHOS::FileManagement::CloudDiskService::Test

#endif // OHOS_FILEMGMT_CLOUD_DISK_XATTR_MOCK_H
