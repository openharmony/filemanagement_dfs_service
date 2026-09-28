/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <memory>

#include "assistant.h"
#include "cloud_asset_read_session_mock.h"
#include "cloud_file_kit_mock.h"
#include "dfs_error.h"

#include "file_operations_cloud.cpp"

#include "ffrt_inner.h"

#include "file_operations_cloud_static_ut_test/file_operations_cloud_static.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;

constexpr int32_t USER_ID = 100;
class FileOperationsCloudStaticUtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FileOperationsCloud> fileOperationsCloud_ = nullptr;
    static inline shared_ptr<AssistantMock> insMock = nullptr;
    static inline shared_ptr<FileOperationsHelperMock> foHelperMock_ = nullptr;
    static inline shared_ptr<DatabaseManagerMock> dbManagerMock_ = nullptr;
    static inline shared_ptr<CloudDiskRdbStoreMock> rdbStoreMock_ = nullptr;
};

void FileOperationsCloudStaticUtTest::SetUpTestCase(void)
{
    fileOperationsCloud_ = make_shared<FileOperationsCloud>();
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileOperationsCloudStaticUtTest::TearDownTestCase(void)
{
    Assistant::ins = nullptr;
    insMock = nullptr;
    fileOperationsCloud_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileOperationsCloudStaticUtTest::SetUp(void)
{
    fileOperationsCloud_ = make_shared<FileOperationsCloud>();
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
    foHelperMock_ = make_shared<FileOperationsHelperMock>();
    FileOperationsHelperMock::proxy_ = foHelperMock_;
    dbManagerMock_ = make_shared<DatabaseManagerMock>();
    DatabaseManagerMock::proxy_ = dbManagerMock_;
    rdbStoreMock_ = make_shared<CloudDiskRdbStoreMock>();
    CloudDiskRdbStoreMock::proxy_ = rdbStoreMock_;
    GTEST_LOG_(INFO) << "SetUp";
}

void FileOperationsCloudStaticUtTest::TearDown(void)
{
    CloudDiskRdbStoreMock::proxy_ = nullptr;
    rdbStoreMock_ = nullptr;
    DatabaseManagerMock::proxy_ = nullptr;
    dbManagerMock_ = nullptr;
    FileOperationsHelperMock::proxy_ = nullptr;
    foHelperMock_ = nullptr;
    Assistant::ins = nullptr;
    insMock = nullptr;
    fileOperationsCloud_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetChildInfosTest001
 * @tc.desc: Verify GetChildInfos returns 0 when IsBundleCloudSyncEnabled is false
 * @tc.type: FUNC
 */
HWTEST_F(FileOperationsCloudStaticUtTest, GetChildInfosTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetChildInfosTest001 Start";
    try {
        CloudDiskFuseData data;
        data.userId = USER_ID;
        auto inoPtr = make_shared<CloudDiskInode>();
        inoPtr->cloudId = "parentCloudId001";
        inoPtr->bundleName = "com.test.GetChildInfosTest001";

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)));
        EXPECT_CALL(*foHelperMock_, FindCloudDiskInode(_, _)).WillOnce(Return(inoPtr));
        EXPECT_CALL(*dbManagerMock_, IsBundleCloudSyncEnabled(_, _)).WillOnce(Return(false));

        vector<CloudDiskFileInfo> childInfos;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        int32_t ret = GetChildInfos(req, ino, childInfos);
        EXPECT_EQ(ret, 0);
        EXPECT_TRUE(childInfos.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetChildInfosTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetChildInfosTest001 End";
}

/**
 * @tc.name: GetChildInfosTest002
 * @tc.desc: Verify GetChildInfos proceeds to readdir when IsBundleCloudSyncEnabled is true
 * @tc.type: FUNC
 */
HWTEST_F(FileOperationsCloudStaticUtTest, GetChildInfosTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetChildInfosTest002 Start";
    try {
        CloudDiskFuseData data;
        data.userId = USER_ID;
        auto inoPtr = make_shared<CloudDiskInode>();
        inoPtr->cloudId = "parentCloudId002";
        inoPtr->bundleName = "com.test.GetChildInfosTest002";

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)));
        EXPECT_CALL(*foHelperMock_, FindCloudDiskInode(_, _)).WillOnce(Return(inoPtr));
        EXPECT_CALL(*dbManagerMock_, IsBundleCloudSyncEnabled(_, _)).WillOnce(Return(true));
        auto rdbStore = make_shared<CloudDiskRdbStore>(inoPtr->bundleName, USER_ID);
        EXPECT_CALL(*dbManagerMock_, GetRdbStore(_, _)).WillOnce(Return(rdbStore));
        EXPECT_CALL(*rdbStoreMock_, ReadDir(_, _)).WillOnce(Return(E_OK));

        vector<CloudDiskFileInfo> childInfos;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 200;
        int32_t ret = GetChildInfos(req, ino, childInfos);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetChildInfosTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetChildInfosTest002 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test
