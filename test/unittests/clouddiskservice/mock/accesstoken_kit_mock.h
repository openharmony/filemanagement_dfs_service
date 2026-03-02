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

#ifndef ACCESSTOKEN_KIT_MOCK_H
#define ACCESSTOKEN_KIT_MOCK_H

#include <string>

void MockGetTokenTypeFlag(int32_t mockRet);
void MockVerifyAccessToken(int32_t mockRet);
void MockGetHapTokenInfo(int32_t mockRet);
void MockGetNativeTokenInfo(int32_t mockRet);
void MockQueryActiveOsAccountIds(bool mockRet);
void MockIsOsAccountVerified(bool mockRet);
void MockSetHapBundleName(const std::string &bundleName);
void MockSetNativeProcessName(const std::string &processName);
void MockSetHapInstIndex(int32_t instIndex);

#endif
