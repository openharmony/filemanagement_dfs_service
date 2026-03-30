/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cloud_sync_common.h"
#include "cloud_upload_callback_client.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

class CloudUploadCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<CloudUploadCallbackClient> callbackClient_;
    void InitCloudUploadCallback();
};

class CloudUploadCallbackDerived : public CloudUploadCallback {
public:
    void OnUploadProgress(const UploadProgressObj &progress) override
    {
        GTEST_LOG_(INFO) << "OnUploadProgress SUCCESS";
    }
};

void CloudUploadCallbackTest::InitCloudUploadCallback(void)
{
    shared_ptr<CloudUploadCallback> callback = make_shared<CloudUploadCallbackDerived>();
    callbackClient_ = new CloudUploadCallbackClient(callback);
}

void CloudUploadCallbackTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudUploadCallbackTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudUploadCallbackTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudUploadCallbackTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnUploadProgressTest
 * @tc.desc: Verify the OnUploadProgress function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudUploadCallbackTest, OnUploadProgressTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUploadProgress start";
    try {
        InitCloudUploadCallback();
        UploadProgressObj progress;
        progress.state = UploadProgressObj::UploadState::RUNNING;
        progress.processed = 1024;
        progress.size = 2048;
        progress.uri = "file://data/storage/el2/base/haps/entry/files/test.jpg";
        progress.error = UploadProgressObj::ErrorType::NO_ERROR;
        callbackClient_->OnUploadProgress(progress);
        EXPECT_TRUE(true);
        callbackClient_->callback_ = nullptr;
        callbackClient_->OnUploadProgress(progress);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnUploadProgress FAILED";
    }
    GTEST_LOG_(INFO) << "OnUploadProgress end";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS