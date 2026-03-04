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

#include <exception>
#include <malloc.h>
#include <stdexcept>
#include <sys/utsname.h>
#include <sys/xattr.h>
#include <thread>

#include "cloud_disk_service.h"
#include "cloud_disk_service_callback_manager.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_syncfolder.h"
#include "cloud_disk_sync_folder.h"
#include "cloud_disk_service_access_token_mock.h"
#include "utils_log.h"

namespace {
bool g_publish = false;
} // namespace

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif

bool SystemAbility::Publish(sptr<IRemoteObject> systemAbility)
{
    return g_publish;
}

bool SystemAbility::AddSystemAbilityListener(int32_t systemAbilityId)
{
    return true;
}
} // namespace OHOS

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline sptr<CloudDiskService> cloudDiskService_;
    static inline shared_ptr<CloudDiskServiceAccessTokenMock> dfsuAccessToken_ = nullptr;
};

void CloudDiskServiceTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    int32_t saID = 5207;
    bool runOnCreate = true;
    cloudDiskService_ = new (std::nothrow) CloudDiskService(saID, runOnCreate);
    ASSERT_TRUE(cloudDiskService_ != nullptr) << "cloudDiskService_ assert failed!";
    dfsuAccessToken_ = make_shared<CloudDiskServiceAccessTokenMock>();
}

void CloudDiskServiceTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    cloudDiskService_ = nullptr;
    dfsuAccessToken_ = nullptr;
}

void CloudDiskServiceTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskServiceTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: PublishSATest001
 * @tc.desc: Verify the PublishSA function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, PublishSATest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PublishSATest001 start";
    try {
        g_publish = true;
        cloudDiskService_->registerToService_ = true;
        cloudDiskService_->PublishSA();
        EXPECT_TRUE(cloudDiskService_->registerToService_);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PublishSATest001 failed";
    }
    GTEST_LOG_(INFO) << "PublishSATest001 end";
}

/**
 * @tc.name: PublishSATest002
 * @tc.desc: Verify the PublishSA function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, PublishSATest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PublishSATest002 start";
    try {
        g_publish = true;
        cloudDiskService_->registerToService_ = false;
        cloudDiskService_->PublishSA();
        EXPECT_TRUE(cloudDiskService_->registerToService_);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PublishSATest002 failed";
    }
    GTEST_LOG_(INFO) << "PublishSATest002 end";
}

/**
 * @tc.name: PublishSATest003
 * @tc.desc: Verify the PublishSA function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, PublishSATest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PublishSATest003 start";
    try {
        g_publish = false;
        cloudDiskService_->registerToService_ = false;
        cloudDiskService_->PublishSA();
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PublishSATest003 failed";
    } catch (...) {
        EXPECT_TRUE(true);
    }
    GTEST_LOG_(INFO) << "PublishSATest003 end";
}

/**
 * @tc.name: OnStartTest001
 * @tc.desc: Verify the OnStart function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, OnStartTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartTest001 start";
    try {
        cloudDiskService_->state_ = ServiceRunningState::STATE_RUNNING;
        cloudDiskService_->OnStart();
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "OnStartTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnStartTest001 end";
}

/**
 * @tc.name: OnStopTest001
 * @tc.desc: Verify the OnStop function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, OnStopTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStopTest001 start";
    try {
        cloudDiskService_->OnStop();
        EXPECT_EQ(cloudDiskService_->state_, ServiceRunningState::STATE_NOT_START);
        EXPECT_EQ(cloudDiskService_->registerToService_, false);
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "OnStopTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnStopTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesInnerTest001
 * @tc.desc: Verify the RegisterSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderChangesInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        std::string syncFolder = "test_syncFolder";
        sptr<IRemoteObject> remoteObj = nullptr;
        uint32_t ret = cloudDiskService_->RegisterSyncFolderChangesInner(syncFolder, remoteObj);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesInnerTest002
 * @tc.desc: Verify the RegisterSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderChangesInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest002 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        sptr<IRemoteObject> remoteObj = nullptr;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->RegisterSyncFolderChangesInner(syncFolder, remoteObj);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest002 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest002 end";
}

/**
 * @tc.name: UnregisterSyncFolderChangesInnerTest001
 * @tc.desc: Verify the UnregisterSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderChangesInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        std::string syncFolder = "test_syncFolder";
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderChangesInner(syncFolder);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest001 end";
}

/**
 * @tc.name: UnregisterSyncFolderChangesInnerTest002
 * @tc.desc: Verify the UnregisterSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderChangesInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest002 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderChangesInner(syncFolder);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest002 end";
}

/**
 * @tc.name: UnregisterSyncFolderChangesInnerTest003
 * @tc.desc: Verify the UnregisterSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderChangesInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest003 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVALID_ARG));
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderChangesInner(syncFolder);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INTERNAL_ERROR);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest003 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest003 end";
}

/**
 * @tc.name: UnregisterSyncFolderChangesInnerTest004
 * @tc.desc: Verify the UnregisterSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderChangesInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest004 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderChangesInner(syncFolder);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest004 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest004 end";
}

/**
 * @tc.name: GetSyncFolderChangesInnerTest001
 * @tc.desc: Verify the GetSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetSyncFolderChangesInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        std::string syncFolder = "test_syncFolder";
        uint64_t count = 0;
        uint64_t startUsn = 0;
        ChangesResult changesResult;
        uint32_t ret = cloudDiskService_->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest001 end";
}

/**
 * @tc.name: GetSyncFolderChangesInnerTest002
 * @tc.desc: Verify the GetSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetSyncFolderChangesInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest002 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        uint64_t count = 0;
        uint64_t startUsn = 0;
        ChangesResult changesResult;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest002 end";
}

/**
 * @tc.name: GetSyncFolderChangesInnerTest003
 * @tc.desc: Verify the GetSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetSyncFolderChangesInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest003 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        uint64_t count = 0;
        uint64_t startUsn = 0;
        ChangesResult changesResult;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVALID_ARG));
        uint32_t ret = cloudDiskService_->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INTERNAL_ERROR);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest003 end";
}

/**
 * @tc.name: GetSyncFolderChangesInnerTest004
 * @tc.desc: Verify the GetSyncFolderChangesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetSyncFolderChangesInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest004 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        uint64_t count = 0;
        uint64_t startUsn = 0;
        ChangesResult changesResult;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        uint32_t ret = cloudDiskService_->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest004 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest001
 * @tc.desc: Verify the SetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        std::string syncFolder = "test_syncFolder";
        std::vector<FileSyncState> fileSyncStates;
        std::vector<FailedList> failedList;
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest001 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest002
 * @tc.desc: Verify the SetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest002 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        std::vector<FileSyncState> fileSyncStates;
        std::vector<FailedList> failedList;
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest002 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest002 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest003
 * @tc.desc: Verify the SetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest003 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        std::vector<FileSyncState> fileSyncStates;
        std::vector<FailedList> failedList;
        FileSyncState fileSyncState1;
        fileSyncStates.push_back(fileSyncState1);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest003 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest003 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest004
 * @tc.desc: Verify the SetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest004 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::vector<FileSyncState> fileSyncStates;
        std::vector<FailedList> failedList;
        FileSyncState fileSyncState1;
        fileSyncStates.push_back(fileSyncState1);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest004 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest004 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest005
 * @tc.desc: Verify the SetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest005 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::vector<FileSyncState> fileSyncStates;
        std::vector<FailedList> failedList;
        FileSyncState fileSyncState1;
        fileSyncState1.path = "testpath";
        fileSyncStates.push_back(fileSyncState1);
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + syncFolder.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"bundlename", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_OK);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest005 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest005 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest001
 * @tc.desc: Verify the GetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        std::string syncFolder = "test_syncFolder";
        std::vector<std::string> pathArray;
        std::vector<ResultList> resultList;
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest001 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest002
 * @tc.desc: Verify the GetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        std::vector<std::string> pathArray;
        std::vector<ResultList> resultList;
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest002 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest003
 * @tc.desc: Verify the GetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        std::vector<std::string> pathArray;
        std::vector<ResultList> resultList;
        std::string path1 = "testpath1";
        pathArray.push_back(path1);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest003 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest004
 * @tc.desc: Verify the GetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::vector<std::string> pathArray;
        std::vector<ResultList> resultList;
        std::string path1 = "testpath1";
        pathArray.push_back(path1);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest004 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest005
 * @tc.desc: Verify the GetFileSyncStatesInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest005 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::vector<std::string> pathArray;
        std::vector<ResultList> resultList;
        std::string path1 = "testpath1";
        pathArray.push_back(path1);
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + syncFolder.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"bundlename", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_OK);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest005 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest005 end";
}

/**
 * @tc.name: RegisterSyncFolderInnerTest001
 * @tc.desc: Verify the RegisterSyncFolderInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "testpath";
        uint32_t ret = cloudDiskService_->RegisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderInnerTest002
 * @tc.desc: Verify the RegisterSyncFolderInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest002 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "testpath";
        uint32_t ret = cloudDiskService_->RegisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest002 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest002 end";
}

/**
 * @tc.name: RegisterSyncFolderInnerTest003
 * @tc.desc: Verify the RegisterSyncFolderInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest003 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "/storage/Users/currentUser/testdir";
        uint32_t ret = cloudDiskService_->RegisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest003 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest003 end";
}

/**
 * @tc.name: UnregisterSyncFolderInnerTest001
 * @tc.desc: Verify the UnregisterSyncFolderInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest001 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "testpath";
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest001 end";
}

/**
 * @tc.name: UnregisterSyncFolderInnerTest002
 * @tc.desc: Verify the UnregisterSyncFolderInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest002 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "testpath";
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest002 end";
}

/**
 * @tc.name: UnregisterSyncFolderInnerTest003
 * @tc.desc: Verify the UnregisterSyncFolderInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest003 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "/storage/Users/currentUser/testdir";
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest003 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest003 end";
}

/**
 * @tc.name: UnregisterSyncFolderInnerTest004
 * @tc.desc: Verify the UnregisterSyncFolderInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest004 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "/storage/Users/currentUser/testdir";
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + path.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"bundlename", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderInner(userId, bundleName, path);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_OK);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest004 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest004 end";
}

/**
 * @tc.name: UnregisterForSaInnerTest001
 * @tc.desc: Verify the UnregisterForSaInner function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterForSaInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest001 start";
    try {
        std::string path = "testpath";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->UnregisterForSaInner(path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_OK);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterForSaInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest001 end";
}

/**
 * @tc.name: UnregisterForSaInnerTest002
 * @tc.desc: Verify the UnregisterForSaInner function with userId == 0
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterForSaInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest002 start";
    try {
        std::string path = "testpath";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetAccountId(_)).WillOnce(Return(E_OK));
        uint32_t ret = cloudDiskService_->UnregisterForSaInner(path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_OK);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterForSaInnerTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest002 end";
}

/**
 * @tc.name: OnAddSystemAbilityTest001
 * @tc.desc: Verify the OnAddSystemAbility function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, OnAddSystemAbilityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest001 start";
    try {
        int32_t systemAbilityId = 100;
        std::string deviceId = "device1";
        cloudDiskService_->OnAddSystemAbility(systemAbilityId, deviceId);
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "OnAddSystemAbilityTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest001 end";
}

/**
 * @tc.name: UnloadSaTest001
 * @tc.desc: Verify the UnloadSa function with sync folder size > 0
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnloadSaTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnloadSaTest001 start";
    try {
        cloudDiskService_->UnloadSa();
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnloadSaTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnloadSaTest001 end";
}

/**
 * @tc.name: GetSyncFolderChangesInnerTest005
 * @tc.desc: Verify the GetSyncFolderChangesInner function with count > GET_SYNC_FOLDER_CHANGE_MAX
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetSyncFolderChangesInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest005 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        uint64_t count = 101;
        uint64_t startUsn = 0;
        ChangesResult changesResult;
        uint32_t ret = cloudDiskService_->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest005 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest005 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest006
 * @tc.desc: Verify the SetFileSyncStatesInner function with size > GET_FILE_SYNC_MAX
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest006 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        std::vector<FileSyncState> fileSyncStates(101);
        std::vector<FailedList> failedList;
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest006 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest006 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest006
 * @tc.desc: Verify the GetFileSyncStatesInner function with size > GET_FILE_SYNC_MAX
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest006 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "test_syncFolder";
        std::vector<std::string> pathArray(101);
        std::vector<ResultList> resultList;
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest006 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest006 end";
}

/**
 * @tc.name: OnStartTest002
 * @tc.desc: Verify the OnStart function with userId == 0
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, OnStartTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartTest002 start";
    try {
        cloudDiskService_->state_ = ServiceRunningState::STATE_NOT_START;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetAccountId(_)).WillOnce(Return(E_OK));
        cloudDiskService_->OnStart();
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "OnStartTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnStartTest002 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesInnerTest003
 * @tc.desc: Verify the RegisterSyncFolderChangesInner function with valid remoteObject
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderChangesInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest003 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        sptr<IRemoteObject> remoteObj = nullptr;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        uint32_t ret = cloudDiskService_->RegisterSyncFolderChangesInner(syncFolder, remoteObj);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest003 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest003 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesInnerTest004
 * @tc.desc: Verify the RegisterSyncFolderChangesInner function with GetCallerBundleName error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderChangesInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest004 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        sptr<IRemoteObject> remoteObj = nullptr;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVALID_ARG));
        uint32_t ret = cloudDiskService_->RegisterSyncFolderChangesInner(syncFolder, remoteObj);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest004 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest004 end";
}

/**
 * @tc.name: UnregisterSyncFolderChangesInnerTest005
 * @tc.desc: Verify the UnregisterSyncFolderChangesInner function with GetCallerBundleName error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderChangesInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest005 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVALID_ARG));
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderChangesInner(syncFolder);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest005 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest005 end";
}

/**
 * @tc.name: GetSyncFolderChangesInnerTest006
 * @tc.desc: Verify the GetSyncFolderChangesInner function with GetCallerBundleName error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetSyncFolderChangesInnerTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest006 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        uint64_t count = 1010;
        uint64_t startUsn = 0;
        ChangesResult changesResult;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVALID_ARG));
        uint32_t ret = cloudDiskService_->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest006 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest006 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest007
 * @tc.desc: Verify the SetFileSyncStatesInner function with GetCallerBundleName error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest007 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::vector<FileSyncState> fileSyncStates;
        std::vector<FailedList> failedList;
        FileSyncState fileSyncState1;
        fileSyncStates.push_back(fileSyncState1);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVALID_ARG));
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest007 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest007 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest007
 * @tc.desc: Verify the GetFileSyncStatesInner function with GetCallerBundleName error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest007 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::vector<std::string> pathArray;
        std::vector<ResultList> resultList;
        std::string path1 = "testpath1";
        pathArray.push_back(path1);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVALID_ARG));
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest007 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest007 end";
}

/**
 * @tc.name: RegisterSyncFolderInnerTest004
 * @tc.desc: Verify the RegisterSyncFolderInner function with permission denied
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest004 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "testpath";
        uint32_t ret = cloudDiskService_->RegisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest004 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest004 end";
}

/**
 * @tc.name: UnregisterSyncFolderInnerTest005
 * @tc.desc: Verify the UnregisterSyncFolderInner function with permission denied
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest005 start";
    try {
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "testpath";
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest005 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest005 end";
}

/**
 * @tc.name: UnregisterForSaInnerTest003
 * @tc.desc: Verify the UnregisterForSaInner function with PathToSandboxPathByPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterForSaInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest003 start";
    try {
        std::string path = "invalid_path";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->UnregisterForSaInner(path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterForSaInnerTest003 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest003 end";
}

/**
 * @tc.name: UnregisterForSaInnerTest004
 * @tc.desc: Verify the UnregisterForSaInner function with PathToMntPathByPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterForSaInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest004 start";
    try {
        std::string path = "/data/service/el2/100/hmdfs/account/files/Docs/test";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->UnregisterForSaInner(path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterForSaInnerTest004 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest004 end";
}

/**
 * @tc.name: UnregisterForSaInnerTest005
 * @tc.desc: Verify the UnregisterForSaInner function with GetSyncFolderValueByIndex failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterForSaInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest005 start";
    try {
        std::string path = "/data/service/el2/100/hmdfs/account/files/Docs/test";
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + path.substr(replacementPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"bundlename", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->UnregisterForSaInner(realpath);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_NOT_REGISTERED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterForSaInnerTest005 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterForSaInnerTest005 end";
}

/**
 * @tc.name: UnloadSaTest002
 * @tc.desc: Verify the UnloadSa function with sync folder size > 0
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnloadSaTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnloadSaTest002 start";
    try {
        std::string path = "/data/service/el2/100/hmdfs/account/files/Docs/test";
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(path);
        struct SyncFolderValue syncFolderValue = {"bundlename", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        cloudDiskService_->UnloadSa();
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnloadSaTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnloadSaTest002 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest008
 * @tc.desc: Verify the SetFileSyncStatesInner function with bundleName mismatch
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest008 start";
    try {
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::vector<FileSyncState> fileSyncStates;
        std::vector<FailedList> failedList;
        FileSyncState fileSyncState1;
        fileSyncState1.path = "testpath";
        fileSyncStates.push_back(fileSyncState1);
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + syncFolder.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"wrong_bundle", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
            .WillOnce(DoAll(SetArgReferee<0>("bundlename"), Return(E_OK)));
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_NOT_REGISTERED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest008 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest008 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest008
 * @tc.desc: Verify the GetFileSyncStatesInner function with bundleName mismatch
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest008 start";
    try {
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::vector<std::string> pathArray;
        std::vector<ResultList> resultList;
        std::string path1 = "testpath1";
        pathArray.push_back(path1);
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + syncFolder.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"wrong_bundle", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
            .WillOnce(DoAll(SetArgReferee<0>("bundlename"), Return(E_OK)));
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_NOT_REGISTERED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest008 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest008 end";
}

/**
 * @tc.name: RegisterSyncFolderInnerTest005
 * @tc.desc: Verify the RegisterSyncFolderInner function with PathToPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest005 start";
    try {
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "invalid_path";
        uint32_t ret = cloudDiskService_->RegisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest005 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderInnerTest005 end";
}

/**
 * @tc.name: UnregisterSyncFolderInnerTest006
 * @tc.desc: Verify the UnregisterSyncFolderInner function with PathToPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderInnerTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest006 start";
    try {
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "invalid_path";
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderInner(userId, bundleName, path);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest006 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest006 end";
}

/**
 * @tc.name: UnregisterSyncFolderInnerTest007
 * @tc.desc: Verify the UnregisterSyncFolderInner function with bundleName mismatch
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderInnerTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest007 start";
    try {
        int32_t userId = 100;
        std::string bundleName = "ohos.clouddiskservice.test";
        std::string path = "/storage/Users/currentUser/testdir";
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + path.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"wrong_bundle", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderInner(userId, bundleName, path);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_NOT_REGISTERED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest007 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderInnerTest007 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesInnerTest005
 * @tc.desc: Verify the RegisterSyncFolderChangesInner function with PathToPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderChangesInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest005 start";
    try {
        std::string syncFolder = "invalid_path";
        sptr<IRemoteObject> remoteObj = nullptr;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->RegisterSyncFolderChangesInner(syncFolder, remoteObj);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest005 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest005 end";
}

/**
 * @tc.name: UnregisterSyncFolderChangesInnerTest006
 * @tc.desc: Verify the UnregisterSyncFolderChangesInner function with PathToPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderChangesInnerTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest006 start";
    try {
        std::string syncFolder = "invalid_path";
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderChangesInner(syncFolder);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest006 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest006 end";
}

/**
 * @tc.name: GetSyncFolderChangesInnerTest007
 * @tc.desc: Verify the GetSyncFolderChangesInner function with PathToPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetSyncFolderChangesInnerTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest007 start";
    try {
        std::string syncFolder = "invalid_path";
        uint64_t count = 10;
        uint64_t startUsn = 0;
        ChangesResult changesResult;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest007 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest007 end";
}

/**
 * @tc.name: SetFileSyncStatesInnerTest009
 * @tc.desc: Verify the SetFileSyncStatesInner function with PathToPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, SetFileSyncStatesInnerTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest009 start";
    try {
        std::string syncFolder = "invalid_path";
        std::vector<FileSyncState> fileSyncStates;
        std::vector<FailedList> failedList;
        FileSyncState fileSyncState1;
        fileSyncStates.push_back(fileSyncState1);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->SetFileSyncStatesInner(syncFolder, fileSyncStates, failedList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest009 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesInnerTest009 end";
}

/**
 * @tc.name: GetFileSyncStatesInnerTest009
 * @tc.desc: Verify the GetFileSyncStatesInner function with PathToPhysicalPath failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetFileSyncStatesInnerTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest009 start";
    try {
        std::string syncFolder = "invalid_path";
        std::vector<std::string> pathArray;
        std::vector<ResultList> resultList;
        std::string path1 = "testpath1";
        pathArray.push_back(path1);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        uint32_t ret = cloudDiskService_->GetFileSyncStatesInner(syncFolder, pathArray, resultList);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_INVALID_ARG);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest009 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesInnerTest009 end";
}

/**
 * @tc.name: GetSyncFolderChangesInnerTest008
 * @tc.desc: Verify the GetSyncFolderChangesInner function with bundleName mismatch
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, GetSyncFolderChangesInnerTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest008 start";
    try {
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        uint64_t count = 10;
        uint64_t startUsn = 0;
        ChangesResult changesResult;
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + syncFolder.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"wrong_bundle", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
            .WillOnce(DoAll(SetArgReferee<0>("bundlename"), Return(E_OK)));
        uint32_t ret = cloudDiskService_->GetSyncFolderChangesInner(syncFolder, count, startUsn, changesResult);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_NOT_REGISTERED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest008 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesInnerTest008 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesInnerTest006
 * @tc.desc: Verify the RegisterSyncFolderChangesInner function with bundleName mismatch
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, RegisterSyncFolderChangesInnerTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest006 start";
    try {
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        sptr<IRemoteObject> remoteObj = nullptr;
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + syncFolder.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"wrong_bundle", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
            .WillOnce(DoAll(SetArgReferee<0>("bundlename"), Return(E_OK)));
        uint32_t ret = cloudDiskService_->RegisterSyncFolderChangesInner(syncFolder, remoteObj);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_NOT_REGISTERED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest006 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesInnerTest006 end";
}

/**
 * @tc.name: UnregisterSyncFolderChangesInnerTest007
 * @tc.desc: Verify the UnregisterSyncFolderChangesInner function with bundleName mismatch
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceTest, UnregisterSyncFolderChangesInnerTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest007 start";
    try {
        std::string syncFolder = "/storage/Users/currentUser/testdir";
        std::string sandboxPath = "/storage/Users/currentUser";
        std::string replacementPath = "/data/service/el2/100/hmdfs/account/files/Docs";
        std::string realpath = replacementPath + syncFolder.substr(sandboxPath.length());
        auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(realpath);
        struct SyncFolderValue syncFolderValue = {"wrong_bundle", "path"};
        CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
            .WillOnce(DoAll(testing::SetArgReferee<0>("bundlename"), testing::Return(E_OK)));
        uint32_t ret = cloudDiskService_->UnregisterSyncFolderChangesInner(syncFolder);
        CloudDiskSyncFolder::GetInstance().DeleteSyncFolder(syncFolderIndex);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, E_SYNC_FOLDER_NOT_REGISTERED);
#else
        EXPECT_EQ(ret, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest007 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderChangesInnerTest007 end";
}

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS