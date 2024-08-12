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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin_loader.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace std;
using namespace OHOS;
using namespace testing::ext;

class PluginLoaderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PluginLoaderTest::SetUpTestCase(void) {}

void PluginLoaderTest::TearDownTestCase(void) {}

void PluginLoaderTest::SetUp(void) {}

void PluginLoaderTest::TearDown(void) {}

HWTEST_F(PluginLoaderTest, PluginLoaderTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PluginLoaderTest_001 Start";
    try {
        PluginLoader pluginLoader;
        pluginLoader.GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PluginLoaderTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "PluginLoaderTest_001 End";
}

HWTEST_F(PluginLoaderTest, PluginLoaderTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PluginLoaderTest_003 Start";
    try {
        PluginLoader pluginLoader;
        bool isSupportCloudSync = false;
        pluginLoader.LoadCloudKitPlugin(isSupportCloudSync);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PluginLoaderTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "PluginLoaderTest_003 End";
}

HWTEST_F(PluginLoaderTest, PluginLoaderTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PluginLoaderTest_004 Start";
    try {
        PluginLoader::GetInstance().LoadCloudKitPlugin(true);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PluginLoaderTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "PluginLoaderTest_004 End";
}

HWTEST_F(PluginLoaderTest, PluginLoaderTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PluginLoaderTest_005 Start";
    try {
        PluginLoader::GetInstance().LoadCloudKitPlugin();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PluginLoaderTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "PluginLoaderTest_005 End";
}
} // OHOS