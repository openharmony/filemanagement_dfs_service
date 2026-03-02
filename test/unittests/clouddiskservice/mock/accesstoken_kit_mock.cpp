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

#include "accesstoken_kit.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {
int32_t g_getTokenTypeFlag = 0;
int32_t g_verifyAccessToken = 0;
int32_t g_getHapTokenInfo = 0;
int32_t g_getNativeTokenInfo = 0;
std::string g_hapBundleName = "";
std::string g_nativeProcessName = "";
int32_t g_hapInstIndex = 0;
}

void MockGetTokenTypeFlag(int32_t mockRet)
{
    g_getTokenTypeFlag = mockRet;
}

void MockVerifyAccessToken(int32_t mockRet)
{
    g_verifyAccessToken = mockRet;
}

void MockGetHapTokenInfo(int32_t mockRet)
{
    g_getHapTokenInfo = mockRet;
}

void MockGetNativeTokenInfo(int32_t mockRet)
{
    g_getNativeTokenInfo = mockRet;
}

void MockSetHapBundleName(const std::string &bundleName)
{
    g_hapBundleName = bundleName;
}

void MockSetNativeProcessName(const std::string &processName)
{
    g_nativeProcessName = processName;
}

void MockSetHapInstIndex(int32_t instIndex)
{
    g_hapInstIndex = instIndex;
}

namespace OHOS::Security::AccessToken {

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID mockRet)
{
    return static_cast<OHOS::Security::AccessToken::ATokenTypeEnum>(g_getTokenTypeFlag);
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    return g_verifyAccessToken;
}

int AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapTokenInfo)
{
    hapTokenInfo.bundleName = g_hapBundleName;
    hapTokenInfo.instIndex = g_hapInstIndex;
    return g_getHapTokenInfo;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfo)
{
    nativeTokenInfo.processName = g_nativeProcessName;
    return g_getNativeTokenInfo;
}

}