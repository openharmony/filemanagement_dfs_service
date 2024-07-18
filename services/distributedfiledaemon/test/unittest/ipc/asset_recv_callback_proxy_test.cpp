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
#include "asset_recv_callback_proxy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asset_obj.h"
#include "asset_recv_callback_mock.h"
#include "dfs_error.h"
#include "message_parcel_mock.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class AssetRecvCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<AssetRecvCallbackProxy> proxy_ = nullptr;
    static inline sptr<IAssetRecvCallbackMock> mock_ = nullptr;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void AssetRecvCallbackProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mock_ = sptr(new IAssetRecvCallbackMock());
    proxy_ = make_shared<AssetRecvCallbackProxy>(mock_);
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void AssetRecvCallbackProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mock_ = nullptr;
    proxy_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
}

void AssetRecvCallbackProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AssetRecvCallbackProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AssetRecvCallbackProxy_OnStart_0100
 * @tc.desc: The execution of the OnStart failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackProxyTest, AssetRecvCallbackProxy_OnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnStart_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    ASSERT_NE(proxy_, nullptr);
    auto ret = proxy_->OnStart("srcNetworkId", "dstNetworkId", "sessionId", "bundleName");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->OnStart("srcNetworkId", "dstNetworkId", "sessionId", "bundleName");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->OnStart("srcNetworkId", "dstNetworkId", "sessionId", "bundleName");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(false));
    ret = proxy_->OnStart("srcNetworkId", "dstNetworkId", "sessionId", "bundleName");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->OnStart("srcNetworkId", "dstNetworkId", "sessionId", "bundleName");
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnStart_0100 End";
}

/**
 * @tc.name: AssetRecvCallbackProxy_OnStart_0200
 * @tc.desc: The execution of the OnStart failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackProxyTest, AssetRecvCallbackProxy_OnStart_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnStart_0200 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    auto testProxy = make_shared<AssetRecvCallbackProxy>(nullptr);
    auto ret = testProxy->OnStart("srcNetworkId", "dstNetworkId", "sessionId", "bundleName");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ASSERT_NE(proxy_, nullptr);
    ret = proxy_->OnStart("srcNetworkId", "dstNetworkId", "sessionId", "bundleName");
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnStart_0200 End";
}

/**
 * @tc.name: AssetRecvCallbackProxy_OnStart_0300
 * @tc.desc: The execution of the OnStart success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackProxyTest, AssetRecvCallbackProxy_OnStart_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnStart_0300 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_OK));
    ASSERT_NE(proxy_, nullptr);
    auto ret = proxy_->OnStart("srcNetworkId", "dstNetworkId", "sessionId", "bundleName");
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnStart_0300 End";
}

/**
 * @tc.name: AssetRecvCallbackProxy_OnFinished_0100
 * @tc.desc: The execution of the OnFinished failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackProxyTest, AssetRecvCallbackProxy_OnFinished_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnFinished_0100 Start";
    sptr<AssetObj> assetObj (new (std::nothrow) AssetObj());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    ASSERT_NE(proxy_, nullptr);
    auto ret = proxy_->OnFinished("srcNetworkId", assetObj, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->OnFinished("srcNetworkId", assetObj, 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(false));
    ret = proxy_->OnFinished("srcNetworkId", assetObj, 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->OnFinished("srcNetworkId", assetObj, 0);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnFinished_0100 End";
}

/**
 * @tc.name: AssetRecvCallbackProxy_OnFinished_0200
 * @tc.desc: The execution of the OnFinished failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackProxyTest, AssetRecvCallbackProxy_OnFinished_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnFinished_0200 Start";
    sptr<AssetObj> assetObj (new (std::nothrow) AssetObj());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    auto testProxy = make_shared<AssetRecvCallbackProxy>(nullptr);
    auto ret = testProxy->OnFinished("srcNetworkId", assetObj, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ASSERT_NE(proxy_, nullptr);
    ret = proxy_->OnFinished("srcNetworkId", assetObj, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnFinished_0200 End";
}

/**
 * @tc.name: AssetRecvCallbackProxy_OnFinished_0300
 * @tc.desc: The execution of the OnFinished success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetRecvCallbackProxyTest, AssetRecvCallbackProxy_OnFinished_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnFinished_0300 Start";
    sptr<AssetObj> assetObj (new (std::nothrow) AssetObj());
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteParcelable(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_OK));
    ASSERT_NE(proxy_, nullptr);
    auto ret = proxy_->OnFinished("srcNetworkId", assetObj, 0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "AssetRecvCallbackProxy_OnFinished_0300 End";
}
}