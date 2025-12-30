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

#include "cloud_sync_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include "cloud_sync_manager_mock.h"
#include "system_mock.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"
 
namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;

const int32_t E_INVAL = 13900020;

class CloudSyncCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<SystemMock> sys = nullptr;
};

void CloudSyncCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    sys = std::make_shared<SystemMock>();
    SystemMock::EnableMock();
    System::ins = sys;
}

void CloudSyncCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    SystemMock::DisableMock();
    System::ins = nullptr;
    sys = nullptr;
}

void CloudSyncCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudSyncCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the CloudSyncCore::Constructor function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, ConstructorTest1, TestSize.Level1)
{
    auto data = CloudSyncCore::Constructor();
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: GetBundleName
 * @tc.desc: Verify the CloudSyncCore::GetBundleName function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, GetBundleNameTest1, TestSize.Level1)
{
    const CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    string ret = cloudSync->GetBundleName();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the CloudSyncCore::DoOn function
 * @tc.type: FUNC
 * @tc.require: IC7I52
 */
HWTEST_F(CloudSyncCoreTest, DoOnTest1, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(nullptr, nullptr);
    std::string event = "progress";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, RegisterCallback(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudSync->DoOn(event, callback);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoOff
 * @tc.desc: Verify the CloudSyncCore::DoOff function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoOffTest3, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(nullptr, nullptr);
    std::string event = "progress";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, RegisterCallback(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudSync->DoOn(event, callback);
    EXPECT_FALSE(ret.IsSuccess());
    EXPECT_CALL(cloudMock, UnRegisterCallback(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    ret = cloudSync->DoOff(event);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the CloudSyncCore::DoOn function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoOnTest2, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(nullptr, nullptr);
    std::string event = "";
    auto ret = cloudSync->DoOn(event, callback);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_PARAMS);
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the CloudSyncCore::DoOn function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoOnTest3, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(nullptr, nullptr);
    std::string event = "progress";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, RegisterCallback(_)).WillOnce(Return(E_OK));
    auto ret = cloudSync->DoOn(event, callback);
    ret = cloudSync->DoOn(event, callback);
    EXPECT_TRUE(ret.IsSuccess());
}

/**
 * @tc.name: DoOff
 * @tc.desc: Verify the CloudSyncCore::DoOff function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoOffTest1, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    std::string event = "progress";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, UnRegisterCallback(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudSync->DoOff(event);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoOff
 * @tc.desc: Verify the CloudSyncCore::DoOff function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoOffTest2, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    std::string event = "";
    auto ret = cloudSync->DoOff(event);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_PARAMS);
}

/**
 * @tc.name: DoStart
 * @tc.desc: Verify the CloudSyncCore::DoStart function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoStartTest1, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StartSync(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudSync->DoStart();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStart
 * @tc.desc: Verify the CloudSyncCore::DoStart function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoStartTest2, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StartSync(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudSync->DoStart();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
    EXPECT_CALL(cloudMock, StopSync(_, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    ret = cloudSync->DoStop();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &error = ret.GetError();
    errorCode = error.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStop
 * @tc.desc: Verify the CloudSyncCore::DoStop function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoStopTest1, TestSize.Level1)
{
    CloudSyncCore *cloudSync = CloudSyncCore::Constructor().GetData().value();
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StopSync(_, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudSync->DoStop();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoGetFileSyncState
 * @tc.desc: Verify the CloudSyncCore::DoGetFileSyncState function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoGetFileSyncStateTest1, TestSize.Level1)
{
    string filePath = "file://com.ohos.camera/test/test.txt";
    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(-1));
    auto ret = CloudSyncCore::DoGetFileSyncState(filePath);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_INVAL);
}

/**
 * @tc.name: DoGetFileSyncState
 * @tc.desc: Verify the CloudSyncCore::DoGetFileSyncState function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoGetFileSyncStateTest2, TestSize.Level1)
{
    string filePath = "/test/test.txt";
    auto ret = CloudSyncCore::DoGetFileSyncState(filePath);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, E_INVAL);
}

/**
 * @tc.name: DoGetCoreFileSyncState
 * @tc.desc: Verify the CloudSyncCore::DoGetCoreFileSyncState function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoGetCoreFileSyncStateTest1, TestSize.Level1)
{
    string filePath = "file://com.ohos.camera/test/test.txt";
    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(-1));
    auto ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
#if CLOUD_ADAPTER_ENABLED
    EXPECT_TRUE(ret.IsSuccess());
#else
    EXPECT_FALSE(ret.IsSuccess());
#endif

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(0)).WillOnce(Return(-1));
    ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
#if CLOUD_ADAPTER_ENABLED
    EXPECT_TRUE(ret.IsSuccess());
#else
    EXPECT_FALSE(ret.IsSuccess());
#endif

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(0)).WillOnce(DoAll(
        Invoke([](const char *path, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = '7';
        }), Return(1)));
    ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, JsErrCode::E_INNER_FAILED);

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(0)).WillOnce(DoAll(
        Invoke([](const char *path, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = '5';
        }), Return(1)));
    ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
    EXPECT_TRUE(ret.IsSuccess());
}

/**
 * @tc.name: DoGetCoreFileSyncState
 * @tc.desc: Verify the CloudSyncCore::DoGetCoreFileSyncState function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoGetCoreFileSyncStateTest2, TestSize.Level1)
{
    string filePath = "file://com.ohos.camera/test/test.txt";
    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(0)).WillOnce(DoAll(
        Invoke([](const char *path, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = '0';
        }), Return(1)));
    auto ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
    EXPECT_TRUE(ret.IsSuccess());

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(0)).WillOnce(DoAll(
        Invoke([](const char *path, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = '1';
        }), Return(1)));
    ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
    EXPECT_TRUE(ret.IsSuccess());

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(0)).WillOnce(DoAll(
        Invoke([](const char *path, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = '2';
        }), Return(1)));
    ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
    EXPECT_TRUE(ret.IsSuccess());

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(0)).WillOnce(DoAll(
        Invoke([](const char *path, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = '3';
        }), Return(1)));
    ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
    EXPECT_TRUE(ret.IsSuccess());

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(0)).WillOnce(DoAll(
        Invoke([](const char *path, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = '4';
        }), Return(1)));
    ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
    EXPECT_TRUE(ret.IsSuccess());
}

/**
 * @tc.name: DoGetCoreFileSyncState
 * @tc.desc: Verify the CloudSyncCore::DoGetCoreFileSyncState function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoGetCoreFileSyncStateTest3, TestSize.Level1)
{
    string filePath = "/test/test.txt";
    auto ret = CloudSyncCore::DoGetCoreFileSyncState(filePath);
    EXPECT_FALSE(ret.IsSuccess());
}

/**
 * @tc.name: DoOptimizeStorage
 * @tc.desc: Verify the CloudSyncCore::DoOptimizeStorage function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoOptimizeStorageTest1, TestSize.Level1)
{
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, OptimizeStorage(_, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = CloudSyncCore::DoOptimizeStorage();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStartOptimizeStorage
 * @tc.desc: Verify the CloudSyncCore::DoStartOptimizeStorage
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoStartOptimizeStorageTest1, TestSize.Level1)
{
    OptimizeSpaceOptions optimizeOptions {};
    int64_t totalSize = 100;
    int32_t agingDays = 30;
    optimizeOptions.totalSize = totalSize;
    optimizeOptions.agingDays = agingDays;
    auto callback = std::make_shared<CloudOptimizeCallbackAniImpl>(nullptr, nullptr);
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, OptimizeStorage(_, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = CloudSyncCore::DoStartOptimizeStorage(optimizeOptions, callback);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStopOptimizeStorage
 * @tc.desc: Verify the CloudSyncCore::DoStopOptimizeStorage function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncCoreTest, DoStopOptimizeStorageTest1, TestSize.Level1)
{
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StopOptimizeStorage()).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = CloudSyncCore::DoStopOptimizeStorage();
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}
} // namespace OHOS::FileManagement::CloudDisk::Test