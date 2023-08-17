/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "softbus_bus_center.h"

#include "gtest/gtest.h"

int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    GTEST_LOG_(INFO) << "GetLocalNodeDeviceInfo start";
    NodeBasicInfo nodeBasicInfo = {
        .networkId = "100",
        .deviceName = "test",
        .deviceTypeId = 1};
    (void)pkgName;
    *info = nodeBasicInfo;
    return 0;
}

int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId,
    NodeDeviceInfoKey key, uint8_t *info, int32_t infoLen)
{
    GTEST_LOG_(INFO) << "GetNodeKeyInfo start";
    (void)pkgName;
    (void)networkId;
    (void)key;
    (void)info;
    (void)infoLen;
    return 0;
}