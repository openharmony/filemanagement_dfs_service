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

#include "download_asset_callback_client.h"
#include "asset_callback_mock.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

class DownloadAssetCallbackClientTest : public testing::Test {
public:
    using TaskId = uint64_t;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DownloadAssetCallbackClientTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void DownloadAssetCallbackClientTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void DownloadAssetCallbackClientTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void DownloadAssetCallbackClientTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnFinishedTest001
 * @tc.desc: Verify the OnFinished function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(DownloadAssetCallbackClientTest, OnFinishedTest001, TestSize.Level1)
{
    DownloadAssetCallbackClient client;
    TaskId taskId = 1;
    std::string uri = "file://data/file/test";
    int32_t result = 0;
    auto callback = [](const std::string &uri, const int32_t result) {
        EXPECT_EQ(uri, "file://data/file/test");
        EXPECT_EQ(result, 0);
    };
    client.GetDownloadTaskCallback(taskId) = callback;
  
    client.OnFinished(taskId, uri, result);
  
    EXPECT_EQ(client.GetDownloadTaskCallback(taskId), nullptr);
}
  
 /**
  * @tc.name: OnFinishedTest002
  * @tc.desc: Verify the OnFinished function
  * @tc.type: FUNC
  * @tc.require: issuesI91IOG
  */
HWTEST_F(DownloadAssetCallbackClientTest, OnFinishedTest002, TestSize.Level1)
{
    DownloadAssetCallbackClient client;
    TaskId taskId = 1;
    std::string uri = "file://data/file/test";
    int32_t result = 0;
  
    client.OnFinished(taskId, uri, result);
  
    EXPECT_EQ(client.GetDownloadTaskCallback(taskId), nullptr);
}
  
 /**
  * @tc.name: RemoveDownloadTaskCallbackTest001
  * @tc.desc: Verify the OnFinished function
  * @tc.type: FUNC
  * @tc.require: issuesI91IOG
  */
HWTEST_F(DownloadAssetCallbackClientTest, RemoveDownloadTaskCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveDownloadTaskCallbackTest001 Start";
    try {
        DownloadAssetCallbackClient client;
        TaskId taskId = 1;
        CloudSyncAssetManager::ResultCallback callback;
        client.callbackListMap_.Insert(taskId, callback);
        client.RemoveDownloadTaskCallback(taskId);

        EXPECT_FALSE(client.callbackListMap_.Find(taskId, callback));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveDownloadTaskCallbackTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "RemoveDownloadTaskCallbackTest001 End";
}
  
 /**
  * @tc.name: GetDownloadTaskCallbackTest001
  * @tc.desc: Verify the GetDownloadTaskCallback function
  * @tc.type: FUNC
  * @tc.require: issuesI91IOG
  */
HWTEST_F(DownloadAssetCallbackClientTest, GetDownloadTaskCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadTaskCallbackTest001 Start";
    try {
        DownloadAssetCallbackClient downloadAssetCallbackClient;
        TaskId taskId = 1;
        CloudSyncAssetManager::ResultCallback callback = [](const std::string &, int32_t) {};
        auto result = downloadAssetCallbackClient.GetDownloadTaskCallback(taskId);
  
        EXPECT_TRUE(result == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDownloadTaskCallbackTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDownloadTaskCallbackTest001 End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
