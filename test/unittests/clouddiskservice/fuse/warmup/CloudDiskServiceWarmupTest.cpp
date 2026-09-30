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

#include <gtest/gtest.h>

#include <cerrno>

#include "WarmupFileSystemMock.h"
#include "WarmupMetaFileMock.h"
#include "cloud_disk_service_logfile.h"

#define stat(path, buffer) OHOS::FileManagement::CloudDiskService::Test::WarmupFileSystemMock::Stat(path, buffer)
#define lstat OHOS::FileManagement::CloudDiskService::Test::WarmupFileSystemMock::Lstat
#define opendir OHOS::FileManagement::CloudDiskService::Test::WarmupFileSystemMock::OpenDir
#define readdir OHOS::FileManagement::CloudDiskService::Test::WarmupFileSystemMock::ReadDir
#define closedir OHOS::FileManagement::CloudDiskService::Test::WarmupFileSystemMock::CloseDir
#define access OHOS::FileManagement::CloudDiskService::Test::WarmupFileSystemMock::Access
#define open OHOS::FileManagement::CloudDiskService::Test::WarmupFileSystemMock::Open
#define ftruncate OHOS::FileManagement::CloudDiskService::Test::WarmupFileSystemMock::Truncate
#define ForceCreateDirectory ForceCreateDirectoryWarmupMock
#define FileUtils WarmupFileIoAdapter
#define GetFilePlaceholderState GetFilePlaceholderStateWarmupMock
#include "cloud_disk_service_logfile.cpp"
#undef GetFilePlaceholderState
#undef FileUtils
#undef ForceCreateDirectory
#undef ftruncate
#undef open
#undef access
#undef closedir
#undef readdir
#undef opendir
#undef lstat
#undef stat

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing::ext;

namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr uint32_t TEST_ROOT_INDEX = 42;
constexpr ino_t ROOT_INODE = 1000;
constexpr ino_t CHILD_INODE = 1001;
const std::string ROOT_PATH = "/warmup/root";
const std::string CHILD_PATH = ROOT_PATH + "/child";
} // namespace

class CloudDiskServiceWarmupTest : public testing::Test {
public:
    void SetUp() override
    {
        ffrt::wait();
        LogFileMgr::GetInstance().CloudDiskServiceClearAll();
        WarmupFileSystemMock::Reset();
        ResetWarmupMetaFileMock();
    }

    void TearDown() override
    {
        ffrt::wait();
        LogFileMgr::GetInstance().CloudDiskServiceClearAll();
        WarmupFileSystemMock::Reset();
        ResetWarmupMetaFileMock();
    }
};

/*
 * @tc.name: WarmupSyncFolder_001
 * @tc.desc: Verify cancellation is checked before touching the filesystem
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_001, TestSize.Level2)
{
    int32_t result =
        LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, [] { return true; });

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(WarmupFileSystemMock::GetStatCalls(), 0U);
}

/*
 * @tc.name: WarmupSyncFolder_002
 * @tc.desc: Verify a missing root reports the filesystem error
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_002, TestSize.Level2)
{
    WarmupFileSystemMock::SetStatError(ROOT_PATH, ENOENT);

    int32_t result = LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, {});

    EXPECT_EQ(result, ENOENT);
}

/*
 * @tc.name: WarmupSyncFolder_003
 * @tc.desc: Verify a non-directory root is rejected
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_003, TestSize.Level2)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFREG, ROOT_INODE);

    int32_t result = LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, {});

    EXPECT_EQ(result, ENOTDIR);
}

/*
 * @tc.name: WarmupSyncFolder_004
 * @tc.desc: Verify failure to initialize a missing root metafile is converted to EIO
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_004, TestSize.Level2)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    SetWarmupGenericDentryResult(EINVAL);

    int32_t result = LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, {});

    EXPECT_EQ(result, EIO);
    EXPECT_EQ(GetWarmupGenericDentryCalls(), 1U);
}

/*
 * @tc.name: WarmupSyncFolder_005
 * @tc.desc: Verify an existing empty root completes without rewriting its metafile
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_005, TestSize.Level1)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetDirectory(ROOT_PATH, {});
    SetWarmupMetaFileExists(ROOT_INODE, true);

    int32_t result = LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupGenericDentryCalls(), 0U);
    EXPECT_EQ(WarmupFileSystemMock::GetOpenDirCalls(), 1U);
}

/*
 * @tc.name: WarmupSyncFolder_006
 * @tc.desc: Verify cancellation after root lookup prevents creating a missing metafile
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_006, TestSize.Level2)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    uint32_t calls = 0;
    auto cancel = [&calls] { return ++calls == 2; };

    int32_t result = LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, cancel);

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(GetWarmupCreateMetaFileCalls(), 0U);
}

/*
 * @tc.name: WarmupSyncFolder_007
 * @tc.desc: Verify cancellation is checked again before creating a missing root metafile.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_007, TestSize.Level2)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    uint32_t calls = 0;
    auto cancel = [&calls] { return ++calls == 3; };

    int32_t result = LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, cancel);

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(calls, 3U);
    EXPECT_EQ(GetWarmupCreateMetaFileCalls(), 0U);
}

/*
 * @tc.name: WarmupSyncFolder_008
 * @tc.desc: Verify a missing root metafile is initialized before an empty directory is traversed.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_008, TestSize.Level1)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetDirectory(ROOT_PATH, {});

    int32_t result = LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupGenericDentryCalls(), 1U);
    EXPECT_GE(GetWarmupCreateMetaFileCalls(), 1U);
}

/*
 * @tc.name: WarmupSyncFolder_009
 * @tc.desc: Verify a directory traversal error is returned by root warmup.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupSyncFolder_009, TestSize.Level2)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetLstatError(ROOT_PATH, EACCES);
    SetWarmupMetaFileExists(ROOT_INODE, true);

    int32_t result = LogFileMgr::GetInstance().WarmupSyncFolder(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH, {});

    EXPECT_EQ(result, EACCES);
}

/*
 * @tc.name: WarmupChildForDir_001
 * @tc.desc: Verify directory warmup observes cancellation before lstat
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_001, TestSize.Level2)
{
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, [] { return true; });

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(WarmupFileSystemMock::GetLstatCalls(), 0U);
}

/*
 * @tc.name: WarmupChildForDir_002
 * @tc.desc: Verify a non-directory path requires no traversal
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_002, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFREG, ROOT_INODE);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(WarmupFileSystemMock::GetOpenDirCalls(), 0U);
}

/*
 * @tc.name: WarmupChildForDir_003
 * @tc.desc: Verify readdir errors are returned after an otherwise empty traversal
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_003, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetDirectory(ROOT_PATH, {".", ".."}, EIO);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, {});

    EXPECT_EQ(result, EIO);
}

/*
 * @tc.name: WarmupChildForDir_004
 * @tc.desc: Verify traversal retains the first child error
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_004, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetDirectory(ROOT_PATH, {"child", "gone"});
    WarmupFileSystemMock::SetLstatError(CHILD_PATH, EACCES);
    WarmupFileSystemMock::SetLstatError(ROOT_PATH + "/gone", ENOENT);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, {});

    EXPECT_EQ(result, EACCES);
}

/*
 * @tc.name: WarmupChildForDir_005
 * @tc.desc: Verify an existing regular-file dentry needs no metafile creation
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_005, TestSize.Level1)
{
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetDirectory(ROOT_PATH, {"child"});
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFREG, CHILD_INODE);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 1U);
    EXPECT_EQ(GetWarmupGenericDentryCalls(), 0U);
}

/*
 * @tc.name: WarmupChildForDir_006
 * @tc.desc: Verify a vanished child directory is treated as a benign race
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_006, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetDirectory(ROOT_PATH, {"child"});
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFDIR, CHILD_INODE);
    WarmupFileSystemMock::SetOpenDirError(CHILD_PATH, ENOENT);
    SetWarmupMetaFileExists(CHILD_INODE, true);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(WarmupFileSystemMock::GetOpenDirCalls(), 2U);
}

/*
 * @tc.name: WarmupChildForDir_007
 * @tc.desc: Verify failure to initialize a new child directory metafile is returned
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_007, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetDirectory(ROOT_PATH, {"child"});
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFDIR, CHILD_INODE);
    SetWarmupGenericDentryResult(EIO);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, {});

    EXPECT_EQ(result, EIO);
    EXPECT_EQ(GetWarmupGenericDentryCalls(), 1U);
}

/*
 * @tc.name: WarmupChildForDir_008
 * @tc.desc: Verify an lstat failure prevents opening the directory.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_008, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstatError(ROOT_PATH, EACCES);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, {});

    EXPECT_EQ(result, EACCES);
    EXPECT_EQ(WarmupFileSystemMock::GetOpenDirCalls(), 0U);
}

/*
 * @tc.name: WarmupChildForDir_009
 * @tc.desc: Verify an opendir failure is returned without reading directory entries.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_009, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetOpenDirError(ROOT_PATH, EACCES);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, {});

    EXPECT_EQ(result, EACCES);
    EXPECT_EQ(WarmupFileSystemMock::GetOpenDirCalls(), 1U);
}

/*
 * @tc.name: WarmupChildForDir_010
 * @tc.desc: Verify cancellation inside the directory loop prevents child traversal.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChildForDir_010, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetDirectory(ROOT_PATH, {"child"});
    uint32_t calls = 0;
    auto cancel = [&calls] { return ++calls == 2; };
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);

    int32_t result = logFile->WarmupChildForDir(ROOT_PATH, 1, cancel);

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(calls, 2U);
    EXPECT_EQ(WarmupFileSystemMock::GetLstatCalls(), 1U);
}

/*
 * @tc.name: WarmupChild_001
 * @tc.desc: Verify child warmup checks cancellation before lstat.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_001, TestSize.Level2)
{
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, [] { return true; });

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(WarmupFileSystemMock::GetLstatCalls(), 0U);
}

/*
 * @tc.name: WarmupChild_002
 * @tc.desc: Verify a stale child is treated as a benign filesystem race.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_002, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstatError(CHILD_PATH, ESTALE);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 0U);
}

/*
 * @tc.name: WarmupChild_003
 * @tc.desc: Verify unsupported child types require no metadata lookup.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_003, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFIFO, CHILD_INODE);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 0U);
}

/*
 * @tc.name: WarmupChild_004
 * @tc.desc: Verify cancellation after metadata lookup prevents child metafile lookup.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_004, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFDIR, CHILD_INODE);
    uint32_t calls = 0;
    auto cancel = [&calls] { return ++calls == 2; };
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, cancel);

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(calls, 2U);
    EXPECT_EQ(GetWarmupCreateMetaFileCalls(), 1U);
}

/*
 * @tc.name: WarmupChild_005
 * @tc.desc: Verify cancellation before child metafile creation leaves only the parent metafile.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_005, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFDIR, CHILD_INODE);
    uint32_t calls = 0;
    auto cancel = [&calls] { return ++calls == 3; };
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, cancel);

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(calls, 3U);
    EXPECT_EQ(GetWarmupCreateMetaFileCalls(), 1U);
}

/*
 * @tc.name: WarmupChild_006
 * @tc.desc: Verify a new child directory metafile is initialized before recursive traversal.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_006, TestSize.Level1)
{
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFDIR, CHILD_INODE);
    WarmupFileSystemMock::SetDirectory(CHILD_PATH, {});
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupGenericDentryCalls(), 1U);
    EXPECT_EQ(WarmupFileSystemMock::GetOpenDirCalls(), 1U);
}

/*
 * @tc.name: WarmupChild_007
 * @tc.desc: Verify recursive ENOTDIR is normalized as a benign race.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_007, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFDIR, CHILD_INODE);
    WarmupFileSystemMock::SetOpenDirError(CHILD_PATH, ENOTDIR);
    SetWarmupMetaFileExists(CHILD_INODE, true);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(WarmupFileSystemMock::GetOpenDirCalls(), 1U);
}

/*
 * @tc.name: WarmupChild_008
 * @tc.desc: Verify recursive ESTALE is normalized as a benign race.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_008, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFDIR, CHILD_INODE);
    WarmupFileSystemMock::SetOpenDirError(CHILD_PATH, ESTALE);
    SetWarmupMetaFileExists(CHILD_INODE, true);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(WarmupFileSystemMock::GetOpenDirCalls(), 1U);
}

/*
 * @tc.name: WarmupChild_009
 * @tc.desc: Verify an unexpected recursive traversal error is propagated.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_009, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFDIR, CHILD_INODE);
    WarmupFileSystemMock::SetOpenDirError(CHILD_PATH, EIO);
    SetWarmupMetaFileExists(CHILD_INODE, true);
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, {});

    EXPECT_EQ(result, EIO);
}

/*
 * @tc.name: WarmupChild_010
 * @tc.desc: Verify a metadata recovery error is propagated before directory handling.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, WarmupChild_010, TestSize.Level2)
{
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFREG, CHILD_INODE);
    WarmupFileSystemMock::SetStatError(ROOT_PATH, EACCES);
    SetWarmupLookupByNameResults({EINVAL});
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);

    int32_t result = logFile->WarmupChild(ROOT_PATH, "child", parentMeta, 1, {});

    EXPECT_EQ(result, EACCES);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 1U);
    EXPECT_EQ(WarmupFileSystemMock::GetLstatCalls(), 2U);
}

/*
 * @tc.name: EnsureWarmupChildMeta_001
 * @tc.desc: Verify cancellation after a failed lookup prevents log production.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, EnsureWarmupChildMeta_001, TestSize.Level2)
{
    SetWarmupLookupByNameResults({EINVAL});
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);
    MetaBase childMeta("child");
    struct stat childStat {};

    int32_t result = logFile->EnsureWarmupChildMeta(CHILD_PATH, parentMeta, childMeta, childStat, [] { return true; });

    EXPECT_EQ(result, ECANCELED);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 1U);
    EXPECT_EQ(WarmupFileSystemMock::GetStatCalls(), 0U);
}

/*
 * @tc.name: EnsureWarmupChildMeta_002
 * @tc.desc: Verify successful log production is followed by a successful metadata lookup.
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, EnsureWarmupChildMeta_002, TestSize.Level1)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetStat(CHILD_PATH, S_IFREG, CHILD_INODE);
    SetWarmupLookupByNameResults({EINVAL, E_OK});
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);
    MetaBase childMeta("child");
    struct stat childStat {};

    int32_t result = logFile->EnsureWarmupChildMeta(CHILD_PATH, parentMeta, childMeta, childStat, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 2U);
    EXPECT_EQ(childMeta.recordId, "record-id");
    EXPECT_EQ(WarmupFileSystemMock::GetPlaceholderStateCalls(), 1U);
    EXPECT_EQ(WarmupFileSystemMock::GetReadFileCalls(), 1U);
    EXPECT_EQ(WarmupFileSystemMock::GetWriteFileCalls(), 1U);
}

/*
 * @tc.name: EnsureWarmupChildMeta_003
 * @tc.desc: Verify a stale child after a second failed lookup is treated as benign.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, EnsureWarmupChildMeta_003, TestSize.Level2)
{
    WarmupFileSystemMock::SetStat(ROOT_PATH, S_IFDIR, ROOT_INODE);
    WarmupFileSystemMock::SetStat(CHILD_PATH, S_IFREG, CHILD_INODE);
    WarmupFileSystemMock::SetLstatError(CHILD_PATH, ESTALE);
    SetWarmupLookupByNameResults({EINVAL, EINVAL});
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);
    MetaBase childMeta("child");
    struct stat childStat {};

    int32_t result = logFile->EnsureWarmupChildMeta(CHILD_PATH, parentMeta, childMeta, childStat, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 2U);
}

/*
 * @tc.name: EnsureWarmupChildMeta_004
 * @tc.desc: Verify a vanished child after log-production failure is treated as benign.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, EnsureWarmupChildMeta_004, TestSize.Level2)
{
    WarmupFileSystemMock::SetStatError(ROOT_PATH, EACCES);
    WarmupFileSystemMock::SetLstatError(CHILD_PATH, ENOENT);
    SetWarmupLookupByNameResults({EINVAL});
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);
    MetaBase childMeta("child");
    struct stat childStat {};

    int32_t result = logFile->EnsureWarmupChildMeta(CHILD_PATH, parentMeta, childMeta, childStat, {});

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 1U);
}

/*
 * @tc.name: EnsureWarmupChildMeta_005
 * @tc.desc: Verify log-production failure is returned while the child still exists.
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceWarmupTest, EnsureWarmupChildMeta_005, TestSize.Level2)
{
    WarmupFileSystemMock::SetStatError(ROOT_PATH, EACCES);
    WarmupFileSystemMock::SetLstat(CHILD_PATH, S_IFREG, CHILD_INODE);
    SetWarmupLookupByNameResults({EINVAL});
    auto logFile = std::make_shared<CloudDiskServiceLogFile>(TEST_USER_ID, TEST_ROOT_INDEX);
    auto parentMeta = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_INODE);
    MetaBase childMeta("child");
    struct stat childStat {};

    int32_t result = logFile->EnsureWarmupChildMeta(CHILD_PATH, parentMeta, childMeta, childStat, {});

    EXPECT_EQ(result, EACCES);
    EXPECT_EQ(GetWarmupLookupByNameCalls(), 1U);
}

/*
 * @tc.name: ScheduleFillChildForDir_001
 * @tc.desc: Verify scheduled fill records the root and completes through the controlled async path
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceWarmupTest, ScheduleFillChildForDir_001, TestSize.Level1)
{
    WarmupFileSystemMock::SetStatError(ROOT_PATH, ENOENT);

    LogFileMgr::GetInstance().ScheduleFillChildForDir(TEST_USER_ID, TEST_ROOT_INDEX, ROOT_PATH);
    ffrt::wait();

    auto logFile = LogFileMgr::GetInstance().GetCloudDiskServiceLogFile(TEST_USER_ID, TEST_ROOT_INDEX);
    ASSERT_NE(logFile, nullptr);
    EXPECT_EQ(logFile->syncFolderPath_, ROOT_PATH);
    EXPECT_GE(WarmupFileSystemMock::GetStatCalls(), 1U);
}

} // namespace OHOS::FileManagement::CloudDiskService::Test
