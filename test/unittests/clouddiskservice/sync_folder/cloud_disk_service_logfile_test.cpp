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

#include "cloud_disk_service_logfile.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assistant.h"
#include "cloud_disk_service_manager_mock.h"

#define stat(path, buf) OHOS::FileManagement::CloudDiskService::Assistant::ins->MockStat(path, buf)
#define access OHOS::FileManagement::CloudDiskService::Assistant::ins->access
#include "cloud_disk_service_logfile.cpp"
#undef stat
#undef access

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
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    logFile_ = make_shared<CloudDiskServiceLogFile>(0, 0);
}

void CloudDiskServiceLogFileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    logFile_ = nullptr;
    Assistant::ins = nullptr;
    insMock_ = nullptr;
}

void CloudDiskServiceLogFileTest::SetUp()
{
}

void CloudDiskServiceLogFileTest::TearDown()
{
}

/**
 * @tc.name: WriteLogFileTest001
 * @tc.desc: Verify the WriteLogFile function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, WriteLogFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteLogFileTest001 start";
    try {
        LogBlock logBlock;
        EXPECT_CALL(*insMock_, FilePosLock(_, _, _, _)).WillOnce(Return(1));
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, WriteLogFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteLogFileTest002 start";
    try {
        LogBlock logBlock;
        EXPECT_CALL(*insMock_, FilePosLock(_, _, _, _)).Times(2).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillOnce(Return(4096));
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ReadLogFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLogFileTest001 start";
    try {
        uint64_t line = 1;
        LogBlock logBlock;
        EXPECT_CALL(*insMock_, FilePosLock(_, _, _, _)).WillOnce(Return(1));
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ReadLogFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadLogFileTest002 start";
    try {
        uint64_t line = 1;
        LogBlock logBlock;
        EXPECT_CALL(*insMock_, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, ReadFile(_, _, _, _)).WillRepeatedly(Return(4096));
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
 * @tc.require: NA
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
 * @tc.require: NA
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
 * @tc.desc: Verify the CloudDiskServiceLogFile function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, CloudDiskServiceLogFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 2;
        EXPECT_CALL(*insMock_, access(_, _)).WillOnce(Return(0));
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
 * @tc.desc: Verify the CloudDiskServiceLogFile function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, CloudDiskServiceLogFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskServiceLogFileTest001 start";
    try {
        uint32_t userId = 3;
        uint32_t syncFolderIndex = 4;
        EXPECT_CALL(*insMock_, access(_, _)).WillOnce(Return(1));
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
 * @tc.name: GenerateLogBlockTest001
 * @tc.desc: Verify the GenerateLogBlock function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, GenerateLogBlockTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateLogBlockTest001 start";
    try {
        EventInfo eventInfo;
        uint64_t parentInode = 1;
        string childRecordId = "childRecordId";
        string parentRecordId = "parentRecordId";
        uint64_t line = 1;
        int32_t res =
            logFile_->GenerateLogBlock(eventInfo, parentInode, childRecordId, parentRecordId, line);
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
 * @tc.require: NA
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
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(Return(1));
        int32_t res =
            logFile_->GenerateChangeData(eventInfo, line, childRecordId, parentRecordId);
        EXPECT_EQ(res, E_OK);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, GenerateChangeDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateChangeDataTest002 start";
    try {
        EventInfo eventInfo;
        uint64_t line = 1;
        string childRecordId = "childRecordId";
        string parentRecordId = "parentRecordId";
        logFile_->changeDatas_.clear();
        for (int i = 0; i < MAX_CHANGEDATAS_SIZE; i++) {
            ChangeData changeData;
            logFile_->changeDatas_.push_back(changeData);
        }
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(Return(0));
        int32_t res =
            logFile_->GenerateChangeData(eventInfo, line, childRecordId, parentRecordId);
        EXPECT_EQ(res, E_OK);
        EXPECT_TRUE(logFile_->changeDatas_.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateChangeDataTest002 failed";
    }
    GTEST_LOG_(INFO) << "GenerateChangeDataTest002 end";
}

/**
 * @tc.name: FillChildForDirTest001
 * @tc.desc: Verify the FillChildForDir function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, FillChildForDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillChildForDirTest002 start";
    try {
        string path = "path";
        uint64_t timestamp = 1;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return 1;
        });
        int32_t res =
            logFile_->FillChildForDir(path, timestamp);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, FillChildForDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillChildForDirTest002 start";
    try {
        string path = "path";
        uint64_t timestamp = 1;
        struct stat statInfo;
        statInfo.st_mode = S_IFREG;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        int32_t res =
            logFile_->FillChildForDir(path, timestamp);
        EXPECT_EQ(res, E_OK);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, FillChildForDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillChildForDirTest003 start";
    try {
        string path = "path";
        uint64_t timestamp = 1;
        struct stat statInfo;
        statInfo.st_mode = S_IFDIR;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        EXPECT_CALL(*insMock_, opendir(_)).WillRepeatedly([&]() {
            errno = ENOENT;
            return nullptr;
        });
        int32_t res =
            logFile_->FillChildForDir(path, timestamp);
        EXPECT_EQ(res, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillChildForDirTest003 failed";
    }
    GTEST_LOG_(INFO) << "FillChildForDirTest003 end";
}

/**
 * @tc.name: StartCallbackTest001
 * @tc.desc: Verify the StartCallback function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, StartCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartCallbackTest001 start";
    try {
        logFile_->StartCallback();
        EXPECT_TRUE(logFile_->needCallback_);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartCallbackTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartCallbackTest001 end";
}

/**
 * @tc.name: StopCallbackTest001
 * @tc.desc: Verify the StopCallback function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, StopCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopCallbackTest001 start";
    try {
        logFile_->StopCallback();
        EXPECT_FALSE(logFile_->needCallback_);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopCallbackTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopCallbackTest001 end";
}

/**
 * @tc.name: ProduceLogTest001
 * @tc.desc: Verify the ProduceLog function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest001 start";
    try {
        EventInfo eventInfo;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce([&]() {
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest002 start";
    try {
        EventInfo eventInfo;
        eventInfo.operateType = OperationType::SYNC_FOLDER_INVALID;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(Return(0));
        CloudDiskServiceManagerMock& mock = CloudDiskServiceManagerMock::GetInstance();
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest003 start";
    try {
        EventInfo eventInfo;
        eventInfo.operateType = OperationType::SYNC_FOLDER_INVALID;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(Return(0));
        CloudDiskServiceManagerMock& mock = CloudDiskServiceManagerMock::GetInstance();
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceLogTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceLogTest004 start";
    try {
        EventInfo eventInfo;
        eventInfo.operateType = OperationType::OPERATION_MAX;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillRepeatedly(Return(0));
        auto res = logFile_->ProduceLog(eventInfo);
        EXPECT_EQ(res, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceLogTest004 failed";
    }
    GTEST_LOG_(INFO) << "ProduceLogTest004 end";
}

/**
 * @tc.name: PraseLogTest001
 * @tc.desc: Verify the PraseLog function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, PraseLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseLogTest001 start";
    try {
        logFile_->currentLine_ = 1;
        uint64_t line = 1;
        ChangeData data;
        bool isEof = true;
        auto res = logFile_->PraseLog(line, data, isEof);
        EXPECT_EQ(res, E_OK);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, PraseLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseLogTest002 start";
    try {
        logFile_->currentLine_ = 0;
        uint64_t line = 1;
        ChangeData data;
        bool isEof = true;
        auto res = logFile_->PraseLog(line, data, isEof);
        EXPECT_EQ(res, E_INVALID_CHANGE_SEQUENCE);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, PraseLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseLogTest003 start";
    try {
        logFile_->userId_ = 1;
        logFile_->currentLine_ = 1;
        uint64_t line = 1;
        ChangeData data;
        bool isEof = true;
        auto res = logFile_->PraseLog(line, data, isEof);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseLogTest003 failed";
    }
    GTEST_LOG_(INFO) << "PraseLogTest003 end";
}

/**
 * @tc.name: ProductLogForOperateTest001
 * @tc.desc: Verify the ProductLogForOperate function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        OperationType operator1 = OperationType::CREATE;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, childRecordId, operator1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        OperationType operator1 = OperationType::DELETE;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, childRecordId, operator1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest003 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        OperationType operator1 = OperationType::MOVE_FROM;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, childRecordId, operator1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest004 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        OperationType operator1 = OperationType::MOVE_TO;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, childRecordId, operator1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest005 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        OperationType operator1 = OperationType::CLOSE_WRITE;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, childRecordId, operator1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest006 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        OperationType operator1 = OperationType::SYNC_FOLDER_INVALID;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, childRecordId, operator1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProductLogForOperateTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProductLogForOperateTest007 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        OperationType operator1 = OperationType::OPERATION_MAX;
        logFile_->ProductLogForOperate(parentMetaFile, path, name, childRecordId, operator1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCreateLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCreateLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return 1;
        });
        auto res = logFile_->ProduceCreateLog(parentMetaFile, path, name, childRecordId);
        EXPECT_EQ(res, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ProduceCreateLogTest001 failed";
    }
    GTEST_LOG_(INFO) << "ProduceCreateLogTest001 end";
}

/**
 * @tc.name: ProduceCreateLogTest002
 * @tc.desc: Verify the ProduceCreateLog function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCreateLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCreateLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(Return(0));
        auto res = logFile_->ProduceCreateLog(parentMetaFile, path, name, childRecordId);
        EXPECT_EQ(res, -1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCreateLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCreateLogTest003 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "";
        string childRecordId = "childRecordId";
        struct stat statInfo;
        statInfo.st_ino = S_IFREG;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceCreateLog(parentMetaFile, path, name, childRecordId);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCreateLogTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCreateLogTest004 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "";
        string childRecordId = "childRecordId";
        struct stat statInfo;
        statInfo.st_ino = S_IFDIR;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceCreateLog(parentMetaFile, path, name, childRecordId);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceUnlinkLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceUnlinkLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string name = "name";
        string childRecordId = "childRecordId";
        auto res = logFile_->ProduceUnlinkLog(parentMetaFile, name, childRecordId);
        EXPECT_EQ(res, -1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceUnlinkLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceUnlinkLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string name = "name";
        string childRecordId = "";
        auto res = logFile_->ProduceUnlinkLog(parentMetaFile, name, childRecordId);
        EXPECT_EQ(res, E_OK);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameOldLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameOldLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string name = "name";
        string childRecordId = "childRecordId";
        auto res = logFile_->ProduceRenameOldLog(parentMetaFile, name, childRecordId);
        EXPECT_EQ(res, -1);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameOldLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameOldLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string name = "name";
        string childRecordId = "";
        auto res = logFile_->ProduceRenameOldLog(parentMetaFile, name, childRecordId);
        EXPECT_EQ(res, E_OK);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameNewLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return 1;
        });
        auto res = logFile_->ProduceRenameNewLog(parentMetaFile, path, name, childRecordId);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameNewLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        logFile_->renameRecordId_ = "renameRecordId";
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(Return(0));
        auto res = logFile_->ProduceRenameNewLog(parentMetaFile, path, name, childRecordId);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameNewLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest003 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "";
        logFile_->renameRecordId_ = "";
        struct stat statInfo;
        statInfo.st_ino = S_IFREG;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceRenameNewLog(parentMetaFile, path, name, childRecordId);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceRenameNewLogTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceRenameNewLogTest004 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "";
        struct stat statInfo;
        statInfo.st_ino = S_IFDIR;
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(DoAll(SetArgPointee<1>(statInfo), Return(0)));
        auto res = logFile_->ProduceRenameNewLog(parentMetaFile, path, name, childRecordId);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCloseAndWriteLogTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest001 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return 1;
        });
        auto res = logFile_->ProduceCloseAndWriteLog(parentMetaFile, path, name, childRecordId);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCloseAndWriteLogTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest002 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "childRecordId";
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(Return(0));
        auto res = logFile_->ProduceCloseAndWriteLog(parentMetaFile, path, name, childRecordId);
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
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceLogFileTest, ProduceCloseAndWriteLogTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProduceCloseAndWriteLogTest003 start";
    try {
        shared_ptr<CloudDiskServiceMetaFile> parentMetaFile =
            make_shared<CloudDiskServiceMetaFile>(0, 0, 0);
        string path = "path";
        string name = "name";
        string childRecordId = "";
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce(Return(0));
        auto res = logFile_->ProduceCloseAndWriteLog(parentMetaFile, path, name, childRecordId);
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
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    logFileMgr_ = make_shared<LogFileMgr>();
}

void LogFileMgrTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    logFileMgr_ = nullptr;
    Assistant::ins = nullptr;
    insMock_ = nullptr;
}

void LogFileMgrTest::SetUp()
{
}

void LogFileMgrTest::TearDown()
{
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 start";
    try {
        LogFileMgr& instance1 = LogFileMgr::GetInstance();
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
 * @tc.require: NA
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
 * @tc.name: PraseRequestTest001
 * @tc.desc: Verify the PraseRequest function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, PraseRequestTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PraseRequestTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 1;
        uint64_t start = 1;
        uint64_t count = 1;
        ChangesResult changeResult;
        logFileMgr_->PraseRequest(userId, syncFolderIndex, start, count, changeResult);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PraseRequestTest001 failed";
    }
    GTEST_LOG_(INFO) << "PraseRequestTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderTest001
 * @tc.desc: Verify the RegisterSyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, RegisterSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 2;
        string path = "path";
        EXPECT_CALL(*insMock_, MockStat(_, _)).WillOnce([&]() {
            errno = ENOENT;
            return 1;
        });
        auto res = logFileMgr_->RegisterSyncFolder(userId, syncFolderIndex, path);
        EXPECT_EQ(res, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderTest001 end";
}

/**
 * @tc.name: UnRegisterSyncFolderTest001
 * @tc.desc: Verify the UnRegisterSyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, UnRegisterSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 2;
        auto res = logFileMgr_->UnRegisterSyncFolder(userId, syncFolderIndex);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderChangesTest001
 * @tc.desc: Verify the RegisterSyncFolderChanges function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, RegisterSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 2;
        logFileMgr_->registerChangeCount_ = 0;
        logFileMgr_->RegisterSyncFolderChanges(userId, syncFolderIndex);
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
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, RegisterSyncFolderChangesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderChangesTest002 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 2;
        logFileMgr_->registerChangeCount_ = 1;
        logFileMgr_->RegisterSyncFolderChanges(userId, syncFolderIndex);
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
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, UnRegisterSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest001 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 2;
        logFileMgr_->registerChangeCount_ = 1;
        logFileMgr_->UnRegisterSyncFolderChanges(userId, syncFolderIndex);
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
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, UnRegisterSyncFolderChangesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest002 start";
    try {
        uint32_t userId = 1;
        uint32_t syncFolderIndex = 2;
        logFileMgr_->registerChangeCount_ = 2;
        logFileMgr_->UnRegisterSyncFolderChanges(userId, syncFolderIndex);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnRegisterSyncFolderChangesTest002 end";
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
        uint32_t syncFolderIndex = 2;
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
        uint32_t userId = 3;
        uint32_t syncFolderIndex = 4;
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
 * @tc.require: NA
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
 * @tc.require: NA
 */
HWTEST_F(LogFileMgrTest, OnDataChangeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDataChangeTest001 start";
    try {
        auto res = logFileMgr_->OnDataChange();
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDataChangeTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnDataChangeTest001 end";
}
}