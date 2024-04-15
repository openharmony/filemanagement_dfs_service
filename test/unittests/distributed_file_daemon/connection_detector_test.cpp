/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "connection_detector.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

constexpr int32_t E_OK = 0;
constexpr int32_t E_Hash_Value = 3556498;
constexpr int32_t E_ERR = -1;

class ConnectionDetectorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ConnectionDetectorTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void ConnectionDetectorTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void ConnectionDetectorTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ConnectionDetectorTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(ConnectionDetectorTest, DfsService_GetConnectionStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetConnectionStatus_001_Start";
    string targetDir = "/test/file";
    string networkId = "test$file@txt";
    bool ret = ConnectionDetector::GetConnectionStatus(targetDir, networkId);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "DfsService_GetConnectionStatus_001_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_GetConnectionStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetConnectionStatus_002_Start";
    string targetDir;
    string networkId;
    bool ret = ConnectionDetector::GetConnectionStatus(targetDir, networkId);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "DfsService_GetConnectionStatus_002_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_MocklispHash_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_MocklispHash_001_Start";
    string str;
    uint64_t ret = ConnectionDetector::MocklispHash(str);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DfsService_MocklispHash_001_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_MocklispHash_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_MocklispHash_002_Start";
    string str = "test";
    uint64_t ret = ConnectionDetector::MocklispHash(str);
    EXPECT_EQ(ret, E_Hash_Value);
    GTEST_LOG_(INFO) << "DfsService_MocklispHash_002_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_ParseHmdfsPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_ParseHmdfsPath_001_Start";
    EXPECT_EQ("/mnt/hmdfs/100/account", ConnectionDetector::ParseHmdfsPath());
    GTEST_LOG_(INFO) << "DfsService_ParseHmdfsPath_001_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_RepeatGetConnectionStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_RepeatGetConnectionStatus_001_Start";
    string targetDir;
    string networkId;
    int32_t ret = ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId);
    EXPECT_EQ(ret, E_ERR);
    GTEST_LOG_(INFO) << "DfsService_RepeatGetConnectionStatus_001_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_RepeatGetConnectionStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_RepeatGetConnectionStatus_002_Start";
    string targetDir = "testDir";
    string networkId = "test@4#";
    int32_t ret = ConnectionDetector::RepeatGetConnectionStatus(targetDir, networkId);
    EXPECT_EQ(ret, E_ERR);
    GTEST_LOG_(INFO) << "DfsService_RepeatGetConnectionStatus_002_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_GetCellByIndex_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetCellByIndex_001_Start";
    string str;
    int targetIndex = 0;
    ConnectionDetector connectionDetector;
    string ret = connectionDetector.GetCellByIndex(str, targetIndex);
    EXPECT_EQ(ret, "");
    GTEST_LOG_(INFO) << "DfsService_GetCellByIndex_001_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_GetCellByIndex_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetCellByIndex_002_Start";
    string str = "test  %/ lld";
    int targetIndex = 0;
    ConnectionDetector connectionDetector;
    string ret = connectionDetector.GetCellByIndex(str, targetIndex);
    EXPECT_EQ(ret, "lld");
    GTEST_LOG_(INFO) << "DfsService_GetCellByIndex_002_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_GetCellByIndex_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetCellByIndex_003_Start";
    string str = "test  %/ lld test %//d    txt jpg";
    int targetIndex = 0;
    ConnectionDetector connectionDetector;
    string ret = connectionDetector.GetCellByIndex(str, targetIndex);
    EXPECT_EQ(ret, "jpg");
    GTEST_LOG_(INFO) << "DfsService_GetCellByIndex_003_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_GetCellByIndex_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetCellByIndex_004_Start";
    string str = "       test";
    int targetIndex = 0;
    ConnectionDetector connectionDetector;
    string ret = connectionDetector.GetCellByIndex(str, targetIndex);
    EXPECT_EQ(ret, "test");
    GTEST_LOG_(INFO) << "DfsService_GetCellByIndex_004_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_MatchConnectionStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_MatchConnectionStatus_001_Start";
    ifstream inputFile;
    ConnectionDetector connectionDetector;
    bool ret = connectionDetector.MatchConnectionStatus(inputFile);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "DfsService_MatchConnectionStatus_001_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_MatchConnectionStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_MatchConnectionStatus_002_Start";
    ifstream inputFile("testName");
    ConnectionDetector connectionDetector;
    bool ret = connectionDetector.MatchConnectionStatus(inputFile);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "DfsService_MatchConnectionStatus_002_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_MatchConnectionGroup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_MatchConnectionGroup_001_Start";
    string fileName;
    string networkId;
    ConnectionDetector connectionDetector;
    bool ret = connectionDetector.MatchConnectionGroup(fileName, networkId);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "DfsService_MatchConnectionGroup_001_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_MatchConnectionGroup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_MatchConnectionGroup_002_Start";
    string fileName = "testName";
    string networkId = "test@txt#1";
    ConnectionDetector connectionDetector;
    bool ret = connectionDetector.MatchConnectionGroup(fileName, networkId);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "DfsService_MatchConnectionGroup_002_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_CheckValidDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_CheckValidDir_001_Start";
    string path;
    ConnectionDetector connectionDetector;
    bool ret = connectionDetector.CheckValidDir(path);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "DfsService_CheckValidDir_001_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_CheckValidDir_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_CheckValidDir_002_Start";
    string path = "test@txt#1";
    ConnectionDetector connectionDetector;
    bool ret = connectionDetector.CheckValidDir(path);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "DfsService_CheckValidDir_002_End";
}

HWTEST_F(ConnectionDetectorTest, DfsService_GetCurrentUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetCurrentUserId_001_Start";
    int32_t ret = ConnectionDetector::GetCurrentUserId();
    EXPECT_EQ(ret, 100);
    GTEST_LOG_(INFO) << "DfsService_GetCurrentUserId_001_End";
}
}