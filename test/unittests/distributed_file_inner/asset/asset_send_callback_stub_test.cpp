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
#include <asset/asset_send_callback_stub.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asset/asset_callback_interface_code.h"
#include "dfs_error.h"
#include "asset/i_asset_recv_callback.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockAssetSendCallbackStub final : public AssetSendCallbackStub {
public:
    MOCK_METHOD2(OnSendResult, int32_t(const sptr<AssetObj> &assetObj, int32_t result));
};

class AssetSendCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<MockAssetSendCallbackStub> mockStub_ = nullptr;
};

void AssetSendCallbackStubTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mockStub_ = make_shared<MockAssetSendCallbackStub>();
}

void AssetSendCallbackStubTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mockStub_ = nullptr;
}

void AssetSendCallbackStubTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AssetSendCallbackStubTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AssetSendCallbackStub_OnRemoteRequest_0100
 * @tc.desc: The execution of the OnRemoteRequest failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetSendCallbackStubTest, AssetSendCallbackStub_OnRemoteRequest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetSendCallbackStub_OnRemoteRequest_0100 Start";
    uint32_t code = static_cast<uint32_t>(AssetCallbackInterfaceCode::ASSET_CALLBACK_ON_SEND_RESULT);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IAssetRecvCallback::GetDescriptor());
    auto ret = mockStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, AssetSendCallbackStub::ASSET_SEND_CALLBACK_DESCRIPTOR_IS_EMPTY);
}

/**
 * @tc.name: AssetSendCallbackStub_OnRemoteRequest_0200
 * @tc.desc: The execution of the OnRemoteRequest failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetSendCallbackStubTest, AssetSendCallbackStub_OnRemoteRequest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetSendCallbackStub_OnRemoteRequest_0200 Start";
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IAssetSendCallback::GetDescriptor());
    uint32_t errCode = 65535;
    auto ret = mockStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);

    data.WriteInterfaceToken(IAssetSendCallback::GetDescriptor());
    mockStub_->opToInterfaceMap_.insert(make_pair(errCode, nullptr));
    ret = mockStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);
    mockStub_->opToInterfaceMap_.erase(errCode);
    GTEST_LOG_(INFO) << "AssetSendCallbackStub_OnRemoteRequest_0200 End";
}
/**
 * @tc.name: AssetSendCallbackStub_OnRemoteRequest_0300
 * @tc.desc: The execution of the OnRemoteRequest success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetSendCallbackStubTest, AssetSendCallbackStub_OnRemoteRequest_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetSendCallbackStub_OnRemoteRequest_0300 Start";
    uint32_t code = static_cast<uint32_t>(AssetCallbackInterfaceCode::ASSET_CALLBACK_ON_SEND_RESULT);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IAssetSendCallback::GetDescriptor());
    auto ret = mockStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_INVAL_ARG);

    data.WriteInterfaceToken(IAssetSendCallback::GetDescriptor());
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    data.WriteParcelable(assetObj);
    ret = mockStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_INVAL_ARG);

    data.WriteInterfaceToken(IAssetSendCallback::GetDescriptor());
    data.WriteParcelable(assetObj);
    data.WriteInt32(0);
    EXPECT_CALL(*mockStub_, OnSendResult(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
    ret = mockStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "AssetSendCallbackStub_OnRemoteRequest_0300 End";
}

/**
 * @tc.name: AssetSendCallbackStub_OnRemoteRequest_0400
 * @tc.desc: verify OnRemoteRequest.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetSendCallbackStubTest, AssetSendCallbackStub_OnRemoteRequest_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetSendCallbackStub_OnRemoteRequest_0400 Start";
    MessageParcel data;
    MessageParcel reply;
    data.WriteInterfaceToken(IAssetSendCallback::GetDescriptor());
    sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
    data.WriteParcelable(assetObj);
    data.WriteInt32(0);
    EXPECT_CALL(*mockStub_, OnSendResult(_, _)).WillOnce(Return(E_OK));
    auto ret = mockStub_->HandleOnSendResult(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "AssetSendCallbackStub_OnRemoteRequest_0400 End";
}
}