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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assistant.h"
#include "cloud_disk_common.h"
#include "cloud_disk_service_manager_mock.h"
#include "file_utils.h"
#include "securec.h"

#include "cloud_disk_service_logfile.cpp"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskServiceLogFileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<CloudDiskServiceLogFile> logFile_;
    static inline shared_ptr<AssistantMock> insMock_;
};

void CloudDiskServiceLogFileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskServiceLogFileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskServiceLogFileTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    logFile_ = make_shared<CloudDiskServiceLogFile>(0, 0);
    insMock_->EnableMock();
}

void CloudDiskServiceLogFileTest::TearDown()
{
    insMock_->DisableMock();
    logFile_ = nullptr;
    Assistant::ins = nullptr;
    insMock_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: WriteLogFileTest001
 * @tc.desc: Verify the WriteLogFile function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, WriteLogFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteLogFileTest001 start";
    try {
        LogBlock logBlock;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(0));
        int32_t res = logFile_->WriteLogFile(logBlock);
        EXPECT_EQ(res, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteLogFileTest001 failed";
    }
    GTEST_LOG_(INFO) << "WriteLogFileTest001 end";
}

/**
 * @tc.name: WriteLogFileTest002
 * @tc.desc: Verify the WriteLogFile function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, WriteLogFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteLogFileTest002 start";
    try {
        LogBlock logBlock;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(LOGGROUP_SIZE));
        EXPECT_CALL(*insMock_, WriteFile(_, _, _, _)).WillRepeatedly(Return(LOGGROUP_SIZE));
        int32_t res = logFile_->WriteLogFile(logBlock);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteLogFileTest002 failed";
    }
    GTEST_LOG_(INFO) << "WriteLogFileTest002 end";
}

/**
 * @tc.name: ReadLogFileTest001
 * @tc.desc: Verify the ReadLogFile function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ReadLogFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLogFileTest001 start";
    try {
        uint64_t line = 1;
        LogBlock logBlock;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(0));
        int32_t res = logFile_->ReadLogFile(line, logBlock);
        EXPECT_EQ(res, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadLogFileTest001 failed";
    }
    GTEST_LOG_(INFO) << "ReadLogFileTest001 end";
}

/**
 * @tc.name: ReadLogFileTest002
 * @tc.desc: Verify the ReadLogFile function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ReadLogFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLogFileTest002 start";
    try {
        uint64_t line = 1;
        LogBlock logBlock;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(LOGGROUP_SIZE));
        int32_t res = logFile_->ReadLogFile(line, logBlock);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadLogFileTest002 failed";
    }
    GTEST_LOG_(INFO) << "ReadLogFileTest002 end";
}

/**
 * @tc.name: OnDataChangeTest001
 * @tc.desc: Verify the OnDataChange function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, OnDataChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDataChangeTest001 start";
    try {
        logFile_->changeDatas_.clear();
        int32_t res = logFile_->OnDataChange();
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDataChangeTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnDataChangeTest001 end";
}

/**
 * @tc.name: OnDataChangeTest002
 * @tc.desc: Verify the OnDataChange function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, OnDataChangeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDataChangeTest002 start";
    try {
        ChangeData changeData;
        logFile_->changeDatas_.push_back(changeData);
        int32_t res = logFile_->OnDataChange();
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDataChangeTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnDataChangeTest002 end";
}

/**
 * @tc.name: CloudDiskServiceLogFileTest001
 * @tc.desc: Verify the CloudDiskServiceLogFile constructor
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CloudDiskServiceLogFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(LOGGROUP_SIZE));
        shared_ptr<CloudDiskServiceLogFile> logFile =
            make_shared<CloudDiskServiceLogFile>(userId, syncFolderIndex);
        EXPECT_EQ(logFile->userId_, userId);
        EXPECT_EQ(logFile->syncFolderIndex_, syncFolderIndex);
        EXPECT_FALSE(logFile->needCallback_);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest001 failed";
    }
    GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest001 end";
}

/**
 * @tc.name: CloudDiskServiceLogFileTest002
 * @tc.desc: Verify the CloudDiskServiceLogFile constructor
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CloudDiskServiceLogFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest002 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock_, ftruncate(_, _)).WillOnce(Return(0));
        shared_ptr<CloudDiskServiceLogFile> logFile =
            make_shared<CloudDiskServiceLogFile>(userId, syncFolderIndex);
        EXPECT_EQ(logFile->userId_, userId);
        EXPECT_EQ(logFile->syncFolderIndex_, syncFolderIndex);
        EXPECT_FALSE(logFile->needCallback_);
        EXPECT_EQ(logFile->currentLine_, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest002 failed";
    }
    GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest002 end";
}

/**
 * @tc.name: CloudDiskServiceLogFileTest003
 * @tc.desc: Verify the CloudDiskServiceLogFile constructor
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CloudDiskServiceLogFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest003 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _))
            .WillOnce(Return(LOGGROUP_SIZE))
            .WillOnce(Return(LOGGROUP_SIZE))
            .WillOnce(Return(LOGGROUP_SIZE));
        shared_ptr<CloudDiskServiceLogFile> logFile =
            make_shared<CloudDiskServiceLogFile>(userId, syncFolderIndex);
        EXPECT_EQ(logFile->userId_, userId);
        EXPECT_EQ(logFile->syncFolderIndex_, syncFolderIndex);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest003 failed";
    }
    GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest003 end";
}

/**
 * @tc.name: GenerateLogBlockTest001
 * @tc.desc: Verify the GenerateLogBlock function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, GenerateLogBlockTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateLogBlockTest001 start";
    try {
        EventInfo eventInfo;
        uint64_t parentInode = 1;
        string childRecordId = "childRecordId";
        string parentRecordId = "parentRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        ctx.bidx = 0;
        ctx.bitPos = 0;
        uint64_t line = 1;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(LOGGROUP_SIZE));
        EXPECT_CALL(*insMock_, WriteFile(_, _, _, _)).WillRepeatedly(Return(LOGGROUP_SIZE));
        int32_t res = logFile_->GenerateLogBlock(eventInfo, parentInode, ctx, parentRecordId, line);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateLogBlockTest001 failed";
    }
    GTEST_LOG_(INFO) << "GenerateLogBlockTest001 end";
}

/**
 * @tc.name: GenerateChangeDataTest001
 * @tc.desc: Verify the GenerateChangeData function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, GenerateChangeDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateChangeDataTest001 start";
    try {
        EventInfo eventInfo;
        uint64_t line = 1;
        string childRecordId = "childRecordId";
        string parentRecordId = "parentRecordId";
        logFile_->changeDatas_.clear();
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(1));
        int32_t res = logFile_->GenerateChangeData(eventInfo, line, childRecordId, parentRecordId);
        EXPECT_EQ(res, E_OK);
        EXPECT_EQ(logFile_->changeDatas_.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateChangeDataTest001 failed";
    }
    GTEST_LOG_(INFO) << "GenerateChangeDataTest001 end";
}

/**
 * @tc.name: GenerateChangeDataTest002
 * @tc.desc: Verify the GenerateChangeData function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, GenerateChangeDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateChangeDataTest002 start";
    try {
        EventInfo eventInfo;
        eventInfo.path = "/data/test";
        eventInfo.name = "file";
        uint64_t line = 1;
        string childRecordId = "child";
        string parentRecordId = "parent";

        logFile_->changeDatas_.clear();
        logFile_->syncFolderPath_ = "/data/test";
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(1));
        logFile_->GenerateChangeData(eventInfo, line, childRecordId, parentRecordId);
        EXPECT_EQ(logFile_->changeDatas_.size(), 1);
        EXPECT_EQ(logFile_->changeDatas_.back().relativePath, "/file");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateChangeDataTest002 failed";
    }
    GTEST_LOG_(INFO) << "GenerateChangeDataTest002 end";
}

/**
 * @tc.name: GenerateChangeDataTest003
 * @tc.desc: Verify the GenerateChangeData function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, GenerateChangeDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateChangeDataTest003 start";
    try {
        EventInfo eventInfo;
        eventInfo.path = "/data/test";
        eventInfo.name = "file";
        uint64_t line = 1;
        string childRecordId = "child";
        string parentRecordId = "parent";

        logFile_->changeDatas_.clear();
        logFile_->syncFolderPath_ = "/other/path";
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(1));
        logFile_->GenerateChangeData(eventInfo, line, childRecordId, parentRecordId);
        EXPECT_EQ(logFile_->changeDatas_.size(), 1);
        EXPECT_EQ(logFile_->changeDatas_.back().relativePath, "/data/test/file");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateChangeDataTest003 failed";
    }
    GTEST_LOG_(INFO) << "GenerateChangeDataTest003 end";
}

/**
 * @tc.name: GenerateChangeDataTest004
 * @tc.desc: Verify the GenerateChangeData function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, GenerateChangeDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateChangeDataTest004 start";
    try {
        EventInfo eventInfo;
        eventInfo.path = "/data/test";
        eventInfo.name = "file";
        uint64_t line = 1;
        string childRecordId = "child";
        string parentRecordId = "parent";

        logFile_->changeDatas_.clear();
        logFile_->syncFolderPath_ = "/data/test";
        for (unsigned int i = 0; i < MAX_CHANGEDATAS_SIZE - 1; i++) {
            logFile_->changeDatas_.push_back(ChangeData());
        }
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(0));
        logFile_->GenerateChangeData(eventInfo, line, childRecordId, parentRecordId);
        EXPECT_TRUE(logFile_->changeDatas_.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateChangeDataTest004 failed";
    }
    GTEST_LOG_(INFO) << "GenerateChangeDataTest004 end";
}

/**
 * @tc.name: FillChildForDirTest001
 * @tc.desc: Verify the FillChildForDir function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, FillChildForDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillChildForDirTest001 start";
    try {
        string path = "path";
        uint64_t timestamp = 1;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return -1;
        });
        int32_t res = logFile_->FillChildForDir(path, timestamp);
        EXPECT_EQ(res, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillChildForDirTest001 failed";
    }
    GTEST_LOG_(INFO) << "FillChildForDirTest001 end";
}

/**
 * @tc.name: FillChildForDirTest002
 * @tc.desc: Verify the FillChildForDir function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, FillChildForDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillChildForDirTest002 start";
    try {
        string path = "path";
        uint64_t timestamp = 1;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Invoke([](const std::string&, struct stat* buf) {
            buf->st_mode = S_IFREG;
            return 0;
        }));
        EXPECT_EQ(logFile_->FillChildForDir(path, timestamp), E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillChildForDirTest002 failed";
    }
    GTEST_LOG_(INFO) << "FillChildForDirTest002 end";
}

/**
 * @tc.name: FillChildForDirTest003
 * @tc.desc: Verify the FillChildForDir function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, FillChildForDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillChildForDirTest003 start";
    try {
        string path = "path";
        uint64_t timestamp = 1;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Invoke([](const std::string&, struct stat* buf) {
            buf->st_mode = S_IFDIR;
            return 0;
        }));
        EXPECT_CALL(*insMock_, opendir(_)).WillOnce(Return(nullptr));
        EXPECT_NE(logFile_->FillChildForDir(path, timestamp), E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillChildForDirTest003 failed";
    }
    GTEST_LOG_(INFO) << "FillChildForDirTest003 end";
}

/**
 * @tc.name: ProduceLogTest001
 * @tc.desc: Verify the ProduceLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest001 start";
    try {
        EventInfo eventInfo;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return 1;
        });
        auto res = logFile_->ProduceLog(eventInfo);
        EXPECT_EQ(res, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceLogTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProduceLogTest001 end";
}

/**
 * @tc.name: ProduceLogTest002
 * @tc.desc: Verify the ProduceLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest002 start";
    try {
        EventInfo eventInfo;
        eventInfo.operateType = OperationType::SYNC_FOLDER_INVALID;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(0));
        CloudDiskServiceManagerMock &mock = CloudDiskServiceManagerMock::GetInstance();
        EXPECT_CALL(mock, UnregisterForSa(_)).WillOnce(Return(1));
        auto res = logFile_->ProduceLog(eventInfo);
        EXPECT_EQ(res, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceLogTest002 failed";
    }
    GTEST_LOG_(INFO) << "ProduceLogTest002 end";
}

/**
 * @tc.name: ProduceLogTest003
 * @tc.desc: Verify the ProduceLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest003 start";
    try {
        EventInfo eventInfo;
        eventInfo.operateType = OperationType::SYNC_FOLDER_INVALID;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(0));
        CloudDiskServiceManagerMock &mock = CloudDiskServiceManagerMock::GetInstance();
        EXPECT_CALL(mock, UnregisterForSa(_)).WillOnce(Return(0));
        auto res = logFile_->ProduceLog(eventInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceLogTest003 failed";
    }
    GTEST_LOG_(INFO) << "ProduceLogTest003 end";
}

/**
 * @tc.name: ProduceLogTest004
 * @tc.desc: Verify the ProduceLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest004 start";
    try {
        EventInfo eventInfo;
        eventInfo.operateType = OperationType::OPERATION_MAX;
        EXPECT_CALL(*insMock_, stat(_, _)).WillRepeatedly(Return(0));
        auto res = logFile_->ProduceLog(eventInfo);
        EXPECT_EQ(res, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceLogTest004 failed";
    }
    GTEST_LOG_(INFO) << "ProduceLogTest004 end";
}

/**
 * @tc.name: ProduceLogTest005
 * @tc.desc: Verify the ProduceLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest005 start";
    try {
        logFile_->StopCallback();
        EventInfo eventInfo;
        eventInfo.path = "path";
        eventInfo.name = "f";
        eventInfo.operateType = OperationType::DELETE;
        struct stat statBuf;
        statBuf.st_ino = 1;
        statBuf.st_mode = S_IFDIR;
        EXPECT_CALL(*insMock_, stat(_, _)).WillRepeatedly(DoAll(SetArgPointee<1>(statBuf), Return(0)));
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(1));
        struct stat fstatBuf = {0};
        fstatBuf.st_size = 0;
        EXPECT_CALL(*insMock_, fstat(_, _)).WillRepeatedly(DoAll(SetArgPointee<1>(fstatBuf), Return(0)));
        EXPECT_CALL(*insMock_, ftruncate(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, WriteFile(_, _, _, _)).WillRepeatedly(ReturnArg<2>());
        int32_t res = logFile_->ProduceLog(eventInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceLogTest005 failed";
    }
    GTEST_LOG_(INFO) << "ProduceLogTest005 end";
}

/**
 * @tc.name: ProduceLogTest006
 * @tc.desc: Verify the ProduceLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest006 start";
    try {
        logFile_->StartCallback();
        EventInfo eventInfo;
        eventInfo.path = "path";
        eventInfo.name = "";
        eventInfo.operateType = OperationType::CREATE;
        struct stat parentStat = {0};
        parentStat.st_ino = 0;
        parentStat.st_mode = S_IFDIR;
        struct stat childStat = {0};
        childStat.st_ino = 1;
        childStat.st_mode = S_IFREG;
        EXPECT_CALL(*insMock_, stat(_, _))
            .WillOnce(DoAll(SetArgPointee<1>(parentStat), Return(0)))
            .WillRepeatedly(DoAll(SetArgPointee<1>(childStat), Return(0)));
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(1));
        struct stat fstatBuf = {0};
        fstatBuf.st_size = 0;
        EXPECT_CALL(*insMock_, fstat(_, _)).WillRepeatedly(DoAll(SetArgPointee<1>(fstatBuf), Return(0)));
        EXPECT_CALL(*insMock_, ftruncate(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, WriteFile(_, _, _, _)).WillRepeatedly(ReturnArg<2>());
        int32_t res = logFile_->ProduceLog(eventInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceLogTest006 failed";
    }
    GTEST_LOG_(INFO) << "ProduceLogTest006 end";
}

/**
 * @tc.name: PraseLogTest001
 * @tc.desc: Verify the PraseLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, PraseLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseLogTest001 start";
    try {
        logFile_->currentLine_ = 1;
        uint64_t line = 1;
        ChangeData data;
        bool isEof = false;
        auto res = logFile_->PraseLog(line, data, isEof);
        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(isEof);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseLogTest001 failed";
    }
    GTEST_LOG_(INFO) << "PraseLogTest001 end";
}

/**
 * @tc.name: PraseLogTest002
 * @tc.desc: Verify the PraseLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, PraseLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseLogTest002 start";
    try {
        ChangeData data;
        bool isEof = false;
        logFile_->currentLine_ = 0;
        logFile_->reversal_ = false;
        uint64_t line = 1;
        auto res = logFile_->PraseLog(line, data, isEof);
        EXPECT_EQ(res, E_INVALID_CHANGE_SEQUENCE);
        EXPECT_FALSE(isEof);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseLogTest002 failed";
    }
    GTEST_LOG_(INFO) << "PraseLogTest002 end";
}

/**
 * @tc.name: PraseLogTest003
 * @tc.desc: Verify the PraseLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, PraseLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseLogTest003 start";
    try {
        logFile_->currentLine_ = 1;
        logFile_->reversal_ = false;
        uint64_t line = 0;
        ChangeData data;
        bool isEof = false;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(0));
        auto res = logFile_->PraseLog(line, data, isEof);
        EXPECT_EQ(res, -1);
        EXPECT_FALSE(isEof);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseLogTest003 failed";
    }
    GTEST_LOG_(INFO) << "PraseLogTest003 end";
}

/**
 * @tc.name: PraseLogTest004
 * @tc.desc: Verify the PraseLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, PraseLogTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseLogTest004 start";
    try {
        logFile_->userId_ = 0;
        logFile_->syncFolderIndex_ = 0;
        logFile_->currentLine_ = 10;
        logFile_->reversal_ = false;
        uint64_t line = 1;
        ChangeData data;
        bool isEof = false;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Invoke(
            [](int, off_t, size_t size, void *buf) -> int64_t {
                auto *group = static_cast<LogGroup*>(buf);
                (void)memset_s(group, LOGGROUP_SIZE, 0, LOGGROUP_SIZE);
                group->logBlockCnt = 1;
                group->nsl[1].line = 1;
                group->nsl[1].timestamp = 1;
                group->nsl[1].parentInode = 1;
                group->nsl[1].operationType = static_cast<uint8_t>(OperationType::CREATE);
                group->nsl[1].bidx = 0;
                group->nsl[1].bitPos = 0;
                (void)memset_s(group->nsl[1].parentRecordId, RECORD_ID_LEN, 0x11, RECORD_ID_LEN);
                return static_cast<int64_t>(size);
            }));
        auto res = logFile_->PraseLog(line, data, isEof);
        EXPECT_EQ(res, E_OK);
        EXPECT_FALSE(isEof);
        EXPECT_EQ(data.updateSequenceNumber, 1);
        EXPECT_EQ(data.operationType, OperationType::CREATE);
        EXPECT_EQ(data.timeStamp, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseLogTest004 failed";
    }
    GTEST_LOG_(INFO) << "PraseLogTest004 end";
}

/**
 * @tc.name: PraseLogTest005
 * @tc.desc: Verify the PraseLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, PraseLogTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseLogTest005 start";
    try {
        logFile_->userId_ = 1;
        logFile_->syncFolderIndex_ = 0;
        logFile_->currentLine_ = 10;
        logFile_->reversal_ = false;
        uint64_t line = 1;
        ChangeData data;
        bool isEof = false;
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Invoke(
            [](int, off_t, size_t size, void *buf) -> int64_t {
                auto *group = static_cast<LogGroup*>(buf);
                (void)memset_s(group, LOGGROUP_SIZE, 0, LOGGROUP_SIZE);
                group->logBlockCnt = 1;
                group->nsl[1].line = 1;
                group->nsl[1].timestamp = 1;
                group->nsl[1].parentInode = 1;
                group->nsl[1].operationType = static_cast<uint8_t>(OperationType::CREATE);
                group->nsl[1].bidx = 0;
                group->nsl[1].bitPos = 0;
                (void)memset_s(group->nsl[1].parentRecordId, RECORD_ID_LEN, 0, RECORD_ID_LEN);
                return static_cast<int64_t>(size);
            }));
        auto res = logFile_->PraseLog(line, data, isEof);
        EXPECT_EQ(res, E_OK);
        EXPECT_FALSE(isEof);
        EXPECT_EQ(data.updateSequenceNumber, 1);
        EXPECT_EQ(data.operationType, OperationType::CREATE);
        EXPECT_EQ(data.timeStamp, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseLogTest005 failed";
    }
    GTEST_LOG_(INFO) << "PraseLogTest005 end";
}

/**
 * @tc.name: GetStartLineTest001
 * @tc.desc: Verify the GetStartLine function (currentLine_ <= LOG_COUNT_MAX && !reversal_)
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, GetStartLineTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetStartLineTest001 start";
    try {
        logFile_->currentLine_ = LOG_COUNT_MAX - 1;
        logFile_->reversal_ = false;
        uint64_t res = logFile_->GetStartLine();
        EXPECT_EQ(res, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetStartLineTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetStartLineTest001 end";
}

/**
 * @tc.name: GetStartLineTest002
 * @tc.desc: Verify the GetStartLine function (currentLine_ > LOG_COUNT_MAX && !reversal_)
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, GetStartLineTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetStartLineTest002 start";
    try {
        logFile_->currentLine_ = LOG_COUNT_MAX + 1;
        logFile_->reversal_ = false;
        uint64_t res = logFile_->GetStartLine();
        uint64_t line = logFile_->currentLine_ - 1;
        uint64_t expected = (line & ~LOG_INDEX_MASK) - LOG_COUNT_MAX + LOGBLOCK_PER_GROUP;
        EXPECT_EQ(res, expected);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetStartLineTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetStartLineTest002 end";
}

/**
 * @tc.name: GetStartLineTest003
 * @tc.desc: Verify the GetStartLine function (currentLine_ <= LOG_COUNT_MAX && reversal_)
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, GetStartLineTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetStartLineTest003 start";
    try {
        logFile_->currentLine_ = 0;
        logFile_->reversal_ = true;
        uint64_t res = logFile_->GetStartLine();
        uint64_t line = logFile_->currentLine_ - 1;
        uint64_t expected = (line & ~LOG_INDEX_MASK) - LOG_COUNT_MAX + LOGBLOCK_PER_GROUP;
        EXPECT_EQ(res, expected);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetStartLineTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetStartLineTest003 end";
}

/**
 * @tc.name: CheckLineIsValidTest001
 * @tc.desc: Verify the CheckLineIsValid function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CheckLineIsValidTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLineIsValidTest001 start";
    try {
        logFile_->currentLine_ = LOG_COUNT_MAX + 100;
        logFile_->reversal_ = true;
        uint64_t line = 0;
        EXPECT_FALSE(logFile_->CheckLineIsValid(line));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckLineIsValidTest001 failed";
    }
    GTEST_LOG_(INFO) << "CheckLineIsValidTest001 end";
}

/**
 * @tc.name: CheckLineIsValidTest002
 * @tc.desc: Verify the CheckLineIsValid function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CheckLineIsValidTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLineIsValidTest002 start";
    try {
        logFile_->reversal_ = false;
        logFile_->currentLine_ = 10;
        uint64_t line = 10;
        EXPECT_FALSE(logFile_->CheckLineIsValid(line));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckLineIsValidTest002 failed";
    }
    GTEST_LOG_(INFO) << "CheckLineIsValidTest002 end";
}

/**
 * @tc.name: CheckLineIsValidTest003
 * @tc.desc: Verify the CheckLineIsValid function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CheckLineIsValidTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLineIsValidTest003 start";
    try {
        logFile_->reversal_ = false;
        logFile_->currentLine_ = 10;
        uint64_t line = 5;
        EXPECT_TRUE(logFile_->CheckLineIsValid(line));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckLineIsValidTest003 failed";
    }
    GTEST_LOG_(INFO) << "CheckLineIsValidTest003 end";
}

/**
 * @tc.name: CheckLineIsValidTest004
 * @tc.desc: Verify the CheckLineIsValid function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CheckLineIsValidTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLineIsValidTest004 start";
    try {
        logFile_->reversal_ = false;
        logFile_->currentLine_ = 10;
        uint64_t line = 10;
        EXPECT_FALSE(logFile_->CheckLineIsValid(line));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckLineIsValidTest004 failed";
    }
    GTEST_LOG_(INFO) << "CheckLineIsValidTest004 end";
}

/**
 * @tc.name: CheckLineIsValidTest005
 * @tc.desc: Verify the CheckLineIsValid function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CheckLineIsValidTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLineIsValidTest005 start";
    try {
        logFile_->reversal_ = true;
        logFile_->currentLine_ = LOG_COUNT_MAX;
        uint64_t line = LOG_COUNT_MAX;
        EXPECT_FALSE(logFile_->CheckLineIsValid(line));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckLineIsValidTest005 failed";
    }
    GTEST_LOG_(INFO) << "CheckLineIsValidTest005 end";
}

/**
 * @tc.name: CheckLineIsValidTest006
 * @tc.desc: Verify the CheckLineIsValid function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, CheckLineIsValidTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLineIsValidTest006 start";
    try {
        logFile_->reversal_ = true;
        logFile_->currentLine_ = 100;
        uint64_t startLine = logFile_->GetStartLine();
        uint64_t line = startLine;
        EXPECT_TRUE(logFile_->CheckLineIsValid(line));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckLineIsValidTest006 failed";
    }
    GTEST_LOG_(INFO) << "CheckLineIsValidTest006 end";
}

/**
 * @tc.name: ProductLogForOperateTest001
 * @tc.desc: Verify the ProductLogForOperate function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        OperationType operator1 = OperationType::CREATE;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, ctx, operator1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProductLogForOperateTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProductLogForOperateTest001 end";
}

/**
 * @tc.name: ProductLogForOperateTest002
 * @tc.desc: Verify the ProductLogForOperate function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        OperationType operator1 = OperationType::DELETE;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, ctx, operator1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProductLogForOperateTest002 failed";
    }
    GTEST_LOG_(INFO) << "ProductLogForOperateTest002 end";
}

/**
 * @tc.name: ProductLogForOperateTest003
 * @tc.desc: Verify the ProductLogForOperate function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest003 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        OperationType operator1 = OperationType::MOVE_FROM;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, ctx, operator1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProductLogForOperateTest003 failed";
    }
    GTEST_LOG_(INFO) << "ProductLogForOperateTest003 end";
}

/**
 * @tc.name: ProductLogForOperateTest004
 * @tc.desc: Verify the ProductLogForOperate function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest004 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        OperationType operator1 = OperationType::MOVE_TO;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, ctx, operator1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProductLogForOperateTest004 failed";
    }
    GTEST_LOG_(INFO) << "ProductLogForOperateTest004 end";
}

/**
 * @tc.name: ProductLogForOperateTest005
 * @tc.desc: Verify the ProductLogForOperate function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest005 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        OperationType operator1 = OperationType::CLOSE_WRITE;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, ctx, operator1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProductLogForOperateTest005 failed";
    }
    GTEST_LOG_(INFO) << "ProductLogForOperateTest005 end";
}

/**
 * @tc.name: ProductLogForOperateTest006
 * @tc.desc: Verify the ProductLogForOperate function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest006 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        OperationType operator1 = OperationType::SYNC_FOLDER_INVALID;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, ctx, operator1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProductLogForOperateTest006 failed";
    }
    GTEST_LOG_(INFO) << "ProductLogForOperateTest006 end";
}

/**
 * @tc.name: ProductLogForOperateTest007
 * @tc.desc: Verify the ProductLogForOperate function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest007 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        OperationType operator1 = OperationType::OPERATION_MAX;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, ctx, operator1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProductLogForOperateTest007 failed";
    }
    GTEST_LOG_(INFO) << "ProductLogForOperateTest007 end";
}

/**
 * @tc.name: ProduceCreateLogTest001
 * @tc.desc: Verify the ProduceCreateLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCreateLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCreateLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        struct stat statInfo;
        statInfo.st_mode = S_IFREG;
        EXPECT_CALL(*insMock_, stat(_, _)).Times(AnyNumber())
            .WillRepeatedly(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceCreateLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceCreateLogTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProduceCreateLogTest001 end";
}

/**
 * @tc.name: ProduceCreateLogTest002
 * @tc.desc: Cover ProduceCreateLog stat child failure path (log 077)
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCreateLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCreateLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Invoke([](const char*, struct stat*) {
            errno = ENOENT;
            return -1;
        }));
        auto res = logFile_->ProduceCreateLog(parentMetaFile, path, name, ctx);
        EXPECT_NE(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceCreateLogTest002 failed";
    }
    GTEST_LOG_(INFO) << "ProduceCreateLogTest002 end";
}

/**
 * @tc.name: ProduceCreateLogTest003
 * @tc.desc: Verify the ProduceCreateLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCreateLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCreateLogTest003 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        struct stat statInfo;
        statInfo.st_mode = S_IFREG;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceCreateLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceCreateLogTest003 failed";
    }
    GTEST_LOG_(INFO) << "ProduceCreateLogTest003 end";
}

/**
 * @tc.name: ProduceCreateLogTest004
 * @tc.desc: Verify the ProduceCreateLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCreateLogTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCreateLogTest004 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        struct stat statInfo;
        statInfo.st_mode = S_IFDIR;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceCreateLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceCreateLogTest004 failed";
    }
    GTEST_LOG_(INFO) << "ProduceCreateLogTest004 end";
}

/**
 * @tc.name: ProduceUnlinkLogTest001
 * @tc.desc: Verify the ProduceUnlinkLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceUnlinkLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceUnlinkLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string name = "name";
        struct LogGenerateCtx ctx;
        auto res = logFile_->ProduceUnlinkLog(parentMetaFile, name, ctx);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceUnlinkLogTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProduceUnlinkLogTest001 end";
}

/**
 * @tc.name: ProduceUnlinkLogTest002
 * @tc.desc: Verify the ProduceUnlinkLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceUnlinkLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceUnlinkLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        auto res = logFile_->ProduceUnlinkLog(parentMetaFile, name, ctx);
        EXPECT_EQ(res, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceUnlinkLogTest002 failed";
    }
    GTEST_LOG_(INFO) << "ProduceUnlinkLogTest002 end";
}

/**
 * @tc.name: ProduceRenameOldLogTest001
 * @tc.desc: Verify the ProduceRenameOldLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameOldLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameOldLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string name = "name";
        struct LogGenerateCtx ctx;
        auto res = logFile_->ProduceRenameOldLog(parentMetaFile, name, ctx);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceRenameOldLogTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProduceRenameOldLogTest001 end";
}

/**
 * @tc.name: ProduceRenameOldLogTest002
 * @tc.desc: Verify the ProduceRenameOldLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameOldLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameOldLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        auto res = logFile_->ProduceRenameOldLog(parentMetaFile, name, ctx);
        EXPECT_EQ(res, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceRenameOldLogTest002 failed";
    }
    GTEST_LOG_(INFO) << "ProduceRenameOldLogTest002 end";
}

/**
 * @tc.name: ProduceRenameNewLogTest001
 * @tc.desc: Verify the ProduceRenameNewLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameNewLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        struct LogGenerateCtx ctx;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Invoke([](const char*, struct stat*) {
            errno = ENOENT;
            return -1;
        }));
        auto res = logFile_->ProduceRenameNewLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceRenameNewLogTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest001 end";
}

/**
 * @tc.name: ProduceRenameNewLogTest002
 * @tc.desc: Verify the ProduceRenameNewLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameNewLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        logFile_->renameRecordId_ = "renameRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(0));
        auto res = logFile_->ProduceRenameNewLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceRenameNewLogTest002 failed";
    }
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest002 end";
}

/**
 * @tc.name: ProduceRenameNewLogTest003
 * @tc.desc: Verify the ProduceRenameNewLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameNewLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest003 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        logFile_->renameRecordId_ = "";
        struct stat statInfo;
        statInfo.st_ino = 1;
        statInfo.st_mode = S_IFREG;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceRenameNewLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceRenameNewLogTest003 failed";
    }
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest003 end";
}

/**
 * @tc.name: ProduceRenameNewLogTest004
 * @tc.desc: Verify the ProduceRenameNewLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameNewLogTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest004 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "dir";
        struct LogGenerateCtx ctx;
        ctx.recordId = "";
        logFile_->renameRecordId_ = "";
        struct stat statInfo;
        statInfo.st_ino = 1;
        statInfo.st_mode = S_IFDIR;
        statInfo.st_size = 0;
        statInfo.st_atime = 0;
        statInfo.st_mtime = 0;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceRenameNewLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceRenameNewLogTest004 failed";
    }
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest004 end";
}

/**
 * @tc.name: ProduceCloseAndWriteLogTest001
 * @tc.desc: Verify the ProduceCloseAndWriteLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCloseAndWriteLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return 1;
        });
        auto res = logFile_->ProduceCloseAndWriteLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest001 end";
}

/**
 * @tc.name: ProduceCloseAndWriteLogTest002
 * @tc.desc: Verify the ProduceCloseAndWriteLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCloseAndWriteLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(0));
        auto res = logFile_->ProduceCloseAndWriteLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest002 failed";
    }
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest002 end";
}

/**
 * @tc.name: ProduceCloseAndWriteLogTest003
 * @tc.desc: Verify the ProduceCloseAndWriteLog function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCloseAndWriteLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest003 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile = make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "";
        struct LogGenerateCtx ctx;
        ctx.recordId = childRecordId;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(Return(0));
        auto res = logFile_->ProduceCloseAndWriteLog(parentMetaFile, path, name, ctx);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest003 failed";
    }
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest003 end";
}

class LogFileMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<LogFileMgr> logFileMgr_;
    static inline shared_ptr<AssistantMock> insMock_;
};

void LogFileMgrTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void LogFileMgrTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void LogFileMgrTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    logFileMgr_ = make_shared<LogFileMgr>();
    insMock_->EnableMock();
}

void LogFileMgrTest::TearDown()
{
    insMock_->DisableMock();
    logFileMgr_ = nullptr;
    Assistant::ins = nullptr;
    insMock_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 start";
    try {
        LogFileMgr &instance1 = LogFileMgr::GetInstance();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 end";
}

/**
 * @tc.name: ProduceRequestTest001
 * @tc.desc: Verify the ProduceRequest function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, ProduceRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRequestTest001 start";
    try {
        EventInfo eventInfo;
        auto res = logFileMgr_->ProduceRequest(eventInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceRequestTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProduceRequestTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderTest001
 * @tc.desc: Verify the RegisterSyncFolder function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, RegisterSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        string path = "path";
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return 1;
        });
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock_, ftruncate(_, _)).WillRepeatedly(Return(0));

        auto res = logFileMgr_->RegisterSyncFolder(userId, syncFolderIndex, path);
        EXPECT_EQ(res, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderTest002
 * @tc.desc: Verify the RegisterSyncFolder function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, RegisterSyncFolderTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderTest002 start";
    try {
        int32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        string path = "path";
        struct stat st;
        st.st_ino = 1;
        EXPECT_CALL(*insMock_, stat(_, _)).WillOnce(DoAll(SetArgPointee<1>(st), Return(0)));
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock_, ftruncate(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, opendir(_)).WillRepeatedly(Return(nullptr));
        auto res = logFileMgr_->RegisterSyncFolder(userId, syncFolderIndex, path);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderTest002 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderTest002 end";
}

/**
 * @tc.name: UnRegisterSyncFolderTest001
 * @tc.desc: Verify the UnRegisterSyncFolder function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, UnRegisterSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 start";
    try {
        EXPECT_CALL(*insMock_, stat(_, _)).Times(AnyNumber()).WillRepeatedly(Return(0));
        int32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        auto res = logFileMgr_->UnRegisterSyncFolder(userId, syncFolderIndex);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 end";
}

/**
 * @tc.name: CloudDiskServiceClearAllTest001
 * @tc.desc: Verify the CloudDiskServiceClearAll function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, CloudDiskServiceClearAllTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskServiceClearAllTest001 start";
    try {
        LogFileMgr::GetInstance().CloudDiskServiceClearAll();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskServiceClearAllTest001 failed";
    }
    GTEST_LOG_(INFO) << "CloudDiskServiceClearAllTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesTest001
 * @tc.desc: Verify the RegisterSyncFolderChanges function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, RegisterSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        logFileMgr_->registerChangeCount_ = 0;
        logFileMgr_->RegisterSyncFolderChanges(userId, syncFolderIndex);
        EXPECT_EQ(logFileMgr_->registerChangeCount_, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesTest002
 * @tc.desc: Verify the RegisterSyncFolderChanges function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, RegisterSyncFolderChangesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest002 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        logFileMgr_->registerChangeCount_ = 1;
        logFileMgr_->RegisterSyncFolderChanges(userId, syncFolderIndex);
        EXPECT_EQ(logFileMgr_->registerChangeCount_, 2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest002 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest002 end";
}

/**
 * @tc.name: UnRegisterSyncFolderChangesTest001
 * @tc.desc: Verify the UnRegisterSyncFolderChanges function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, UnRegisterSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        logFileMgr_->registerChangeCount_ = 1;
        logFileMgr_->UnRegisterSyncFolderChanges(userId, syncFolderIndex);
        EXPECT_EQ(logFileMgr_->registerChangeCount_, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest001 end";
}

/**
 * @tc.name: UnRegisterSyncFolderChangesTest002
 * @tc.desc: Verify the UnRegisterSyncFolderChanges function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, UnRegisterSyncFolderChangesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest002 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        logFileMgr_->registerChangeCount_ = 2;
        logFileMgr_->UnRegisterSyncFolderChanges(userId, syncFolderIndex);
        EXPECT_EQ(logFileMgr_->registerChangeCount_, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest002 end";
}

/**
 * @tc.name: PraseRequestTest001
 * @tc.desc: Verify the PraseRequest function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, PraseRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseRequestTest001 start";
    try {
        int32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        uint64_t start = 0;
        uint64_t count = 1;
        ChangesResult changesResult;
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(0));
        int32_t ret = logFileMgr_->PraseRequest(userId, syncFolderIndex, start, count, changesResult);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(changesResult.nextUsn, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseRequestTest001 failed";
    }
    GTEST_LOG_(INFO) << "PraseRequestTest001 end";
}

/**
 * @tc.name: PraseRequestTest002
 * @tc.desc: Verify the PraseRequest function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, PraseRequestTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseRequestTest002 start";
    try {
        int32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        uint64_t start = 1;
        uint64_t count = 1;
        ChangesResult changeResult;
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Invoke(
            [](int, off_t, size_t size, void *buf) -> int64_t {
                if (buf == nullptr) return -1;
                auto *group = static_cast<LogGroup*>(buf);
                (void)memset_s(group, LOGGROUP_SIZE, 0, LOGGROUP_SIZE);
                group->logBlockCnt = 1;
                group->nsl[0].line = 1;
                group->nsl[0].timestamp = 1;
                group->nsl[0].parentInode = 1;
                return static_cast<int64_t>(size);
            }));
        int32_t ret = logFileMgr_->PraseRequest(userId, syncFolderIndex, start, count, changeResult);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseRequestTest002 failed";
    }
    GTEST_LOG_(INFO) << "PraseRequestTest002 end";
}

/**
 * @tc.name: PraseRequestTest003
 * @tc.desc: Verify the PraseRequest function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, PraseRequestTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseRequestTest003 start";
    try {
        int32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        uint64_t start = 1;
        uint64_t count = 1;
        ChangesResult changeResult;
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(-1));
        int32_t ret = logFileMgr_->PraseRequest(userId, syncFolderIndex, start, count, changeResult);
        EXPECT_NE(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseRequestTest003 failed";
    }
    GTEST_LOG_(INFO) << "PraseRequestTest003 end";
}

/**
 * @tc.name: GetCloudDiskServiceLogFileTest001
 * @tc.desc: Verify the GetCloudDiskServiceLogFile function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, GetCloudDiskServiceLogFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskServiceLogFileTest001 start";
    try {
        logFileMgr_->LogFiles_.clear();
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        auto res = logFileMgr_->GetCloudDiskServiceLogFile(userId, syncFolderIndex);
        EXPECT_EQ(res->userId_, userId);
        EXPECT_EQ(res->syncFolderIndex_, syncFolderIndex);
        EXPECT_FALSE(logFileMgr_->LogFiles_.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskServiceLogFileTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskServiceLogFileTest001 end";
}

/**
 * @tc.name: GetCloudDiskServiceLogFileTest002
 * @tc.desc: Verify the GetCloudDiskServiceLogFile function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, GetCloudDiskServiceLogFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskServiceLogFileTest002 start";
    try {
        logFileMgr_->LogFiles_.clear();
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        LogFileKey key(userId, syncFolderIndex);
        shared_ptr<CloudDiskServiceLogFile> newLogFile = make_shared<CloudDiskServiceLogFile>(0, 0);
        logFileMgr_->LogFiles_[key] = newLogFile;
        auto res = logFileMgr_->GetCloudDiskServiceLogFile(userId, syncFolderIndex);
        EXPECT_EQ(res, newLogFile);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskServiceLogFileTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskServiceLogFileTest002 end";
}

/**
 * @tc.name: CallBackTest001
 * @tc.desc: Verify the CallBack function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, CallBackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CallBackTest001 start";
    try {
        logFileMgr_->CallBack(nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CallBackTest001 failed";
    }
    GTEST_LOG_(INFO) << "CallBackTest001 end";
}

/**
 * @tc.name: OnDataChangeTest001
 * @tc.desc: Verify the OnDataChange function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, OnDataChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDataChangeTest001 start";
    try {
        logFileMgr_->CloudDiskServiceClearAll();
        auto res = logFileMgr_->OnDataChange();
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDataChangeTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnDataChangeTest001 end";
}

/**
 * @tc.name: OnDataChangeTest002
 * @tc.desc: Verify the OnDataChange function
 * @tc.type: FUNC
 */
HWTEST_F(LogFileMgrTest, OnDataChangeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDataChangeTest002 start";
    try {
        int32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        EXPECT_CALL(*insMock_, access(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock_, ftruncate(_, _)).WillRepeatedly(Return(0));
        logFileMgr_->GetCloudDiskServiceLogFile(userId, syncFolderIndex);
        auto res = logFileMgr_->OnDataChange();
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDataChangeTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnDataChangeTest002 end";
}

} // namespace OHOS::FileManagement::CloudDiskService::Test