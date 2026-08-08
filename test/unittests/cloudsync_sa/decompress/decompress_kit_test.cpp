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
 * @tc.name: HandleConfigUpdate_001
 * @tc.desc: Test DecompressKitImpl HandleConfigUpdate returns true
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, HandleConfigUpdate_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleConfigUpdate_001 begin";
    DecompressKit *instance = DecompressKit::GetInstance();
    ASSERT_NE(instance, nullptr);
    bool result = instance->HandleConfigUpdate();
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "HandleConfigUpdate_001 end";
}

/**
 * @tc.name: BaseClassDefaultImpl_001
 * @tc.desc: Test base DecompressKit default HandleConfigUpdate returns false
 * @tc.type: FUNC
 * @tc.require: I5NJ2K
 */
HWTEST_F(DecompressKitTest, BaseClassDefaultImpl_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BaseClassDefaultImpl_001 begin";
    DecompressKit localInstance;
    EXPECT_FALSE(localInstance.HandleConfigUpdate());
    GTEST_LOG_(INFO) << "BaseClassDefaultImpl_001 end";
}
