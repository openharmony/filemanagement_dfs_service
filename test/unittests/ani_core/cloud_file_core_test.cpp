/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cloud_file_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;

class CloudFileCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudFileCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudFileCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudFileCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudFileCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the CloudFileCore::Constructor function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCoreTest, ConstructorTest1, TestSize.Level1)
{
    auto data = CloudFileCore::Constructor();
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the CloudFileCore::DoOn function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCoreTest, DoOnTest1, TestSize.Level1)
{
    CloudFileCore *download = CloudFileCore::Constructor().GetData().value();
    auto callback = std::make_shared<CloudDownloadCallbackAniImpl>(nullptr, nullptr);
    std::string event = "progress";
    auto ret = download->DoOn(event, callback);
    EXPECT_TRUE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, 0);
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the CloudFileCore::DoOn function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCoreTest, DoOnTest2, TestSize.Level1)
{
    CloudFileCore *download = CloudFileCore::Constructor().GetData().value();
    auto callback = std::make_shared<CloudDownloadCallbackAniImpl>(nullptr, nullptr);
    std::string event = "";
    auto ret = download->DoOn(event, callback);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_PARAMS);
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the CloudFileCore::DoOn function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCoreTest, DoOnTest3, TestSize.Level1)
{
    CloudFileCore *download = CloudFileCore::Constructor().GetData().value();
    auto callback = std::make_shared<CloudDownloadCallbackAniImpl>(nullptr, nullptr);
    std::string event = "progress";
    auto ret = download->DoOn(event, callback);
    ret = download->DoOn(event, callback);
    EXPECT_TRUE(ret.IsSuccess());
}

/**
 * @tc.name: DoOff
 * @tc.desc: Verify the CloudFileCore::DoOff function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCoreTest, DoOffTest1, TestSize.Level1)
{
    CloudFileCore *download = CloudFileCore::Constructor().GetData().value();
    std::string event = "progress";
    auto ret = download->DoOff(event);
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    if (ret.IsSuccess()) {
        EXPECT_EQ(errorCode, 0);
    } else {
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_IPCSS);
    }
}

/**
 * @tc.name: DoOff
 * @tc.desc: Verify the CloudFileCore::DoOff function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCoreTest, DoOffTest2, TestSize.Level1)
{
    CloudFileCore *download = CloudFileCore::Constructor().GetData().value();
    std::string event = "";
    auto ret = download->DoOff(event);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_PARAMS);
}

/**
 * @tc.name: DoStart
 * @tc.desc: Verify the CloudFileCore::DoStart function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCoreTest, DoStartTest1, TestSize.Level1)
{
    CloudFileCore *download = CloudFileCore::Constructor().GetData().value();
    std::string uri = "testuri";
    auto ret = download->DoStart(uri);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_IPCSS);
}

/**
 * @tc.name: DoStop
 * @tc.desc: Verify the CloudFileCore::DoStop function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCoreTest, DoStopTest1, TestSize.Level1)
{
    CloudFileCore *download = CloudFileCore::Constructor().GetData().value();
    std::string uri = "testuri";
    auto ret = download->DoStop(uri);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_IPCSS);
}
} // namespace OHOS::FileManagement::CloudDisk::Test