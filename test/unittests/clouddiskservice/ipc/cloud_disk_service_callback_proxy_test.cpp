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
 * @tc.desc: Verify the OnChangeData function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest001 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
        string sandboxPath = "/sandbox";
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "file123";
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
 * @tc.desc: Verify the OnChangeData function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackProxyTest, OnChangeDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest002 start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(-1));
        string sandboxPath = "/sandbox";
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "file123";
        vector<ChangeData> changeData = {data};
        proxy_->OnChangeData(sandboxPath, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest002 end";
}
} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS