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

#include "cloud_pref_impl.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativePreferences;
constexpr int32_t CURRENT_USER = 100;
class CloudPrefImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<CloudPrefImpl> cloudPtr_{nullptr};
};

void CloudPrefImplTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudPrefImplTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudPrefImplTest::SetUp(void)
{
    auto preference = std::make_shared<Preferences>();
    auto preferencesHelper = NativePreferences::PreferencesHelper::GetInstance();
    EXPECT_CALL(*preferencesHelper, GetPreferences(_, _)).WillOnce(Return(preference));
    std::string fileName = "test";
    cloudPtr_ = std::make_shared<CloudPrefImpl>(fileName);
    if (cloudPtr_ == nullptr) {
        GTEST_LOG_(INFO) << "cloudPtr_ == nullptr";
    }
    if (cloudPtr_->pref_ == nullptr) {
        GTEST_LOG_(INFO) << "cloudPtr_->pref == nullptr";
    }
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudPrefImplTest::TearDown(void)
{
    NativePreferences::PreferencesHelper::DeleteInstance();
    cloudPtr_.reset();
    cloudPtr_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CloudPrefImplTest001
 * @tc.desc: Verify the CloudPrefImpl function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, CloudPrefImpTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudPrefImpTest001 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        CloudPrefImpl cloudPreImpl("");
        EXPECT_EQ(cloudPreImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CloudPrefImpTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudPrefImpTest001 End";
}

/**
 * @tc.name: CloudPrefImplTest002
 * @tc.desc: Verify the CloudPrefImpl function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, CloudPrefImpTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudPrefImpTest002 Start";
    try {
        auto preferences = std::make_shared<Preferences>();
        auto preferencesHelper = NativePreferences::PreferencesHelper::GetInstance();
        EXPECT_CALL(*preferencesHelper, GetPreferences(_, _)).WillOnce(Return(preferences));
        const std::string bundleName = "";
        std::string tableName = "testTable";
        CloudPrefImpl cloudPreImpl(CURRENT_USER, bundleName, tableName);
        EXPECT_NE(cloudPreImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CloudPrefImpTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudPrefImpTest002 End";
}

/**
 * @tc.name: CloudPrefImplTest003
 * @tc.desc: Verify the CloudPrefImpl function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, CloudPrefImplTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudPrefImplTest003 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        const std::string bundleName = "";
        std::string tableName = "testTable";
        CloudPrefImpl cloudPrefImpl(CURRENT_USER, bundleName, tableName);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CloudPrefImplTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudPrefImplTest003 End";
}

/**
 * @tc.name: SetStringTest001
 * @tc.desc: Verify the SetString function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetStringTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetStringTest001 Start";
    try {
        EXPECT_CALL(*(cloudPtr_->pref_), PutString(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*(cloudPtr_->pref_), FlushSync()).WillOnce(Return(-1));
        std::string key;
        std::string value;
        cloudPtr_->SetString(key, value);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetStringTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetStringTest001 End";
}

/**
 * @tc.name: SetStringTest002
 * @tc.desc: Verify the SetString function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetStringTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetStringTest002 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        CloudPrefImpl cloudPrefImpl("");
        std::string key;
        std::string value;
        cloudPrefImpl.SetString(key, value);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetStringTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetStringTest002 End";
}

/**
 * @tc.name: GetStringTest001
 * @tc.desc: Verify the GetString function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetStringTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetStringTest001 Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        std::string key;
        std::string value = "value";
        std::string ret;
        EXPECT_CALL(*(cloudPtr_->pref_), GetString(_, _)).WillOnce(Return(value));
        cloudPtr_->GetString(key, ret);
        EXPECT_EQ(value, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetStringTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetStringTest001 End";
}

/**
 * @tc.name: GetStringTest002
 * @tc.desc: Verify the GetString function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetStringTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetStringTest002 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        CloudPrefImpl cloudPrefImpl("");
        std::string key;
        std::string value = "value";
        cloudPrefImpl.GetString(key, value);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetStringTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetStringTest002 End";
}

/**
 * @tc.name: SetIntTest001
 * @tc.desc: Verify the SetInt function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetIntTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetIntTest001 Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        EXPECT_CALL(*(cloudPtr_->pref_), PutInt(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*(cloudPtr_->pref_), FlushSync()).WillOnce(Return(-1));
        std::string key;
        int value = 0;
        cloudPtr_->SetInt(key, value);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetIntTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetIntTest001 End";
}

/**
 * @tc.name: SetIntTest002
 * @tc.desc: Verify the SetInt function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetIntTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetIntTest002 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        CloudPrefImpl cloudPrefImpl("");
        std::string key;
        int value = 0;
        cloudPrefImpl.SetInt(key, value);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetIntTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetIntTest002 End";
}

/**
 * @tc.name: GetIntTest001
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetIntTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIntTest001 Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        std::string key;
        int32_t value = 0;
        int32_t ret = -1;
        EXPECT_CALL(*(cloudPtr_->pref_), GetInt(_, _)).WillOnce(Return(value));
        cloudPtr_->GetInt(key, ret);
        EXPECT_EQ(value, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetIntTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetIntTest001 End";
}

/**
 * @tc.name: GetIntTest002
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetIntTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIntTest002 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        CloudPrefImpl cloudPrefImpl("");
        std::string key;
        int32_t value = 0;
        cloudPrefImpl.GetInt(key, value);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetIntTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetIntTest002 End";
}

/**
 * @tc.name: ClearTest001
 * @tc.desc: Verify the Clear function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, ClearTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearTest001 Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        auto preferencesHelper = NativePreferences::PreferencesHelper::GetInstance();
        EXPECT_CALL(*preferencesHelper, DeletePreferences(_)).WillOnce(Return());
        EXPECT_CALL(*(cloudPtr_->pref_), Clear()).WillOnce(Return(0));
        cloudPtr_->Clear();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ClearTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "ClearTest001 End";
}

/**
 * @tc.name: ClearTest002
 * @tc.desc: Verify the Clear function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, ClearTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearTest002 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        CloudPrefImpl cloudPrefImpl("");
        cloudPrefImpl.Clear();
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ClearTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "ClearTest002 End";
}

/**
 * @tc.name: DeleteTest001
 * @tc.desc: Verify the Delete function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, DeleteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteTest001 Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        EXPECT_CALL(*(cloudPtr_->pref_), Delete(_)).WillOnce(Return(0));
        EXPECT_CALL(*(cloudPtr_->pref_), FlushSync()).WillOnce(Return(-1));
        std::string key;
        cloudPtr_->Delete(key);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " DeleteTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DeleteTest001 End";
}

/**
 * @tc.name: DeleteTest002
 * @tc.desc: Verify the Delete function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, DeleteTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteTest002 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        CloudPrefImpl cloudPrefImpl("");
        std::string key;
        cloudPrefImpl.Delete(key);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " DeleteTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DeleteTest002 End";
}

/**
 * @tc.name: SetLongTest
 * @tc.desc: Verify the SetLong function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetLong_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetLong_001 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        CloudPrefImpl cloudPrefImpl("");
        string key = "test_key";
        int64_t value = -1;
        ;
        cloudPrefImpl.SetLong(key, value);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetLong_001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetLong_001 End";
}

/**
 * @tc.name: SetLongTest
 * @tc.desc: Verify the SetLong function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetLong_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetLong_002 Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        EXPECT_CALL(*(cloudPtr_->pref_), PutLong(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*(cloudPtr_->pref_), FlushSync()).WillOnce(Return(-1));
        string key = "test_key";
        int64_t value = 89;
        cloudPtr_->SetLong(key, value);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetLong_002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetLong_002 End";
}

/**
 * @tc.name: GetLongTest001
 * @tc.desc: Verify the GetLong function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetLongTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLongTest001 Start";
    NativePreferences::PreferencesHelper::DeleteInstance();
    CloudPrefImpl cloudPrefImpl("");
    std::string key;
    int64_t value = 0;
    cloudPrefImpl.GetLong(key, value);
    EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    GTEST_LOG_(INFO) << "GetLongTest001 End";
}

/**
 * @tc.name: GetLongTest002
 * @tc.desc: Verify the GetLong function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetLongTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLongTest002 Start";
    EXPECT_NE(cloudPtr_->pref_, nullptr);
    std::string key;
    int64_t value = 0;
    int64_t ret = -1;
    EXPECT_CALL(*(cloudPtr_->pref_), GetLong(_, _)).WillOnce(Return(value));
    cloudPtr_->GetLong(key, ret);
    EXPECT_EQ(value, ret);
    GTEST_LOG_(INFO) << "GetLongTest002 End";
}

/**
 * @tc.name: SetBoolTest
 * @tc.desc: Verify the SetLong function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetBool_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetBool_001 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        bool value = true;
        cloudPrefImpl.SetBool(key, value);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetBool_001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetBool_001 End";
}

/**
 * @tc.name: SetBoolTest
 * @tc.desc: Verify the SetLong function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetBool_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetBool_002 Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        EXPECT_CALL(*(cloudPtr_->pref_), PutBool(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*(cloudPtr_->pref_), FlushSync()).WillOnce(Return(-1));
        string key = "test_key1";
        bool value = false;
        cloudPtr_->SetBool(key, value);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetBool_002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetBool_002 End";
}

/**
 * @tc.name: GetBoolTest
 * @tc.desc: Verify the GetBool function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetBoolTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetBoolTest001 Start";
    try {
        NativePreferences::PreferencesHelper::DeleteInstance();
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        bool value = true;
        cloudPrefImpl.GetBool(key, value);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetBoolTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBoolTest001 End";
}

/**
 * @tc.name: GetBoolTest002
 * @tc.desc: Verify the GetBool function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetBoolTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBoolTest002 Start";
    EXPECT_NE(cloudPtr_->pref_, nullptr);
    std::string key;
    bool value = true;
    bool ret = false;
    EXPECT_CALL(*(cloudPtr_->pref_), GetBool(_, _)).WillOnce(Return(value));
    cloudPtr_->GetBool(key, ret);
    EXPECT_EQ(value, ret);

    GTEST_LOG_(INFO) << "GetBoolTest002 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test