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

#include "cloud_sync_common.h"
#include "cloud_upload_callback_stub.h"
#include "dfs_error.h"
#include "i_cloud_upload_callback.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockUploadCallback final : public CloudUploadCallbackStub {
public:
    MOCK_METHOD1(OnUploadProgress, void(const UploadProgressObj &progress));
};

class CloudUploadCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudUploadCallbackStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudUploadCallbackStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudUploadCallbackStubTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudUploadCallbackStubTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: HandleOnUploadProgressTest002
 * @tc.desc: Verify the HandleOnUploadProgress function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackStubTest, HandleOnUploadProgressTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnUploadProgressTest002 Start";
    try {
        MockUploadCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        UploadProgressObj progress;
        progress.state = UploadProgressObj::UploadState::WAITING;
        progress.processed = 1024;
        progress.size = 2048;
        progress.uri = "file://data/storage/el2/base/haps/entry/files/test.jpg";
        progress.error = UploadProgressObj::ErrorType::NO_ERROR;
        EXPECT_TRUE(progress.Marshalling(data));

        EXPECT_NE(E_OK,
                  callback.OnRemoteRequest(ICloudUploadCallback::SERVICE_CMD_ON_UPLOAD_PROCESS, data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnUploadProgressTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnUploadProgressTest002 End";
}

/**
 * @tc.name: HandleOnUploadProgressTest003
 * @tc.desc: Verify the HandleOnUploadProgress function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackStubTest, HandleOnUploadProgressTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnUploadProgressTest003 Start";
    try {
        MockUploadCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudUploadCallback::GetDescriptor()));
        callback.opToInterfaceMap_.clear();

        UploadProgressObj progress;
        progress.state = UploadProgressObj::UploadState::WAITING;
        progress.processed = 1024;
        progress.size = 2048;
        progress.uri = "file://data/storage/el2/base/haps/entry/files/test.jpg";
        progress.error = UploadProgressObj::ErrorType::NO_ERROR;
        EXPECT_TRUE(progress.Marshalling(data));

        EXPECT_NE(E_OK,
                  callback.OnRemoteRequest(ICloudUploadCallback::SERVICE_CMD_ON_UPLOAD_PROCESS, data, reply, option));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnUploadProgressTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnUploadProgressTest003 End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS