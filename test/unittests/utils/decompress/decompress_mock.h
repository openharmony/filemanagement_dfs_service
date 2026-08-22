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

#ifndef DECOMPRESS_MOCK_H
#define DECOMPRESS_MOCK_H

#include <cstdint>
#include <string>
#include <vector>

namespace DecompressMock {

struct State {
    bool openEnabled = false;
    int openReturnFd = -1;
    int openErrno = 0;
    int openCallCount = 0;
    int openFailAfterCount = -1;

    bool closeEnabled = false;
    int closeReturnVal = 0;

    bool ioctlEnabled = false;
    int ioctlReturnVal = -1;
    int ioctlErrno = 0;
    int ioctlCallCount = 0;
    int ioctlSuccessAfterCount = -1;
    int ioctlRetryErrno = 0;

    bool realpathEnabled = false;
    bool realpathFail = false;

    bool dlopenEnabled = false;
    bool dlopenSucceed = false;
    void *dlopenHandle = reinterpret_cast<void *>(0x12345678);

    bool dlsymEnabled = false;
    bool dlsymSucceed = false;
    int32_t unsupportedListRet = 0;
    std::vector<std::string> unsupportedList;
    int32_t systemFeatureRet = 0;
    bool systemFeatureValue = false;

    bool fopenEnabled = false;
    bool fopenSucceed = false;
    FILE *fopenHandle = reinterpret_cast<FILE *>(0xDEADBEEF);

    bool getlineEnabled = false;
    bool getlineSucceed = false;
    ssize_t getlineRetVal = 1;
    char getlineContent[256] = "1\n";
};

extern State g;

void Reset();

} // namespace DecompressMock

#endif // DECOMPRESS_MOCK_H