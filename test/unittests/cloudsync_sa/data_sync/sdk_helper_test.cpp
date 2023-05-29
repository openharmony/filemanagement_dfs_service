/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dfs_error.h"
#include "dk_record.h"
#include "sdk_helper.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SdkHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<SdkHelper> sdkHelper_;
};
void SdkHelperTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SdkHelperTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SdkHelperTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    sdkHelper_ = std::make_shared<SdkHelper>();
}

void SdkHelperTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: InitDriveKitInstanceFailTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitDriveKitInstanceFailTest, TestSize.Level1)
{
    int32_t userId = -1;
    string appBundleName = "com.ohos.test";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: InitContainerFailTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitContainerFailTest, TestSize.Level1)
{
    int32_t userId = 100;
    string appBundleName = "com.ohos.test.mock";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: InitOKTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitOKTest, TestSize.Level1)
{
    int32_t userId = 100;
    string appBundleName = "com.ohos.test12312";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: GetLockOKTest
 * @tc.desc: Verify the GetLock function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetLockOKTest, TestSize.Level1)
{
    DriveKit::DKLock lock_;
    auto ret = sdkHelper_->GetLock(lock_);
    EXPECT_EQ(E_OK, ret);
}
} // namespace OHOS::FileManagement::CloudSync::Test