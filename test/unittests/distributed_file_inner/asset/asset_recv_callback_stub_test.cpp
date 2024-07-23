/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <asset/asset_recv_callback_stub.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"
#include "asset/asset_callback_interface_code.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockAssetRecvCallbackStubStub final : public AssetRecvCallbackStub {
public:
    MOCK_METHOD4(OnStart, int32_t(const std::string &srcNetworkId,
                                  const std::string &dstNetworkId,
                                  const std::string &sessionId,
                                  const std::string &dstBundleName));
    MOCK_METHOD3(OnFinished, int32_t(const std::string &srcNetworkId,
                                     const sptr<AssetObj> &assetObj,
                                     int32_t result));
};

class AssetRecvCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<MockAssetRecvCallbackStubStub> mockStub_ = nullptr;
};

void AssetRecvCallbackStubTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mockStub_ = make_shared<MockAssetRecvCallbackStubStub>();
}

void AssetRecvCallbackStubTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mockStub_ = nullptr;
}

void AssetRecvCallbackStubTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AssetRecvCallbackStubTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AssetRecvCallbackStub_OnRemoteRequest_0100
 * @tc.desc: The execution of the OnRemoteRequest failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackStubTest, AssetRecvCallbackStub_OnRemoteRequest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_OnRemoteRequest_0100 Start";
    uint32_t code = static_cast<uint32_t>(AssetCallbackInterfaceCode::ASSET_CALLBACK_ON_START);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto ret = mockStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, AssetRecvCallbackStub::ASSET_RECV_CALLBACK_DESCRIPTOR_IS_EMPTY);

    data.WriteInterfaceToken(IAssetRecvCallback::GetDescriptor());
    uint32_t errCode = 65535;
    ret = mockStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);

    mockStub_->opToInterfaceMap_.insert(make_pair(errCode, nullptr));
    data.WriteInterfaceToken(IAssetRecvCallback::GetDescriptor());
    ret = mockStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);
    mockStub_->opToInterfaceMap_.erase(errCode);
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_OnRemoteRequest_0100 End";
}

/**
 * @tc.name: AssetRecvCallbackStub_OnRemoteRequest_0200
 * @tc.desc: The execution of the OnRemoteRequest success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackStubTest, AssetRecvCallbackStub_OnRemoteRequest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_OnRemoteRequest_0200 Start";
    uint32_t code = static_cast<uint32_t>(AssetCallbackInterfaceCode::ASSET_CALLBACK_ON_START);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IAssetRecvCallback::GetDescriptor());
    data.WriteString("srcNetworkId");
    data.WriteString("destNetworkId");
    data.WriteString("sessionId");
    data.WriteString("dstBundleName");

    EXPECT_CALL(*mockStub_, OnStart(_, _, _, _)).WillOnce(Return(E_OK));
    auto ret = mockStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_OnRemoteRequest_0200 End";
}

/**
 * @tc.name: AssetRecvCallbackStub_HandleOnStart_0100
 * @tc.desc: verify HandleOnStart.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackStubTest, AssetRecvCallbackStub_HandleOnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_HandleOnStart_0100 Start";
    MessageParcel data;
    MessageParcel reply;
    auto ret = mockStub_->HandleOnStart(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    data.WriteString("srcNetworkId");
    ret = mockStub_->HandleOnStart(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    data.WriteString("srcNetworkId");
    data.WriteString("destNetworkId");
    ret = mockStub_->HandleOnStart(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    data.WriteString("srcNetworkId");
    data.WriteString("destNetworkId");
    data.WriteString("sessionId");
    ret = mockStub_->HandleOnStart(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    data.WriteString("srcNetworkId");
    data.WriteString("destNetworkId");
    data.WriteString("sessionId");
    data.WriteString("dstBundleName");
    EXPECT_CALL(*mockStub_, OnStart(_, _, _, _)).WillOnce(Return(E_PERMISSION_DENIED));
    ret = mockStub_->HandleOnStart(data, reply);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_HandleOnStart_0100 End";
}

/**
 * @tc.name: AssetRecvCallbackStub_HandleOnFinished_0100
 * @tc.desc: verify HandleOnFinished.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackStubTest, AssetRecvCallbackStub_HandleOnFinished_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_HandleOnFinished_0100 Start";
    MessageParcel data;
    MessageParcel reply;
    auto ret = mockStub_->HandleOnFinished(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    
    data.WriteString("srcNetworkId");
    ret = mockStub_->HandleOnFinished(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    data.WriteString("srcNetworkId");
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    data.WriteParcelable(assetObj);
    ret = mockStub_->HandleOnFinished(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    data.WriteString("srcNetworkId");
    data.WriteParcelable(assetObj);
    data.WriteInt32(0);
    EXPECT_CALL(*mockStub_, OnFinished(_, _, _)).WillOnce(Return(E_PERMISSION_DENIED));
    ret = mockStub_->HandleOnFinished(data, reply);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_HandleOnFinished_0100 End";
}

/**
 * @tc.name: AssetRecvCallbackStub_HandleOnFinished_0200
 * @tc.desc: verify HandleOnFinished.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackStubTest, AssetRecvCallbackStub_HandleOnFinished_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_HandleOnFinished_0200 Start";
    MessageParcel data;
    MessageParcel reply;

    data.WriteString("srcNetworkId");
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    data.WriteParcelable(assetObj);
    data.WriteInt32(0);
    EXPECT_CALL(*mockStub_, OnFinished(_, _, _)).WillOnce(Return(E_OK));
    auto ret = mockStub_->HandleOnFinished(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "AssetRecvCallbackStub_HandleOnFinished_0200 End";
}
}