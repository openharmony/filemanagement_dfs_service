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

#include "cloud_file_daemon_interface_code.h"
#include "ipc/cloud_daemon_stub.h"
#include "utils_log.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockCloudDaemonStub : public CloudDaemonStub {
public:
    MOCK_METHOD3(StartFuse, int32_t(int32_t userId, int32_t deviceFd, const std::string &path));
};

class CloudDaemonStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudDaemonStub> cloudDaemonStub_;
};

void CloudDaemonStubTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDaemonStubTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDaemonStubTest::SetUp(void)
{
    cloudDaemonStub_ = std::make_shared<MockCloudDaemonStub>();
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonStubTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonStubTest, OnRemoteRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequestTest Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        int ret = cloudDaemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(CloudFileDaemonInterfaceCode::CLOUD_DAEMON_CMD_START_FUSE), data, reply, option);
        EXPECT_EQ(ret, ICloudDaemon::CLOUD_DAEMON_DESCRIPTOR_IS_EMPTY);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnRemoteRequestTest  ERROR";
    }
    GTEST_LOG_(INFO) << "OnRemoteRequestTest End";
}

/**
 * @tc.name: OnRemoteRequestTest002
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonStubTest, OnRemoteRequestTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequestTest Start";
    try {
        int32_t code = 1000;
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;
        int ret = cloudDaemonStub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnRemoteRequestTest  ERROR";
    }
    GTEST_LOG_(INFO) << "OnRemoteRequestTest End";
}

/**
 * @tc.name: OnRemoteRequestTest003
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonStubTest, OnRemoteRequestTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequestTest Start";
    try {
        MessageParcel data;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudDaemon::GetDescriptor()));
        MessageParcel reply;
        MessageOption option;
        int ret = cloudDaemonStub_->OnRemoteRequest(
            static_cast<uint32_t>(CloudFileDaemonInterfaceCode::CLOUD_DAEMON_CMD_START_FUSE), data, reply, option);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnRemoteRequestTest  ERROR";
    }
    GTEST_LOG_(INFO) << "OnRemoteRequestTest End";
}

} // namespace OHOS::FileManagement::CloudSync::Test