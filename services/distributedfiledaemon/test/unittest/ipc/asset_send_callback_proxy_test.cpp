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
#include "asset_send_callback_proxy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "iremote_stub.h"

#include "asset_obj.h"
#include "asset_send_callback_mock.h"
#include "dfs_error.h"
#include "message_parcel_mock.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class AssetSendCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<AssetSendCallbackProxy> proxy_ = nullptr;
    static inline sptr<IAssetSendCallbackMock> mock_ = nullptr;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void AssetSendCallbackProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mock_ = sptr(new IAssetSendCallbackMock());
    proxy_ = make_shared<AssetSendCallbackProxy>(mock_);
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void AssetSendCallbackProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mock_ = nullptr;
    proxy_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
}

void AssetSendCallbackProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AssetSendCallbackProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AssetSendCallbackProxyTest_OnSendResult_0100
 * @tc.desc: The execution of the OnSendResult failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetSendCallbackProxyTest, AssetSendCallbackProxyTest_OnSendResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetSendCallbackProxyTest_OnSendResult_0100 Start";
    sptr<AssetObj> assetObj (new (std::nothrow) AssetObj());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    ASSERT_NE(proxy_, nullptr);
    auto ret = proxy_->OnSendResult(assetObj, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(false));
    ret = proxy_->OnSendResult(assetObj, 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->OnSendResult(assetObj, 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    auto testProxy = make_shared<AssetSendCallbackProxy>(nullptr);
    ret = testProxy->OnSendResult(assetObj, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->OnSendResult(assetObj, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "AssetSendCallbackProxyTest_OnSendResult_0100 End";
}

/**
 * @tc.name: AssetSendCallbackProxyTest_OnSendResult_0200
 * @tc.desc: The execution of the OnSendResult success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetSendCallbackProxyTest, AssetSendCallbackProxyTest_OnSendResult_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetSendCallbackProxyTest_OnSendResult_0200 Start";
    sptr<AssetObj> assetObj (new (std::nothrow) AssetObj());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_OK));
    auto ret = proxy_->OnSendResult(assetObj, 0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "AssetSendCallbackProxyTest_OnSendResult_0200 End";
}
}