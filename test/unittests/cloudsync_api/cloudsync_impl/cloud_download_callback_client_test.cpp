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

#include "cloud_download_callback_client.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

class CloudDownloadCallbackClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class MockCloudDownloadCallback final : public CloudDownloadCallback {
public:
    void OnDownloadProcess(const DownloadProgressObj &progress)
    {
        return;
    }
};

void CloudDownloadCallbackClientTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDownloadCallbackClientTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDownloadCallbackClientTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudDownloadCallbackClientTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnDownloadProcessTest001
 * @tc.desc: Verify the OnDownloadProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackClientTest, OnDownloadProcessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadProcess Start";
    try {
        DownloadProgressObj progress;
        std::shared_ptr<CloudDownloadCallback> callback = nullptr;
        CloudDownloadCallbackClient cloudCallback(callback);
        cloudCallback.OnDownloadProcess(progress);
        EXPECT_TRUE(true);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDownloadProcess ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadProcess End";
}

/**
 * @tc.name: OnDownloadProcessTest002
 * @tc.desc: Verify the OnDownloadProcess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadCallbackClientTest, OnDownloadProcessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadProcess Start";
    try {
        DownloadProgressObj progress;
        std::shared_ptr<CloudDownloadCallback> callback = std::make_shared<MockCloudDownloadCallback>();
        CloudDownloadCallbackClient cloudCallback(callback);
        cloudCallback.OnDownloadProcess(progress);
        EXPECT_TRUE(true);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDownloadProcess ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadProcess End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
