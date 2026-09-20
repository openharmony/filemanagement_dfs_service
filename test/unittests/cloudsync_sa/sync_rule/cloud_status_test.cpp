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
#include "cloud_file_kit_mock.cpp"
#include "settings_data_manager_mock.h"

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
    static inline shared_ptr<CloudFileKitMock> ability = make_shared<CloudFileKitMock>();
    static inline shared_ptr<SettingsDataManagerMock> settingsDataManagerMock_ = nullptr;
};
void CloudStatusTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    settingsDataManagerMock_ = make_shared<SettingsDataManagerMock>();
    SettingsDataManagerMock::proxy_ = settingsDataManagerMock_;
}

void CloudStatusTest::TearDownTestCase(void)
{
    ability = nullptr;
    settingsDataManagerMock_ = nullptr;
    SettingsDataManagerMock::proxy_ = nullptr;
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
    EXPECT_EQ(ret, E_NULLPTR);
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
    const string bundleName = "ohos.com.test";
    const int32_t userId = 1;
    CloudFile::CloudFileKit::instance_ = ability.get();
    EXPECT_CALL(*ability, GetCloudUserInfo(_, _, _)).WillOnce(Return(E_RDB)).WillRepeatedly(Return(E_OK));
    auto ret = cloudStatus.GetCurrentCloudInfo(bundleName, userId);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetCurrentCloudInfo003
 * @tc.desc: Verify the CloudStatus::GetCurrentCloudInfo function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, GetCurrentCloudInfo003, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.test";
    const int32_t userId = 1;
    CloudFile::CloudFileKit::instance_ = ability.get();
    EXPECT_CALL(*ability, GetAppSwitchStatus(_, _, _)).WillOnce(Return(E_RDB)).WillRepeatedly(Return(E_OK));
    auto ret = cloudStatus.GetCurrentCloudInfo(bundleName, userId);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetCurrentCloudInfo004
 * @tc.desc: Verify the CloudStatus::GetCurrentCloudInfo function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, GetCurrentCloudInfo004, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "com.ohos.photos";
    const int32_t userId = 1;
    auto ret = cloudStatus.GetCurrentCloudInfo(bundleName, userId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetCurrentCloudInfo005
 * @tc.desc: Verify the CloudStatus::GetCurrentCloudInfo function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, GetCurrentCloudInfo005, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "com.ohos.ailife";
    const int32_t userId = 1;
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
    EXPECT_EQ(ret, false);
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
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSwitchOn001
 * @tc.desc: Verify the CloudStatus::IsSwitchOn function when switch status is CLOUD_SPACE
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsSwitchOn001, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "com.ohos.photos";
    const int32_t userId = 1;
    EXPECT_CALL(*settingsDataManagerMock_, GetSwitchStatus()).WillOnce(Return(SwitchStatus::CLOUD_SPACE));
    auto ret = cloudStatus.IsSwitchOn(bundleName, userId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsSwitchOn002
 * @tc.desc: Verify the CloudStatus::IsSwitchOn function when switch status does not match
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsSwitchOn002, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "com.ohos.photos";
    const int32_t userId = 1;
    EXPECT_CALL(*settingsDataManagerMock_, GetSwitchStatus()).WillOnce(Return(SwitchStatus::NONE));
    auto ret = cloudStatus.IsSwitchOn(bundleName, userId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSwitchOn003
 * @tc.desc: Verify the CloudStatus::IsSwitchOn function when switch status is AI_FAMILY
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsSwitchOn003, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "com.ohos.ailife";
    const int32_t userId = 1;
    EXPECT_CALL(*settingsDataManagerMock_, GetSwitchStatus()).WillOnce(Return(SwitchStatus::AI_FAMILY));
    auto ret = cloudStatus.IsSwitchOn(bundleName, userId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsSwitchOn004
 * @tc.desc: Verify the CloudStatus::IsSwitchOn function when switch status does not match
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsSwitchOn004, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "com.ohos.ailife";
    const int32_t userId = 1;
    EXPECT_CALL(*settingsDataManagerMock_, GetSwitchStatus()).WillOnce(Return(SwitchStatus::NONE));
    auto ret = cloudStatus.IsSwitchOn(bundleName, userId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsSwitchOn005
 * @tc.desc: Verify the CloudStatus::IsSwitchOn function falls back to IsCloudStatusOkay
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudStatusTest, IsSwitchOn005, TestSize.Level1)
{
    CloudStatus cloudStatus;
    const string bundleName = "ohos.com.demo";
    const int32_t userId = 1;
    auto ret = cloudStatus.IsSwitchOn(bundleName, userId);
    EXPECT_EQ(ret, false);
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