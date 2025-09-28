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

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskServiceStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<CloudDiskServiceAccessTokenMock> dfsuAccessToken_;
    static inline shared_ptr<AssistantMock> insMock_;
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
}

void CloudDiskServiceStaticTest::TearDown()
{
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
        fileSyncStates.path = "testpath";
        int32_t userId = 1;
        FailedList failed;
        auto res = SetFileSyncStates(fileSyncStates, userId, failed);
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
        fileSyncStates.path = "";
        fileSyncStates.state = static_cast<SyncState>(6);
        int32_t userId = 1;
        FailedList failed;
        auto res = SetFileSyncStates(fileSyncStates, userId, failed);
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
        fileSyncStates.path = "";
        fileSyncStates.state = SyncState::SYNCING;
        int32_t userId = 1;
        FailedList failed;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).WillOnce(Return(1));
        auto res = SetFileSyncStates(fileSyncStates, userId, failed);
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
        fileSyncStates.path = "";
        fileSyncStates.state = SyncState::SYNCING;
        int32_t userId = 1;
        FailedList failed;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).WillOnce(Return(0));
        auto res = SetFileSyncStates(fileSyncStates, userId, failed);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 end";
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
        string path = "path";
        int32_t userId = 1;
        auto res = GetFileSyncState(path, userId);
        EXPECT_EQ(res.error, ErrorReason::NO_SUCH_FILE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileSyncStateTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStateTest001 end";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test