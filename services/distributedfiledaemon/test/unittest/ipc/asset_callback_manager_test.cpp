/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "asset_callback_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "asset_obj.h"
#include "asset_recv_callback_mock.h"
#include "asset_send_callback_mock.h"
#include "dfs_error.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class AssetCallbackManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AssetCallbackManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void AssetCallbackManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AssetCallbackManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AssetCallbackManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AssetCallbackManager_AddRecvCallback_0100
 * @tc.desc: verify AddRecvCallback failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetCallbackManagerTest, AssetCallbackManager_AddRecvCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetCallbackManager_AddRecvCallback_0100 Start";
    AssetCallbackManager::GetInstance().AddRecvCallback(nullptr);
    AssetCallbackManager::GetInstance().recvCallbackList_.emplace_back(nullptr);
    EXPECT_EQ(AssetCallbackManager::GetInstance().recvCallbackList_.size(), 1); // 1: vec size
    auto recvCallback = sptr(new IAssetRecvCallbackMock());
    AssetCallbackManager::GetInstance().AddRecvCallback(recvCallback);
    EXPECT_EQ(AssetCallbackManager::GetInstance().recvCallbackList_.size(), 2); // 2: vec size
    AssetCallbackManager::GetInstance().AddRecvCallback(recvCallback);
    EXPECT_EQ(AssetCallbackManager::GetInstance().recvCallbackList_.size(), 2); // 2: vec size
    auto recvCallback2 = sptr(new IAssetRecvCallbackMock());
    AssetCallbackManager::GetInstance().AddRecvCallback(recvCallback2);
    EXPECT_EQ(AssetCallbackManager::GetInstance().recvCallbackList_.size(), 3); // 3: vec size
    AssetCallbackManager::GetInstance().recvCallbackList_.clear();
    GTEST_LOG_(INFO) << "AssetCallbackManager_AddRecvCallback_0100 End";
}

/**
 * @tc.name: AssetCallbackManager_RemoveRecvCallback_0100
 * @tc.desc: verify RemoveRecvCallback.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetCallbackManagerTest, AssetCallbackManager_RemoveRecvCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetCallbackManager_AddRecvCallback_0100 Start";
    auto recvCallback = sptr(new IAssetRecvCallbackMock());
    AssetCallbackManager::GetInstance().AddRecvCallback(recvCallback);
    AssetCallbackManager::GetInstance().RemoveRecvCallback(nullptr);
    EXPECT_EQ(AssetCallbackManager::GetInstance().recvCallbackList_.size(), 1);
    AssetCallbackManager::GetInstance().RemoveRecvCallback(recvCallback);
    EXPECT_EQ(AssetCallbackManager::GetInstance().recvCallbackList_.size(), 0);
    GTEST_LOG_(INFO) << "AssetCallbackManager_AddRecvCallback_0100 End";
}

/**
 * @tc.name: AssetCallbackManager_SendCallback_0100
 * @tc.desc: verify AddSendCallback and RemoveSendCallback.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetCallbackManagerTest, AssetCallbackManager_SendCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetCallbackManager_SendCallback_0100 Start";
    auto sendCallback = sptr(new IAssetSendCallbackMock());
    AssetCallbackManager::GetInstance().AddSendCallback("", sendCallback);
    EXPECT_EQ(AssetCallbackManager::GetInstance().sendCallbackMap_.size(), 0);
    AssetCallbackManager::GetInstance().AddSendCallback("test", sendCallback);
    EXPECT_EQ(AssetCallbackManager::GetInstance().sendCallbackMap_.size(), 1);
    AssetCallbackManager::GetInstance().AddSendCallback("test", sendCallback);
    EXPECT_EQ(AssetCallbackManager::GetInstance().sendCallbackMap_.size(), 1);

    AssetCallbackManager::GetInstance().RemoveSendCallback("test2");
    EXPECT_EQ(AssetCallbackManager::GetInstance().sendCallbackMap_.size(), 1);
    AssetCallbackManager::GetInstance().RemoveSendCallback("test");
    EXPECT_EQ(AssetCallbackManager::GetInstance().sendCallbackMap_.size(), 0);
    GTEST_LOG_(INFO) << "AssetCallbackManager_SendCallback_0100 End";
}

/**
 * @tc.name: AssetCallbackManager_NotifyAssetRecv_0100
 * @tc.desc: verify NotifyAssetRecvStart, NotifyAssetRecvProgress and NotifyAssetRecvFinished.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetCallbackManagerTest, AssetCallbackManager_NotifyAssetRecv_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetCallbackManager_NotifyAssetRecv_0100 Start";
    auto recvCallback = sptr(new IAssetRecvCallbackMock());
    // OnStart OnFinished 预期只能调用一次
    EXPECT_CALL(*recvCallback, OnStart(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*recvCallback, OnRecvProgress(_, _, _, _)).Times(3).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*recvCallback, OnFinished(_, _, _)).Times(2).WillRepeatedly(Return(E_OK));
    try {
        AssetCallbackManager::GetInstance().AddRecvCallback(recvCallback);
        AssetCallbackManager::GetInstance().recvCallbackList_.emplace_back(nullptr);
        AssetCallbackManager::GetInstance().NotifyAssetRecvStart("srcNetworkId", "dstNetworkId",
            "sessionId", "dstBundleName");

        sptr<AssetObj> assetObj1 (new (std::nothrow) AssetObj());
        ASSERT_TRUE(assetObj1 != nullptr) << "assetObj1 assert failed!";
        AssetCallbackManager::GetInstance().NotifyAssetRecvProgress("srcNetworkId", assetObj1, 1024, 256);

        sptr<AssetObj> assetObj2 (new (std::nothrow) AssetObj());
        ASSERT_TRUE(assetObj2 != nullptr) << "assetObj2 assert failed!";
        AssetCallbackManager::GetInstance().NotifyAssetRecvProgress("srcNetworkId", nullptr, 1024, 512);
        AssetCallbackManager::GetInstance().NotifyAssetRecvProgress("srcNetworkId", assetObj2, 1024, 512);

        sptr<AssetObj> assetObj3 (new (std::nothrow) AssetObj());
        ASSERT_TRUE(assetObj3 != nullptr) << "assetObj3 assert failed!";
        AssetCallbackManager::GetInstance().NotifyAssetRecvFinished("srcNetworkId", nullptr, 0);
        AssetCallbackManager::GetInstance().NotifyAssetRecvFinished("srcNetworkId", assetObj3, 0);
        AssetCallbackManager::GetInstance().recvCallbackList_.clear();
        EXPECT_TRUE(true);
    } catch(...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "AssetCallbackManager_NotifyAssetRecv_0100 End";
}

/**
 * @tc.name: AssetCallbackManager_NotifyAssetSendResult_0100
 * @tc.desc: verify NotifyAssetSendResult.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetCallbackManagerTest, AssetCallbackManager_NotifyAssetSendResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AssetCallbackManager_NotifyAssetSendResult_0100 Start";
    auto sendCallback = sptr(new IAssetSendCallbackMock());
    // 预期只能调用一次
    EXPECT_CALL(*sendCallback, OnSendResult(_, _)).WillOnce(Return(E_OK));
    try {
        AssetCallbackManager::GetInstance().AddSendCallback("test", sendCallback);
        AssetCallbackManager::GetInstance().AddSendCallback("test2", nullptr);

        sptr<AssetObj> assetObj (new (std::nothrow) AssetObj());
        ASSERT_TRUE(assetObj != nullptr) << "assetObj assert failed!";
        AssetCallbackManager::GetInstance().NotifyAssetSendResult("test3", assetObj, 0);
        AssetCallbackManager::GetInstance().NotifyAssetSendResult("test", assetObj, 0);
        AssetCallbackManager::GetInstance().NotifyAssetSendResult("test2", assetObj, 0);
        AssetCallbackManager::GetInstance().sendCallbackMap_.clear();
        EXPECT_TRUE(true);
    } catch(...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "AssetCallbackManager_NotifyAssetSendResult_0100 End";
}
}