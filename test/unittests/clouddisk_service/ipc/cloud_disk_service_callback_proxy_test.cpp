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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_disk_service_callback_proxy.h"

#include "cloud_disk_service_callback_mock.h"
#include "dfs_error.h"
#include "iremote_stub.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace std;
using namespace testing;
using namespace testing::ext;

class CloudDiskServiceCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<CloudDiskServiceCallbackProxy> proxy_ = nullptr;
    sptr<CloudDiskServiceCallbackMock> mock_ = nullptr;
};

void CloudDiskServiceCallbackProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskServiceCallbackProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskServiceCallbackProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    mock_ = sptr(new CloudDiskServiceCallbackMock());
    proxy_ = make_shared<CloudDiskServiceCallbackProxy>(mock_);
}

void CloudDiskServiceCallbackProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    mock_ = nullptr;
    proxy_ = nullptr;
}

/**
 * @tc.name: OnChangeDataTest001
 * @tc.desc: Verify the OnChangeData function with valid data and success response
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest001 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox/path";
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "file321";
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest001 end";
}

/**
 * @tc.name: OnChangeDataTest002
 * @tc.desc: Verify the OnChangeData function with valid data and failure response
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest002 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(-1));
        string sandboxPath = "/sandbox/path";
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "file321";
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest002 end";
}

/**
 * @tc.name: OnChangeDataTest003
 * @tc.desc: Verify the OnChangeData function with empty change data
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest003 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox/path";
        vector<ChangeData> changeData;
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest003 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest003 end";
}

/**
 * @tc.name: OnChangeDataTest004
 * @tc.desc: Verify the OnChangeData function with multiple change data items
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest004 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox/path";
        ChangeData data1;
        data1.updateSequenceNumber = 1;
        data1.fileId = "file1";
        ChangeData data2;
        data2.updateSequenceNumber = 2;
        data2.fileId = "file2";
        ChangeData data3;
        data3.updateSequenceNumber = 3;
        data3.fileId = "file3";
        vector<ChangeData> changeData = {data1, data2, data3};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest004 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest004 end";
}

/**
 * @tc.name: OnChangeDataTest005
 * @tc.desc: Verify the OnChangeData function with empty sandbox path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest005 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "";
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "file321";
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest005 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest005 end";
}

/**
 * @tc.name: OnChangeDataTest006
 * @tc.desc: Verify the OnChangeData function with long sandbox path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest006 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/very/long/sandbox/path/that/might/cause/issues/if/not/handled/properly";
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "file321";
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest006 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest006 end";
}

/**
 * @tc.name: OnChangeDataTest007
 * @tc.desc: Verify the OnChangeData function with large change data vector
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest007 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox/path";
        vector<ChangeData> changeData;
        for (int i = 0; i < 100; i++) {
            ChangeData data;
            data.updateSequenceNumber = i;
            data.fileId = "file" + to_string(i);
            changeData.push_back(data);
        }
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest007 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest007 end";
}

/**
 * @tc.name: OnChangeDataTest008
 * @tc.desc: Verify the OnChangeData function with zero sequence number
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest008 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox/path";
        ChangeData data;
        data.updateSequenceNumber = 0;
        data.fileId = "file321";
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest008 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest008 end";
}

/**
 * @tc.name: OnChangeDataTest009
 * @tc.desc: Verify the OnChangeData function with maximum sequence number
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest009 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox/path";
        ChangeData data;
        data.updateSequenceNumber = UINT32_MAX;
        data.fileId = "file321";
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest009 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest009 end";
}

/**
 * @tc.name: OnChangeDataTest010
 * @tc.desc: Verify the OnChangeData function with empty file id
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest010 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox/path";
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "";
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest010 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest010 end";
}

/**
 * @tc.name: OnChangeDataTest011
 * @tc.desc: Verify the OnChangeData function with long file id
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest011 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox/path";
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = string(1000, 'a');
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest011 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest011 end";
}
} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS
