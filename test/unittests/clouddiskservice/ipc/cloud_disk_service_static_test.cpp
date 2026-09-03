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

#include <atomic>
#include <cstring>
#include <thread>
#include <vector>

#include "assistant.h"
#include "cloud_disk_service_access_token_mock.h"
#include "cloud_disk_service_utils.h"
#include "message_parcel_mock.h"
#include "securec.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::Storage::DistributedFile;

namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t TEST_CLOUD_DISK_SERVICE_SA_ID = 5207;
constexpr bool TEST_RUN_ON_CREATE = true;
constexpr size_t PLACEHOLDER_XATTR_VALUE_SIZE = 1;
constexpr int32_t MOCK_SYSCALL_FAILED = -1;
constexpr int32_t TEST_PARENT_FD = 10;
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

namespace {
const string PLACEHOLDER_TEST_PATH = "/mnt/hmdfs/1/account/device_view/local/files/Docs/testfile.txt";
const string PLACEHOLDER_TEST_SYNC_FOLDER = "/storage/Users/currentUser/sync";
const string PLACEHOLDER_TEST_PHYSICAL_SYNC_FOLDER = "/data/service/el2/100/hmdfs/account/files/Docs/sync";
const string PLACEHOLDER_TEST_MNT_SYNC_FOLDER = "/mnt/hmdfs/100/account/device_view/local/files/Docs/sync";
const string PLACEHOLDER_TEST_BUNDLE_NAME = "com.test.placeholder";
const char *PLACEHOLDER_TEST_XATTR = CLOUD_DISK_FILE_SYNC_STATE_XATTR;
constexpr uint8_t PLACEHOLDER_TEST_VALUE_LOCAL =
    static_cast<uint8_t>(PLACEHOLDER_STATE_UNHYDRATED << FILE_SYNC_STATE_PLACEHOLDER_SHIFT);
constexpr uint8_t PLACEHOLDER_TEST_VALUE_HYDRATING =
    static_cast<uint8_t>(PLACEHOLDER_STATE_PARTIALLY_HYDRATED << FILE_SYNC_STATE_PLACEHOLDER_SHIFT);
constexpr uint8_t PLACEHOLDER_TEST_VALUE_OTHER = static_cast<uint8_t>(SyncState::SYNCING);

void ExpectPlaceholderXattrValue(const shared_ptr<AssistantMock> &mock, uint8_t placeholderValue)
{
    EXPECT_CALL(*mock, getxattr(_, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .WillOnce(Invoke([placeholderValue](const char *, const char *, void *value, size_t size) {
            static_cast<uint8_t *>(value)[0] = placeholderValue;
            return static_cast<ssize_t>(size);
        }));
}

void ExpectPlaceholderXattrValueAtPath(const shared_ptr<AssistantMock> &mock,
                                       const std::string &path,
                                       uint8_t placeholderValue)
{
    EXPECT_CALL(*mock, getxattr(StrEq(path), StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .WillOnce(Invoke([placeholderValue](const char *, const char *, void *value, size_t size) {
            static_cast<uint8_t *>(value)[0] = placeholderValue;
            return static_cast<ssize_t>(size);
        }));
}

void ExpectPlaceholderXattrFailed(const shared_ptr<AssistantMock> &mock, int32_t error)
{
    EXPECT_CALL(*mock, getxattr(_, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .WillOnce(Invoke([error](const char *, const char *, void *, size_t) {
            errno = error;
            return static_cast<ssize_t>(MOCK_SYSCALL_FAILED);
        }));
}

void ExpectPlaceholderXattrSecondFailed(const shared_ptr<AssistantMock> &mock, int32_t error)
{
    EXPECT_CALL(*mock, getxattr(_, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .WillOnce(Invoke([error](const char *, const char *, void *, size_t) {
            errno = error;
            return static_cast<ssize_t>(MOCK_SYSCALL_FAILED);
        }));
}

void ExpectPlaceholderPathType(const shared_ptr<AssistantMock> &mock, const std::string &path, mode_t mode)
{
    struct stat statInfo = {};
    statInfo.st_mode = mode;
    EXPECT_CALL(*mock, MockStat(StrEq(path), _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
}

void ClearPlaceholderXattrExpectations(const shared_ptr<AssistantMock> &mock)
{
    Mock::VerifyAndClearExpectations(mock.get());
}

void AddPlaceholderSyncFolder()
{
    CloudDiskSyncFolder::GetInstance().ClearMap();
    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(PLACEHOLDER_TEST_PHYSICAL_SYNC_FOLDER);
    SyncFolderValue syncFolderValue = {PLACEHOLDER_TEST_BUNDLE_NAME, PLACEHOLDER_TEST_PHYSICAL_SYNC_FOLDER};
    CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
}

void AddPlaceholderSyncFolder(const std::string &physicalSyncFolder,
                              const std::string &bundleName = PLACEHOLDER_TEST_BUNDLE_NAME)
{
    CloudDiskSyncFolder::GetInstance().ClearMap();
    auto syncFolderIndex = CloudDisk::CloudFileUtils::DentryHash(physicalSyncFolder);
    SyncFolderValue syncFolderValue = {bundleName, physicalSyncFolder};
    CloudDiskSyncFolder::GetInstance().AddSyncFolder(syncFolderIndex, syncFolderValue);
}

void ExpectPlaceholderCaller(const shared_ptr<CloudDiskServiceAccessTokenMock> &mock)
{
    EXPECT_CALL(*mock, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*mock, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(PLACEHOLDER_TEST_BUNDLE_NAME), Return(E_OK)));
}

class DehydrateCallbackTableStub final : public IRemoteStub<ICloudDiskServiceCallbackTable> {
public:
    explicit DehydrateCallbackTableStub(bool allow) : allow_(allow) {}

    void OnCallback(const CloudDiskCallbackReqHead &reqHead, CloudDiskCallbackContext &reqContext) override
    {
        ++callbackCount_;
        if (reqHead.callbackType == CloudDiskCallbackType::DEHYDRATE && reqContext.dehydrateData != nullptr) {
            reqContext.dehydrateData->allow = allow_;
        }
    }

    int32_t OnRemoteRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return E_OK;
    }

    bool allow_;
    uint32_t callbackCount_ = 0;
};
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
    PlaceholderTaskManager::GetInstance().StopWorkerPool();
    Mock::VerifyAndClearExpectations(insMock_.get());
    Mock::VerifyAndClearExpectations(dfsuAccessToken_.get());
    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    DfsMessageParcel::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
    Assistant::mockFdApi = false;
    CloudDiskSyncFolder::GetInstance().ClearMap();
    PlaceholderCallbackManager::GetInstance().ClearBySyncFolder(PLACEHOLDER_TEST_BUNDLE_NAME, 1);
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
 * @tc.desc: Verify placeholder xattr value '1' is treated as placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrValue(insMock_, PLACEHOLDER_TEST_VALUE_LOCAL);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest002
 * @tc.desc: Verify placeholder xattr value '2' is treated as placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrValue(insMock_, PLACEHOLDER_TEST_VALUE_HYDRATING);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest003
 * @tc.desc: Verify placeholder xattr values other than '1' and '2' are not placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest003 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrValue(insMock_, PLACEHOLDER_TEST_VALUE_OTHER);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_OK);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest003 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest003 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest004
 * @tc.desc: Verify first getxattr missing file failure is converted to file not exist
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest004 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrFailed(insMock_, ENOENT);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_FILE_NOT_EXIST);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest004 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest004 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest005
 * @tc.desc: Verify second getxattr failure is converted to service error code
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest005 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrSecondFailed(insMock_, EACCES);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_ACCES);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest005 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest005 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest006
 * @tc.desc: Verify first getxattr EOPNOTSUPP is converted to not supported
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest006 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrFailed(insMock_, EOPNOTSUPP);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_NOT_SUPPORTED);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest006 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest006 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest007
 * @tc.desc: Verify first getxattr EINVAL is converted to invalid argument
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest007 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrFailed(insMock_, EINVAL);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest007 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest007 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest008
 * @tc.desc: Verify first getxattr ENODATA means the file is not a placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest008 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrFailed(insMock_, ENODATA);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_OK);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest008 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest008 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest009
 * @tc.desc: Verify first getxattr unexpected errno is converted to try again
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest009 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrFailed(insMock_, EIO);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_TRY_AGAIN);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest009 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest009 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest010
 * @tc.desc: Verify second getxattr EOPNOTSUPP is converted to not supported
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest010 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrSecondFailed(insMock_, EOPNOTSUPP);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_NOT_SUPPORTED);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest010 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest010 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest011
 * @tc.desc: Verify second getxattr ENAMETOOLONG is converted to name too long
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest011 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrSecondFailed(insMock_, ENAMETOOLONG);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_NAME_TOO_LONG);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest011 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest011 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest012
 * @tc.desc: Verify second getxattr ERANGE is converted to invalid argument
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest012 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrSecondFailed(insMock_, ERANGE);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest012 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest012 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest013
 * @tc.desc: Verify second getxattr unexpected errno is converted to try again
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest013 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrSecondFailed(insMock_, EIO);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_TRY_AGAIN);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest013 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest013 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest014
 * @tc.desc: Verify second getxattr ENODATA means the file is not a placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, QueryPlaceholderByXattrTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest014 start";
    try {
        bool isPlaceholder = false;
        ExpectPlaceholderXattrSecondFailed(insMock_, ENODATA);

        auto res = QueryPlaceholderByXattr(PLACEHOLDER_TEST_PATH, isPlaceholder);

        EXPECT_EQ(res, E_OK);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest014 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest014 end";
}

/**
 * @tc.name: ConvertPlaceholderXattrErrnoTest001
 * @tc.desc: Verify placeholder xattr errno conversion handles ENODATA as non-placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderXattrErrnoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderXattrErrnoTest001 start";
    EXPECT_EQ(ConvertPlaceholderXattrErrno(ENODATA), E_OK);
    EXPECT_EQ(ConvertPlaceholderXattrErrno(ERANGE), E_INVALID_ARG);
    EXPECT_EQ(ConvertPlaceholderXattrErrno(ENAMETOOLONG), E_NAME_TOO_LONG);
    EXPECT_EQ(ConvertPlaceholderXattrErrno(ENOENT), E_FILE_NOT_EXIST);
    EXPECT_EQ(ConvertPlaceholderXattrErrno(EIO), E_TRY_AGAIN);
    GTEST_LOG_(INFO) << "ConvertPlaceholderXattrErrnoTest001 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest001
 * @tc.desc: Verify IsPlaceholderFileInner returns sync folder path conversion error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest001 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string syncFolder = "/test/mockFailed";
        string path = "a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(syncFolder, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(res, E_SYNC_FOLDER_PATH_NOT_EXIST);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest001 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest002
 * @tc.desc: Verify IsPlaceholderFileInner queries xattr with a path relative to the sync folder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest002 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/a.txt";
        bool isPlaceholder = false;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        AddPlaceholderSyncFolder();
        ExpectPlaceholderCaller(dfsuAccessToken_);
        ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_MNT_SYNC_FOLDER + "/" + path, S_IFREG);
        ExpectPlaceholderXattrValueAtPath(insMock_, PLACEHOLDER_TEST_MNT_SYNC_FOLDER + "/" + path,
                                          PLACEHOLDER_TEST_VALUE_LOCAL);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest002 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest002 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest003
 * @tc.desc: Verify IsPlaceholderFileInner rejects an empty relative path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest003 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest003 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest003 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest004
 * @tc.desc: Verify IsPlaceholderFileInner rejects an absolute path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest004 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "/dir/a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest004 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest004 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest005
 * @tc.desc: Verify IsPlaceholderFileInner rejects path traversal
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest005 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/../a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest005 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest005 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest006
 * @tc.desc: Verify IsPlaceholderFileInner rejects a relative path ending with slash
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest006 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest006 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest006 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest007
 * @tc.desc: Verify IsPlaceholderFileInner falls back to account id when caller user id is zero
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest007 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/a.txt";
        bool isPlaceholder = false;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        AddPlaceholderSyncFolder();
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetAccountId(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_USER_ID), Return(E_OK)));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
            .WillOnce(DoAll(SetArgReferee<0>(PLACEHOLDER_TEST_BUNDLE_NAME), Return(E_OK)));
        ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_MNT_SYNC_FOLDER + "/" + path, S_IFREG);
        ExpectPlaceholderXattrValueAtPath(insMock_, PLACEHOLDER_TEST_MNT_SYNC_FOLDER + "/" + path,
                                          PLACEHOLDER_TEST_VALUE_HYDRATING);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest007 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest007 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest008
 * @tc.desc: Verify IsPlaceholderFileInner returns try again when bundle name query fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest008 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_TRY_AGAIN));
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(res, E_TRY_AGAIN);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest008 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest008 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest009
 * @tc.desc: Verify IsPlaceholderFileInner returns not registered when sync folder has no record
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest009 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        ExpectPlaceholderCaller(dfsuAccessToken_);
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(res, E_SYNC_FOLDER_NOT_REGISTERED);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest009 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest009 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest010
 * @tc.desc: Verify IsPlaceholderFileInner returns not registered when caller bundle mismatches
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest010 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        AddPlaceholderSyncFolder();
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
            .WillOnce(DoAll(SetArgReferee<0>("com.test.other"), Return(E_OK)));
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        EXPECT_EQ(res, E_SYNC_FOLDER_NOT_REGISTERED);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest010 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest010 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest011
 * @tc.desc: Verify IsPlaceholderFileInner returns xattr error when placeholder query fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest011 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        AddPlaceholderSyncFolder();
        ExpectPlaceholderCaller(dfsuAccessToken_);
        ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_MNT_SYNC_FOLDER + "/" + path, S_IFREG);
        ExpectPlaceholderXattrFailed(insMock_, ENOENT);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        EXPECT_EQ(res, E_FILE_NOT_EXIST);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest011 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest011 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest012
 * @tc.desc: Verify IsPlaceholderFileInner returns invalid arg when mnt sync folder conversion fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest012 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string syncFolder = "/storage/Users/currentUser/mockMntPhysicalPathFailed";
        string physicalSyncFolder = "/data/service/el2/100/hmdfs/account/files/Docs/mockMntPhysicalPathFailed";
        string path = "dir/a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        AddPlaceholderSyncFolder(physicalSyncFolder);
        ExpectPlaceholderCaller(dfsuAccessToken_);
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(syncFolder, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest012 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest012 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest013
 * @tc.desc: Verify IsPlaceholderFileInner rejects an xattr path outside the sync folder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest013 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string syncFolder = "raw_sync_folder";
        string path = "dir/a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        AddPlaceholderSyncFolder(syncFolder);
        ExpectPlaceholderCaller(dfsuAccessToken_);
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(syncFolder, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest013 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest013 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest014
 * @tc.desc: Verify IsPlaceholderFileInner rejects a directory path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest014 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        AddPlaceholderSyncFolder();
        ExpectPlaceholderCaller(dfsuAccessToken_);
        ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_MNT_SYNC_FOLDER + "/" + path, S_IFDIR);
        EXPECT_CALL(*insMock_, getxattr(_, _, _, _)).Times(0);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        EXPECT_EQ(res, E_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest014 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest014 end";
}

/**
 * @tc.name: IsPlaceholderFileInnerTest015
 * @tc.desc: Verify IsPlaceholderFileInner returns false when placeholder xattr does not exist
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, IsPlaceholderFileInnerTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest015 start";
    try {
        CloudDiskService cloudDiskService(TEST_CLOUD_DISK_SERVICE_SA_ID, TEST_RUN_ON_CREATE);
        string path = "dir/a.txt";
        bool isPlaceholder = true;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        AddPlaceholderSyncFolder();
        ExpectPlaceholderCaller(dfsuAccessToken_);
        ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_MNT_SYNC_FOLDER + "/" + path, S_IFREG);
        ExpectPlaceholderXattrFailed(insMock_, ENODATA);
#endif

        auto res = cloudDiskService.IsPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, path, isPlaceholder);

#ifdef SUPPORT_CLOUD_DISK_SERVICE
        CloudDiskSyncFolder::GetInstance().ClearMap();
        EXPECT_EQ(res, E_OK);
        EXPECT_FALSE(isPlaceholder);
#else
        EXPECT_EQ(res, E_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest015 failed";
    }
    ClearPlaceholderXattrExpectations(insMock_);
    GTEST_LOG_(INFO) << "IsPlaceholderFileInnerTest015 end";
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
    EXPECT_TRUE(IsPathInSyncFolder("/storage/Users/currentUser/Docs/", "/storage/Users/currentUser/Docs/file.txt"));
    EXPECT_TRUE(IsPathInSyncFolder("/storage/Users/currentUser/Docs", "/storage/Users/currentUser/Docs/file.txt"));
    EXPECT_FALSE(IsPathInSyncFolder("/storage/Users/currentUser/Docs", "/storage/Users/currentUser/Docs"));
    EXPECT_FALSE(IsPathInSyncFolder("/storage/Users/currentUser/Docs", "/storage/Users/currentUser/Docs2/file.txt"));
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
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "", TEST_USER_ID, path), E_INVALID_ARG);

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath absolute relative path";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "/file.txt", TEST_USER_ID, path), E_INVALID_ARG);

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath path ends with slash";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir/", TEST_USER_ID, path), E_INVALID_ARG);

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath rejects dot segment";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir/./file.txt", TEST_USER_ID, path), E_INVALID_ARG);

    GTEST_LOG_(INFO) << "[BRANCH] BuildCreatePlaceholderPath rejects dot dot segment";
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir/../file.txt", TEST_USER_ID, path), E_INVALID_ARG);

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
    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, TEST_USER_ID, path), E_OK);
    EXPECT_EQ(path.parentMntPath, TEST_SYNC_FOLDER_MNT);
    EXPECT_EQ(path.fileName, "placeholder.txt");

    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir/placeholder.txt", TEST_USER_ID, path), E_OK);
    EXPECT_EQ(path.parentMntPath, TEST_SYNC_FOLDER_MNT + "/dir");
    EXPECT_EQ(path.fileName, "placeholder.txt");

    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER, "dir//placeholder.txt", TEST_USER_ID, path), E_OK);
    EXPECT_EQ(path.parentMntPath, TEST_SYNC_FOLDER_MNT + "/dir/");
    EXPECT_EQ(path.fileName, "placeholder.txt");

    EXPECT_EQ(BuildCreatePlaceholderPath(TEST_SYNC_FOLDER + "/", TEST_RELATIVE_PATH, TEST_USER_ID, path), E_OK);
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
    EXPECT_EQ(BuildCreatePlaceholderPath("/storage/Users/currentUser/mockMntFailed", "file.txt", TEST_USER_ID, path),
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
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_ACCES);
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
 * @tc.desc: Verify CreatePlaceholderFileAt sets placeholder attributes in userspace
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest007 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt userspace attributes success";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;
    info.logicalSize = 4096;
    info.atimeMs = 1234;
    info.mtimeMs = 5678;
    bool xattrChecked = false;
    bool timesChecked = false;

    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, ftruncate(11, 4096)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([&xattrChecked](int, const char *, const void *value, size_t, int) {
            xattrChecked = value != nullptr && *static_cast<const uint8_t *>(value) == PLACEHOLDER_TEST_VALUE_LOCAL;
            return 0;
        }));
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Invoke([&timesChecked](int, const struct timespec *times) {
        timesChecked = times != nullptr && times[0].tv_sec == 1 && times[0].tv_nsec == 234000000 &&
                       times[1].tv_sec == 5 && times[1].tv_nsec == 678000000;
        return 0;
    }));
    EXPECT_CALL(*insMock_, Ioctl(_, _, _)).Times(0);
    EXPECT_CALL(*insMock_, UnlinkAt(_, _, _)).Times(0);
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_OK);
    EXPECT_TRUE(xattrChecked);
    EXPECT_TRUE(timesChecked);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest007 end";
}

/**
 * @tc.name: CreatePlaceholderBranchTest008
 * @tc.desc: Verify CreatePlaceholderFileAt rolls back when setting size fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderBranchTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest008 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt ftruncate failed rollback";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;

    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Invoke([](int, off_t) {
        errno = ENOSPC;
        return -1;
    }));
    EXPECT_CALL(*insMock_, fsetxattr(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*insMock_, futimens(_, _)).Times(0);
    EXPECT_CALL(*insMock_, UnlinkAt(10, StrEq("placeholder.txt"), 0)).WillOnce(Return(0));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_NO_SPACE_LEFT);
    GTEST_LOG_(INFO) << "CreatePlaceholderBranchTest008 end";
}

/**
 * @tc.name: CreatePlaceholderAttributeTest001
 * @tc.desc: Verify CreatePlaceholderFileAt rolls back when setting xattr fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderAttributeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderAttributeTest001 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt fsetxattr failed rollback";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;

    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Invoke([](int, const char *, const void *, size_t, int) {
            errno = EOPNOTSUPP;
            return -1;
        }));
    EXPECT_CALL(*insMock_, futimens(_, _)).Times(0);
    EXPECT_CALL(*insMock_, UnlinkAt(10, StrEq("placeholder.txt"), 0)).WillOnce(Return(0));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_NOT_SUPPORTED);
    GTEST_LOG_(INFO) << "CreatePlaceholderAttributeTest001 end";
}

/**
 * @tc.name: CreatePlaceholderAttributeTest002
 * @tc.desc: Verify CreatePlaceholderFileAt rolls back when setting times fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderAttributeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderAttributeTest002 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt futimens failed rollback";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;

    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Invoke([](int, const struct timespec *) {
        errno = EACCES;
        return -1;
    }));
    EXPECT_CALL(*insMock_, UnlinkAt(10, StrEq("placeholder.txt"), 0)).WillOnce(Return(0));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_ACCES);
    GTEST_LOG_(INFO) << "CreatePlaceholderAttributeTest002 end";
}

/**
 * @tc.name: CreatePlaceholderAttributeTest003
 * @tc.desc: Verify the original attribute error is returned when rollback fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderAttributeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderAttributeTest003 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileAt rollback unlink failed";
    CreatePlaceholderPath path;
    path.parentMntPath = TEST_SYNC_FOLDER_MNT;
    path.fileName = "placeholder.txt";
    PlaceholderInfo info;

    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Invoke([](int, off_t) {
        errno = EIO;
        return -1;
    }));
    EXPECT_CALL(*insMock_, UnlinkAt(10, StrEq("placeholder.txt"), 0)).WillOnce(Invoke([](int, const char *, int) {
        errno = EACCES;
        return -1;
    }));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_TRY_AGAIN);
    GTEST_LOG_(INFO) << "CreatePlaceholderAttributeTest003 end";
}

/**
 * @tc.name: SetPlaceholderFileAttributesBranchTest001
 * @tc.desc: Verify SetPlaceholderFileAttributes returns success when futimens succeeds
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetPlaceholderFileAttributesBranchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetPlaceholderFileAttributesBranchTest001 start";
    PlaceholderInfo info;
    info.logicalSize = 4096;
    info.atimeMs = 1234;
    info.mtimeMs = 5678;

    EXPECT_CALL(*insMock_, ftruncate(11, 4096)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Return(0));
    EXPECT_EQ(SetPlaceholderFileAttributes(11, info), E_OK);
    GTEST_LOG_(INFO) << "SetPlaceholderFileAttributesBranchTest001 end";
}

/**
 * @tc.name: SetPlaceholderFileAttributesBranchTest002
 * @tc.desc: Verify SetPlaceholderFileAttributes returns errno when futimens fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetPlaceholderFileAttributesBranchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetPlaceholderFileAttributesBranchTest002 start";
    PlaceholderInfo info;

    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Invoke([](int, const struct timespec *) {
        errno = EACCES;
        return -1;
    }));
    EXPECT_EQ(SetPlaceholderFileAttributes(11, info), EACCES);
    GTEST_LOG_(INFO) << "SetPlaceholderFileAttributesBranchTest002 end";
}

/**
 * @tc.name: CreatePlaceholderFileAtAttributesBranchTest001
 * @tc.desc: Verify CreatePlaceholderFileAt succeeds when setting attributes succeeds
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileAtAttributesBranchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileAtAttributesBranchTest001 start";
    CreatePlaceholderPath path = {TEST_SYNC_FOLDER_MNT, TEST_RELATIVE_PATH};
    PlaceholderInfo info;

    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, UnlinkAt(_, _, _)).Times(0);
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_OK);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileAtAttributesBranchTest001 end";
}

/**
 * @tc.name: CreatePlaceholderFileAtAttributesBranchTest002
 * @tc.desc: Verify CreatePlaceholderFileAt rolls back when setting attributes fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileAtAttributesBranchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileAtAttributesBranchTest002 start";
    CreatePlaceholderPath path = {TEST_SYNC_FOLDER_MNT, TEST_RELATIVE_PATH};
    PlaceholderInfo info;

    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Invoke([](int, const struct timespec *) {
        errno = EACCES;
        return -1;
    }));
    EXPECT_CALL(*insMock_, UnlinkAt(10, StrEq(TEST_RELATIVE_PATH), 0)).WillOnce(Return(0));
    EXPECT_EQ(CreatePlaceholderFileAt(path, info), E_ACCES);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileAtAttributesBranchTest002 end";
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
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EFBIG), E_FILE_TOO_LARGE);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError invalid argument errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EINVAL), E_INVALID_ARG);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EISDIR), E_INVALID_ARG);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(ELOOP), E_INVALID_ARG);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError name too long errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(ENAMETOOLONG), E_NAME_TOO_LONG);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError missing sync folder errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(ENOENT), E_SYNC_FOLDER_PATH_NOT_EXIST);
    GTEST_LOG_(INFO) << "[BRANCH] ConvertErrnoToCloudDiskError permission denied errno";
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EACCES), E_ACCES);
    EXPECT_EQ(ConvertErrnoToCloudDiskError(EPERM), E_PERM);
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
    EXPECT_EQ(NormalizeCreatePlaceholderError(E_NAME_TOO_LONG), E_NAME_TOO_LONG);
    EXPECT_EQ(NormalizeCreatePlaceholderError(E_FILE_TOO_LARGE), E_FILE_TOO_LARGE);
    EXPECT_EQ(NormalizeCreatePlaceholderError(E_PERM), E_PERM);
    EXPECT_EQ(NormalizeCreatePlaceholderError(E_ACCES), E_ACCES);
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
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
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
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
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
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_EQ(service.CreatePlaceholderFileInner(syncFolder, "file.txt", info), E_SYNC_FOLDER_PATH_NOT_EXIST);
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
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(-1));
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info), E_FILE_ALREADY_EXISTS);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest006 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest008
 * @tc.desc: Verify CreatePlaceholderFileInner keeps file name too long errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest008 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner file name too long";
    RegisterPlaceholderSyncFolder();
    CloudDiskService service;
    PlaceholderInfo info;

    Assistant::mockErrno = ENAMETOOLONG;
    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(TEST_PARENT_FD));
    EXPECT_CALL(*insMock_, OpenAt(TEST_PARENT_FD, _, _, _)).WillOnce(Return(MOCK_SYSCALL_FAILED));
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info), E_NAME_TOO_LONG);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest008 end";
}

/**
 * @tc.name: CreatePlaceholderFileInnerBranchTest007
 * @tc.desc: Verify CreatePlaceholderFileInner returns success after setting placeholder attributes
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CreatePlaceholderFileInnerBranchTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest007 start";
    GTEST_LOG_(INFO) << "[BRANCH] CreatePlaceholderFileInner userspace attributes success";
    RegisterPlaceholderSyncFolder();
    CloudDiskService service;
    PlaceholderInfo info;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, Open(_, _, _)).WillOnce(Return(10));
    EXPECT_CALL(*insMock_, OpenAt(10, _, _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, Ioctl(_, _, _)).Times(0);
    EXPECT_CALL(*insMock_, Unlink(_)).Times(0);
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info), E_OK);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileInnerBranchTest007 end";
}

/**
 * @tc.name: PlaceholderCustomInfoAttributesTest001
 * @tc.desc: Verify non-empty custom information is persisted as raw xattr bytes
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderCustomInfoAttributesTest001, TestSize.Level1)
{
    PlaceholderInfo info;
    PlaceholderCustomInfo customInfo;
    customInfo.data = {0x00, 0x7F, 0xFF};

    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_CUSTOM_INFO_XATTR), _, customInfo.data.size(), 0))
        .WillOnce(Invoke([&customInfo](int, const char *, const void *value, size_t size, int) {
            EXPECT_EQ(std::memcmp(value, customInfo.data.data(), size), 0);
            return 0;
        }));
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Return(0));

    EXPECT_EQ(SetPlaceholderFileAttributes(11, info, nullptr, customInfo), E_OK);
}

/**
 * @tc.name: PlaceholderCustomInfoAttributesTest002
 * @tc.desc: Verify empty custom information preserves the existing xattr during update
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderCustomInfoAttributesTest002, TestSize.Level1)
{
    PlaceholderInfo info;
    PlaceholderCustomInfo customInfo;
    uint8_t oldPlaceholderState = PLACEHOLDER_STATE_NONE;
    uint8_t rawState = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);

    EXPECT_CALL(*insMock_, ftruncate(11, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fgetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([rawState](int, const char *, void *value, size_t) {
            *static_cast<uint8_t *>(value) = rawState;
            return static_cast<ssize_t>(sizeof(uint8_t));
        }));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t), 0))
        .WillOnce(Return(0));
    EXPECT_CALL(*insMock_, fsetxattr(11, StrEq(CLOUD_DISK_CUSTOM_INFO_XATTR), _, _, _)).Times(0);
    EXPECT_CALL(*insMock_, futimens(11, _)).WillOnce(Return(0));

    EXPECT_EQ(SetPlaceholderFileAttributes(11, info, &oldPlaceholderState, customInfo), E_OK);
    EXPECT_EQ(oldPlaceholderState, PLACEHOLDER_STATE_UNHYDRATED);
}

/**
 * @tc.name: PlaceholderCustomInfoServiceValidationTest001
 * @tc.desc: Verify service entry points reject oversized custom information before side effects
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderCustomInfoServiceValidationTest001, TestSize.Level1)
{
    CloudDiskService service;
    PlaceholderInfo info;
    PlaceholderCustomInfo customInfo;
    customInfo.data.resize(PLACEHOLDER_CUSTOM_INFO_MAX_SIZE + 1);

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).Times(0);
    EXPECT_EQ(service.CreatePlaceholderFileInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info, customInfo),
              E_INVALID_ARG);
    EXPECT_EQ(service.UpdatePlaceholderInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, info, customInfo), E_INVALID_ARG);
}

/**
 * @tc.name: PlaceholderCustomInfoGetTest001
 * @tc.desc: Verify service reads custom information from a placeholder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderCustomInfoGetTest001, TestSize.Level1)
{
    RegisterPlaceholderSyncFolder();
    CloudDiskService service;
    PlaceholderCustomInfo customInfo;
    const std::vector<uint8_t> storedData = {1, 2, 3, 4};
    const std::string hmdfsPath = TEST_SYNC_FOLDER_MNT + "/" + TEST_RELATIVE_PATH;
    struct stat statInfo = {};
    statInfo.st_mode = S_IFREG;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, access(StrEq(hmdfsPath), F_OK)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, MockStat(StrEq(hmdfsPath), _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
    EXPECT_CALL(*insMock_, Open(StrEq(hmdfsPath), _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, fgetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](int, const char *, void *value, size_t) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
            return static_cast<ssize_t>(sizeof(uint8_t));
        }));
    EXPECT_CALL(*insMock_, fgetxattr(11, StrEq(CLOUD_DISK_CUSTOM_INFO_XATTR), IsNull(), 0))
        .WillOnce(Return(static_cast<ssize_t>(storedData.size())));
    EXPECT_CALL(*insMock_, fgetxattr(11, StrEq(CLOUD_DISK_CUSTOM_INFO_XATTR), _, storedData.size()))
        .WillOnce(Invoke([&storedData](int, const char *, void *value, size_t size) {
            if (memcpy_s(value, size, storedData.data(), storedData.size()) != EOK) {
                return static_cast<ssize_t>(MOCK_SYSCALL_FAILED);
            }
            return static_cast<ssize_t>(size);
        }));

    EXPECT_EQ(service.GetPlaceholderCustomInfoInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, customInfo), E_OK);
    EXPECT_EQ(customInfo.data, storedData);
}

/**
 * @tc.name: PlaceholderCustomInfoGetTest002
 * @tc.desc: Verify missing custom information and non-placeholder files are distinguished
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderCustomInfoGetTest002, TestSize.Level1)
{
    RegisterPlaceholderSyncFolder();
    CloudDiskService service;
    PlaceholderCustomInfo customInfo;
    const std::string hmdfsPath = TEST_SYNC_FOLDER_MNT + "/" + TEST_RELATIVE_PATH;
    struct stat statInfo = {};
    statInfo.st_mode = S_IFREG;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, access(StrEq(hmdfsPath), F_OK)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, MockStat(StrEq(hmdfsPath), _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
    EXPECT_CALL(*insMock_, Open(StrEq(hmdfsPath), _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, fgetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](int, const char *, void *value, size_t) {
            *static_cast<uint8_t *>(value) = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
            return static_cast<ssize_t>(sizeof(uint8_t));
        }));
    EXPECT_CALL(*insMock_, fgetxattr(11, StrEq(CLOUD_DISK_CUSTOM_INFO_XATTR), IsNull(), 0))
        .WillOnce(Invoke([](int, const char *, void *, size_t) {
            errno = ENODATA;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_EQ(service.GetPlaceholderCustomInfoInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, customInfo),
              E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND);

    Mock::VerifyAndClearExpectations(insMock_.get());
    Mock::VerifyAndClearExpectations(dfsuAccessToken_.get());
    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, access(StrEq(hmdfsPath), F_OK)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, MockStat(StrEq(hmdfsPath), _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
    EXPECT_CALL(*insMock_, Open(StrEq(hmdfsPath), _, _)).WillOnce(Return(11));
    EXPECT_CALL(*insMock_, fgetxattr(11, StrEq(CLOUD_DISK_FILE_SYNC_STATE_XATTR), _, sizeof(uint8_t)))
        .WillOnce(Invoke([](int, const char *, void *value, size_t) {
            *static_cast<uint8_t *>(value) = 0;
            return static_cast<ssize_t>(sizeof(uint8_t));
        }));
    EXPECT_EQ(service.GetPlaceholderCustomInfoInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, customInfo),
              E_NOT_A_PLACEHOLDER);
}

/**
 * @tc.name: PlaceholderCustomInfoGetTest003
 * @tc.desc: Verify nonexistent and unauthorized custom information queries are rejected
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderCustomInfoGetTest003, TestSize.Level1)
{
    RegisterPlaceholderSyncFolder();
    CloudDiskService service;
    PlaceholderCustomInfo customInfo;
    const std::string hmdfsPath = TEST_SYNC_FOLDER_MNT + "/" + TEST_RELATIVE_PATH;

    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, access(StrEq(hmdfsPath), F_OK)).WillOnce(Invoke([](const char *, int) {
        errno = ENOENT;
        return -1;
    }));
    EXPECT_EQ(service.GetPlaceholderCustomInfoInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, customInfo),
              E_FILE_NOT_EXIST);

    Mock::VerifyAndClearExpectations(insMock_.get());
    Mock::VerifyAndClearExpectations(dfsuAccessToken_.get());
    CloudDiskSyncFolder::GetInstance().ClearMap();
    RegisterPlaceholderSyncFolder("wrong.bundle");
    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(TEST_USER_ID));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_BUNDLE), Return(E_OK)));
    EXPECT_CALL(*insMock_, access(_, _)).Times(0);
    EXPECT_EQ(service.GetPlaceholderCustomInfoInner(TEST_SYNC_FOLDER, TEST_RELATIVE_PATH, customInfo),
              E_SYNC_FOLDER_PATH_UNAUTHORIZED);
}

/**
 * @tc.name: PlaceholderStateOnlyMarkTest001
 * @tc.desc: Verify marking preserves file data and low sync-state bits
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderStateOnlyMarkTest001, TestSize.Level1)
{
    PlaceholderStatePathContext context = {PLACEHOLDER_TEST_PATH, "", TEST_USER_ID, 1};
    const std::string parentPath = PLACEHOLDER_TEST_PATH.substr(0, PLACEHOLDER_TEST_PATH.rfind('/'));
    constexpr int32_t fileFd = 88;
    constexpr uint8_t syncState = static_cast<uint8_t>(SyncState::SYNCING);
    struct stat fileStat = {};
    fileStat.st_mode = S_IFREG;
    struct stat parentStat = {};
    parentStat.st_mode = S_IFDIR;
    parentStat.st_ino = 123;

    EXPECT_CALL(*insMock_, MockStat(StrEq(PLACEHOLDER_TEST_PATH), _))
        .WillOnce(DoAll(SetArgPointee<1>(fileStat), Return(0)));
    EXPECT_CALL(*insMock_, Open(StrEq(PLACEHOLDER_TEST_PATH), _, _)).WillOnce(Return(fileFd));
    EXPECT_CALL(*insMock_, fgetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .Times(2)
        .WillRepeatedly(Invoke([](int, const char *, void *value, size_t size) {
            static_cast<uint8_t *>(value)[0] = syncState;
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*insMock_, fsetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE, 0))
        .WillOnce(Invoke([](int, const char *, const void *value, size_t, int) {
            EXPECT_EQ(*static_cast<const uint8_t *>(value),
                      MakeFileSyncState(PLACEHOLDER_STATE_FULLY_HYDRATED, syncState));
            return 0;
        }));
    EXPECT_CALL(*insMock_, MockStat(StrEq(parentPath), _)).WillOnce(DoAll(SetArgPointee<1>(parentStat), Return(0)));
    EXPECT_CALL(*insMock_, ftruncate(_, _)).Times(0);
    EXPECT_CALL(*insMock_, fsetxattr(_, StrEq(CLOUD_DISK_CUSTOM_INFO_XATTR), _, _, _)).Times(0);

    EXPECT_EQ(ChangePlaceholderStateOnly(context, PlaceholderStateTransition::MARK), E_OK);
}

/**
 * @tc.name: PlaceholderStateOnlyMarkTest002
 * @tc.desc: Verify marking rejects an existing placeholder without writing state
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderStateOnlyMarkTest002, TestSize.Level1)
{
    PlaceholderStatePathContext context = {PLACEHOLDER_TEST_PATH, "", TEST_USER_ID, 1};
    constexpr int32_t fileFd = 89;
    ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_PATH, S_IFREG);
    EXPECT_CALL(*insMock_, Open(StrEq(PLACEHOLDER_TEST_PATH), _, _)).WillOnce(Return(fileFd));
    EXPECT_CALL(*insMock_, fgetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .WillOnce(Invoke([](int, const char *, void *value, size_t size) {
            static_cast<uint8_t *>(value)[0] = MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, 0);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*insMock_, fsetxattr(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*insMock_, ftruncate(_, _)).Times(0);

    EXPECT_EQ(ChangePlaceholderStateOnly(context, PlaceholderStateTransition::MARK), E_IS_A_PLACEHOLDER);
}

/**
 * @tc.name: PlaceholderStateOnlyUnmarkTest001
 * @tc.desc: Verify unmark rejects normal and incompletely hydrated files without writes
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderStateOnlyUnmarkTest001, TestSize.Level1)
{
    PlaceholderStatePathContext context = {PLACEHOLDER_TEST_PATH, "", TEST_USER_ID, 1};
    constexpr int32_t fileFd = 90;
    const std::vector<std::pair<uint8_t, int32_t>> cases = {
        {PLACEHOLDER_STATE_NONE, E_NOT_A_PLACEHOLDER},
        {PLACEHOLDER_STATE_UNHYDRATED, E_PLACEHOLDER_NOT_FULLY_HYDRATED},
        {PLACEHOLDER_STATE_PARTIALLY_HYDRATED, E_PLACEHOLDER_NOT_FULLY_HYDRATED},
    };

    for (const auto &item : cases) {
        uint8_t state = item.first;
        int32_t expected = item.second;
        ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_PATH, S_IFREG);
        EXPECT_CALL(*insMock_, Open(StrEq(PLACEHOLDER_TEST_PATH), _, _)).WillOnce(Return(fileFd));
        EXPECT_CALL(*insMock_, fgetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
            .WillOnce(Invoke([state](int, const char *, void *value, size_t size) {
                static_cast<uint8_t *>(value)[0] = MakeFileSyncState(state, 0);
                return static_cast<ssize_t>(size);
            }));
        EXPECT_EQ(ChangePlaceholderStateOnly(context, PlaceholderStateTransition::UNMARK), expected);
        Mock::VerifyAndClearExpectations(insMock_.get());
    }
}

/**
 * @tc.name: PlaceholderStateOnlyUnmarkTest002
 * @tc.desc: Verify unmark preserves data, metadata, custom information, and low sync-state bits
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderStateOnlyUnmarkTest002, TestSize.Level1)
{
    PlaceholderStatePathContext context = {PLACEHOLDER_TEST_PATH, "", TEST_USER_ID, 1};
    const std::string parentPath = PLACEHOLDER_TEST_PATH.substr(0, PLACEHOLDER_TEST_PATH.rfind('/'));
    constexpr int32_t fileFd = 91;
    constexpr uint8_t syncState = static_cast<uint8_t>(SyncState::SYNC_CONFLICTED);
    struct stat fileStat = {};
    fileStat.st_mode = S_IFREG;
    struct stat parentStat = {};
    parentStat.st_mode = S_IFDIR;
    parentStat.st_ino = 456;

    EXPECT_CALL(*insMock_, MockStat(StrEq(PLACEHOLDER_TEST_PATH), _))
        .WillOnce(DoAll(SetArgPointee<1>(fileStat), Return(0)));
    EXPECT_CALL(*insMock_, Open(StrEq(PLACEHOLDER_TEST_PATH), _, _)).WillOnce(Return(fileFd));
    EXPECT_CALL(*insMock_, fgetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .Times(2)
        .WillRepeatedly(Invoke([](int, const char *, void *value, size_t size) {
            static_cast<uint8_t *>(value)[0] = MakeFileSyncState(PLACEHOLDER_STATE_FULLY_HYDRATED, syncState);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*insMock_, fsetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE, 0))
        .WillOnce(Invoke([](int, const char *, const void *value, size_t, int) {
            EXPECT_EQ(*static_cast<const uint8_t *>(value), MakeFileSyncState(PLACEHOLDER_STATE_NONE, syncState));
            return 0;
        }));
    EXPECT_CALL(*insMock_, MockStat(StrEq(parentPath), _)).WillOnce(DoAll(SetArgPointee<1>(parentStat), Return(0)));
    EXPECT_CALL(*insMock_, ftruncate(_, _)).Times(0);
    EXPECT_CALL(*insMock_, fsetxattr(_, StrEq(CLOUD_DISK_CUSTOM_INFO_XATTR), _, _, _)).Times(0);

    EXPECT_EQ(ChangePlaceholderStateOnly(context, PlaceholderStateTransition::UNMARK), E_OK);
}

/**
 * @tc.name: PlaceholderStateOnlyServiceValidationTest001
 * @tc.desc: Verify service entry validation and sync-folder ownership checks
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, PlaceholderStateOnlyServiceValidationTest001, TestSize.Level1)
{
    CloudDiskService service;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_EQ(service.MarkFileAsPlaceholderInner("", PLACEHOLDER_TEST_PATH), E_INVALID_ARG);
    EXPECT_EQ(service.UnmarkPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, ""), E_INVALID_ARG);

    AddPlaceholderSyncFolder(PLACEHOLDER_TEST_PHYSICAL_SYNC_FOLDER, "wrong.bundle");
    ExpectPlaceholderCaller(dfsuAccessToken_);
    EXPECT_EQ(service.MarkFileAsPlaceholderInner(PLACEHOLDER_TEST_SYNC_FOLDER, "file.txt"),
              E_SYNC_FOLDER_PATH_UNAUTHORIZED);
    ExpectPlaceholderCaller(dfsuAccessToken_);
    EXPECT_EQ(service.UnmarkPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, "file.txt"),
              E_SYNC_FOLDER_PATH_UNAUTHORIZED);
#else
    EXPECT_EQ(service.MarkFileAsPlaceholderInner("", PLACEHOLDER_TEST_PATH), E_NOT_SUPPORTED);
    EXPECT_EQ(service.UnmarkPlaceholderFileInner(PLACEHOLDER_TEST_SYNC_FOLDER, ""), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: DehydrateStatePreconditionTest001
 * @tc.desc: Verify normal, unhydrated, and partially hydrated files follow the dehydration state contract.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, DehydrateStatePreconditionTest001, TestSize.Level1)
{
    PlaceholderStatePathContext context = {PLACEHOLDER_TEST_PATH, "", TEST_USER_ID, 1,
                                           PLACEHOLDER_TEST_BUNDLE_NAME, PLACEHOLDER_TEST_SYNC_FOLDER};
    constexpr int32_t fileFd = 92;
    const std::vector<std::pair<uint8_t, int32_t>> cases = {
        {PLACEHOLDER_STATE_NONE, E_NOT_A_PLACEHOLDER},
        {PLACEHOLDER_STATE_UNHYDRATED, E_OK},
        {PLACEHOLDER_STATE_PARTIALLY_HYDRATED, E_PLACEHOLDER_NOT_FULLY_HYDRATED},
    };
    for (const auto &item : cases) {
        ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_PATH, S_IFREG);
        EXPECT_CALL(*insMock_, Open(StrEq(PLACEHOLDER_TEST_PATH), _, _)).WillOnce(Return(fileFd));
        EXPECT_CALL(*insMock_, fgetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
            .WillOnce(Invoke([state = item.first](int, const char *, void *value, size_t size) {
                static_cast<uint8_t *>(value)[0] = MakeFileSyncState(state, 0);
                return static_cast<ssize_t>(size);
            }));
        EXPECT_CALL(*insMock_, ftruncate(_, _)).Times(0);
        EXPECT_EQ(DehydratePlaceholderFile(context, PLACEHOLDER_TEST_PATH), item.second);
        Mock::VerifyAndClearExpectations(insMock_.get());
    }
}

/**
 * @tc.name: DehydrateAuthorizationTest001
 * @tc.desc: Verify denied authorization preserves file data and placeholder state.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, DehydrateAuthorizationTest001, TestSize.Level1)
{
    PlaceholderStatePathContext context = {PLACEHOLDER_TEST_PATH, "", TEST_USER_ID, 1,
                                           PLACEHOLDER_TEST_BUNDLE_NAME, PLACEHOLDER_TEST_SYNC_FOLDER};
    auto callback = sptr(new DehydrateCallbackTableStub(false));
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(
        context.bundleName, context.syncFolderIndex, callback), E_OK);
    constexpr int32_t fileFd = 93;
    ExpectPlaceholderPathType(insMock_, PLACEHOLDER_TEST_PATH, S_IFREG);
    EXPECT_CALL(*insMock_, Open(StrEq(PLACEHOLDER_TEST_PATH), _, _)).WillOnce(Return(fileFd));
    EXPECT_CALL(*insMock_, fgetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .WillOnce(Invoke([](int, const char *, void *value, size_t size) {
            static_cast<uint8_t *>(value)[0] = MakeFileSyncState(PLACEHOLDER_STATE_FULLY_HYDRATED, 0);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*insMock_, ftruncate(_, _)).Times(0);
    EXPECT_CALL(*insMock_, fsetxattr(_, _, _, _, _)).Times(0);

    EXPECT_EQ(DehydratePlaceholderFile(context, "file.txt"), E_DEHYDRATE_DENIED);
    EXPECT_EQ(callback->callbackCount_, 1U);
}

/**
 * @tc.name: DehydrateSuccessTest001
 * @tc.desc: Verify authorized dehydration rebuilds a sparse file, preserves low bits, and changes state 3 to 1.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, DehydrateSuccessTest001, TestSize.Level1)
{
    PlaceholderStatePathContext context = {PLACEHOLDER_TEST_PATH, "", TEST_USER_ID, 1,
                                           PLACEHOLDER_TEST_BUNDLE_NAME, PLACEHOLDER_TEST_SYNC_FOLDER};
    auto callback = sptr(new DehydrateCallbackTableStub(true));
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(
        context.bundleName, context.syncFolderIndex, callback), E_OK);
    constexpr int32_t fileFd = 94;
    constexpr off_t logicalSize = 4096;
    constexpr uint8_t syncState = static_cast<uint8_t>(SyncState::SYNCING);
    struct stat fileStat = {};
    fileStat.st_mode = S_IFREG;
    fileStat.st_size = logicalSize;

    EXPECT_CALL(*insMock_, MockStat(StrEq(PLACEHOLDER_TEST_PATH), _))
        .WillOnce(DoAll(SetArgPointee<1>(fileStat), Return(0)));
    EXPECT_CALL(*insMock_, Open(StrEq(PLACEHOLDER_TEST_PATH), _, _)).WillOnce(Return(fileFd));
    EXPECT_CALL(*insMock_, fgetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .Times(2)
        .WillRepeatedly(Invoke([](int, const char *, void *value, size_t size) {
            static_cast<uint8_t *>(value)[0] =
                MakeFileSyncState(PLACEHOLDER_STATE_FULLY_HYDRATED, syncState);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*insMock_, fstat(fileFd, _)).WillOnce(DoAll(SetArgPointee<1>(fileStat), Return(0)));
    {
        InSequence sequence;
        EXPECT_CALL(*insMock_, ftruncate(fileFd, 0)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, ftruncate(fileFd, logicalSize)).WillOnce(Return(0));
    }
    EXPECT_CALL(*insMock_, fsetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE, 0))
        .WillOnce(Invoke([](int, const char *, const void *value, size_t, int) {
            EXPECT_EQ(*static_cast<const uint8_t *>(value),
                      MakeFileSyncState(PLACEHOLDER_STATE_UNHYDRATED, syncState));
            return 0;
        }));
    EXPECT_CALL(*insMock_, fsetxattr(_, StrEq(CLOUD_DISK_CUSTOM_INFO_XATTR), _, _, _)).Times(0);

    EXPECT_EQ(DehydratePlaceholderFile(context, "file.txt"), E_OK);
}

/**
 * @tc.name: DehydrateExtendRetryTest001
 * @tc.desc: Verify sparse-file re-extension retries and returns try-again without changing state.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, DehydrateExtendRetryTest001, TestSize.Level1)
{
    PlaceholderStatePathContext context = {PLACEHOLDER_TEST_PATH, "", TEST_USER_ID, 1,
                                           PLACEHOLDER_TEST_BUNDLE_NAME, PLACEHOLDER_TEST_SYNC_FOLDER};
    auto callback = sptr(new DehydrateCallbackTableStub(true));
    ASSERT_EQ(PlaceholderCallbackManager::GetInstance().RegisterCallbackTable(
        context.bundleName, context.syncFolderIndex, callback), E_OK);
    constexpr int32_t fileFd = 95;
    constexpr off_t logicalSize = 4096;
    struct stat fileStat = {};
    fileStat.st_mode = S_IFREG;
    fileStat.st_size = logicalSize;

    EXPECT_CALL(*insMock_, MockStat(StrEq(PLACEHOLDER_TEST_PATH), _))
        .WillOnce(DoAll(SetArgPointee<1>(fileStat), Return(0)));
    EXPECT_CALL(*insMock_, Open(StrEq(PLACEHOLDER_TEST_PATH), _, _)).WillOnce(Return(fileFd));
    EXPECT_CALL(*insMock_, fgetxattr(fileFd, StrEq(PLACEHOLDER_TEST_XATTR), _, PLACEHOLDER_XATTR_VALUE_SIZE))
        .WillOnce(Invoke([](int, const char *, void *value, size_t size) {
            static_cast<uint8_t *>(value)[0] = MakeFileSyncState(PLACEHOLDER_STATE_FULLY_HYDRATED, 0);
            return static_cast<ssize_t>(size);
        }));
    EXPECT_CALL(*insMock_, fstat(fileFd, _)).WillOnce(DoAll(SetArgPointee<1>(fileStat), Return(0)));
    EXPECT_CALL(*insMock_, ftruncate(fileFd, 0)).WillOnce(Return(0));
    EXPECT_CALL(*insMock_, ftruncate(fileFd, logicalSize)).Times(DEHYDRATE_EXTEND_MAX_ATTEMPTS)
        .WillRepeatedly(Invoke([](int, off_t) {
            errno = EIO;
            return -1;
        }));
    EXPECT_CALL(*insMock_, fsetxattr(_, _, _, _, _)).Times(0);

    EXPECT_EQ(DehydratePlaceholderFile(context, "file.txt"), E_TRY_AGAIN);
}

/**
 * @tc.name: DehydrateFileMutexTest001
 * @tc.desc: Verify concurrent dehydration of the same file uses the same serialization lock.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, DehydrateFileMutexTest001, TestSize.Level1)
{
    std::mutex &mutex = GetDehydrateFileMutex(PLACEHOLDER_TEST_PATH);
    std::atomic<bool> started{false};
    std::atomic<bool> acquired{false};
    mutex.lock();
    std::thread contender([&mutex, &started, &acquired]() {
        started.store(true);
        std::lock_guard<std::mutex> lock(mutex);
        acquired.store(true);
    });
    while (!started.load()) {
        std::this_thread::yield();
    }
    EXPECT_FALSE(acquired.load());
    mutex.unlock();
    contender.join();
    EXPECT_TRUE(acquired.load());
}

/**
 * @tc.name: DehydrateServiceValidationTest001
 * @tc.desc: Validate arguments and folder ownership without requiring cloud-disk permission.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, DehydrateServiceValidationTest001, TestSize.Level1)
{
    CloudDiskService service;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).Times(0);
    EXPECT_EQ(service.DehydrateInner("", "file.txt"), E_INVALID_ARG);
    AddPlaceholderSyncFolder(PLACEHOLDER_TEST_PHYSICAL_SYNC_FOLDER, TEST_BUNDLE);
    ExpectPlaceholderCaller(dfsuAccessToken_);
    EXPECT_EQ(service.DehydrateInner(PLACEHOLDER_TEST_SYNC_FOLDER, "file.txt"), E_SYNC_FOLDER_PATH_UNAUTHORIZED);
#else
    EXPECT_EQ(service.DehydrateInner("", "file.txt"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: StartHydrationInner_001
 * @tc.desc: Validate hydration arguments and folder ownership without requiring cloud-disk permission.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, StartHydrationInner_001, TestSize.Level2)
{
    CloudDiskService service;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).Times(0);
    EXPECT_EQ(service.StartHydrationInner("", "file.txt", CLOUD_DISK_HYDRATE_PRIORITY_NORMAL), E_INVALID_ARG);
    EXPECT_EQ(service.StartHydrationInner(PLACEHOLDER_TEST_SYNC_FOLDER, "", CLOUD_DISK_HYDRATE_PRIORITY_NORMAL),
              E_INVALID_ARG);
    EXPECT_EQ(service.StartHydrationInner(PLACEHOLDER_TEST_SYNC_FOLDER, "file.txt", -1), E_INVALID_ARG);
    AddPlaceholderSyncFolder(PLACEHOLDER_TEST_PHYSICAL_SYNC_FOLDER, TEST_BUNDLE);
    ExpectPlaceholderCaller(dfsuAccessToken_);
    EXPECT_EQ(service.StartHydrationInner(PLACEHOLDER_TEST_SYNC_FOLDER, "file.txt", CLOUD_DISK_HYDRATE_PRIORITY_NORMAL),
              E_SYNC_FOLDER_PATH_UNAUTHORIZED);
#else
    EXPECT_EQ(service.StartHydrationInner("", "file.txt", CLOUD_DISK_HYDRATE_PRIORITY_NORMAL), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: CancelHydrationInner_001
 * @tc.desc: Validate cancellation arguments and folder ownership without requiring cloud-disk permission.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CancelHydrationInner_001, TestSize.Level2)
{
    CloudDiskService service;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).Times(0);
    EXPECT_EQ(service.CancelHydrationInner("", "file.txt"), E_INVALID_ARG);
    AddPlaceholderSyncFolder(PLACEHOLDER_TEST_PHYSICAL_SYNC_FOLDER, TEST_BUNDLE);
    ExpectPlaceholderCaller(dfsuAccessToken_);
    EXPECT_EQ(service.CancelHydrationInner(PLACEHOLDER_TEST_SYNC_FOLDER, "file.txt"), E_SYNC_FOLDER_PATH_UNAUTHORIZED);
#else
    EXPECT_EQ(service.CancelHydrationInner("", "file.txt"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: CancelHydrationInner_002
 * @tc.desc: An owner can reach task cancellation without cloud-disk permission.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CancelHydrationInner_002, TestSize.Level2)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    AddPlaceholderSyncFolder();
    CloudDiskService service;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).Times(0);
    ExpectPlaceholderCaller(dfsuAccessToken_);
    std::string path = PLACEHOLDER_TEST_MNT_SYNC_FOLDER + "/file.txt";
    EXPECT_CALL(*insMock_, access(StrEq(path), F_OK)).WillOnce(Return(0));
    EXPECT_EQ(service.CancelHydrationInner(PLACEHOLDER_TEST_SYNC_FOLDER, "file.txt"), E_NO_HYDRATION_IN_PROGRESS);
#else
    CloudDiskService service;
    EXPECT_EQ(service.CancelHydrationInner(PLACEHOLDER_TEST_SYNC_FOLDER, "file.txt"), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: ExecuteInner_001
 * @tc.desc: Reject empty request keys and foreign folders without requiring cloud-disk permission.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ExecuteInner_001, TestSize.Level2)
{
    CloudDiskService service;
    CallbackExecuteRequest request;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).Times(0);
    EXPECT_EQ(service.ExecuteInner(request), E_INVALID_ARG);
    request.reqKey = {1};
    request.syncFolder = PLACEHOLDER_TEST_SYNC_FOLDER;
    AddPlaceholderSyncFolder(PLACEHOLDER_TEST_PHYSICAL_SYNC_FOLDER, TEST_BUNDLE);
    ExpectPlaceholderCaller(dfsuAccessToken_);
    EXPECT_EQ(service.ExecuteInner(request), E_CALLBACK_NOT_REGISTERED);
#else
    EXPECT_EQ(service.ExecuteInner(request), E_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: ExecuteInner_002
 * @tc.desc: Hide sync folder ownership and registration failures behind CALLBACK_NOT_REGISTERED.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ExecuteInner_002, TestSize.Level2)
{
    CloudDiskService service;
    CallbackExecuteRequest request;
    request.reqKey = {1};
    request.syncFolder = PLACEHOLDER_TEST_SYNC_FOLDER;
    request.filePath = "file.txt";
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).Times(0);
    ExpectPlaceholderCaller(dfsuAccessToken_);
    EXPECT_EQ(service.ExecuteInner(request), E_CALLBACK_NOT_REGISTERED);
}

/**
 * @tc.name: ExecuteInner_003
 * @tc.desc: A registered sync folder still requires a registered callback table for Execute.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ExecuteInner_003, TestSize.Level2)
{
    AddPlaceholderSyncFolder();
    CloudDiskService service;
    CallbackExecuteRequest request;
    request.reqKey = {1};
    request.syncFolder = PLACEHOLDER_TEST_SYNC_FOLDER;
    request.filePath = "file.txt";
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).Times(0);
    ExpectPlaceholderCaller(dfsuAccessToken_);
    EXPECT_EQ(service.ExecuteInner(request), E_CALLBACK_NOT_REGISTERED);
}

/**
 * @tc.name: DehydratePlaceholderFile_001
 * @tc.desc: An active hydration task blocks dehydration before opening or truncating the file.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, DehydratePlaceholderFile_001, TestSize.Level2)
{
    PlaceholderStatePathContext context;
    context.syncFolder = PLACEHOLDER_TEST_SYNC_FOLDER;
    context.hmdfsPath = PLACEHOLDER_TEST_PATH;
    context.bundleName = PLACEHOLDER_TEST_BUNDLE_NAME;
    context.syncFolderIndex = 1;
    UniqueFd outputFd(dup(STDOUT_FILENO));
    ASSERT_GE(outputFd.Get(), 0);
    PlaceholderTaskManager::RequestKey reqKey;
    ASSERT_EQ(PlaceholderTaskManager::GetInstance().CreateHydrateTask(
        context.syncFolder, "file.txt", context.bundleName,
        context.syncFolderIndex, CLOUD_DISK_HYDRATE_PRIORITY_NORMAL, std::move(outputFd), reqKey), E_OK);
    EXPECT_CALL(*insMock_, Open(_, _, _)).Times(0);
    EXPECT_EQ(DehydratePlaceholderFile(context, "file.txt"), E_HYDRATE_IN_PROGRESS);
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
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(info.logicalSize), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(info.atimeMs), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(info.mtimeMs), Return(true)));

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
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(logicalSize), Return(true)));
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
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(logicalSize), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(atimeMs), Return(true)));
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
/**
 * @tc.name: SystemAccessorPermission_001
 * @tc.desc: All four system API methods reject missing permission before other work.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SystemAccessorPermission_001, TestSize.Level2)
{
    CloudDiskService service;
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(PERM_CLOUD_DISK_SERVICE))
        .Times(4).WillRepeatedly(Return(false));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).Times(0);
    EXPECT_EQ(service.StartHydrationByPathInner("/path", 0, 2), E_PERMISSION_DENIED);
    EXPECT_EQ(service.DehydrateFileByPathInner("/path"), E_PERMISSION_DENIED);
    EXPECT_EQ(service.RegisterProgressCallbackInner(nullptr), E_PERMISSION_DENIED);
    EXPECT_EQ(service.UnregisterProgressCallbackInner(), E_PERMISSION_DENIED);
#else
    EXPECT_EQ(service.StartHydrationByPathInner("/path", 0, 2), E_NOT_SUPPORTED);
    EXPECT_EQ(service.DehydrateFileByPathInner("/path"), E_NOT_SUPPORTED);
    EXPECT_EQ(service.RegisterProgressCallbackInner(nullptr), E_NOT_SUPPORTED);
    EXPECT_EQ(service.UnregisterProgressCallbackInner(), E_NOT_SUPPORTED);
#endif
}
/**
 * @tc.name: SystemAccessorPermission_002
 * @tc.desc: An ordinary permitted app cannot use system-only operations.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SystemAccessorPermission_002, TestSize.Level2)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    CloudDiskService service;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(PERM_CLOUD_DISK_SERVICE))
        .Times(4).WillRepeatedly(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).Times(4).WillRepeatedly(Return(false));
    EXPECT_EQ(service.StartHydrationByPathInner("/path", 0, 2), E_PERMISSION_SYSTEM);
    EXPECT_EQ(service.DehydrateFileByPathInner("/path"), E_PERMISSION_SYSTEM);
    EXPECT_EQ(service.RegisterProgressCallbackInner(nullptr), E_PERMISSION_SYSTEM);
    EXPECT_EQ(service.UnregisterProgressCallbackInner(), E_PERMISSION_SYSTEM);
#endif
}
/**
 * @tc.name: SystemAccessorPath_001
 * @tc.desc: Reject non-sandbox and traversal paths and invalid explicit enums.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SystemAccessorPath_001, TestSize.Level2)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    CloudDiskService service;
    EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(PERM_CLOUD_DISK_SERVICE))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillRepeatedly(Return(true));
    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).Times(0);
    for (const auto &path : {"", "relative/file", "/data/service/el2/101/file",
        "/storage/Users/currentUser/../other/file", "/storage/Users/currentUser/root/."}) {
        EXPECT_EQ(service.StartHydrationByPathInner(path, 0, 2), E_INVALID_ARG);
        EXPECT_EQ(service.DehydrateFileByPathInner(path), E_INVALID_ARG);
    }
    EXPECT_EQ(service.StartHydrationByPathInner("/path", 2, 2), E_INVALID_ARG);
    EXPECT_EQ(service.StartHydrationByPathInner("/path", 0, 3), E_INVALID_ARG);
#endif
}
/**
 * @tc.name: SystemAccessorPath_002
 * @tc.desc: Resolve the current user's longest registered root without requiring file-manager/provider bundle equality.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SystemAccessorPath_002, TestSize.Level2)
{
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    auto &folders = CloudDiskSyncFolder::GetInstance();
    folders.ClearMap();
    const std::string physical = "/data/service/el2/100/hmdfs/account/files/Docs/sync";
    folders.AddSyncFolder(1, {"provider.parent", physical});
    folders.AddSyncFolder(2, {"provider.child", physical + "/child"});
    folders.AddSyncFolder(3, {"provider.other.user",
        "/data/service/el2/101/hmdfs/account/files/Docs/sync/child"});
    EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillRepeatedly(Return(100));
    EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).Times(0);
    EXPECT_CALL(*insMock_, access(_, F_OK)).WillRepeatedly(Return(0));
    PlaceholderStatePathContext context;
    std::string relative;
    const std::string path = "/storage/Users/currentUser/sync/child/file.txt";
    ASSERT_EQ(ResolveSystemAccessorPath(path, context, relative), E_OK);
    EXPECT_EQ(context.userId, 100);
    EXPECT_EQ(context.syncFolderIndex, 2u);
    EXPECT_EQ(context.bundleName, "provider.child");
    EXPECT_EQ(context.syncFolder, "/storage/Users/currentUser/sync/child");
    EXPECT_EQ(context.absolutePath, path);
    EXPECT_EQ(relative, "file.txt");
    folders.ClearMap();
    EXPECT_EQ(ResolveSystemAccessorPath(path, context, relative), E_CALLBACK_NOT_REGISTERED);
#endif
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
