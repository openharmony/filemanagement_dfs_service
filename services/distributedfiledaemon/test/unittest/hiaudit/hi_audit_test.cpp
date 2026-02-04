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
#include "hi_audit.h"
#include "zip_utils.h"

#include <directory_ex.h>
#include <dirent.h>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace testing;
using namespace testing::ext;
constexpr int MAX_LOG_FILE_SIZE = 3 * 1024 * 1024;
constexpr int MAX_INPUT_CONTENT_SIZE = 2 * 1024;
constexpr int MAX_ZIP_NUM = 10;
constexpr const char *HIAUDIT_PATH = "/data/log/hiaudit/";
constexpr const char *DAEMON_PATH = "/data/log/hiaudit/distributedfiledaemon/";
constexpr const char *DAEMON_NAME = "distributedfiledaemon";
constexpr int ERROR_GET_HANDLE = 1002;
constexpr int HIVIEW_UID = 1201;
constexpr int LOG_UID = 1007;
constexpr int INVALID_FD = -1;

class HiAuditTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

int GetDirZipFileNum()
{
    uint32_t zipFileCount = 0;
    std::string oldestAuditFile;
    DIR* dir = opendir(DAEMON_PATH);
    if (dir == nullptr) {
        GTEST_LOG_(ERROR) << "Failed open dir, errno: " << errno;
        return -1; //  -1: Invalid file count
    }
    struct dirent *ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        std::string subName = std::string(ptr->d_name);
        std::string zipTag = ".zip";
        if (subName.find(DAEMON_NAME) == 0 &&
            subName.rfind(zipTag) == subName.length() - zipTag.length()) {
            zipFileCount = zipFileCount + 1;
        }
    }
    closedir(dir);
    return zipFileCount;
}

void RollBackDir()
{
    if (!ForceCreateDirectory(HIAUDIT_PATH)) {
        GTEST_LOG_(ERROR) << "Failed create dir, errno: " << errno;
    } else {
        chown(HIAUDIT_PATH, HIVIEW_UID, LOG_UID);
    }
}

/**
 * @tc.number: HiAudit_Write_001
 * @tc.name: HiAudit_Write_001
 * @tc.desc: Verify Constructor function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_Write_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_Write_001 start";
    AuditLog auditLog = { false, 1, "FAILED TO Mount", "ADD", "Mount", "FAIL" };
    int zipFileNum = 0;
    while (zipFileNum <= MAX_ZIP_NUM) {
        GTEST_LOG_(INFO) << "HiauditTest HiAudit_Write_001 write num: " << zipFileNum;
        while (HiAudit::GetInstance().writeLogSize_ <  MAX_LOG_FILE_SIZE) {
            HiAudit::GetInstance().Write(auditLog);
        }
        HiAudit::GetInstance().Write(auditLog);
        EXPECT_GT(HiAudit::GetInstance().writeLogSize_, 0);
        EXPECT_GT(HiAudit::GetInstance().writeFd_, 0);
        ++zipFileNum;
    }
    std::string extendStr = std::string(MAX_INPUT_CONTENT_SIZE, 'A');
    auditLog.extend = extendStr;
    HiAudit::GetInstance().Write(auditLog);
    if (HiAudit::GetInstance().writeFd_ > 0) {
        fdsan_close_with_tag(HiAudit::GetInstance().writeFd_, LOG_DOMAIN);
        HiAudit::GetInstance().writeFd_ = INVALID_FD;
    }
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_Write_001 end";
}

/**
 * @tc.number: HiAudit_CleanOldAuditFile_001
 * @tc.name: HiAudit_CleanOldAuditFile_001
 * @tc.desc: Verify CleanOldAuditFile function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_CleanOldAuditFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_CleanOldAuditFile_001 start";
    // 在/data/log/hiaudit/distributedfiledaemon目录下创建test.txt文件
    std::string testFilePath = std::string(HIAUDIT_PATH) + "distributedfiledaemon/test.txt";
    std::ofstream testFile(testFilePath);
    if (testFile.is_open()) {
        testFile << "test content";
        testFile.close();
    }
    // 在/data/log/hiaudit/distributedfiledaemon目录下创建distributedfiledaemon.txt文件
    std::string daemonFilePath = std::string(HIAUDIT_PATH) + "distributedfiledaemon/distributedfiledaemon.txt";
    std::ofstream daemonFile(daemonFilePath);
    if (daemonFile.is_open()) {
        daemonFile << "daemon content";
        daemonFile.close();
    }
    std::string zipFileName = std::string(HIAUDIT_PATH) + "distributedfiledaemon/" + "distributedfiledaemon_test.zip";
    zipFile compressZip = zipOpen(zipFileName.c_str(), APPEND_STATUS_CREATE);
    ASSERT_TRUE(compressZip != nullptr) << "Failed create zip file, errno: " << errno;
    zipClose(compressZip, nullptr);
    int zipFileNum = GetDirZipFileNum();
    EXPECT_EQ(zipFileNum, MAX_ZIP_NUM + 1);
    HiAudit::GetInstance().CleanOldAuditFile();
    zipFileNum = GetDirZipFileNum();
    EXPECT_EQ(zipFileNum, MAX_ZIP_NUM);
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_CleanOldAuditFile_001 end";
}

/**
 * @tc.number: HiAudit_ZipAuditLog_001
 * @tc.name: HiAudit_ZipAuditLog_001
 * @tc.desc: Verify ZipAuditLog function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_ZipAuditLog_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipAuditLog_001 start";
    HiAudit::GetInstance().ZipAuditLog();
    HiAudit::GetInstance().ZipAuditLog();
    std::string auditCsvPath = std::string(HIAUDIT_PATH) + "distributedfiledaemon/distributedfiledaemon_audit.csv";
    std::ifstream auditCsvFile(auditCsvPath);
    EXPECT_FALSE(auditCsvFile.good());
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipAuditLog_001 end";
}

/**
 * @tc.number: HiAudit_ZipUtils_AddFileInZip_001
 * @tc.name: HiAudit_ZipUtils_AddFileInZip_001
 * @tc.desc: Verify AddFileInZip function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_ZipUtils_AddFileInZip_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipUtils_AddFileInZip_001 start";
    std::string zipFileName = std::string(HIAUDIT_PATH) + "distributedfiledaemon/" + "audit.zip";
    zipFile compressZip = zipOpen(zipFileName.c_str(), APPEND_STATUS_CREATE);
    ASSERT_TRUE(compressZip != nullptr) << "Failed create zip file, errno: " << errno;
    std::string auditLogFileName = std::string(HIAUDIT_PATH) + "distributedfiledaemon/test01.txt";
    KeepStatus keepParentPathStatus = KeepStatus::KEEP_NONE_PARENT_PATH;
    std::string dstFileName = "auditLogContent.csv";
    int ret = ZipUtil::AddFileInZip(compressZip,
        auditLogFileName, keepParentPathStatus, dstFileName);
    zipClose(compressZip, nullptr);
    EXPECT_EQ(ret, ERROR_GET_HANDLE);
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipUtils_AddFileInZip_001 end";
}

/**
 * @tc.number: HiAudit_Init_001
 * @tc.name: HiAudit_Init_001
 * @tc.desc: Verify Init function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_Init_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_Init_001 start";
    HiAudit::GetInstance().Init();
    EXPECT_GT(HiAudit::GetInstance().writeFd_, 0);
    if (HiAudit::GetInstance().writeFd_ > 0) {
        fdsan_close_with_tag(HiAudit::GetInstance().writeFd_, LOG_DOMAIN);
        HiAudit::GetInstance().writeFd_ = INVALID_FD;
    }
    // mkdir and open file failed
    if (!ForceRemoveDirectoryBMS(HIAUDIT_PATH)) {
        GTEST_LOG_(INFO) << "HiAudit_Init_001 ForceRemoveDirectoryBMS testPath failed!";
    }
    HiAudit::GetInstance().Init();
    EXPECT_EQ(HiAudit::GetInstance().writeLogSize_, 0);
    EXPECT_TRUE(HiAudit::GetInstance().writeFd_ == INVALID_FD);
    RollBackDir();
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_Init_001 end";
}

/**
 * @tc.number: HiAudit_GetWriteFilePath_001
 * @tc.name: HiAudit_GetWriteFilePath_001
 * @tc.desc: Verify GetWriteFilePath function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_GetWriteFilePath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_GetWriteFilePath_001 start";
    if (!ForceRemoveDirectoryBMS(HIAUDIT_PATH)) {
        GTEST_LOG_(INFO) << "HiAudit_GetWriteFilePath_001 ForceRemoveDirectoryBMS testPath failed!";
    }
    HiAudit::GetInstance().writeLogSize_ = MAX_LOG_FILE_SIZE;
    HiAudit::GetInstance().GetWriteFilePath();
    EXPECT_EQ(HiAudit::GetInstance().writeLogSize_, 0);
    EXPECT_TRUE(HiAudit::GetInstance().writeFd_ == INVALID_FD);
    RollBackDir();
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_GetWriteFilePath_001 end";
}

/**
 * @tc.number: HiAudit_CleanOldAuditFile_002
 * @tc.name: HiAudit_CleanOldAuditFile_002
 * @tc.desc: Verify CleanOldAuditFile function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_CleanOldAuditFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_CleanOldAuditFile_002 start";
    if (!ForceRemoveDirectoryBMS(HIAUDIT_PATH)) {
        GTEST_LOG_(INFO) << "HiAudit_CleanOldAuditFile_002 ForceRemoveDirectoryBMS testPath failed!";
    }
    HiAudit::GetInstance().CleanOldAuditFile();
    std::string logPath = std::string(HIAUDIT_PATH) + "distributedfiledaemon";
    DIR* dir = opendir(logPath.c_str());
    EXPECT_EQ(dir, nullptr);
    RollBackDir();
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_CleanOldAuditFile_002 end";
}

/**
 * @tc.number: HiAudit_WriteToFile_001
 * @tc.name: HiAudit_WriteToFile_001
 * @tc.desc: Verify WriteToFile function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_WriteToFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_WriteToFile_001 start";
    if (!ForceRemoveDirectoryBMS(HIAUDIT_PATH)) {
        GTEST_LOG_(INFO) << "HiAudit_WriteToFile_001 ForceRemoveDirectoryBMS testPath failed!";
    }
    HiAudit::GetInstance().WriteToFile("test content");
    EXPECT_TRUE(HiAudit::GetInstance().writeFd_ == INVALID_FD);
    RollBackDir();
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_WriteToFile_001 end";
}

/**
 * @tc.number: HiAudit_ZipAuditLog_002
 * @tc.name: HiAudit_ZipAuditLog_002
 * @tc.desc: Verify ZipAuditLog function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_ZipAuditLog_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipAuditLog_002 start";
    if (!ForceRemoveDirectoryBMS(HIAUDIT_PATH)) {
        GTEST_LOG_(INFO) << "HiAudit_ZipAuditLog_002 ForceRemoveDirectoryBMS testPath failed!";
    }
    HiAudit::GetInstance().ZipAuditLog();
    EXPECT_TRUE(HiAudit::GetInstance().writeFd_ == INVALID_FD);
    std::string logPath = std::string(HIAUDIT_PATH) + "distributedfiledaemon";
    DIR* dir = opendir(logPath.c_str());
    EXPECT_EQ(dir, nullptr);
    RollBackDir();
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipAuditLog_002 end";
}

/**
 * @tc.number: HiAudit_ZipUtils_AddFileInZip_002
 * @tc.name: HiAudit_ZipUtils_AddFileInZip_002
 * @tc.desc: Verify AddFileInZip function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_ZipUtils_AddFileInZip_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipUtils_AddFileInZip_002 start";
    if (!ForceRemoveDirectoryBMS(HIAUDIT_PATH)) {
        GTEST_LOG_(INFO) << "HiAudit_ZipUtils_AddFileInZip_002 ForceRemoveDirectoryBMS testPath failed!";
    }
    std::string zipFileName = std::string(HIAUDIT_PATH) + "distributedfiledaemon/" + "audit.zip";
    zipFile compressZip = zipOpen(zipFileName.c_str(), APPEND_STATUS_CREATE);
    std::string auditLogFileName = std::string(HIAUDIT_PATH) + "distributedfiledaemon/" + "audit.csv";
    KeepStatus keepParentPathStatus = KeepStatus::KEEP_NONE_PARENT_PATH;
    std::string dstFileName = "auditLogContent";
    int ret = ZipUtil::AddFileInZip(compressZip, auditLogFileName, keepParentPathStatus, dstFileName);
    zipClose(compressZip, nullptr);
    EXPECT_EQ(ret, ERROR_GET_HANDLE);
    RollBackDir();
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipUtils_AddFileInZip_002 end";
}

/**
 * @tc.number: HiAudit_ZipUtils_GetDestFilePath_001
 * @tc.name: HiAudit_ZipUtils_GetDestFilePath_001
 * @tc.desc: Verify GetDestFilePath function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_ZipUtils_GetDestFilePath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipUtils_GetDestFilePath_001 start";
    if (!ForceRemoveDirectoryBMS(HIAUDIT_PATH)) {
        GTEST_LOG_(INFO) << "HiAudit_ZipUtils_GetDestFilePath_001 ForceRemoveDirectoryBMS testPath failed!";
    }
    std::string srcFile = "";
    std::string destFilePath = "test";
    KeepStatus keepParentPathStatus = KeepStatus::KEEP_NONE_PARENT_PATH;
    std::string resStr = ZipUtil::GetDestFilePath(srcFile, destFilePath, keepParentPathStatus);
    EXPECT_EQ(resStr, destFilePath);

    srcFile = "simple.txt/";
    destFilePath = "";
    resStr = ZipUtil::GetDestFilePath(srcFile, destFilePath, keepParentPathStatus);
    EXPECT_EQ(resStr, srcFile);

    srcFile = "test//simple.txt";
    resStr = ZipUtil::GetDestFilePath(srcFile, destFilePath, keepParentPathStatus);
    EXPECT_EQ(resStr, "simple.txt");

    srcFile = "test/test/simple.txt";
    resStr = ZipUtil::GetDestFilePath(srcFile, destFilePath, keepParentPathStatus);
    EXPECT_EQ(resStr, "simple.txt");

    srcFile = "simple.txt/";
    keepParentPathStatus = KeepStatus::KEEP_ONE_PARENT_PATH;
    resStr = ZipUtil::GetDestFilePath(srcFile, destFilePath, keepParentPathStatus);
    EXPECT_EQ(resStr, "/" + srcFile);

    srcFile = "test//simple.txt";
    resStr = ZipUtil::GetDestFilePath(srcFile, destFilePath, keepParentPathStatus);
    EXPECT_EQ(resStr, srcFile);

    srcFile = "test/test/simple.txt";
    destFilePath = "";
    resStr = ZipUtil::GetDestFilePath(srcFile, destFilePath, keepParentPathStatus);
    EXPECT_EQ(resStr, "test/simple.txt");
    RollBackDir();
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipUtils_GetDestFilePath_001 end";
}

/**
 * @tc.number: HiAudit_ZipUtils_CloseZipFile_001
 * @tc.name: HiAudit_ZipUtils_CloseZipFile_001
 * @tc.desc: Verify CloseZipFile function works correctly
 */
HWTEST_F(HiAuditTest, HiAudit_ZipUtils_CloseZipFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipUtils_CloseZipFile_001 start";
    zipFile compressZip = nullptr;
    ZipUtil::CloseZipFile(compressZip);
    EXPECT_EQ(compressZip, nullptr);
    GTEST_LOG_(INFO) << "HiauditTest HiAudit_ZipUtils_CloseZipFile_001 end";
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS