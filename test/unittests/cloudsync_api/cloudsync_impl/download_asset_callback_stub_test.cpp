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

#include "asset_callback_mock.h"
#include "cloud_download_uri_manager.h"
#include "dfs_error.h"
#include "download_asset_callback_stub.h"
#include "message_parcel_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::Storage::DistributedFile;

class DownloadAssetCallbackStubMock : public DownloadAssetCallbackStub {
public:
    DownloadAssetCallbackStubMock() {}
    void OnFinished(const TaskId taskId, const std::string &uri, const int32_t result) override {}
};

class DownloadAssetCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DownloadAssetCallbackStub> downloadAssetCallbackStub_;
    std::shared_ptr<MessageParcelMock> messageParcelMock_;
};

void DownloadAssetCallbackStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void DownloadAssetCallbackStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void DownloadAssetCallbackStubTest::SetUp(void)
{
    downloadAssetCallbackStub_ = std::make_shared<DownloadAssetCallbackStubMock>();
    messageParcelMock_ = std::make_shared<MessageParcelMock>();
    DfsMessageParcel::messageParcel = messageParcelMock_;
    std::cout << "SetUp" << std::endl;
}

void DownloadAssetCallbackStubTest::TearDown(void)
{
    DfsMessageParcel::messageParcel = nullptr;
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DownloadAssetCallbackStubTest, OnRemoteRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_EQ(E_SERVICE_DESCRIPTOR_IS_EMPTY,
                  downloadAssetCallbackStub_->OnRemoteRequest(IDownloadAssetCallback::SERVICE_CMD_ON_DOWNLOAD_FINSHED,
                                                              data, reply, option));
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
HWTEST_F(DownloadAssetCallbackStubTest, OnRemoteRequestTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        uint32_t code = 100;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken())
            .WillOnce(Return(IDownloadAssetCallback::GetDescriptor()));
        int32_t ret = downloadAssetCallbackStub_->OnRemoteRequest(code, data, reply, option);
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
HWTEST_F(DownloadAssetCallbackStubTest, OnRemoteRequestTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest Start";
    try {
        uint32_t code = IDownloadAssetCallback::SERVICE_CMD_ON_DOWNLOAD_FINSHED;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken())
            .WillOnce(Return(IDownloadAssetCallback::GetDescriptor()));
        int32_t ret = downloadAssetCallbackStub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnRemoteRequest ERROR";
    }
    GTEST_LOG_(INFO) << "OnRemoteRequest End";
}

/**
 * @tc.name: HandleOnFinishedTest002
 * @tc.desc: Verify HandleOnFinished function - ReadUint64(taskId) failed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DownloadAssetCallbackStubTest, HandleOnFinishedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFinishedTest002 Start";
    try {
        EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(false));

        MessageParcel data;
        MessageParcel reply;
        int ret = downloadAssetCallbackStub_->HandleOnFinished(data, reply);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFinishedTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnFinishedTest002 End";
}

/**
 * @tc.name: HandleOnFinishedTest003
 * @tc.desc: Verify HandleOnFinished function - ReadString(uri) failed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DownloadAssetCallbackStubTest, HandleOnFinishedTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFinishedTest003 Start";
    try {
        EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));

        MessageParcel data;
        MessageParcel reply;
        int ret = downloadAssetCallbackStub_->HandleOnFinished(data, reply);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFinishedTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnFinishedTest003 End";
}

/**
 * @tc.name: HandleOnFinishedTest004
 * @tc.desc: Verify HandleOnFinished function - ReadInt32(result) failed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DownloadAssetCallbackStubTest, HandleOnFinishedTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFinishedTest004 Start";
    try {
        EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("uri"), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));

        MessageParcel data;
        MessageParcel reply;
        int ret = downloadAssetCallbackStub_->HandleOnFinished(data, reply);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFinishedTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnFinishedTest004 End";
}

/**
 * @tc.name: HandleOnFinishedTest005
 * @tc.desc: Verify HandleOnFinished function - all reads succeed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DownloadAssetCallbackStubTest, HandleOnFinishedTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFinishedTest005 Start";
    try {
        EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("uri"), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(true)));

        MessageParcel data;
        MessageParcel reply;
        int ret = downloadAssetCallbackStub_->HandleOnFinished(data, reply);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFinishedTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnFinishedTest005 End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
