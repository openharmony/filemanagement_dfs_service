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

#include <gtest/gtest.h>

#include "decompress_kit.h"
#include "decompress_kit_impl.h"
#include "utils_log.h"

using namespace testing::ext;
using namespace OHOS::FileManagement::Decompress;

class DecompressKitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DecompressKitTest::SetUpTestCase(void)
{
}

void DecompressKitTest::TearDownTestCase(void)
{
}

void DecompressKitTest::SetUp(void)
{
}

void DecompressKitTest::TearDown(void)
{
}

/**
 * @tc.name: GetInstance_001
 * @tc.desc: Test GetInstance returns non-null after DecompressKitImpl auto-registration
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstance_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    GTEST_LOG_(INFO) << "GetInstance_001 end";
}

/**
 * @tc.name: RegisterDecompressInstance_001
 * @tc.desc: Test RegisterDecompressInstance returns false when instance already registered
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, RegisterDecompressInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDecompressInstance_001 begin";
    DecompressKit *newInstance = new (std::nothrow) DecompressKit();
    ASSERT_NE(newInstance, nullptr);
    bool result = DecompressKit::RegisterDecompressInstance(newInstance);
    EXPECT_FALSE(result);
    delete newInstance;
    GTEST_LOG_(INFO) << "RegisterDecompressInstance_001 end";
}

/**
 * @tc.name: GetCloudVersionFilePath_001
 * @tc.desc: Test DecompressKitImpl GetCloudVersionFilePath returns empty string
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, GetCloudVersionFilePath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudVersionFilePath_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string result = instance->GetCloudVersionFilePath();
    EXPECT_EQ(result, "");
    GTEST_LOG_(INFO) << "GetCloudVersionFilePath_001 end";
}

/**
 * @tc.name: GetParamConfigProperty_001
 * @tc.desc: Test DecompressKitImpl GetParamConfigProperty returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, GetParamConfigProperty_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetParamConfigProperty_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string line = "test";
    std::string key = "key";
    std::string value = "value";
    bool result = instance->GetParamConfigProperty(line, key, value);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "GetParamConfigProperty_001 end";
}

/**
 * @tc.name: GetVersionConfigValue_001
 * @tc.desc: Test DecompressKitImpl GetVersionConfigValue returns empty string
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, GetVersionConfigValue_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetVersionConfigValue_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string result = instance->GetVersionConfigValue("/test/path", "configKey");
    EXPECT_EQ(result, "");
    GTEST_LOG_(INFO) << "GetVersionConfigValue_001 end";
}

/**
 * @tc.name: IsUpdateVersionCompatible_001
 * @tc.desc: Test DecompressKitImpl IsUpdateVersionCompatible returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, IsUpdateVersionCompatible_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsUpdateVersionCompatible_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->IsUpdateVersionCompatible("/test/path");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsUpdateVersionCompatible_001 end";
}

/**
 * @tc.name: IsNeedCopy_001
 * @tc.desc: Test DecompressKitImpl IsNeedCopy returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, IsNeedCopy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsNeedCopy_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->IsNeedCopy("/src/dir", "/dest/dir");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "IsNeedCopy_001 end";
}

/**
 * @tc.name: CompareVersion_001
 * @tc.desc: Test DecompressKitImpl CompareVersion returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, CompareVersion_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompareVersion_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->CompareVersion("1.0", "2.0");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "CompareVersion_001 end";
}

/**
 * @tc.name: DoParamDirCopy_001
 * @tc.desc: Test DecompressKitImpl DoParamDirCopy returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, DoParamDirCopy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoParamDirCopy_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->DoParamDirCopy("/src/dir", "/dest/dir");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "DoParamDirCopy_001 end";
}

/**
 * @tc.name: DoConfigFileCopy_001
 * @tc.desc: Test DecompressKitImpl DoConfigFileCopy returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, DoConfigFileCopy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoConfigFileCopy_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->DoConfigFileCopy("/src/dir", "/dest/dir");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "DoConfigFileCopy_001 end";
}

/**
 * @tc.name: DoMkDir_001
 * @tc.desc: Test DecompressKitImpl DoMkDir returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, DoMkDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoMkDir_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->DoMkDir("/test/dir");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "DoMkDir_001 end";
}

/**
 * @tc.name: DoRmDir_001
 * @tc.desc: Test DecompressKitImpl DoRmDir returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, DoRmDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRmDir_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->DoRmDir("/test/dir");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "DoRmDir_001 end";
}

/**
 * @tc.name: DoCopy_001
 * @tc.desc: Test DecompressKitImpl DoCopy returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, DoCopy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCopy_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->DoCopy("/src/path", "/dest/path");
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "DoCopy_001 end";
}

/**
 * @tc.name: BaseClassDefaultImpl_001
 * @tc.desc: Test base DecompressKit default implementations via local instance
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, BaseClassDefaultImpl_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseClassDefaultImpl_001 begin";
    DecompressKit localInstance;
    std::string line = "line";
    std::string key = "key";
    std::string value = "value";
    EXPECT_EQ(localInstance.GetCloudVersionFilePath(), "");
    EXPECT_TRUE(localInstance.GetParamConfigProperty(line, key, value));
    EXPECT_EQ(localInstance.GetVersionConfigValue("/test/path", "configKey"), "");
    EXPECT_TRUE(localInstance.IsUpdateVersionCompatible("/test/path"));
    EXPECT_TRUE(localInstance.IsNeedCopy("/src/dir", "/dest/dir"));
    EXPECT_TRUE(localInstance.CompareVersion("1.0", "2.0"));
    EXPECT_TRUE(localInstance.DoParamDirCopy("/src/dir", "/dest/dir"));
    EXPECT_TRUE(localInstance.DoConfigFileCopy("/src/dir", "/dest/dir"));
    EXPECT_TRUE(localInstance.DoMkDir("/test/dir"));
    EXPECT_TRUE(localInstance.DoRmDir("/test/dir"));
    EXPECT_TRUE(localInstance.DoCopy("/src/path", "/dest/path"));
    GTEST_LOG_(INFO) << "BaseClassDefaultImpl_001 end";
}
