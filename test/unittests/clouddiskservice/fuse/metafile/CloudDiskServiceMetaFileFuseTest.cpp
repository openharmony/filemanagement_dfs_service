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
#include <cstring>
#include <fcntl.h>
#include <securec.h>

#include "MetaFileIoMock.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_metafile.h"
#include "convertor.h"
#include "file_utils.h"

#define access OHOS::FileManagement::CloudDiskService::Test::MetaFileIoMock::Access
#define open OHOS::FileManagement::CloudDiskService::Test::MetaFileIoMock::Open
#define fstat OHOS::FileManagement::CloudDiskService::Test::MetaFileIoMock::Fstat
#define ftruncate OHOS::FileManagement::CloudDiskService::Test::MetaFileIoMock::Truncate
#define FileUtils MetaFileIoAdapter
#define ForceCreateDirectory ForceCreateDirectoryMetaFileMock
#include "cloud_disk_service_metafile.cpp"
#undef ForceCreateDirectory
#undef FileUtils
#undef ftruncate
#undef fstat
#undef open
#undef access

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing::ext;

namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr uint32_t TEST_ROOT_INDEX = 42;
constexpr uint64_t CHILD_INODE = 1000;
constexpr uint64_t PARENT_INODE = 2000;
constexpr int TEST_FD = 900;
constexpr size_t META_PARENT_DEPTH_LIMIT = 1024;
constexpr uint32_t TEST_RECORD_HASH = 7;
const std::string RECORD_ID = "0123456789abcdef";

CloudDiskServiceDcacheHeader MakeHeader(const std::string &parent, const std::string &recordId, uint32_t hash)
{
    CloudDiskServiceDcacheHeader header{};
    (void)memcpy_s(header.parentDentryfile, sizeof(header.parentDentryfile), parent.data(),
                   std::min(parent.size(), sizeof(header.parentDentryfile)));
    (void)memcpy_s(header.selfRecordId, sizeof(header.selfRecordId), recordId.data(),
                   std::min(recordId.size(), sizeof(header.selfRecordId)));
    header.selfHash = hash;
    return header;
}

std::shared_ptr<CloudDiskServiceMetaFile> MakeValidMetaFile(uint64_t inode)
{
    MetaFileIoMock::SetAccessResult(-1, ENOENT);
    MetaFileIoMock::PushOpenResult(TEST_FD);
    return std::make_shared<CloudDiskServiceMetaFile>(TEST_USER_ID, TEST_ROOT_INDEX, inode);
}

void QueueExistingParent(const std::string &childName)
{
    MetaFileIoMock::PushOpenResult(TEST_FD + 1);
    MetaFileIoMock::PushFstatResult(0, sizeof(CloudDiskServiceDcacheHeader));
    MetaFileIoMock::PushReadObject(MakeHeader(ROOT_PARENTDENTRYFILE, RECORD_ID, TEST_RECORD_HASH));

    CloudDiskServiceDentryGroup group{};
    group.bitmap[0] = 1;
    group.nsl[0].revalidate = VALIDATE;
    group.nsl[0].namelen = static_cast<uint16_t>(childName.size());
    (void)memcpy_s(group.nsl[0].recordId, sizeof(group.nsl[0].recordId), RECORD_ID.data(), RECORD_ID.size());
    (void)memcpy_s(group.fileName, sizeof(group.fileName), childName.data(),
                   std::min(childName.size(), sizeof(group.fileName)));
    MetaFileIoMock::PushReadObject(group);
}

void QueueExistingParentWithoutDentry()
{
    MetaFileIoMock::PushOpenResult(TEST_FD + 1);
    MetaFileIoMock::PushFstatResult(0, sizeof(CloudDiskServiceDcacheHeader));
    MetaFileIoMock::PushReadObject(MakeHeader(ROOT_PARENTDENTRYFILE, RECORD_ID, TEST_RECORD_HASH));
}

void QueueParentChain(size_t parentCount, const std::string &childName, bool terminateAtRoot)
{
    for (size_t index = 0; index < parentCount; ++index) {
        std::string nextParent = Convertor::ConvertToHex(PARENT_INODE + index + 1);
        if (terminateAtRoot && index + 1 == parentCount) {
            nextParent = ROOT_PARENTDENTRYFILE;
        }
        MetaFileIoMock::PushOpenResult(TEST_FD + 1);
        MetaFileIoMock::PushFstatResult(0, sizeof(CloudDiskServiceDcacheHeader));
        MetaFileIoMock::PushReadObject(MakeHeader(nextParent, RECORD_ID, TEST_RECORD_HASH));

        CloudDiskServiceDentryGroup group{};
        group.bitmap[0] = 1;
        group.nsl[0].revalidate = VALIDATE;
        group.nsl[0].namelen = static_cast<uint16_t>(childName.size());
        (void)memcpy_s(group.nsl[0].recordId, sizeof(group.nsl[0].recordId), RECORD_ID.data(), RECORD_ID.size());
        (void)memcpy_s(group.fileName, sizeof(group.fileName), childName.data(),
                       std::min(childName.size(), sizeof(group.fileName)));
        MetaFileIoMock::PushReadObject(group);
    }
}
} // namespace

class CloudDiskServiceMetaFileFuseTest : public testing::Test {
public:
    void SetUp() override
    {
        MetaFileIoMock::Reset();
        MetaFileMgr::GetInstance().CloudDiskServiceClearAll();
    }

    void TearDown() override
    {
        MetaFileMgr::GetInstance().CloudDiskServiceClearAll();
        MetaFileIoMock::Reset();
    }
};

/*
 * @tc.name: Constructor_001
 * @tc.desc: Verify non-creating construction leaves a missing metafile invalid
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, Constructor_001, TestSize.Level2)
{
    MetaFileIoMock::PushOpenResult(-1, ENOENT);

    CloudDiskServiceMetaFile metaFile(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE, false);

    EXPECT_FALSE(metaFile.IsValid());
    ASSERT_EQ(MetaFileIoMock::GetOpenFlags().size(), 1U);
    EXPECT_NE(MetaFileIoMock::GetOpenFlags()[0] & O_NOFOLLOW, 0);
    EXPECT_EQ(MetaFileIoMock::GetOpenFlags()[0] & O_ACCMODE, O_RDONLY);
}

/*
 * @tc.name: Constructor_002
 * @tc.desc: Verify non-creating construction rejects a short metafile
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, Constructor_002, TestSize.Level2)
{
    MetaFileIoMock::PushOpenResult(TEST_FD);
    MetaFileIoMock::PushFstatResult(0, sizeof(CloudDiskServiceDcacheHeader) - 1);

    CloudDiskServiceMetaFile metaFile(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE, false);

    EXPECT_FALSE(metaFile.IsValid());
}

/*
 * @tc.name: Constructor_003
 * @tc.desc: Verify non-creating construction rejects an undecodable header
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, Constructor_003, TestSize.Level2)
{
    MetaFileIoMock::PushOpenResult(TEST_FD);
    MetaFileIoMock::PushFstatResult(0, sizeof(CloudDiskServiceDcacheHeader));
    MetaFileIoMock::PushReadResult(sizeof(CloudDiskServiceDcacheHeader) - 1);

    CloudDiskServiceMetaFile metaFile(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE, false);

    EXPECT_FALSE(metaFile.IsValid());
}

/*
 * @tc.name: Constructor_004
 * @tc.desc: Verify non-creating construction decodes a complete existing header
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, Constructor_004, TestSize.Level1)
{
    constexpr uint32_t SELF_HASH = 73;
    MetaFileIoMock::PushOpenResult(TEST_FD);
    MetaFileIoMock::PushFstatResult(0, sizeof(CloudDiskServiceDcacheHeader));
    MetaFileIoMock::PushReadObject(MakeHeader(ROOT_PARENTDENTRYFILE, RECORD_ID, SELF_HASH));

    CloudDiskServiceMetaFile metaFile(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE, false);

    EXPECT_TRUE(metaFile.IsValid());
    EXPECT_EQ(metaFile.parentDentryFile_, ROOT_PARENTDENTRYFILE);
    EXPECT_EQ(metaFile.selfRecordId_, RECORD_ID);
    EXPECT_EQ(metaFile.selfHash_, SELF_HASH);
}

/*
 * @tc.name: Constructor_005
 * @tc.desc: Verify the creating constructor opens a missing metafile with O_CREAT
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, Constructor_005, TestSize.Level1)
{
    auto metaFile = MakeValidMetaFile(CHILD_INODE);

    EXPECT_TRUE(metaFile->IsValid());
    ASSERT_EQ(MetaFileIoMock::GetOpenFlags().size(), 1U);
    EXPECT_NE(MetaFileIoMock::GetOpenFlags()[0] & O_CREAT, 0);
    EXPECT_EQ(MetaFileIoMock::GetForceCreateDirectoryCalls(), 1U);
}

/*
 * @tc.name: Constructor_006
 * @tc.desc: Verify non-creating construction rejects a metafile when fstat fails
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, Constructor_006, TestSize.Level2)
{
    MetaFileIoMock::PushOpenResult(TEST_FD);
    MetaFileIoMock::PushFstatResult(-1, 0, EIO);

    CloudDiskServiceMetaFile metaFile(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE, false);

    EXPECT_FALSE(metaFile.IsValid());
}

/*
 * @tc.name: Constructor_007
 * @tc.desc: Verify creating construction opens and decodes an existing metafile without O_CREAT
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, Constructor_007, TestSize.Level1)
{
    constexpr uint32_t SELF_HASH = 74;
    MetaFileIoMock::SetAccessResult(0);
    MetaFileIoMock::PushOpenResult(TEST_FD);
    MetaFileIoMock::PushReadObject(MakeHeader(ROOT_PARENTDENTRYFILE, RECORD_ID, SELF_HASH));

    CloudDiskServiceMetaFile metaFile(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE);

    EXPECT_TRUE(metaFile.IsValid());
    EXPECT_EQ(metaFile.parentDentryFile_, ROOT_PARENTDENTRYFILE);
    EXPECT_EQ(metaFile.selfRecordId_, RECORD_ID);
    EXPECT_EQ(metaFile.selfHash_, SELF_HASH);
    ASSERT_EQ(MetaFileIoMock::GetOpenFlags().size(), 1U);
    EXPECT_EQ(MetaFileIoMock::GetOpenFlags()[0] & O_CREAT, 0);
    EXPECT_EQ(MetaFileIoMock::GetOpenFlags()[0] & O_ACCMODE, O_RDWR);
    EXPECT_EQ(MetaFileIoMock::GetForceCreateDirectoryCalls(), 1U);
}

/*
 * @tc.name: GetCloudDiskServiceMetaFileIfExists_001
 * @tc.desc: Verify manager lookup does not manufacture a missing metafile
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetCloudDiskServiceMetaFileIfExists_001, TestSize.Level2)
{
    MetaFileIoMock::PushOpenResult(-1, ENOENT);

    auto metaFile =
        MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFileIfExists(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE);

    EXPECT_EQ(metaFile, nullptr);
}

/*
 * @tc.name: GetCloudDiskServiceMetaFileIfExists_002
 * @tc.desc: Verify manager lookup returns a valid decoded metafile
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetCloudDiskServiceMetaFileIfExists_002, TestSize.Level1)
{
    MetaFileIoMock::PushOpenResult(TEST_FD);
    MetaFileIoMock::PushFstatResult(0, sizeof(CloudDiskServiceDcacheHeader));
    MetaFileIoMock::PushReadObject(MakeHeader(ROOT_PARENTDENTRYFILE, RECORD_ID, 1));

    auto metaFile =
        MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFileIfExists(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE);

    ASSERT_NE(metaFile, nullptr);
    EXPECT_TRUE(metaFile->IsValid());
}

/*
 * @tc.name: GetRelativePathIfExists_001
 * @tc.desc: Verify null input is rejected and output is cleared
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_001, TestSize.Level2)
{
    std::string path = "stale";

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(nullptr, path);

    EXPECT_EQ(result, E_PATH_NOT_EXIST);
    EXPECT_TRUE(path.empty());
}

/*
 * @tc.name: GetRelativePathIfExists_002
 * @tc.desc: Verify a root metafile resolves without opening a parent
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_002, TestSize.Level1)
{
    auto metaFile = MakeValidMetaFile(CHILD_INODE);
    metaFile->parentDentryFile_ = ROOT_PARENTDENTRYFILE;
    std::string path;

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(path, "/");
}

/*
 * @tc.name: GetRelativePathIfExists_003
 * @tc.desc: Verify a self-referential parent chain is rejected
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_003, TestSize.Level2)
{
    auto metaFile = MakeValidMetaFile(CHILD_INODE);
    metaFile->parentDentryFile_ = metaFile->selfInode_;
    std::string path = "stale";

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_EQ(result, E_PATH_NOT_EXIST);
    EXPECT_TRUE(path.empty());
}

/*
 * @tc.name: GetRelativePathIfExists_004
 * @tc.desc: Verify a missing parent metafile breaks path resolution without creating it
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_004, TestSize.Level2)
{
    auto metaFile = MakeValidMetaFile(CHILD_INODE);
    metaFile->parentDentryFile_ = Convertor::ConvertToHex(PARENT_INODE);
    MetaFileIoMock::PushOpenResult(-1, ENOENT);
    std::string path = "stale";

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_EQ(result, E_PATH_NOT_EXIST);
    EXPECT_TRUE(path.empty());
}

/*
 * @tc.name: GetRelativePathIfExists_005
 * @tc.desc: Verify a valid existing parent chain produces the relative path
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_005, TestSize.Level1)
{
    auto metaFile = MakeValidMetaFile(CHILD_INODE);
    metaFile->parentDentryFile_ = Convertor::ConvertToHex(PARENT_INODE);
    metaFile->selfRecordId_ = RECORD_ID;
    metaFile->selfHash_ = 1;
    QueueExistingParent("leaf");
    std::string path;

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(path, "/leaf/");
}

/*
 * @tc.name: GetRelativePathIfExists_006
 * @tc.desc: Verify a dentry name containing a path separator is rejected
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_006, TestSize.Level2)
{
    auto metaFile = MakeValidMetaFile(CHILD_INODE);
    metaFile->parentDentryFile_ = Convertor::ConvertToHex(PARENT_INODE);
    metaFile->selfRecordId_ = RECORD_ID;
    metaFile->selfHash_ = 1;
    QueueExistingParent("/");
    std::string path = "stale";

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_EQ(result, E_PATH_NOT_EXIST);
    EXPECT_TRUE(path.empty());
}

/*
 * @tc.name: GetRelativePathIfExists_007
 * @tc.desc: Verify a non-null invalid metafile is rejected and output is cleared
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_007, TestSize.Level2)
{
    MetaFileIoMock::PushOpenResult(-1, ENOENT);
    auto metaFile = std::make_shared<CloudDiskServiceMetaFile>(TEST_USER_ID, TEST_ROOT_INDEX, CHILD_INODE, false);
    std::string path = "stale";

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_FALSE(metaFile->IsValid());
    EXPECT_EQ(result, E_PATH_NOT_EXIST);
    EXPECT_TRUE(path.empty());
}

/*
 * @tc.name: GetRelativePathIfExists_008
 * @tc.desc: Verify parent dentry lookup failure aborts path reconstruction without creating metadata
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_008, TestSize.Level2)
{
    auto metaFile = MakeValidMetaFile(CHILD_INODE);
    metaFile->parentDentryFile_ = Convertor::ConvertToHex(PARENT_INODE);
    metaFile->selfRecordId_ = RECORD_ID;
    metaFile->selfHash_ = 1;
    QueueExistingParentWithoutDentry();
    std::string path = "stale";

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_EQ(result, E_PATH_NOT_EXIST);
    EXPECT_TRUE(path.empty());
}

/*
 * @tc.name: GetRelativePathIfExists_009
 * @tc.desc: Verify cumulative parent names that exceed PATH_MAX are rejected
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_009, TestSize.Level2)
{
    constexpr size_t PARENT_COUNT = 17;
    const std::string component(DENTRY_NAME_LEN * 15, 'a');
    auto metaFile = MakeValidMetaFile(CHILD_INODE);
    metaFile->parentDentryFile_ = Convertor::ConvertToHex(PARENT_INODE);
    metaFile->selfRecordId_ = RECORD_ID;
    metaFile->selfHash_ = 1;
    QueueParentChain(PARENT_COUNT, component, true);
    std::string path = "stale";

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_EQ(result, E_PATH_NOT_EXIST);
    EXPECT_TRUE(path.empty());
}

/*
 * @tc.name: GetRelativePathIfExists_010
 * @tc.desc: Verify a valid mock parent chain beyond the depth limit is rejected
 * @tc.type: RELI
 */
HWTEST_F(CloudDiskServiceMetaFileFuseTest, GetRelativePathIfExists_010, TestSize.Level2)
{
    auto metaFile = MakeValidMetaFile(CHILD_INODE);
    metaFile->parentDentryFile_ = Convertor::ConvertToHex(PARENT_INODE);
    metaFile->selfRecordId_ = RECORD_ID;
    metaFile->selfHash_ = 1;
    QueueParentChain(META_PARENT_DEPTH_LIMIT, "a", false);
    std::string path = "stale";

    int32_t result = MetaFileMgr::GetInstance().GetRelativePathIfExists(metaFile, path);

    EXPECT_EQ(result, E_PATH_NOT_EXIST);
    EXPECT_TRUE(path.empty());
}

} // namespace OHOS::FileManagement::CloudDiskService::Test
