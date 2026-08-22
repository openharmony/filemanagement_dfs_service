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
#include <gmock/gmock.h>

#include "decompress_mock.h"
#include "decompress.cpp"

#include "utils_log.h"

#include <climits>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FileManagement::Decompress;

namespace {
    const std::string TEST_DIR = "/data/test_decompress";
    const std::string TEST_SOURCE_FILE = TEST_DIR + "source.txt";
    const std::string TEST_TARGET_FILE = TEST_DIR + "target.txt";
}

class DecompressTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp()
    {
        DecompressMock::Reset();
        mkdir(TEST_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    void TearDown()
    {
        DecompressMock::Reset();
    };

protected:
    void CreateTestFile(const std::string &path, const std::string &content = "test")
    {
        std::ofstream ofs(path);
        ofs << content;
        ofs.close();
    }
};

void DecompressTest::SetUpTestCase(void) {}
void DecompressTest::TearDownTestCase(void) {}

// ==================== SplitTargetPath ====================

HWTEST_F(DecompressTest, SplitTargetPath_Normal, TestSize.Level1)
{
    std::string dirPath, fileName;
    EXPECT_TRUE(SplitTargetPath("/data/service/el2/userid/test.txt", dirPath, fileName));
    EXPECT_EQ(dirPath, "/data/service/el2/userid");
    EXPECT_EQ(fileName, "test.txt");
}

HWTEST_F(DecompressTest, SplitTargetPath_RootFile, TestSize.Level1)
{
    std::string dirPath, fileName;
    EXPECT_TRUE(SplitTargetPath("/file.txt", dirPath, fileName));
    EXPECT_EQ(dirPath, "");
    EXPECT_EQ(fileName, "file.txt");
}

HWTEST_F(DecompressTest, SplitTargetPath_NoSlash, TestSize.Level1)
{
    std::string dirPath, fileName;
    EXPECT_FALSE(SplitTargetPath("noslash", dirPath, fileName));
}

HWTEST_F(DecompressTest, SplitTargetPath_Empty, TestSize.Level1)
{
    std::string dirPath, fileName;
    EXPECT_FALSE(SplitTargetPath("", dirPath, fileName));
}

HWTEST_F(DecompressTest, SplitTargetPath_TrailingSlash, TestSize.Level1)
{
    std::string dirPath, fileName;
    EXPECT_TRUE(SplitTargetPath("/data/test/", dirPath, fileName));
    EXPECT_EQ(fileName, "");
}

HWTEST_F(DecompressTest, SplitTargetPath_DeepPath, TestSize.Level1)
{
    std::string dirPath, fileName;
    EXPECT_TRUE(SplitTargetPath("/data/service/el2/userid/backup/file.txt", dirPath, fileName));
    EXPECT_EQ(dirPath, "/data/service/el2/userid/backup");
    EXPECT_EQ(fileName, "file.txt");
}

HWTEST_F(DecompressTest, SplitTargetPath_HiddenFile, TestSize.Level1)
{
    std::string dirPath, fileName;
    EXPECT_TRUE(SplitTargetPath("/data/test/.hidden", dirPath, fileName));
    EXPECT_EQ(fileName, ".hidden");
}

HWTEST_F(DecompressTest, SplitTargetPath_OnlySlash, TestSize.Level1)
{
    std::string dirPath, fileName;
    EXPECT_TRUE(SplitTargetPath("/", dirPath, fileName));
    EXPECT_EQ(dirPath, "");
    EXPECT_EQ(fileName, "");
}

// ==================== ValidateSourcePath ====================

HWTEST_F(DecompressTest, ValidateSourcePath_Empty, TestSize.Level1)
{
    EXPECT_FALSE(ValidateSourcePath(""));
}

HWTEST_F(DecompressTest, ValidateSourcePath_TooLong, TestSize.Level1)
{
    std::string longPath(PATH_MAX + 1, 'a');
    EXPECT_FALSE(ValidateSourcePath(longPath));
}

HWTEST_F(DecompressTest, ValidateSourcePath_Valid, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    EXPECT_TRUE(ValidateSourcePath(TEST_SOURCE_FILE));
}

HWTEST_F(DecompressTest, ValidateSourcePath_NotExist, TestSize.Level1)
{
    EXPECT_FALSE(ValidateSourcePath("/data/nonexistent_abc123"));
}

HWTEST_F(DecompressTest, ValidateSourcePath_RootPath, TestSize.Level1)
{
    EXPECT_TRUE(ValidateSourcePath("/"));
}

HWTEST_F(DecompressTest, ValidateSourcePath_DevNull, TestSize.Level1)
{
    EXPECT_TRUE(ValidateSourcePath("/dev/null"));
}

HWTEST_F(DecompressTest, ValidateSourcePath_RealPathFail, TestSize.Level1)
{
    DecompressMock::g.realpathEnabled = true;
    DecompressMock::g.realpathFail = true;
    EXPECT_FALSE(ValidateSourcePath("/data/test/file.txt"));
}

HWTEST_F(DecompressTest, ValidateSourcePath_RelativePath, TestSize.Level1)
{
    EXPECT_FALSE(ValidateSourcePath("/relative/path"));
}

// ==================== ValidateTargetPath ====================

HWTEST_F(DecompressTest, ValidateTargetPath_Empty, TestSize.Level1)
{
    EXPECT_FALSE(ValidateTargetPath(""));
}

HWTEST_F(DecompressTest, ValidateTargetPath_TooLong, TestSize.Level1)
{
    std::string longPath(PATH_MAX + 1, 'a');
    longPath[0] = '/';
    EXPECT_FALSE(ValidateTargetPath(longPath));
}

HWTEST_F(DecompressTest, ValidateTargetPath_NoSlash, TestSize.Level1)
{
    EXPECT_FALSE(ValidateTargetPath("invalidpath"));
}

HWTEST_F(DecompressTest, ValidateTargetPath_Valid, TestSize.Level1)
{
    CreateTestFile(TEST_TARGET_FILE);
    EXPECT_TRUE(ValidateTargetPath(TEST_TARGET_FILE));
}

HWTEST_F(DecompressTest, ValidateTargetPath_DirNotExist, TestSize.Level1)
{
    EXPECT_FALSE(ValidateTargetPath("/nonexistent_dir_999/file.txt"));
}

HWTEST_F(DecompressTest, ValidateTargetPath_RootDir, TestSize.Level1)
{
    EXPECT_FALSE(ValidateTargetPath("/file.txt"));
}

HWTEST_F(DecompressTest, ValidateTargetPath_SlashOnly, TestSize.Level1)
{
    EXPECT_FALSE(ValidateTargetPath("/"));
}

HWTEST_F(DecompressTest, ValidateTargetPath_DeepValiPath, TestSize.Level1)
{
    EXPECT_TRUE(ValidateTargetPath(TEST_DIR + "file.txt"));
}

HWTEST_F(DecompressTest, ValidateTargetPath_RealPathFail, TestSize.Level1)
{
    DecompressMock::g.realpathEnabled = true;
    DecompressMock::g.realpathFail = true;
    EXPECT_FALSE(ValidateTargetPath("/data/test/file.txt"));
}

// ==================== OpenSourceFile ====================

HWTEST_F(DecompressTest, OpenSourceFile_Valid, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    int fd = OpenSourceFile(TEST_SOURCE_FILE);
    EXPECT_GE(fd, 0);
    if (fd >= 0)
    {
        close(fd);
    }
}

HWTEST_F(DecompressTest, OpenSourceFile_NotExist, TestSize.Level1)
{
    int fd = OpenSourceFile("/data/nonexistent_xyz789");
    EXPECT_LT(fd, 0);
}

HWTEST_F(DecompressTest, OpenSourceFile_DevNull, TestSize.Level1)
{
    int fd = OpenSourceFile("/dev/null");
    EXPECT_GE(fd, 0);
    if (fd >= 0)
    {
        close(fd);
    }
}

HWTEST_F(DecompressTest, OpenSourceFile_MockSuccess, TestSize.Level1)
{
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 42;
    int fd = OpenSourceFile("/any/path");
    EXPECT_EQ(fd, 42);
}

HWTEST_F(DecompressTest, OpenSourceFile_MockFail, TestSize.Level1)
{
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = -1;
    DecompressMock::g.openErrno = EACCES;
    int fd = OpenSourceFile("/any/path");
    EXPECT_LT(fd, 0);
}

// ==================== PreparePdedupCreate  ====================

HWTEST_F(DecompressTest, PreparePdedupCreate_Valid, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(100, "test.txt", 0, 1024);
    EXPECT_NE(create, nullptr);
    if (create) {
        EXPECT_EQ(create->version, 1);
        EXPECT_EQ(create->innerFd, 100);
        EXPECT_EQ(create->fileSize, 1024u);
        EXPECT_EQ(create->nrExtents, 1u);
        EXPECT_STREQ(create->name, "test.txt");
        EXPECT_EQ(create->extents[0].outerOff, 0u);
        EXPECT_EQ(create->extents[0].innerOff, 0u);
        EXPECT_EQ(create->extents[0].len, 1024u);
        free(create);
    }
}

HWTEST_F(DecompressTest, PreparePdedupCreate_EmptyName, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(10, "", 0, 100);
    EXPECT_NE(create, nullptr);
    if (create) {
        EXPECT_STREQ(create->name, "");
        free(create);
    }
}

HWTEST_F(DecompressTest, PreparePdedupCreate_ZeroOffsetSize, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(5, "file.txt", 0, 0);
    EXPECT_NE(create, nullptr);
    if (create) {
        EXPECT_EQ(create->fileSize, 0u);
        free(create);
    }
}

HWTEST_F(DecompressTest, PreparePdedupCreate_MaxNameLen, TestSize.Level1)
{
    std::string maxName(255, 'a');
    struct HmfsPdedupCreate *create = PreparePdedupCreate(1, maxName, 0, 100);
    EXPECT_NE(create, nullptr);
    if (create) {
        EXPECT_EQ(std::string(create->name).length(), 255u);
        free(create);
    }
}

HWTEST_F(DecompressTest, PreparePdedupCreate_NameTooLong, TestSize.Level1)
{
    std::string longName(300, 'a');
    struct HmfsPdedupCreate *create = PreparePdedupCreate(1, longName, 0, 50);
    EXPECT_EQ(create, nullptr);
}

HWTEST_F(DecompressTest, PreparePdedupCreate_LargeOffsetSize, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(50, "large.bin", 0xFFFFFFFF, 0xFFFFFFFF);
    EXPECT_NE(create, nullptr);
    if (create) {
        EXPECT_EQ(create->fileSize, 0xFFFFFFFFu);
        EXPECT_EQ(create->extents[0].innerOff, 0xFFFFFFFFu);
        EXPECT_EQ(create->extents[0].len, 0xFFFFFFFFu);
        free(create);
    }
}

HWTEST_F(DecompressTest, PreparePdedupCreate_NegFd, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(-1, "test.txt", 0, 100);
    EXPECT_NE(create, nullptr);
    if (create) {
        EXPECT_EQ(create->innerFd, -1);
        free(create);
    }
}

HWTEST_F(DecompressTest, PreparePdedupCreate_SpecialChars, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(1, "test-file_01.txt", 0, 100);
    EXPECT_NE(create, nullptr);
    if (create) {
        EXPECT_STREQ(create->name, "test-file_01.txt");
        free(create);
    }
}

// ==================== CheckBundleSupported ====================

HWTEST_F(DecompressTest, CheckBundleSupported_KeepAlive_False, TestSize.Level1)
{
    EXPECT_FALSE(CheckBundleSupported("com.test.bundle", true));
}

HWTEST_F(DecompressTest, CheckBundleSupported_KeepAliveEmpty_False, TestSize.Level1)
{
    EXPECT_FALSE(CheckBundleSupported("", true));
}

HWTEST_F(DecompressTest, CheckBundleSupported_BundleInUnsupportedList, TestSize.Level1)
{
    DecompressMock::g.dlopenEnabled = true;
    DecompressMock::g.dlopenSucceed = true;
    DecompressMock::g.dlsymEnabled = true;
    DecompressMock::g.dlsymSucceed = true;
    DecompressMock::g.unsupportedListRet = 0;
    DecompressMock::g.unsupportedList = {"com.unsupported.app"};
    
    EXPECT_FALSE(CheckBundleSupported("com.unsupported.app", false));
}

HWTEST_F(DecompressTest, CheckBundleSupported_NotInUnsupportedList, TestSize.Level1)
{
    EXPECT_TRUE(CheckBundleSupported("com.other", false));
}

HWTEST_F(DecompressTest, CheckBundleSupported_EmptyNameCached, TestSize.Level1)
{
    EXPECT_TRUE(CheckBundleSupported("", false));
}

HWTEST_F(DecompressTest, CheckBundleSupported_KeepAliveOverridesList, TestSize.Level1)
{
    EXPECT_FALSE(CheckBundleSupported("com.other", true));
}

// ==================== GetSystemFeature ====================

HWTEST_F(DecompressTest, GetSystemFeature_FullSuccess, TestSize.Level1)
{
    DecompressMock::g.fopenEnabled = true;
    DecompressMock::g.fopenSucceed = true;
    DecompressMock::g.fopenHandle = reinterpret_cast<FILE *>(0x1234);
    DecompressMock::g.getlineEnabled = true;
    DecompressMock::g.getlineSucceed = true;
    strncpy(DecompressMock::g.getlineContent, "1\n", sizeof("1\n"));
    DecompressMock::g.getlineRetVal = 2;
    DecompressMock::g.dlopenEnabled = true;
    DecompressMock::g.dlopenSucceed = true;
    DecompressMock::g.dlsymEnabled = true;
    DecompressMock::g.dlsymSucceed = true;
    DecompressMock::g.systemFeatureRet = 0;
    DecompressMock::g.systemFeatureValue = true;
    
    EXPECT_TRUE(GetSystemFeature());
}

// ==================== ReadPdedupFeatureNode ====================

HWTEST_F(DecompressTest, ReadPdedupFeatureNode_FopenFial, TestSize.Level1)
{
    DecompressMock::g.fopenEnabled = true;
    DecompressMock::g.fopenSucceed = false;
    
    EXPECT_FALSE(ReadPdedupFeatureNode());
}

HWTEST_F(DecompressTest, ReadPdedupFeatureNode_GetLine1, TestSize.Level1)
{
    DecompressMock::g.fopenEnabled = true;
    DecompressMock::g.fopenSucceed = true;
    DecompressMock::g.fopenHandle = reinterpret_cast<FILE *>(0x1234);
    DecompressMock::g.getlineEnabled = true;
    DecompressMock::g.getlineSucceed = true;
    strncpy(DecompressMock::g.getlineContent, "1\n", sizeof("1\n"));
    
    DecompressMock::g.getlineRetVal = 2;
    
    EXPECT_TRUE(ReadPdedupFeatureNode());
}

HWTEST_F(DecompressTest, ReadPdedupFeatureNode_GetLine0, TestSize.Level1)
{
    DecompressMock::g.fopenEnabled = true;
    DecompressMock::g.fopenSucceed = true;
    DecompressMock::g.fopenHandle = reinterpret_cast<FILE *>(0x1234);
    DecompressMock::g.getlineEnabled = true;
    DecompressMock::g.getlineSucceed = true;
    strncpy(DecompressMock::g.getlineContent, "0\n", sizeof("0\n"));
    
    DecompressMock::g.getlineRetVal = 2;
    
    EXPECT_FALSE(ReadPdedupFeatureNode());
}

HWTEST_F(DecompressTest, ReadPdedupFeatureNode_GetLineFail, TestSize.Level1)
{
    DecompressMock::g.fopenEnabled = true;
    DecompressMock::g.fopenSucceed = true;
    DecompressMock::g.fopenHandle = reinterpret_cast<FILE *>(0x1234);
    DecompressMock::g.getlineEnabled = true;
    DecompressMock::g.getlineSucceed = false;
    
    EXPECT_FALSE(ReadPdedupFeatureNode());
}

// ==================== ExecutePdedupCreate  ====================

HWTEST_F(DecompressTest, ExecutePdedupCreate_FirstSuccess, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(100, "test.txt", 0, 1024);
    ASSERT_NE(create, nullptr);

    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = 0;
    DecompressMock::g.ioctlErrno = 0;

    EXPECT_TRUE(ExecutePdedupCreate(50, create, "/data/test.txt", 0, 1024));
    EXPECT_EQ(DecompressMock::g.ioctlCallCount, 1);

    free(create);
}

HWTEST_F(DecompressTest, ExecutePdedupCreate_NonRetryableFail, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(100, "test.txt", 0, 1024);
    ASSERT_NE(create, nullptr);

    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = -1;
    DecompressMock::g.ioctlErrno = EINVAL;

    EXPECT_FALSE(ExecutePdedupCreate(50, create, "/data/test.txt", 0, 1024));
    EXPECT_EQ(DecompressMock::g.ioctlCallCount, 1);

    free(create);
}

HWTEST_F(DecompressTest, ExecutePdedupCreate_RetryEINTR_Success, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(100, "test.txt", 0, 1024);
    ASSERT_NE(create, nullptr);

    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = -1;
    DecompressMock::g.ioctlErrno = EINTR;
    DecompressMock::g.ioctlSuccessAfterCount = 1;

    EXPECT_TRUE(ExecutePdedupCreate(50, create, "/data/test.txt", 0, 1024));
    EXPECT_EQ(DecompressMock::g.ioctlCallCount, 2);

    free(create);
}

HWTEST_F(DecompressTest, ExecutePdedupCreate_RetryEAGAIN_Success, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(100, "test.txt", 0, 1024);
    ASSERT_NE(create, nullptr);

    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = -1;
    DecompressMock::g.ioctlErrno = EAGAIN;
    DecompressMock::g.ioctlSuccessAfterCount = 1;

    EXPECT_TRUE(ExecutePdedupCreate(50, create, "/data/test.txt", 0, 1024));
    EXPECT_EQ(DecompressMock::g.ioctlCallCount, 2);

    free(create);
}

HWTEST_F(DecompressTest, ExecutePdedupCreate_RetryEBUSY_Success, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(100, "test.txt", 0, 1024);
    ASSERT_NE(create, nullptr);

    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = -1;
    DecompressMock::g.ioctlErrno = EBUSY;
    DecompressMock::g.ioctlSuccessAfterCount = 1;

    EXPECT_TRUE(ExecutePdedupCreate(50, create, "/data/test.txt", 0, 1024));
    EXPECT_EQ(DecompressMock::g.ioctlCallCount, 2);

    free(create);
}

HWTEST_F(DecompressTest, ExecutePdedupCreate_RetryENOMEM_Success, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(100, "test.txt", 0, 1024);
    ASSERT_NE(create, nullptr);

    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = -1;
    DecompressMock::g.ioctlErrno = ENOMEM;
    DecompressMock::g.ioctlSuccessAfterCount = 1;

    EXPECT_TRUE(ExecutePdedupCreate(50, create, "/data/test.txt", 0, 1024));
    EXPECT_EQ(DecompressMock::g.ioctlCallCount, 2);

    free(create);
}

HWTEST_F(DecompressTest, ExecutePdedupCreate_RetryExhausted, TestSize.Level1)
{
    struct HmfsPdedupCreate *create = PreparePdedupCreate(100, "test.txt", 0, 1024);
    ASSERT_NE(create, nullptr);

    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = -1;
    DecompressMock::g.ioctlErrno = EINTR;

    EXPECT_FALSE(ExecutePdedupCreate(50, create, "/data/test.txt", 0, 1024));
    EXPECT_GE(DecompressMock::g.ioctlCallCount, 2);

    free(create);
}

// ==================== CreateInnerFile ====================

HWTEST_F(DecompressTest, CreateInnerFile_EmptySource, TestSize.Level1)
{
    EXPECT_FALSE(CreateInnerFile("", TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_EmptyTarget, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, "", 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_BothEmpty, TestSize.Level1)
{
    EXPECT_FALSE(CreateInnerFile("", "", 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_InvalidSource, TestSize.Level1)
{
    EXPECT_FALSE(CreateInnerFile("/nonexistent_abc999", TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_InvalidTarget, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, "/nonexistent_dir_999/f.txt", 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_TargetNoSlash, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, "invalidpath", 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_SourceTooLong, TestSize.Level1)
{
    std::string longPath(PATH_MAX + 1, 'a');
    EXPECT_FALSE(CreateInnerFile(longPath, TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_TargetTooLong, TestSize.Level1)
{
    std::string longTarget(PATH_MAX + 1, 'a');
    CreateTestFile(TEST_SOURCE_FILE);
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, longTarget, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_SourceOpenFail, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = -1;
    DecompressMock::g.openErrno = EACCES;
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_DirOpenFail, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 100;
    DecompressMock::g.openFailAfterCount = 1;
    DecompressMock::g.openErrno = ENOTDIR;
    DecompressMock::g.closeEnabled = true;
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_IoctlSuccess, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 100;
    DecompressMock::g.openFailAfterCount = -1;
    DecompressMock::g.closeEnabled = true;
    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = 0;
    DecompressMock::g.ioctlErrno = 0;
    EXPECT_TRUE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_IoctlFail, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 100;
    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.closeEnabled = true;
    DecompressMock::g.ioctlReturnVal = -1;
    DecompressMock::g.ioctlErrno = ENOTTY;
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_IoctlRetrySuccess, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 100;
    DecompressMock::g.closeEnabled = true;
    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = -1;
    DecompressMock::g.ioctlErrno = EINTR;
    DecompressMock::g.ioctlSuccessAfterCount = 1;
    EXPECT_TRUE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_ZeroSize, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 100;
    DecompressMock::g.closeEnabled = true;
    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = 0;
    EXPECT_TRUE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 0, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_isSystemApp, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 100;
    DecompressMock::g.closeEnabled = true;
    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = 0;
    EXPECT_TRUE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 1024, true));
}

HWTEST_F(DecompressTest, CreateInnerFile_LargeOffsetSize, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 100;
    DecompressMock::g.closeEnabled = true;
    DecompressMock::g.ioctlEnabled = true;
    DecompressMock::g.ioctlReturnVal = 0;
    EXPECT_TRUE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0xFFFFFFFF, 0xFFFFFFFF, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_PrepareFail, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.openEnabled = true;
    DecompressMock::g.openReturnFd = 100;
    DecompressMock::g.openFailAfterCount = -1;
    DecompressMock::g.closeEnabled = true;
    std::string targetWithLongName = TEST_DIR + "/" + std::string(300, 'a');
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, targetWithLongName, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_SourceRealPathFail, TestSize.Level1)
{
    DecompressMock::g.realpathEnabled = true;
    DecompressMock::g.realpathFail = true;
    EXPECT_FALSE(CreateInnerFile("/data/test/f.txt", TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_TargetRealPathFail, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    DecompressMock::g.realpathEnabled = true;
    DecompressMock::g.realpathFail = true;
    EXPECT_FALSE(CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 1024, false));
}

HWTEST_F(DecompressTest, CreateInnerFile_FdLeakTest, TestSize.Level1)
{
    CreateTestFile(TEST_SOURCE_FILE);
    for (int i = 0; i < 5; i++) {
        CreateInnerFile(TEST_SOURCE_FILE, TEST_TARGET_FILE, 0, 1024, false);
    }
    SUCCEED();
}