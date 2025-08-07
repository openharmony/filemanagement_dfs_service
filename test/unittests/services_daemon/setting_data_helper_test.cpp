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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fuse_manager.cpp"

#include "data_helper_mock.h"
#include "data_sync_const.h"
#include "dfs_error.h"
#include "setting_data_helper.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::DataShare;

class SettingDataHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DataHelperMock> mock_ = nullptr;
};

void SettingDataHelperTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SettingDataHelperTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SettingDataHelperTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    mock_ = make_shared<DataHelperMock>();
    DataHelperMock::ins = mock_;
}

void SettingDataHelperTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    DataHelperMock::ins = nullptr;
    mock_ = nullptr;
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, GetInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest start";
    try {
        SettingDataHelper::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest failed";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest end";
}

/**
 * @tc.name: SetUserDataTest
 * @tc.desc: Verify the SetUserData function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, SetUserDataTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetUserDataTest start";
    try {
        SettingDataHelper::GetInstance().SetUserData(nullptr);
        EXPECT_EQ(SettingDataHelper::GetInstance().data_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetUserDataTest failed";
    }
    GTEST_LOG_(INFO) << "SetUserDataTest end";
}


/**
 * @tc.name: IsDataShareReadyTest001
 * @tc.desc: Verify the IsDataShareReady function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, IsDataShareReadyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDataShareReadyTest001 start";
    try {
        SettingDataHelper::GetInstance().isDataShareReady_ = true;
        bool ret = SettingDataHelper::GetInstance().IsDataShareReady();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsDataShareReadyTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsDataShareReadyTest001 end";
}

/**
 * @tc.name: IsDataShareReadyTest002
 * @tc.desc: Verify the IsDataShareReady function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, IsDataShareReadyTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDataShareReadyTest002 start";
    try {
        EXPECT_CALL(*mock_, Create()).WillOnce(Return(make_pair(DataShare::E_DATA_SHARE_NOT_READY, nullptr)));
        SettingDataHelper::GetInstance().isDataShareReady_ = false;
        bool ret = SettingDataHelper::GetInstance().IsDataShareReady();
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsDataShareReadyTest002 failed";
    }
    GTEST_LOG_(INFO) << "IsDataShareReadyTest002 end";
}

/**
 * @tc.name: IsDataShareReadyTest003
 * @tc.desc: Verify the IsDataShareReady function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, IsDataShareReadyTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDataShareReadyTest003 start";
    try {
        EXPECT_CALL(*mock_, Create()).WillOnce(Return(make_pair(DataShare::E_OK, nullptr)));
        SettingDataHelper::GetInstance().isDataShareReady_ = false;
        bool ret = SettingDataHelper::GetInstance().IsDataShareReady();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsDataShareReadyTest003 failed";
    }
    GTEST_LOG_(INFO) << "IsDataShareReadyTest003 end";
}

/**
 * @tc.name: IsDataShareReadyTest004
 * @tc.desc: Verify the IsDataShareReady function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, IsDataShareReadyTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDataShareReadyTest004 start";
    try {
        shared_ptr<DataShareHelperMock> helper =  std::make_shared<DataShareHelperMock>();;
        EXPECT_CALL(*helper, Release()).WillOnce(Return(true));
        EXPECT_CALL(*mock_, Create()).WillOnce(Return(make_pair(DataShare::E_OK, helper)));
        SettingDataHelper::GetInstance().isDataShareReady_ = false;
        bool ret = SettingDataHelper::GetInstance().IsDataShareReady();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsDataShareReadyTest004 failed";
    }
    GTEST_LOG_(INFO) << "IsDataShareReadyTest004 end";
}

/**
 * @tc.name: IsDataShareReadyTest005
 * @tc.desc: Verify the IsDataShareReady function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, IsDataShareReadyTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDataShareReadyTest005 start";
    try {
        EXPECT_CALL(*mock_, Create()).WillOnce(Return(make_pair(DataShare::E_ERROR, nullptr)));
        SettingDataHelper::GetInstance().isDataShareReady_ = false;
        bool ret = SettingDataHelper::GetInstance().IsDataShareReady();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsDataShareReadyTest005 failed";
    }
    GTEST_LOG_(INFO) << "IsDataShareReadyTest005 end";
}

/**
 * @tc.name: GetSwitchStatus001
 * @tc.desc: Verify the GetSwitchStatus function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, GetSwitchStatus001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatus001 start";
    try {
        EXPECT_CALL(*mock_, GetSwitchStatus()).WillOnce(Return(AI_FAMILY));
        string bundle = SettingDataHelper::GetInstance().GetActiveBundle();
        EXPECT_EQ(bundle, HDC_BUNDLE_NAME);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatus001 failed";
    }
    GTEST_LOG_(INFO) << "GetSwitchStatus001 end";
}

/**
 * @tc.name: GetSwitchStatus002
 * @tc.desc: Verify the GetSwitchStatus function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, GetSwitchStatus002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatus002 start";
    try {
        EXPECT_CALL(*mock_, GetSwitchStatus()).WillOnce(Return(CLOUD_SPACE));
        string bundle = SettingDataHelper::GetInstance().GetActiveBundle();
        EXPECT_EQ(bundle, GALLERY_BUNDLE_NAME);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatus002 failed";
    }
    GTEST_LOG_(INFO) << "GetSwitchStatus002 end";
}

/**
 * @tc.name: GetSwitchStatus003
 * @tc.desc: Verify the GetSwitchStatus function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, GetSwitchStatus003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSwitchStatus003 start";
    try {
        EXPECT_CALL(*mock_, GetSwitchStatus()).WillOnce(Return(SwitchStatus::NONE));
        string bundle = SettingDataHelper::GetInstance().GetActiveBundle();
        EXPECT_EQ(bundle, GALLERY_BUNDLE_NAME);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSwitchStatus003 failed";
    }
    GTEST_LOG_(INFO) << "GetSwitchStatus003 end";
}

/**
 * @tc.name: InitActiveBundle001
 * @tc.desc: Verify the InitActiveBundle function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, InitActiveBundle001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitActiveBundle001 start";
    try {
        SettingDataHelper::GetInstance().isBundleInited_ = true;
        bool ret = SettingDataHelper::GetInstance().InitActiveBundle();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitActiveBundle001 failed";
    }
    GTEST_LOG_(INFO) << "InitActiveBundle001 end";
}

/**
 * @tc.name: InitActiveBundle002
 * @tc.desc: Verify the InitActiveBundle function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, InitActiveBundle002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitActiveBundle002 start";
    try {
        SettingDataHelper::GetInstance().isBundleInited_ = false;
        SettingDataHelper::GetInstance().isDataShareReady_ = false;

        EXPECT_CALL(*mock_, Create()).WillOnce(Return(make_pair(DataShare::E_DATA_SHARE_NOT_READY, nullptr)));

        bool ret = SettingDataHelper::GetInstance().InitActiveBundle();
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitActiveBundle002 failed";
    }
    GTEST_LOG_(INFO) << "InitActiveBundle002 end";
}

/**
 * @tc.name: InitActiveBundle003
 * @tc.desc: Verify the InitActiveBundle function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, InitActiveBundle003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitActiveBundle003 start";
    try {
        SettingDataHelper::GetInstance().isBundleInited_ = false;
        SettingDataHelper::GetInstance().isDataShareReady_ = false;

        shared_ptr<DataShareHelperMock> helper =  std::make_shared<DataShareHelperMock>();
        EXPECT_CALL(*helper, Release()).WillOnce(Return(true));
        EXPECT_CALL(*mock_, Create()).WillOnce(Return(make_pair(DataShare::E_OK, helper)));
        EXPECT_CALL(*mock_, GetSwitchStatus()).WillOnce(Return(CLOUD_SPACE));

        bool ret = SettingDataHelper::GetInstance().InitActiveBundle();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitActiveBundle003 failed";
    }
    GTEST_LOG_(INFO) << "InitActiveBundle003 end";
}

/**
 * @tc.name: UpdateActiveBundle001
 * @tc.desc: Verify the UpdateActiveBundle function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, UpdateActiveBundle001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateActiveBundle001 start";
    try {
        EXPECT_CALL(*mock_, GetSwitchStatus()).WillOnce(Return(CLOUD_SPACE));

        SettingDataHelper::GetInstance().UpdateActiveBundle();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateActiveBundle001 failed";
    }
    GTEST_LOG_(INFO) << "UpdateActiveBundle001 end";
}

/**
 * @tc.name: RegisterObserver001
 * @tc.desc: Verify the RegisterObserver function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, RegisterObserver001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterObserver001 start";
    try {
        SettingDataHelper::GetInstance().RegisterObserver();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterObserver001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterObserver001 end";
}

/**
 * @tc.name: SetActiveBundle001
 * @tc.desc: Verify the SetActiveBundle function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, SetActiveBundle001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetActiveBundle001 start";
    try {
        SettingDataHelper::GetInstance().data_ = nullptr;
        SettingDataHelper::GetInstance().SetActiveBundle(GALLERY_BUNDLE_NAME);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetActiveBundle001 failed";
    }
    GTEST_LOG_(INFO) << "SetActiveBundle001 end";
}

/**
 * @tc.name: SetActiveBundle002
 * @tc.desc: Verify the SetActiveBundle function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, SetActiveBundle002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetActiveBundle002 start";
    try {
        struct FuseData data;
        data.activeBundle = GALLERY_BUNDLE_NAME;
        SettingDataHelper::GetInstance().data_ = &data;
        SettingDataHelper::GetInstance().SetActiveBundle(HDC_BUNDLE_NAME);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetActiveBundle002 failed";
    }
    GTEST_LOG_(INFO) << "SetActiveBundle002 end";
}

/**
 * @tc.name: SetActiveBundle003
 * @tc.desc: Verify the SetActiveBundle function
 * @tc.type: FUNC
 */
HWTEST_F(SettingDataHelperTest, SetActiveBundle003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetActiveBundle003 start";
    try {
        struct FuseData data;
        data.activeBundle = GALLERY_BUNDLE_NAME;
        SettingDataHelper::GetInstance().data_ = &data;
        SettingDataHelper::GetInstance().SetActiveBundle(GALLERY_BUNDLE_NAME);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetActiveBundle003 failed";
    }
    GTEST_LOG_(INFO) << "SetActiveBundle003 end";
}
} // namespace OHOS::FileManagement::CloudFile