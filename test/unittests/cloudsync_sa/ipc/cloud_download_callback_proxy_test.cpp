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

#include "cloud_download_callback_proxy.h"
#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "i_cloud_download_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudDownloadCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<CloudDownloadCallbackProxy> proxy_ = nullptr;
    sptr<CloudDownloadCallbackMock> mock_ = nullptr;
};

void CloudDownloadCallbackProxyTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDownloadCallbackProxyTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDownloadCallbackProxyTest::SetUp(void)
{
    mock_ = sptr(new CloudDownloadCallbackMock());
    proxy_ = make_shared<CloudDownloadCallbackProxy>(mock_);
    std::cout << "SetUp" << std::endl;
}

void CloudDownloadCallbackProxyTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnDownloadProcessTest001
 * @tc.desc: Verify the OnDownloadProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackProxyTest, OnDownloadProcessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadProcessTest001 Start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
        DownloadProgressObj progress;
        proxy_->OnDownloadProcess(progress);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDownloadProcessTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadProcessTest001 End";
}

/**
 * @tc.name: OnDownloadProcessTest002
 * @tc.desc: Verify the OnDownloadProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackProxyTest, OnDownloadProcessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadProcessTest002 Start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));
        DownloadProgressObj progress;
        proxy_->OnDownloadProcess(progress);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDownloadProcessTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadProcessTest002 End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
