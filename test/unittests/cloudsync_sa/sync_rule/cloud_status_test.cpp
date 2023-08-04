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

#include "cloud_status.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudStatusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void CloudStatusTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudStatusTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudStatusTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudStatusTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetCurrentCloudInfo001
 * @tc.desc: Verify the CloudStatus::GetCurrentCloudInfo function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, GetCurrentCloudInfo001, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.test";
    const int32_t userId = 1;
    auto ret = cloudStatus.GetCurrentCloudInfo(bundleName, userId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetCurrentCloudInfo002
 * @tc.desc: Verify the CloudStatus::GetCurrentCloudInfo function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, GetCurrentCloudInfo002, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "";
    const int32_t userId = -1;
    auto ret = cloudStatus.GetCurrentCloudInfo(bundleName, userId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: IsCloudStatusOkay001
 * @tc.desc: Verify the CloudStatus::IsCloudStatusOkay function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsCloudStatusOkay001, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.demo";
    const int32_t userId = -1;
    auto ret = cloudStatus.IsCloudStatusOkay(bundleName, userId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsCloudStatusOkay002
 * @tc.desc: Verify the CloudStatus::IsCloudStatusOkay function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsCloudStatusOkay002, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.demo1";
    const int32_t userId = 1;
    auto ret = cloudStatus.IsCloudStatusOkay(bundleName, userId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsCloudStatusOkay003
 * @tc.desc: Verify the CloudStatus::IsCloudStatusOkay function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsCloudStatusOkay003, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.demo1";
    const int32_t userId = 1;
    cloudStatus.userInfo_.cloudStatus = DriveKit::DKCloudStatus::DK_CLOUD_STATUS_UNKNOWN;
    auto ret = cloudStatus.IsCloudStatusOkay(bundleName, userId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ChangeAppSwitch001
 * @tc.desc: Verify the CloudStatus::ChangeAppSwitch function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, ChangeAppSwitch001, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.demo2";
    const int32_t userId = -1;
    bool appSwitchStatus = true;
    auto ret = cloudStatus.ChangeAppSwitch(bundleName, userId, appSwitchStatus);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: ChangeAppSwitch002
 * @tc.desc: Verify the CloudStatus::ChangeAppSwitch function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, ChangeAppSwitch002, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.demo3";
    const int32_t userId = 1;
    bool appSwitchStatus = true;
    auto ret = cloudStatus.ChangeAppSwitch(bundleName, userId, appSwitchStatus);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: ChangeAppSwitch003
 * @tc.desc: Verify the CloudStatus::ChangeAppSwitch function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, ChangeAppSwitch003, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.demo3";
    const int32_t userId = 1;
    bool appSwitchStatus = true;
    cloudStatus.appSwitches_.clear();
    auto ret = cloudStatus.ChangeAppSwitch(bundleName, userId, appSwitchStatus);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: IsAccountIdChanged001
 * @tc.desc: Verify the CloudStatus::IsAccountIdChanged function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsAccountIdChanged001, TestSize.Level1)
{
    CloudStatus cloudStatus;
    std::string accountId = "IsAccountIdChanged";
    cloudStatus.userInfo_.accountId = "CloudStatusTest";
    auto ret = cloudStatus.IsAccountIdChanged(accountId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsAccountIdChanged002
 * @tc.desc: Verify the CloudStatus::IsAccountIdChanged function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsAccountIdChanged002, TestSize.Level1)
{
    CloudStatus cloudStatus;
    std::string accountId = "IsAccountIdChanged";
    cloudStatus.userInfo_.accountId = "";
    auto ret = cloudStatus.IsAccountIdChanged(accountId);
    EXPECT_EQ(ret, false);
}
} // namespace OHOS::FileManagement::CloudSync::Test
