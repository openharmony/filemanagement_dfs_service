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

#include <gtest/gtest.h>

#include "cloud_upload_callback_client_manager.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

class CloudUploadCallbackClientManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudUploadCallbackClientManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudUploadCallbackClientManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudUploadCallbackClientManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudUploadCallbackClientManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: AddCallbackTest001
 * @tc.desc: Verify the AddCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackClientManagerTest, AddCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest001 start";
    try {
        UploadCallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudUploadCallback>();
        int32_t ret = CloudUploadCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest001 end";
}

/**
 * @tc.name: AddCallbackTest002
 * @tc.desc: Verify the AddCallback function callbackId = "";.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackClientManagerTest, AddCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest002 start";
    try {
        UploadCallbackInfo callbackInfo;
        callbackInfo.callbackId = "";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudUploadCallback>();
        int32_t ret = CloudUploadCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest002 end";
}

/**
 * @tc.name: AddCallbackTest003
 * @tc.desc: Verify the AddCallback function callbackId = "";.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackClientManagerTest, AddCallbackTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest003 start";
    try {
        UploadCallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = nullptr;
        int32_t ret = CloudUploadCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest003 end";
}

/**
 * @tc.name: RemoveCallbackTest001
 * @tc.desc: Verify the RemoveCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackClientManagerTest, RemoveCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveCallbackTest001 start";
    try {
        UploadCallbackInfo callbackInfo;
        callbackInfo.callbackId = "";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = nullptr;
        int32_t ret = CloudUploadCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RemoveCallbackTest001 end";
}

/**
 * @tc.name: RemoveCallbackTest002
 * @tc.desc: Verify the RemoveCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackClientManagerTest, RemoveCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveCallbackTest002 start";
    try {
        UploadCallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b68090";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudUploadCallback>();
        int32_t ret = CloudUploadCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_EQ(ret, E_OK);
        UploadCallbackInfo callbackInfo1;
        callbackInfo1.callbackId = "0x00000000000001";
        callbackInfo1.bundleName = "com.ohos.photos";
        callbackInfo1.callback = make_shared<CloudUploadCallback>();
        ret = CloudUploadCallbackClientManager::GetInstance().RemoveCallback(callbackInfo1);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveCallbackTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RemoveCallbackTest002 end";
}

/**
 * @tc.name: GetAllCallbackTest001
 * @tc.desc: Verify the GetAllCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackClientManagerTest, GetAllCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAllCallbackTest001 start";
    try {
        std::vector<UploadCallbackInfo> callbackInfos;
        int32_t ret = CloudUploadCallbackClientManager::GetInstance().GetAllCallback(callbackInfos);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAllCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetAllCallbackTest001 end";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS