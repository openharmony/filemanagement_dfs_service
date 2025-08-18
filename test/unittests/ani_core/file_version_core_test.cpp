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

#include "file_version_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager_mock.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;

class FileVersionCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FileVersionCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileVersionCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileVersionCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileVersionCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the FileVersionCore::Constructor function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(FileVersionCoreTest, ConstructorTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "FileVersionCore::Constructor begin.";
        auto data = FileVersionCore::Constructor();
        EXPECT_TRUE(data.IsSuccess());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileVersionCore::Constructor ERROR.";
    }
}

/**
 * @tc.name: GetHistoryVersionListTest1
 * @tc.desc: Verify the FileVersionCore::GetHistoryVersionList function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(FileVersionCoreTest, GetHistoryVersionListTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "FileVersionCore::GetHistoryVersionListTest1 begin.";
        FileVersionCore *fileVersion = FileVersionCore::Constructor().GetData().value();
        auto &instance = CloudSyncManagerImplMock::GetInstance();
        string uri = "";
        int32_t numLimit = 10;
        EXPECT_CALL(instance, GetHistoryVersionList(_, _, _)).WillOnce(Return(E_OK));
        auto data = fileVersion->GetHistoryVersionList(uri, numLimit);
        EXPECT_TRUE(data.IsSuccess());

        EXPECT_CALL(instance, GetHistoryVersionList(_, _, _)).WillOnce(Return(OHOS::FileManagement::E_INNER_FAILED));
        auto ret = fileVersion->GetHistoryVersionList(uri, numLimit);
        EXPECT_FALSE(ret.IsSuccess());
        const auto &err = ret.GetError();
        int errorCode = err.GetErrNo();
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_INNER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileVersionCore::GetHistoryVersionListTest1 ERROR.";
    }
}

/**
 * @tc.name: DownloadHistoryVersionTest1
 * @tc.desc: Verify the FileVersionCore::DownloadHistoryVersion function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(FileVersionCoreTest, DownloadHistoryVersionTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "FileVersionCore::DownloadHistoryVersionTest1 begin.";
        FileVersionCore *fileVersion = FileVersionCore::Constructor().GetData().value();
        auto &instance = CloudSyncManagerImplMock::GetInstance();
        string uri = "";
        uint64_t versoinId = 10;
        auto callback = make_shared<VersionDownloadCallbackAniImpl>(nullptr, nullptr);
        EXPECT_CALL(instance, DownloadHistoryVersion(_, _, _, _, _)).WillOnce(Return(E_OK));
        auto data = fileVersion->DownloadHistoryVersion(uri, versoinId, callback);
        EXPECT_TRUE(data.IsSuccess());

        EXPECT_CALL(instance, DownloadHistoryVersion(_, _, _, _, _))
            .WillOnce(Return(OHOS::FileManagement::E_INNER_FAILED));
        auto ret = fileVersion->DownloadHistoryVersion(uri, versoinId, callback);
        EXPECT_FALSE(ret.IsSuccess());
        const auto &err = ret.GetError();
        int errorCode = err.GetErrNo();
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_INNER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileVersionCore::DownloadHistoryVersionTest1 ERROR.";
    }
}

/**
 * @tc.name: ReplaceFileWithHistoryVersionTest1
 * @tc.desc: Verify the FileVersionCore::ReplaceFileWithHistoryVersion function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(FileVersionCoreTest, ReplaceFileWithHistoryVersionTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "FileVersionCore::ReplaceFileWithHistoryVersionTest1 begin.";
        FileVersionCore *fileVersion = FileVersionCore::Constructor().GetData().value();
        auto &instance = CloudSyncManagerImplMock::GetInstance();
        string uri = "";
        string versionUri = "";
        EXPECT_CALL(instance, ReplaceFileWithHistoryVersion(_, _)).WillOnce(Return(E_OK));
        auto data = fileVersion->ReplaceFileWithHistoryVersion(uri, versionUri);
        EXPECT_TRUE(data.IsSuccess());

        EXPECT_CALL(instance, ReplaceFileWithHistoryVersion(_, _))
            .WillOnce(Return(OHOS::FileManagement::E_INNER_FAILED));
        auto ret = fileVersion->ReplaceFileWithHistoryVersion(uri, versionUri);
        EXPECT_FALSE(ret.IsSuccess());
        const auto &err = ret.GetError();
        int errorCode = err.GetErrNo();
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_INNER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileVersionCore::ReplaceFileWithHistoryVersionTest1 ERROR.";
    }
}

/**
 * @tc.name: IsConflictTest1
 * @tc.desc: Verify the FileVersionCore::IsConflict function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(FileVersionCoreTest, IsConflictTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "FileVersionCore::IsConflictTest1 begin.";
        FileVersionCore *fileVersion = FileVersionCore::Constructor().GetData().value();
        auto &instance = CloudSyncManagerImplMock::GetInstance();
        string uri = "";
        EXPECT_CALL(instance, IsFileConflict(_, _)).WillOnce(Return(E_OK));
        auto data = fileVersion->IsConflict(uri);
        EXPECT_TRUE(data.IsSuccess());

        EXPECT_CALL(instance, IsFileConflict(_, _)).WillOnce(Return(OHOS::FileManagement::E_INNER_FAILED));
        auto ret = fileVersion->IsConflict(uri);
        EXPECT_FALSE(ret.IsSuccess());
        const auto &err = ret.GetError();
        int errorCode = err.GetErrNo();
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_INNER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileVersionCore::IsConflictTest1 ERROR.";
    }
}

/**
 * @tc.name: ClearFileConflictTest1
 * @tc.desc: Verify the FileVersionCore::ClearFileConflict function
 * @tc.type: FUNC
 * @tc.require: ICU4T4
 */
HWTEST_F(FileVersionCoreTest, ClearFileConflictTest1, TestSize.Level1)
{
    try {
        GTEST_LOG_(INFO) << "FileVersionCore::ClearFileConflictTest1 begin.";
        FileVersionCore *fileVersion = FileVersionCore::Constructor().GetData().value();
        auto &instance = CloudSyncManagerImplMock::GetInstance();
        string uri = "";
        EXPECT_CALL(instance, ClearFileConflict(_)).WillOnce(Return(E_OK));
        auto data = fileVersion->ClearFileConflict(uri);
        EXPECT_TRUE(data.IsSuccess());

        EXPECT_CALL(instance, ClearFileConflict(_)).WillOnce(Return(OHOS::FileManagement::E_INNER_FAILED));
        auto ret = fileVersion->ClearFileConflict(uri);
        EXPECT_FALSE(ret.IsSuccess());
        const auto &err = ret.GetError();
        int errorCode = err.GetErrNo();
        EXPECT_EQ(errorCode, OHOS::FileManagement::E_INNER_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileVersionCore::ClearFileConflictTest1 ERROR.";
    }
}
} // namespace OHOS::FileManagement::CloudDisk::Test
