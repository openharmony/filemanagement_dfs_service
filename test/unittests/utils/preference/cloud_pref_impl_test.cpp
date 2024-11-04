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
#include <sys/stat.h>
#include <unistd.h>
#include "utils_log.h"

#define USERID 100
namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativePreferences;

class PreferencesMock : public Preferences {
public:
    MOCK_METHOD2(Get, PreferencesValue(const std::string &key, const PreferencesValue &defValue));
    MOCK_METHOD2(Put, int(const std::string &key, const PreferencesValue &value));
    MOCK_METHOD2(GetInt, int(const std::string &key, const int &defValue));
    MOCK_METHOD2(GetString, std::string(const std::string &key, const std::string &defValue));
    MOCK_METHOD2(GetBool, bool(const std::string &key, const bool &defValue));
    MOCK_METHOD2(GetFloat, float(const std::string &key, const float &defValue));
    MOCK_METHOD2(GetDouble, double(const std::string &key, const double &defValue));
    MOCK_METHOD2(GetLong, int64_t(const std::string &key, const int64_t &defValue));
    MOCK_METHOD0(GetAll, std::map<std::string, PreferencesValue>());
    MOCK_METHOD1(HasKey, bool(const std::string &key));
    MOCK_METHOD2(PutInt, int(const std::string &key, int value));
    MOCK_METHOD2(PutString, int(const std::string &key, const std::string &value));
    MOCK_METHOD2(PutBool, int(const std::string &key, bool value));
    MOCK_METHOD2(PutLong, int(const std::string &key, int64_t value));
    MOCK_METHOD2(PutFloat, int(const std::string &key, float value));
    MOCK_METHOD2(PutDouble, int(const std::string &key, double value));
    MOCK_METHOD1(Delete, int(const std::string &key));
    MOCK_METHOD0(Clear, int());
    MOCK_METHOD0(Flush, void());
    MOCK_METHOD0(FlushSync, int());
    MOCK_METHOD2(RegisterObserver, int(std::shared_ptr<PreferencesObserver> preferencesObserver, RegisterMode mode));
    MOCK_METHOD2(UnRegisterObserver, int(std::shared_ptr<PreferencesObserver> preferencesObserver, RegisterMode mode));
};

class CloudPrefImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t userId_;
    std::string fileName_;
    std::shared_ptr<CloudPrefImpl> cloudPtr_;
    static inline std::shared_ptr<PreferencesMock> preferencesMock_;
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
    userId_ = USERID;
    fileName_ = "test";
    cloudPtr_ = std::make_shared<CloudPrefImpl>(fileName_);
    if (cloudPtr_ == nullptr) {
        GTEST_LOG_(INFO) << "cloudPtr_ == nullptr";
    }

    preferencesMock_ = std::make_shared<PreferencesMock>();
    if (preferencesMock_ == nullptr) {
        GTEST_LOG_(INFO) << "preferencesMock_ == nullptr";
    }
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudPrefImplTest::TearDown(void)
{
    if (cloudPtr_ != nullptr) {
        cloudPtr_ = nullptr;
    }

    if (preferencesMock_ != nullptr) {
        preferencesMock_ = nullptr;
    }
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
        CloudPrefImpl cloudPreImpl(fileName_);
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
        const std::string bundleName = "";
        std::string fileDir = "/data/service/el1/public/cloudfile/";
        std::string tableName = "testTable";
        EXPECT_EQ(access(fileDir.c_str(), F_OK), 0);
        CloudPrefImpl cloudPreImpl(userId_, bundleName, tableName);
        EXPECT_NE(cloudPreImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CloudPrefImplTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudPrefImplTest003 End";
}

/**
 * @tc.name: SetStringTest
 * @tc.desc: Verify the SetString function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetStringTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetStringTest Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        std::string key;
        std::string value;
        cloudPtr_->SetString(key, value);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetStringTest ERROR";
    }
    GTEST_LOG_(INFO) << "SetStringTest End";
}

/**
 * @tc.name: GetStringTest
 * @tc.desc: Verify the GetString function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetStringTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetStringTest Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        std::string key;
        std::string value;
        cloudPtr_->GetString(key, value);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetStringTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetStringTest End";
}

/**
 * @tc.name: SetIntTest
 * @tc.desc: Verify the SetInt function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetIntTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetIntTest Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        std::string key;
        int value = 0;
        cloudPtr_->SetInt(key, value);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetIntTest ERROR";
    }
    GTEST_LOG_(INFO) << "SetIntTest End";
}

/**
 * @tc.name: GetIntTest
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetIntTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIntTest Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        std::string key;
        int32_t value = 0;
        cloudPtr_->GetInt(key, value);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetIntTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetIntTest End";
}

/**
 * @tc.name: ClearTest
 * @tc.desc: Verify the Clear function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, ClearTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearTest Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        cloudPtr_->Clear();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ClearTest ERROR";
    }
    GTEST_LOG_(INFO) << "ClearTest End";
}

/**
 * @tc.name: DeleteTest
 * @tc.desc: Verify the Delete function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, DeleteTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteTest Start";
    try {
        EXPECT_NE(cloudPtr_->pref_, nullptr);
        std::string key;
        cloudPtr_->Delete(key);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " DeleteTest ERROR";
    }
    GTEST_LOG_(INFO) << "DeleteTest End";
}

HWTEST_F(CloudPrefImplTest, CloudPrefImpl_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CloudPrefImpl_001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "testBundle";
        string tableName = "testTable";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);
        EXPECT_NE(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CloudPrefImpl_001 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudPrefImpl_001 End";
}

HWTEST_F(CloudPrefImplTest, CloudPrefImpl_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CloudPrefImpl_002 Start";
    try {
        int32_t userId = -1;
        string bundleName = "testBundle";
        string tableName = "testTable";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);
        EXPECT_NE(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CloudPrefImpl_002 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudPrefImpl_002 End";
}

HWTEST_F(CloudPrefImplTest, CloudPrefImpl_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CloudPrefImpl_003 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "testTable";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);
        EXPECT_NE(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CloudPrefImpl_003 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudPrefImpl_003 End";
}

HWTEST_F(CloudPrefImplTest, CloudPrefImpl_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "CloudPrefImpl_004 Start";
    try {
        int32_t userId = 123;
        string bundleName = "test_key";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);
        EXPECT_EQ(cloudPrefImpl.pref_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CloudPrefImpl_004 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudPrefImpl_004 End";
}

HWTEST_F(CloudPrefImplTest, SetString_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetString_001 Start";
    try {
        PreferencesMock preferencesMock;
        string key = "";
        string value = "test_value";
        EXPECT_CALL(preferencesMock, PutString(_, _)).WillOnce(Return(X_OK));
        EXPECT_CALL(preferencesMock, FlushSync()).WillOnce(Return(X_OK));

        EXPECT_EQ(preferencesMock.PutString(key, value), X_OK);
        EXPECT_EQ(preferencesMock.FlushSync(), X_OK);
        cloudPtr_->SetString(key, value);
        cloudPtr_->GetString(key, value);
        EXPECT_EQ(key, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetString_001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetString_001 End";
}

HWTEST_F(CloudPrefImplTest, SetString_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetString_002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        string value = "";
        cloudPrefImpl.SetString(key, value);
        cloudPrefImpl.GetString(key, value);
        EXPECT_EQ(key, "test_key");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetString_002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetString_002 End";
}

HWTEST_F(CloudPrefImplTest, SetString_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetString_003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "test_key";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        string value = "test_value";
        cloudPrefImpl.SetString(key, value);
        cloudPrefImpl.GetString(key, value);
        EXPECT_EQ(value, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetString_003 ERROR";
    }
    GTEST_LOG_(INFO) << "SetString_003 End";
}

HWTEST_F(CloudPrefImplTest, SetString_004, TestSize.Level0)
{
    string key = "";
    string value = "test_value";

    EXPECT_CALL(*preferencesMock_, PutString(_, _)).WillOnce(Return(X_OK));
    EXPECT_CALL(*preferencesMock_, FlushSync()).WillOnce(Return(X_OK));

    cloudPtr_->SetString(key, value);
    EXPECT_EQ(key, "");
    EXPECT_EQ(preferencesMock_->PutString(key, value), X_OK);
    EXPECT_EQ(preferencesMock_->FlushSync(), X_OK);
}

HWTEST_F(CloudPrefImplTest, SetLong_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetLong_001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "test_key";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        int64_t value = -1;;
        cloudPrefImpl.SetLong(key, value);
        cloudPrefImpl.GetLong(key, value);
        EXPECT_EQ(key, "test_key");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetLong_001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetLong_001 End";
}

HWTEST_F(CloudPrefImplTest, SetLong_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetLong_002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "test_key";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        int64_t value = 89;
        cloudPrefImpl.SetLong(key, value);
        cloudPrefImpl.GetLong(key, value);
        EXPECT_EQ(key, "test_key");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetLong_002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetLong_002 End";
}

HWTEST_F(CloudPrefImplTest, SetLong_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetLong_003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "test_key";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "";
        int64_t value = -1;
        cloudPrefImpl.SetLong(key, value);
        cloudPrefImpl.GetLong(key, value);
        EXPECT_EQ(key, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetLong_003 ERROR";
    }
    GTEST_LOG_(INFO) << "SetLong_003 End";
}

HWTEST_F(CloudPrefImplTest, SetLong_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetLong_004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "test_key";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "";
        int64_t value = 89;
        cloudPrefImpl.SetLong(key, value);
        cloudPrefImpl.GetLong(key, value);
        EXPECT_EQ(key, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetLong_004 ERROR";
    }
    GTEST_LOG_(INFO) << "SetLong_004 End";
}

HWTEST_F(CloudPrefImplTest, SetInt_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetInt_001 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        int value = 1;
        cloudPrefImpl.SetInt(key, value);
        cloudPrefImpl.GetInt(key, value);
        EXPECT_EQ(key, "test_key");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetInt_001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetInt_001 End";
}

HWTEST_F(CloudPrefImplTest, SetInt_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetInt_002 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "";
        int value = 0;
        cloudPrefImpl.SetInt(key, value);
        cloudPrefImpl.GetInt(key, value);
        EXPECT_EQ(key, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetInt_002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetInt_002 End";
}

HWTEST_F(CloudPrefImplTest, SetInt_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetInt_003 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        int value = 0x7fffffff;
        cloudPrefImpl.SetInt(key, value);
        cloudPrefImpl.GetInt(key, value);
        EXPECT_EQ(key, "test_key");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetInt_003 ERROR";
    }
    GTEST_LOG_(INFO) << "SetInt_003 End";
}

HWTEST_F(CloudPrefImplTest, SetInt_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetInt_004 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        int value = 0xffffffff;
        cloudPrefImpl.SetInt(key, value);
        cloudPrefImpl.GetInt(key, value);
        EXPECT_EQ(key, "test_key");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetInt_004 ERROR";
    }
    GTEST_LOG_(INFO) << "SetInt_004 End";
}

HWTEST_F(CloudPrefImplTest, SetBool_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetBool_001 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        bool value = true;
        cloudPrefImpl.SetBool(key, value);
        cloudPrefImpl.GetBool(key, value);
        EXPECT_FALSE(value);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetBool_001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetBool_001 End";
}

HWTEST_F(CloudPrefImplTest, SetBool_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetBool_002 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        bool value = false;
        cloudPrefImpl.SetBool(key, value);
        cloudPrefImpl.GetBool(key, value);
        EXPECT_FALSE(value);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetBool_002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetBool_002 End";
}

HWTEST_F(CloudPrefImplTest, SetBool_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetBool_003 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        bool value = true;
        cloudPrefImpl.SetBool(key, value);
        cloudPrefImpl.GetBool(key, value);
        EXPECT_FALSE(value);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetBool_003 ERROR";
    }
    GTEST_LOG_(INFO) << "SetBool_003 End";
}

HWTEST_F(CloudPrefImplTest, SetBool_004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "SetBool_004 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        bool value = false;
        cloudPrefImpl.SetBool(key, value);
        cloudPrefImpl.GetBool(key, value);
        EXPECT_FALSE(value);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetBool_004 ERROR";
    }
    GTEST_LOG_(INFO) << "SetBool_004 End";
}

HWTEST_F(CloudPrefImplTest, Delete_001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Delete_001 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "test_key";
        cloudPrefImpl.Delete(key);
        EXPECT_EQ(key, "test_key");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Delete_001 ERROR";
    }
    GTEST_LOG_(INFO) << "Delete_001 End";
}

HWTEST_F(CloudPrefImplTest, Delete_002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Delete_001 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "";
        cloudPrefImpl.Delete(key);
        EXPECT_EQ(key, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Delete_002 ERROR";
    }
    GTEST_LOG_(INFO) << "Delete_002 End";
}

HWTEST_F(CloudPrefImplTest, Delete_003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "Delete_003 Start";
    try {
        int32_t userId = 123;
        string bundleName = "";
        string tableName = "";
        CloudPrefImpl cloudPrefImpl(userId, bundleName, tableName);

        string key = "/";
        cloudPrefImpl.Delete(key);
        EXPECT_EQ(key, "/");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Delete_003 ERROR";
    }
    GTEST_LOG_(INFO) << "Delete_003 End";
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
    std::string key;
    std::string value;
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->SetString(key, value);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "SetStringTest001 End";
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
    std::string key;
    std::string value;
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->GetString(key, value);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "GetStringTest001 End";
}

/**
 * @tc.name: SetLongTest001
 * @tc.desc: Verify the SetLong function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetLongTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetLongTest001 Start";
    std::string key;
    int64_t value = 0;
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->SetLong(key, value);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "SetLongTest001 End";
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
    std::string key;
    int64_t value = 0;
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->GetLong(key, value);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "GetLongTest001 End";
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
    std::string key;
    int value = 0;
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->SetInt(key, value);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "SetIntTest001 End";
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
    std::string key;
    int32_t value = 0;
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->GetInt(key, value);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "GetIntTest001 End";
}

/**
 * @tc.name: SetBoolTest001
 * @tc.desc: Verify the SetBool function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, SetBoolTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetBoolTest001 Start";
    std::string key;
    bool value = true;
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->SetBool(key, value);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "SetBoolTest001 End";
}

/**
 * @tc.name: GetBoolTest001
 * @tc.desc: Verify the GetBool function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudPrefImplTest, GetBoolTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBoolTest001 Start";
    std::string key;
    bool value = true;
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->GetBool(key, value);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "GetBoolTest001 End";
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
    cloudPtr_->pref_ = nullptr;
    cloudPtr_->Clear();
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "ClearTest001 End";
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
    cloudPtr_->pref_ = nullptr;
    std::string key;
    cloudPtr_->Delete(key);
    EXPECT_EQ(cloudPtr_->pref_, nullptr);
    
    GTEST_LOG_(INFO) << "DeleteTest001 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test