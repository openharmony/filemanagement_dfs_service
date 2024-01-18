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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cloud_download_callback_stub.h"
#include "cloud_download_uri_manager.h"
#include "i_cloud_download_callback_mock.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDownloadCallbackStubMock : public CloudDownloadCallbackStub {
public:
    CloudDownloadCallbackStubMock() {}
    void OnDownloadProcess(const DownloadProgressObj& progress) override
    {}
};

class CloudDownloadCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudDownloadCallbackStub> cloudDownloadCallbackStub_;
};

void CloudDownloadCallbackStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDownloadCallbackStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDownloadCallbackStubTest::SetUp(void)
{
    cloudDownloadCallbackStub_ = std::make_shared<CloudDownloadCallbackStubMock>();
    std::cout << "SetUp" << std::endl;
}

void CloudDownloadCallbackStubTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackStubTest, OnRemoteRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_EQ(E_SERVICE_DESCRIPTOR_IS_EMPTY, cloudDownloadCallbackStub_->OnRemoteRequest(
            ICloudDownloadCallback::SERVICE_CMD_ON_PROCESS, data, reply, option));
    } catch(...) {
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
HWTEST_F(CloudDownloadCallbackStubTest, OnRemoteRequestTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        uint32_t code = 100;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudDownloadCallback::GetDescriptor()));
        int32_t ret = cloudDownloadCallbackStub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_NE(E_OK, ret);
    } catch(...) {
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
HWTEST_F(CloudDownloadCallbackStubTest, OnRemoteRequestTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        uint32_t code = ICloudDownloadCallback::SERVICE_CMD_ON_PROCESS;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudDownloadCallback::GetDescriptor()));
        int32_t ret = cloudDownloadCallbackStub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_NE(E_OK, ret);
    } catch(...) {
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
HWTEST_F(CloudDownloadCallbackStubTest, HandleOnProcessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnProcess Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        DownloadProgressObj progress;
        progress.path = "/data";
        bool prog = data.WriteParcelable(&progress);
        EXPECT_TRUE(prog);
        int ret = cloudDownloadCallbackStub_->HandleOnProcess(data, reply);
        EXPECT_EQ(E_OK, ret);
    } catch(...) {
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
HWTEST_F(CloudDownloadCallbackStubTest, HandleOnProcessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnProcess Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        DownloadProgressObj progress;
        progress.path = "";
        bool prog = data.WriteParcelable(&progress);
        EXPECT_TRUE(prog);
        int32_t ret = cloudDownloadCallbackStub_->HandleOnProcess(data, reply);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnProcess ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnProcess End";
}

/**
 * @tc.name: HandleOnProcessTest003
 * @tc.desc: Verify the HandleOnProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackStubTest, HandleOnProcessTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnProcess Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        DownloadProgressObj progress;
        progress.path = "/data";
        progress.state = DownloadProgressObj::STOPPED;
        bool prog = data.WriteParcelable(&progress);
        EXPECT_TRUE(prog);
        cloudDownloadCallbackStub_->HandleOnProcess(data, reply);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnProcess ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnProcess End";
}

} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS