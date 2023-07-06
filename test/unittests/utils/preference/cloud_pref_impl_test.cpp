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

class CloudPrefImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    int32_t userId_;
    std::string fileName_;
    std::shared_ptr<CloudPrefImpl> cloudPtr_;
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
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudPrefImplTest::TearDown(void)
{
    if (cloudPtr_ != nullptr) {
        cloudPtr_ = nullptr;
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

} // namespace OHOS::FileManagement::CloudSync::Test