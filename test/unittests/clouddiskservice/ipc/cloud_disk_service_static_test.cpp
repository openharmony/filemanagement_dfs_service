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

#include "cloud_disk_service.cpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assistant.h"
#include "cloud_disk_service_access_token_mock.h"
#include "cloud_disk_service_utils.h"
#include "clouddiskservice_ioctl.h"
#include "message_parcel_mock.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudFile;
using namespace OHOS::Storage::DistributedFile;

namespace {
constexpr int32_t TEST_USER_ID = 100;
const std::string TEST_BUNDLE = "ohos.clouddiskservice.test";
const std::string TEST_SYNC_FOLDER = "/storage/Users/currentUser/testdir";
const std::string TEST_SYNC_FOLDER_PHYSICAL = "/data/service/el2/100/hmdfs/account/files/Docs/testdir";
const std::string TEST_SYNC_FOLDER_MNT = "/mnt/hmdfs/100/account/device_view/local/files/Docs/testdir";
const std::string TEST_RELATIVE_PATH = "placeholder.txt";

void RegisterPlaceholderSyncFolder(const std::string &bundleName = TEST_BUNDLE)
{
    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(TEST_SYNC_FOLDER_PHYSICAL);
    SyncFolderValue syncFolderValue = {bundleName, TEST_SYNC_FOLDER_PHYSICAL};
    CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
}
} // namespace

class CloudDiskServiceStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<CloudDiskServiceAccessTokenMock> dfsuAccessToken_;
    static inline shared_ptr<AssistantMock> insMock_;
    shared_ptr<MessageParcelMock> messageParcelMock_;
};

void CloudDiskServiceStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dfsuAccessToken_ = make_shared<CloudDiskServiceAccessTokenMock>();
    CloudDiskServiceAccessTokenMock::dfsuAccessToken = dfsuAccessToken_;
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
}

void CloudDiskServiceStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    CloudDiskServiceAccessTokenMock::dfsuAccessToken = nullptr;
    dfsuAccessToken_ = nullptr;
    Assistant::ins = nullptr;
    insMock_ = nullptr;
}

void CloudDiskServiceStaticTest::SetUp()
{
    Assistant::mockErrno = 0;
    Assistant::mockFdApi = true;
    messageParcelMock_ = make_shared<MessageParcelMock>();
    DfsMessageParcel::messageParcel = messageParcelMock_;
    CloudDiskSyncFolder::GetInstance().ClearMap();
}

void CloudDiskServiceStaticTest::TearDown()
{
    Mock::VerifyAndClearExpectations(insMock_.get());
    Mock::VerifyAndClearExpectations(dfsuAccessToken_.get());
    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    DfsMessageParcel::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
    Assistant::mockFdApi = false;
    CloudDiskSyncFolder::GetInstance().ClearMap();
}

/**
 * @tc.name: CheckPermissionsTest001
 * @tc.desc: Verify the CheckPermissions function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckPermissionsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionsTest001 start";
    try {
        string permission = "";
        bool isSystemApp = true;
        auto res = CheckPermissions(permission, isSystemApp);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermissionsTest001 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermissionsTest001 end";
}

/**
 * @tc.name: CheckPermissionsTest002
 * @tc.desc: Verify the CheckPermissions function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckPermissionsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionsTest002 start";
    try {
        string permission = "permission";
        bool isSystemApp = true;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        auto res = CheckPermissions(permission, isSystemApp);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermissionsTest002 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermissionsTest002 end";
}

/**
 * @tc.name: CheckPermissionsTest003
 * @tc.desc: Verify the CheckPermissions function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckPermissionsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionsTest003 start";
    try {
        string permission = "permission";
        bool isSystemApp = true;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        auto res = CheckPermissions(permission, isSystemApp);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermissionsTest003 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermissionsTest003 end";
}

/**
 * @tc.name: GetErrorNumTest001
 * @tc.desc: Verify the GetErrorNum function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetErrorNumTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorNumTest001 start";
    try {
        int32_t error = EDQUOT;
        auto res = GetErrorNum(error);
        EXPECT_EQ(res, static_cast<int32_t>(ErrorReason::NO_SPACE_LEFT));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetErrorNumTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetErrorNumTest001 end";
}

/**
 * @tc.name: GetErrorNumTest002
 * @tc.desc: Verify the GetErrorNum function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetErrorNumTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorNumTest002 start";
    try {
        int32_t error = ERANGE;
        auto res = GetErrorNum(error);
        EXPECT_EQ(res, static_cast<int32_t>(ErrorReason::OUT_OF_RANGE));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetErrorNumTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetErrorNumTest002 end";
}

/**
 * @tc.name: GetErrorNumTest003
 * @tc.desc: Verify the GetErrorNum function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetErrorNumTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorNumTest003 start";
    try {
        int32_t error = ENOENT;
        auto res = GetErrorNum(error);
        EXPECT_EQ(res, static_cast<int32_t>(ErrorReason::NO_SUCH_FILE));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetErrorNumTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetErrorNumTest003 end";
}

/**
 * @tc.name: GetErrorNumTest004
 * @tc.desc: Verify the GetErrorNum function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetErrorNumTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorNumTest004 start";
    try {
        int32_t error = -1;
        auto res = GetErrorNum(error);
        EXPECT_EQ(res, static_cast<int32_t>(ErrorReason::NO_SYNC_STATE));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetErrorNumTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetErrorNumTest004 end";
}

/**
 * @tc.name: SetFileSyncStatesTest001
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest001 start";
    try {
        FileSyncState fileSyncStates;
        fileSyncStates.path = "/storage/Users/currentUser/testfile.txt";
        string syncFolder = "/storage/Users/currentUser/";
        fileSyncStates.state = SyncState::SYNCING;
        int32_t userId = 1;
        FailedList failed;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).WillOnce(Return(0));
        auto res = SetFileSyncStates(fileSyncStates, userId, failed, syncFolder);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest001 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest001 end";
}

/**
 * @tc.name: SetFileSyncStatesTest002
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest002 start";
    try {
        FileSyncState fileSyncStates;
        fileSyncStates.path = "/storage/Users/currentUser/testfile.txt";
        string syncFolder = "/storage/Users/otherUser";
        int32_t userId = 1;
        FailedList failed;
        auto res = SetFileSyncStates(fileSyncStates, userId, failed, syncFolder);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest002 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest002 end";
}

/**
 * @tc.name: SetFileSyncStatesTest003
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest003 start";
    try {
        FileSyncState fileSyncStates;
        fileSyncStates.path = "/storage/Users/currentUser/testfile.txt";
        string syncFolder = "/storage/Users/currentUser";
        fileSyncStates.state = static_cast<SyncState>(6);
        int32_t userId = 1;
        FailedList failed;
        auto res = SetFileSyncStates(fileSyncStates, userId, failed, syncFolder);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest003 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest003 end";
}

/**
 * @tc.name: SetFileSyncStatesTest004
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 start";
    try {
        FileSyncState fileSyncStates;
        fileSyncStates.path = "/storage/Users/currentUser/testfile.txt";
        string syncFolder = "/storage/Users/currentUser";
        fileSyncStates.state = SyncState::SYNCING;
        int32_t userId = 1;
        FailedList failed;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).WillOnce(Return(1));
        auto res = SetFileSyncStates(fileSyncStates, userId, failed, syncFolder);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 end";
}

/**
 * @tc.name: SetFileSyncStatesTest005
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest005 start";
    try {
        FileSyncState fileSyncStates;
        fileSyncStates.path = "/storage/Users/currentUser/testfile.txt";
        string syncFolder = "";
        fileSyncStates.state = SyncState::SYNCING;
        int32_t userId = 1;
        FailedList failed;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).WillOnce(Return(1));
        auto res = SetFileSyncStates(fileSyncStates, userId, failed, syncFolder);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest005 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest005 end";
}

/**
 * @tc.name: SetFileSyncStatesTest006
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest006 start";
    try {
        FileSyncState fileSyncStates;
        fileSyncStates.path = "/test/mockFailed";
        string syncFolder = "/test";
        int32_t userId = 1;
        FailedList failed;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).Times(0);
        auto res = SetFileSyncStates(fileSyncStates, userId, failed, syncFolder);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest006 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest006 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest001
 * @tc.desc: Verify placeholder xattr value 1 and 2 are treated as placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 start";
    try {
        const string path = "/mnt/hmdfs/1/account/device_view/local/files/Docs/testfile.txt";
        bool isPlaceholder = false;
        EXPECT_CALL(*insMock_, getxattr(_, StrEq("user.clouddisk.placeholder"), nullptr, 0)).WillOnce(Return(1));
        EXPECT_CALL(*insMock_, getxattr(_, StrEq("user.clouddisk.placeholder"), _, 1))
            .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
                static_cast<char *>(value)[0] = 1;
                return static_cast<ssize_t>(size);
            }));

        auto res = QueryPlaceholderByXattr(path, isPlaceholder);

        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(isPlaceholder);

        Mock::VerifyAndClearExpectations(insMock_.get());
        isPlaceholder = false;
        EXPECT_CALL(*insMock_, getxattr(_, StrEq("user.clouddisk.placeholder"), nullptr, 0)).WillOnce(Return(1));
        EXPECT_CALL(*insMock_, getxattr(_, StrEq("user.clouddisk.placeholder"), _, 1))
            .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
                static_cast<char *>(value)[0] = 2;
                return static_cast<ssize_t>(size);
            }));

        res = QueryPlaceholderByXattr(path, isPlaceholder);

        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 failed";
    }
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest002
 * @tc.desc: Verify placeholder xattr values other than 1 and 2 are not placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 start";
    try {
        const string path = "/mnt/hmdfs/1/account/device_view/local/files/Docs/testfile.txt";
        bool isPlaceholder = true;
        EXPECT_CALL(*insMock_, getxattr(_, StrEq("user.clouddisk.placeholder"), nullptr, 0)).WillOnce(Return(1));
        EXPECT_CALL(*insMock_, getxattr(_, StrEq("user.clouddisk.placeholder"), _, 1))
            .WillOnce(Invoke([](const char *, const char *, void *value, size_t size) {
                static_cast<char *>(value)[0] = 3;
                return static_cast<ssize_t>(size);
            }));

        auto res = QueryPlaceholderByXattr(path, isPlaceholder);

        EXPECT_EQ(res, E_OK);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 failed";
    }
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 end";
}

/**
 * @tc.name: GetFileSyncStateTest001
 * @tc.desc: Verify the GetFileSyncState function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetFileSyncStateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStateTest001 start";
    try {
        string path = "/storage/Users/currentUser/testfile.txt";
        string syncFolder = "/storage/Users/currentUser/";
        int32_t userId = 1;
        auto res = GetFileSyncState(path, userId, syncFolder);
        EXPECT_FALSE(res.isSuccess);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileSyncStateTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStateTest001 end";
}

/**
 * @tc.name: GetFileSyncStateTest002
 * @tc.desc: Verify the GetFileSyncState function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetFileSyncStateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStateTest002 start";
    try {
        string path = "/storage/Users/currentUser/testfile.txt";
        string syncFolder = "/storage/Users/otherUser";
        int32_t userId = 1;
        auto res = GetFileSyncState(path, userId, syncFolder);
        EXPECT_EQ(res.isSuccess, false);
        EXPECT_EQ(res.error, ErrorReason::INVALID_ARGUMENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileSyncStateTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStateTest002 end";
}

/**
 * @tc.name: GetFileSyncStateTest003
 * @tc.desc: Verify the GetFileSyncState function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetFileSyncStateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStateTest003 start";
    try {
        string path = "/test/mockFailed";
        string syncFolder = "";
        int32_t userId = 1;
        auto res = GetFileSyncState(path, userId, syncFolder);
        EXPECT_EQ(res.isSuccess, false);
        EXPECT_EQ(res.error, ErrorReason::NO_SUCH_FILE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileSyncStateTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStateTest003 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest001
 * @tc.desc: Verify sync folder prefix check rejects sibling paths and accepts child paths
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest001 start";
    GTEST_LOG_(INFO) << "[BRANCH] IsPathInSyncFolder child path and sibling prefix";
    EXPECT_TRUE(IsPathInSyncFolder("/storage/Users/currentUser/Docs/",
                                   "/storage/Users/currentUser/Docs/file.txt"));
    EXPECT_TRUE(IsPathInSyncFolder("/storage/Users/currentUser/Docs",
                                   "/storage/Users/currentUser/Docs/file.txt"));
    EXPECT_FALSE(IsPathInSyncFolder("/storage/Users/currentUser/Docs",
                                    "/storage/Users/currentUser/Docs"));
    EXPECT_FALSE(IsPathInSyncFolder("/storage/Users/currentUser/Docs",
                                    "/storage/Users/currentUser/Docs2/file.txt"));
    EXPECT_FALSE(IsPathInSyncFolder("", "/storage/Users/currentUser/Docs/file.txt"));

    GTEST_LOG_(INFO) << "[BRANCH] HasInvalidRelativePathSegment validates relative paths";
    EXPECT_FALSE(HasInvalidRelativePathSegment("dir/file.txt"));
    EXPECT_TRUE(HasInvalidRelativePathSegment(""));
    EXPECT_FALSE(HasInvalidRelativePathSegment("/file.txt"));
    EXPECT_FALSE(HasInvalidRelativePathSegment("dir/"));
    EXPECT_TRUE(HasInvalidRelativePathSegment("dir/./file.txt"));
    EXPECT_TRUE(HasInvalidRelativePathSegment("dir/../file.txt"));
    EXPECT_FALSE(HasInvalidRelativePathSegment("dir//file.txt"));
    EXPECT_EQ(JoinSyncFolderAndRelativePath("/storage/Users/currentUser/Docs", "file.txt"),
              "/storage/Users/currentUser/Docs/file.txt");
    EXPECT_EQ(JoinSyncFolderAndRelativePath("/storage/Users/currentUser/Docs/", "file.txt"),
              "/storage/Users/currentUser/Docs/file.txt");
    EXPECT_EQ(JoinSyncFolderAndRelativePath("/", "file.txt"), "/file.txt");
    EXPECT_EQ(JoinSyncFolderAndRelativePath("", "file.txt"), "file.txt");
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest001 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest002
 * @tc.desc: Verify BuildCreatePlaceholderPath rejects invalid relative paths
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest002 start";
    CreatePlaceholderPath path;

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath empty relative path";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER,
                                         "",
                                         TEST_USER_ID,
                                         path),
              E_INVALID_ARG);

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath absolute relative path";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "/file.txt", TEST_USER_ID, path),
              E_INVALID_ARG);

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath path ends with slash";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir/", TEST_USER_ID, path),
              E_INVALID_ARG);

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath rejects dot segment";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir/./file.txt", TEST_USER_ID, path),
              E_INVALID_ARG);

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath rejects dot dot segment";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir/../file.txt", TEST_USER_ID, path),
              E_INVALID_ARG);

    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest002 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest003
 * @tc.desc: Verify BuildCreatePlaceholderPath maps parent mount path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest003 start";
    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath success";
    CreatePlaceholderPath path;
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, TEST_USER_ID, path),
              E_OK);
    EXPECT_EQ(path.parentMntPath, TEST_SYNC_FOLDER_MNT);
    EXPECT_EQ(path.fileName, "placeholder.txt");

    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir/placeholder.txt", TEST_USER_ID, path),
              E_OK);
    EXPECT_EQ(path.parentMntPath, TEST_SYNC_FOLDER_MNT + "/dir");
    EXPECT_EQ(path.fileName, "placeholder.txt");

    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir//placeholder.txt", TEST_USER_ID, path),
              E_OK);
    EXPECT_EQ(path.parentMntPath, TEST_SYNC_FOLDER_MNT + "/dir/");
    EXPECT_EQ(path.fileName, "placeholder.txt");

    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER + "/", TEST_RELATIVE_PATH, TEST_USER_ID, path),
              E_OK);
    EXPECT_EQ(path.parentMntPath, TEST_SYNC_FOLDER_MNT + "/");
    EXPECT_EQ(path.fileName, "placeholder.txt");

    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest003 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest004
 * @tc.desc: Verify BuildCreatePlaceholderPath returns sync folder mnt conversion errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest004 start";
    CreatePlaceholderPath path;

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath sync folder mnt conversion failed";
    EXPECT_EQ(BuildCreatePlaceholderPath("/storage/Users/currentUser/mockMntFailed",
                                         "file.txt",
                                         TEST_USER_ID,
                                         path),
              E_SYNC_FOLDER_PATH_NOT_EXIST);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest004 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest005
 * @tc.desc: Verify CreatePlaceholderFileAt maps open parent failures
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest005 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt open parent denied";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;

    Assistant::mockErrno = EACCES;
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*insMock_, OpenAt(_, _, _, _)).Times(0);
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest005 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest006
 * @tc.desc: Verify CreatePlaceholderFileAt maps openat failures
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest006 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt openat existing file";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;

    Assistant::mockErrno = EEXIST;
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_FILE_ALREADY_EXISTS);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest006 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest007
 * @tc.desc: Verify CreatePlaceholderFileAt succeeds when file creation succeeds
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest007 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt success with ioctl enabled";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;
    info.logicalSize = 4096;
    info.atimeMs = 100;
    info.mtimeMs = 200;
    bool ioctlPayloadChecked = false;

    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, Ioctl(11, HMDFS_IOC_CREATE_PLACEHOLDER, _))
        .WillOnce(Invoke([&ioctlPayloadChecked](int, int, void *arg) {
            auto *create = reinterpret_cast<HmdfsPlaceholderAttr*>(arg);
            ioctlPayloadChecked = create != nullptr && create->logicalSize == 4096 &&
                create->atimeMs == 100 && create->mtimeMs == 200;
            return 0;
        }));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_OK);
    EXPECT_TRUE(ioctlPayloadChecked);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest007 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest008
 * @tc.desc: Verify CreatePlaceholderFileAt rolls back file when ioctl is unsupported
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest008 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt ioctl not supported rollback";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;

    Assistant::mockErrno = ENOTTY;
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, Ioctl(11, HMDFS_IOC_CREATE_PLACEHOLDER, _)).WillOnce(Return(-1));
    EXPECT_CALL(*insMock_, UnlinkAt(10, StrEq("placeholder.txt"), 0)).WillOnce(Return(0));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_NOT_SUPPORTED);
    Mock::VerifyAndClearExpectations(insMock_.get());

    Assistant::mockErrno = ENOTTY;
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, Ioctl(11, HMDFS_IOC_CREATE_PLACEHOLDER, _)).WillOnce(Return(-1));
    EXPECT_CALL(*insMock_, UnlinkAt(10, StrEq("placeholder.txt"), 0))
        .WillOnce(Invoke([](int, const char *, int) {
            errno = EACCES;
            return -1;
        }));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_NOT_SUPPORTED);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest008 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest009
 * @tc.desc: Verify ConvertErrnoToCloudDiskError maps errno branches through the public utility
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest009 start";
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError create errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EEXIST), E_FILE_ALREADY_EXISTS);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(ENOTDIR), E_NOT_A_DIRECTORY);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(ENOSPC), E_NO_SPACE_LEFT);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EDQUOT), E_NO_SPACE_LEFT);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError invalid argument errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EINVAL), E_INVALID_ARG);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EISDIR), E_INVALID_ARG);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(ELOOP), E_INVALID_ARG);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError missing sync folder errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(ENOENT), E_SYNC_FOLDER_PATH_NOT_EXIST);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError permission denied errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EACCES), E_PERMISSION_DENIED);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EPERM), E_PERMISSION_DENIED);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError unsupported errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(ENOTTY), E_NOT_SUPPORTED);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EOPNOTSUPP), E_NOT_SUPPORTED);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError retryable errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EIO), E_TRY_AGAIN);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EBUSY), E_TRY_AGAIN);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest009 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest010
 * @tc.desc: Verify NormalizeCreatePlaceholderError keeps service errors and maps errno
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest010 start";
    GTEST_LOG_(INFO) << "[BRANCH] NormalizeCreatePlaceholderError service error";
    EXPECT_EQ(NormalizeCreatePlaceholderError(E_PERMISSION_DENIED), E_PERMISSION_DENIED);
    EXPECT_EQ(NormalizeCreatePlaceholderError(E_FILE_ALREADY_EXISTS), E_FILE_ALREADY_EXISTS);
    EXPECT_EQ(NormalizeCreatePlaceholderError(E_NO_SPACE_LEFT), E_NO_SPACE_LEFT);
    EXPECT_EQ(NormalizeCreatePlaceholderError(E_NOT_A_DIRECTORY), E_NOT_A_DIRECTORY);
    GTEST_LOG_(INFO) << "[BRANCH] NormalizeCreatePlaceholderError errno fallback";
    EXPECT_EQ(NormalizeCreatePlaceholderError(ENOENT), E_SYNC_FOLDER_PATH_NOT_EXIST);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest010 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest001
 * @tc.desc: Verify CreatePlaceholderFileInner returns sync folder path errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest001 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner sync folder physical path failed";
    CloudDiskService service;
    PlaceholderInfo info;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).Times(0);
    EXPECT_EQ(service.CreatePlaceholderFileInner("/test/mockFailed", TEST_RELATIVE_PATH, info),
              E_SYNC_FOLDER_PATH_NOT_EXIST);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest001 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest002
 * @tc.desc: Verify CreatePlaceholderFileInner returns bundle query errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest002 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner get bundle failed";
    CloudDiskService service;
    PlaceholderInfo info;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(EIO));
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info), E_TRY_AGAIN);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest002 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest003
 * @tc.desc: Verify CreatePlaceholderFileInner rejects unregistered sync folders
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest003 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner sync folder not registered";
    CloudDiskService service;
    PlaceholderInfo info;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info),
              E_SYNC_FOLDER_NOT_REGISTERED);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest003 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest004
 * @tc.desc: Verify CreatePlaceholderFileInner rejects bundle mismatches
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest004 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner bundle mismatch";
    RegisterPlaceholderSyncFolder("wrong.bundle");
    CloudDiskService service;
    PlaceholderInfo info;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info),
              E_SYNC_FOLDER_NOT_REGISTERED);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest004 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest005
 * @tc.desc: Verify CreatePlaceholderFileInner returns parent mnt conversion failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest005 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner BuildCreatePlaceholderPath failed";
    const std::string syncFolder = "/storage/Users/currentUser/mockMntFailed";
    const std::string physicalPath = "/data/service/el2/100/hmdfs/account/files/Docs/mockMntFailed";
    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(physicalPath);
    CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, {TEST_BUNDLE, physicalPath});
    CloudDiskService service;
    PlaceholderInfo info;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_EQ(service.CreatePlaceholderFileInner(syncFolder, "file.txt", info),
              E_SYNC_FOLDER_PATH_NOT_EXIST);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest005 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest006
 * @tc.desc: Verify CreatePlaceholderFileInner returns file creation errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest006 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner CreatePlaceholderFileAt failed";
    RegisterPlaceholderSyncFolder();
    CloudDiskService service;
    PlaceholderInfo info;

    Assistant::mockErrno = EEXIST;
    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info), E_FILE_ALREADY_EXISTS);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest006 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest007
 * @tc.desc: Verify CreatePlaceholderFileInner returns success after file create and ioctl
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest007 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner file create and ioctl success";
    RegisterPlaceholderSyncFolder();
    CloudDiskService service;
    PlaceholderInfo info;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, Ioctl(11, HMDFS_IOC_CREATE_PLACEHOLDER, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, Unlink(_)).Times(0);
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info), E_OK);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest007 end";
}

/**
 * @tc.name: PlaceholderInfoParcelTest001
 * @tc.desc: Verify PlaceholderInfo keeps atimeMs through parcel marshalling
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderInfoParcelTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest001 start";
    GTEST_LOG_(INFO) << "[BRANCH] PlaceholderInfo parcel uses atimeMs metadata";
    MessageParcel parcel;
    PlaceholderInfo info;
    info.logicalSize = 1024;
    info.atimeMs = 11;
    info.mtimeMs = 22;

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.logicalSize)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.atimeMs)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.mtimeMs)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(info.logicalSize), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(info.atimeMs), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(info.mtimeMs), Return(true)));

    ASSERT_TRUE(info.Marshalling(parcel));
    auto result = PlaceholderInfo::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->logicalSize, info.logicalSize);
    EXPECT_EQ(result->atimeMs, info.atimeMs);
    EXPECT_EQ(result->mtimeMs, info.mtimeMs);
    delete result;
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest001 end";
}

/**
 * @tc.name: PlaceholderInfoParcelTest002
 * @tc.desc: Verify PlaceholderInfo marshalling fails when logicalSize write fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderInfoParcelTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest002 start";
    GTEST_LOG_(INFO) << "[BRANCH] PlaceholderInfo marshalling WriteUint64(logicalSize) failed";
    MessageParcel parcel;
    PlaceholderInfo info;
    info.logicalSize = 1024;

    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.logicalSize)).WillOnce(Return(false));
    EXPECT_FALSE(info.Marshalling(parcel));
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest002 end";
}

/**
 * @tc.name: PlaceholderInfoParcelTest003
 * @tc.desc: Verify PlaceholderInfo marshalling fails when atimeMs write fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderInfoParcelTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest003 start";
    GTEST_LOG_(INFO) << "[BRANCH] PlaceholderInfo marshalling WriteUint64(atimeMs) failed";
    MessageParcel parcel;
    PlaceholderInfo info;
    info.logicalSize = 1024;
    info.atimeMs = 11;

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.logicalSize)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.atimeMs)).WillOnce(Return(false));
    EXPECT_FALSE(info.Marshalling(parcel));
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest003 end";
}

/**
 * @tc.name: PlaceholderInfoParcelTest004
 * @tc.desc: Verify PlaceholderInfo marshalling fails when mtimeMs write fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderInfoParcelTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest004 start";
    GTEST_LOG_(INFO) << "[BRANCH] PlaceholderInfo marshalling WriteUint64(mtimeMs) failed";
    MessageParcel parcel;
    PlaceholderInfo info;
    info.logicalSize = 1024;
    info.atimeMs = 11;
    info.mtimeMs = 22;

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.logicalSize)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.atimeMs)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(info.mtimeMs)).WillOnce(Return(false));
    EXPECT_FALSE(info.Marshalling(parcel));
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest004 end";
}

/**
 * @tc.name: PlaceholderInfoParcelTest005
 * @tc.desc: Verify PlaceholderInfo ReadFromParcel fails when logicalSize read fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderInfoParcelTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest005 start";
    GTEST_LOG_(INFO) << "[BRANCH] PlaceholderInfo ReadUint64(logicalSize) failed";
    MessageParcel parcel;
    PlaceholderInfo info;

    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(false));
    EXPECT_FALSE(info.ReadFromParcel(parcel));
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest005 end";
}

/**
 * @tc.name: PlaceholderInfoParcelTest006
 * @tc.desc: Verify PlaceholderInfo ReadFromParcel fails when atimeMs read fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderInfoParcelTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest006 start";
    GTEST_LOG_(INFO) << "[BRANCH] PlaceholderInfo ReadUint64(atimeMs) failed";
    MessageParcel parcel;
    PlaceholderInfo info;
    constexpr uint64_t logicalSize = 1024;

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(logicalSize), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(false));
    EXPECT_FALSE(info.ReadFromParcel(parcel));
    EXPECT_EQ(info.logicalSize, logicalSize);
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest006 end";
}

/**
 * @tc.name: PlaceholderInfoParcelTest007
 * @tc.desc: Verify PlaceholderInfo ReadFromParcel fails when mtimeMs read fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderInfoParcelTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest007 start";
    GTEST_LOG_(INFO) << "[BRANCH] PlaceholderInfo ReadUint64(mtimeMs) failed";
    MessageParcel parcel;
    PlaceholderInfo info;
    constexpr uint64_t logicalSize = 1024;
    constexpr uint64_t atimeMs = 11;

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(logicalSize), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(atimeMs), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(false));
    EXPECT_FALSE(info.ReadFromParcel(parcel));
    EXPECT_EQ(info.logicalSize, logicalSize);
    EXPECT_EQ(info.atimeMs, atimeMs);
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest007 end";
}

/**
 * @tc.name: PlaceholderInfoParcelTest008
 * @tc.desc: Verify PlaceholderInfo unmarshalling returns null when ReadFromParcel fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderInfoParcelTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest008 start";
    GTEST_LOG_(INFO) << "[BRANCH] PlaceholderInfo unmarshalling ReadFromParcel failed";
    MessageParcel parcel;

    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(false));
    auto result = PlaceholderInfo::Unmarshalling(parcel);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "PlaceholderInfoParcelTest008 end";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
