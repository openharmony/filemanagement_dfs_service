/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "system_func_mock.h"

using namespace OHOS::FileManagement::CloudSync::Test;

int32_t mockRename(const char* s1, const char* s2)
{
    if (SystemFuncMock::proxy_ != nullptr) {
        return SystemFuncMock::proxy_->rename(s1, s2);
    }
    return 0;
}

int32_t mockMkdir(const char* s, uint32_t mode)
{
    if (SystemFuncMock::proxy_ != nullptr) {
        return SystemFuncMock::proxy_->mkdir(s, mode);
    }
    return 0;
}

int32_t mockAccess(const char* s, int32_t mode)
{
    if (SystemFuncMock::proxy_ != nullptr) {
        return SystemFuncMock::proxy_->access(s, mode);
    }
    return 0;
}

int32_t mockRemove(const char* s)
{
    if (SystemFuncMock::proxy_ != nullptr) {
        return SystemFuncMock::proxy_->remove(s);
    }
    return 0;
}