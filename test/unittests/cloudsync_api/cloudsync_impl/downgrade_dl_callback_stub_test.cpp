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

#include "cloud_download_callback_stub.h"
#include "cloud_download_uri_manager.h"
#include "dfs_error.h"
#include "downgrade_download_callback_stub.h"
#include "i_cloud_download_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DowngradeDownloadCallbackStubMock : public DowngradeDownloadCallbackStub {
public:
    DowngradeDownloadCallbackStubMock() {}
    void OnDownloadProcess(const DowngradeProgress &progress) override {}
};

class DowngradeDownloadCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DowngradeDownloadCallbackStub> downgradeStub_;
};

void DowngradeDownloadCallbackStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void DowngradeDownloadCallbackStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void DowngradeDownloadCallbackStubTest::SetUp(void)
{
    downgradeStub_ = std::make_shared<DowngradeDownloadCallbackStubMock>();
    std::cout << "SetUp" << std::endl;
}

void DowngradeDownloadCallbackStubTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DowngradeDownloadCallbackStubTest, OnRemoteRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_EQ(E_SERVICE_DESCRIPTOR_IS_EMPTY,
                  downgradeStub_->OnRemoteRequest(ICloudDownloadCallback::SERVICE_CMD_ON_PROCESS, data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnRemoteRequest ERROR";
    }
    GTEST_LOG_(INFO) << "OnRemoteRequest End";
}

/**
 * @tc.name: OnRemoteRequestTest002
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DowngradeDownloadCallbackStubTest, OnRemoteRequestTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        uint32_t code = 100;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudDownloadCallback::GetDescriptor()));
        int32_t ret = downgradeStub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnRemoteRequest ERROR";
    }
    GTEST_LOG_(INFO) << "OnRemoteRequest End";
}

/**
 * @tc.name: OnRemoteRequestTest003
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(DowngradeDownloadCallbackStubTest, OnRemoteRequestTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        uint32_t code = ICloudDownloadCallback::SERVICE_CMD_ON_PROCESS;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudDownloadCallback::GetDescriptor()));
        int32_t ret = downgradeStub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnRemoteRequest ERROR";
    }
    GTEST_LOG_(INFO) << "OnRemoteRequest End";
}

/**
 * @tc.name: HandleOnProcessTest001
 * @tc.desc: Verify the HandleOnProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DowngradeDownloadCallbackStubTest, HandleOnProcessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnProcess Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        DowngradeProgress progress;
        bool prog = data.WriteParcelable(&progress);
        EXPECT_TRUE(prog);
        int ret = downgradeStub_->HandleOnProcess(data, reply);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnProcess ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnProcess End";
}

/**
 * @tc.name: HandleOnProcessTest002
 * @tc.desc: Verify the HandleOnProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DowngradeDownloadCallbackStubTest, HandleOnProcessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnProcess Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        int32_t ret = downgradeStub_->HandleOnProcess(data, reply);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnProcess ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnProcess End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS