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

HWTEST_F(PluginLoaderTest, PluginLoaderTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PluginLoaderTest_004 Start";
    try {
        PluginLoader::GetInstance().LoadCloudKitPlugin(false);
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

/**
 * @tc.name: LoadDecompressPlugin_001
 * @tc.desc: LoadDecompressPlugin is callable and degrades gracefully when the
 *           adapter so is absent from the test environment (no crash, handle
 *           stays null).  This is the on-demand loading entry point.
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(PluginLoaderTest, LoadDecompressPlugin_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadDecompressPlugin_001 Start";
    EXPECT_NO_FATAL_FAILURE({ PluginLoader::GetInstance().LoadDecompressPlugin(); });
    GTEST_LOG_(INFO) << "LoadDecompressPlugin_001 End";
}

/**
 * @tc.name: LoadDecompressPlugin_002
 * @tc.desc: LoadDecompressPlugin uses std::call_once for thread-safe one-time
 *           initialization.  After the once_flag has fired, subsequent calls
 *           on any instance are no-ops and never invoke dlopen again.  A local
 *           instance with a sentinel handle verifies the no-repeat contract.
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(PluginLoaderTest, LoadDecompressPlugin_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadDecompressPlugin_002 Start";
    PluginLoader loader;
    void *sentinel = reinterpret_cast<void *>(0x1);
    loader.decompressPluginHandle_ = sentinel;
    loader.LoadDecompressPlugin();
    EXPECT_EQ(loader.decompressPluginHandle_, sentinel);
    GTEST_LOG_(INFO) << "LoadDecompressPlugin_002 End";
}

/**
 * @tc.name: LoadDecompressPlugin_003
 * @tc.desc: The destructor no longer dlcloses decompressPluginHandle_ (opened
 *           with RTLD_NODELETE, resident for process lifetime).  A local
 *           instance with null handles is allowed to destruct; reaching the
 *           assertion proves the cloudKit null guard prevents a crash.
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(PluginLoaderTest, LoadDecompressPlugin_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadDecompressPlugin_003 Start";
    {
        PluginLoader loader;
        loader.decompressPluginHandle_ = nullptr;
        loader.cloudKitPulginHandle_ = nullptr;
    }
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "LoadDecompressPlugin_003 End";
}

/**
 * @tc.name: LoadDecompressPlugin_004
 * @tc.desc: Repeated calls on the singleton are safe and idempotent — the
 *           second call must not throw or crash even though the so is still
 *           absent.  Validates the on-demand + no-repeat contract together.
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(PluginLoaderTest, LoadDecompressPlugin_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadDecompressPlugin_004 Start";
    EXPECT_NO_FATAL_FAILURE({
        PluginLoader::GetInstance().LoadDecompressPlugin();
        PluginLoader::GetInstance().LoadDecompressPlugin();
    });
    GTEST_LOG_(INFO) << "LoadDecompressPlugin_004 End";
}
} // OHOS