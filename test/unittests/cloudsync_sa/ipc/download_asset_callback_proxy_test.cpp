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
#include <memory>

#include "download_asset_callback_proxy.h"
#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "asset_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class DownloadAssetCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<DownloadAssetCallbackProxy> proxy_ = nullptr;
    sptr<DownloadAssetCallbackMock> mock_ = nullptr;
};

void DownloadAssetCallbackProxyTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void DownloadAssetCallbackProxyTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void DownloadAssetCallbackProxyTest::SetUp(void)
{
    mock_ = sptr(new DownloadAssetCallbackMock());
    proxy_ = make_shared<DownloadAssetCallbackProxy>(mock_);
    std::cout << "SetUp" << std::endl;
}

void DownloadAssetCallbackProxyTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnFinishedTest
 * @tc.desc: Verify the OnFinished function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DownloadAssetCallbackProxyTest, OnFinishedTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFinished Start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
        DownloadAssetCallback::TaskId taskId = 1;
        std::string uri = "uri";
        int32_t result = 0;
        proxy_->OnFinished(taskId, uri, result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnFinished ERROR";
    }
    GTEST_LOG_(INFO) << "OnFinished End";
}

/**
 * @tc.name: OnFinishedTest002
 * @tc.desc: Verify the OnFinished function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(DownloadAssetCallbackProxyTest, OnFinishedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFinishedTest002 Start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
        DownloadAssetCallback::TaskId taskId = 1;
        std::string uri = "uri";
        int32_t result = 0;
        proxy_->OnFinished(taskId, uri, result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnFinishedTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnFinishedTest002 End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
