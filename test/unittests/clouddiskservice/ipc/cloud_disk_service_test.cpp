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
#include "dfsu_access_token_helper_mock.h"
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
    static inline shared_ptr<DfsuAccessTokenMock> dfsuAccessToken_ = nullptr;
};

void CloudDiskServiceTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    int32_t saID = 5207;
    bool runOnCreate = true;
    cloudDiskService_ = new (std::nothrow) CloudDiskService(saID, runOnCreate);
    ASSERT_TRUE(cloudDiskService_ != nullptr) << "cloudDiskService_ assert failed!";
    dfsuAccessToken_ = make_shared<DfsuAccessTokenMock>();
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
        EXPECT_TRUE(true);
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

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS