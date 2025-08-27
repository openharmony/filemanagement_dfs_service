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

#include "file_sync_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager.h"
#include "cloud_sync_manager_mock.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;

class FileSyncCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FileSyncCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileSyncCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileSyncCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileSyncCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the FileSyncCore::Constructor function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, ConstructorTest1, TestSize.Level1)
{
    auto data = FileSyncCore::Constructor();
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the FileSyncCore::Constructor function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, ConstructorTest2, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    auto data = FileSyncCore::Constructor(bundleName);
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the FileSyncCore::Constructor function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, ConstructorTest3, TestSize.Level1)
{
    std::string bundleName = "";
    auto data = FileSyncCore::Constructor(bundleName);
    EXPECT_FALSE(data.IsSuccess());
}

/**
 * @tc.name: GetBundleName
 * @tc.desc: Verify the FileSyncCore::GetBundleName function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, GetBundleNameTest1, TestSize.Level1)
{
    const FileSyncCore *fileSync = FileSyncCore::Constructor().GetData().value();
    string ret = fileSync->GetBundleName();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: GetBundleName
 * @tc.desc: Verify the FileSyncCore::GetBundleName function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, GetBundleNameTest2, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    const FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    string ret = fileSync->GetBundleName();
    EXPECT_EQ(ret, bundleName);
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the FileSyncCore::DoOn function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, DoOnTest1, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(nullptr, nullptr);
    std::string event = "";
    auto ret = fileSync->DoOn(event, callback);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_PARAMS);
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the FileSyncCore::DoOn function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, DoOnTest2, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(nullptr, nullptr);
    std::string event = "";
    auto ret = fileSync->DoOn(event, callback);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_PARAMS);
    ret = fileSync->DoOff(event);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &error = ret.GetError();
    errorCode = error.GetErrNo();
    EXPECT_EQ(errorCode, E_PARAMS);
}

/**
 * @tc.name: DoOff
 * @tc.desc: Verify the FileSyncCore::DoOff function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, DoOffTest1, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    std::string event = "progress";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, UnRegisterFileSyncCallback(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = fileSync->DoOff(event);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoOff
 * @tc.desc: Verify the FileSyncCore::DoOff function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, DoOffTest2, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    std::string event = "";
    auto ret = fileSync->DoOff(event);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_PARAMS);
}

/**
 * @tc.name: DoStart
 * @tc.desc: Verify the FileSyncCore::DoStart function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, DoStartTest1, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StartFileSync(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = fileSync->DoStart();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStart
 * @tc.desc: Verify the FileSyncCore::DoStart function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, DoStartTest2, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StartFileSync(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = fileSync->DoStart();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
    EXPECT_CALL(cloudMock, StopFileSync(_, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    ret = fileSync->DoStop();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &error = ret.GetError();
    errorCode = error.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStop
 * @tc.desc: Verify the FileSyncCore::DoStop function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, DoStopTest1, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StopFileSync(_, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = fileSync->DoStop();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoGetLastSyncTime
 * @tc.desc: Verify the FileSyncCore::DoGetLastSyncTime function
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(FileSyncCoreTest, DoGetLastSyncTimeTest1, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    FileSyncCore *fileSync = FileSyncCore::Constructor(bundleName).GetData().value();
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, GetSyncTime(_, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = fileSync->DoGetLastSyncTime();
    EXPECT_FALSE(ret.IsSuccess());
}
} // namespace OHOS::FileManagement::CloudDisk::Test