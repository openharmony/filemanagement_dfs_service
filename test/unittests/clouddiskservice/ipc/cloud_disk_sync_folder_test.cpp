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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_disk_sync_folder.h"

#include "assistant.h"
#include "cloud_disk_sync_folder_system_mock.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace std;
using namespace testing;
using namespace testing::ext;

class CloudDiskSyncFolderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AssistantMock> insMock;
    static inline shared_ptr<SyncFolderAssistantMock> syncFolderMock;
};

void CloudDiskSyncFolderTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskSyncFolderTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskSyncFolderTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
    syncFolderMock = make_shared<SyncFolderAssistantMock>();
    SyncFolderAssistant::ins = syncFolderMock;
}

void CloudDiskSyncFolderTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    Assistant::ins = nullptr;
    insMock = nullptr;
    SyncFolderAssistant::ins = nullptr;
    syncFolderMock = nullptr;
}

/**
 * @tc.name: GetCloudDiskSyncFolderInstanceTest001
 * @tc.desc: Verify the GetCloudDiskSyncFolderInstance function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetCloudDiskSyncFolderInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskSyncFolderInstanceTest001 start";
    try {
        CloudDiskSyncFolder& instance1 = CloudDiskSyncFolder::GetInstance();
        CloudDiskSyncFolder& instance2 = CloudDiskSyncFolder::GetInstance();
        EXPECT_EQ(&instance1, &instance2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskSyncFolderInstanceTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskSyncFolderInstanceTest001 end";
}

/**
 * @tc.name: AddSyncFolderTest001
 * @tc.desc: Verify the AddSyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, AddSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddSyncFolderTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex = 12345;
        SyncFolderValue value;
        value.bundleName = "com.test.example";
        value.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        EXPECT_EQ(syncFolder.GetSyncFolderSize(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "AddSyncFolderTest001 end";
}

/**
 * @tc.name: DeleteSyncFolderTest001
 * @tc.desc: Verify the DeleteSyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, DeleteSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteSyncFolderTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex = 12345;
        SyncFolderValue value;
        value.bundleName = "com.test.example";
        value.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        EXPECT_TRUE(syncFolder.CheckSyncFolder(syncFolderIndex));
        EXPECT_EQ(syncFolder.GetSyncFolderSize(), 1);

        syncFolder.DeleteSyncFolder(syncFolderIndex);
        EXPECT_FALSE(syncFolder.CheckSyncFolder(syncFolderIndex));
        EXPECT_EQ(syncFolder.GetSyncFolderSize(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "DeleteSyncFolderTest001 end";
}

/**
 * @tc.name: GetSyncFolderSizeTest001
 * @tc.desc: Verify the GetSyncFolderSize function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetSyncFolderSizeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderSizeTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex1 = 12345;
        SyncFolderValue value1;
        value1.bundleName = "com.test.example";
        value1.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex1, value1);

        uint32_t syncFolderIndex2 = 67890;
        SyncFolderValue value2;
        value2.bundleName = "com.test.example";
        value2.path = "/test456";
        syncFolder.AddSyncFolder(syncFolderIndex2, value2);
        EXPECT_EQ(syncFolder.GetSyncFolderSize(), 2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderSizeTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderSizeTest001 end";
}

/**
 * @tc.name: CheckSyncFolderTest001
 * @tc.desc: Verify the CheckSyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, CheckSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSyncFolderTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex = 12345;
        SyncFolderValue value;
        value.bundleName = "com.test.example";
        value.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        bool ret = syncFolder.CheckSyncFolder(syncFolderIndex);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "CheckSyncFolderTest001 end";
}

/**
 * @tc.name: CheckSyncFolderTest002
 * @tc.desc: Verify the CheckSyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, CheckSyncFolderTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSyncFolderTest002 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex = 12345;
        SyncFolderValue value;
        value.bundleName = "com.test.example";
        value.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        uint32_t syncFolderIndex_notExists = 999;
        bool ret = syncFolder.CheckSyncFolder(syncFolderIndex_notExists);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckSyncFolderTest002 failed";
    }
    GTEST_LOG_(INFO) << "CheckSyncFolderTest002 end";
}

/**
 * @tc.name: GetSyncFolderByIndexTest001
 * @tc.desc: Verify the GetSyncFolderByIndex function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetSyncFolderByIndexTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderByIndexTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex = 12345;
        SyncFolderValue value;
        value.bundleName = "com.test.example";
        value.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        string path;
        bool ret = syncFolder.GetSyncFolderByIndex(syncFolderIndex, path);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(path, "/test123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderByIndexTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderByIndexTest001 end";
}

/**
 * @tc.name: GetSyncFolderByIndexTest002
 * @tc.desc: Verify the GetSyncFolderByIndex function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetSyncFolderByIndexTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderByIndexTest002 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex = 12345;
        SyncFolderValue value;
        value.bundleName = "com.test.example";
        value.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        string path;
        uint32_t syncFolderIndex_notExists = 999;
        bool ret = syncFolder.GetSyncFolderByIndex(syncFolderIndex_notExists, path);
        EXPECT_EQ(ret, false);
        EXPECT_EQ(path, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderByIndexTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderByIndexTest002 end";
}

/**
 * @tc.name: GetIndexBySyncFolderTest001
 * @tc.desc: Verify the GetIndexBySyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetIndexBySyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex = 12345;
        SyncFolderValue value;
        value.bundleName = "com.test.example";
        value.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        uint32_t index = 0;
        bool ret = syncFolder.GetIndexBySyncFolder(index, "/test123");
        EXPECT_EQ(ret, true);
        EXPECT_EQ(index, syncFolderIndex);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest001 end";
}

/**
 * @tc.name: GetIndexBySyncFolderTest002
 * @tc.desc: Verify the GetIndexBySyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetIndexBySyncFolderTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest002 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex = 12345;
        SyncFolderValue value;
        value.bundleName = "com.test.example";
        value.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        uint32_t index = 0;
        bool ret = syncFolder.GetIndexBySyncFolder(index, "/not_exists");
        EXPECT_EQ(ret, false);
        EXPECT_EQ(index, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest002 end";
}

/**
 * @tc.name: GetIndexBySyncFolderTest003
 * @tc.desc: Verify the GetIndexBySyncFolder function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetIndexBySyncFolderTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest003 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex1 = 12345;
        SyncFolderValue value1;
        value1.bundleName = "com.test.example";
        value1.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex1, value1);

        uint32_t syncFolderIndex2 = 67890;
        SyncFolderValue value2;
        value2.bundleName = "com.test.example";
        value2.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex2, value2);

        uint32_t index = 0;
        bool ret = syncFolder.GetIndexBySyncFolder(index, "/test123");
        EXPECT_EQ(ret, true);
        EXPECT_EQ(index, syncFolderIndex2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetIndexBySyncFolderTest003 end";
}

/**
 * @tc.name: GetSyncFolderMapTest001
 * @tc.desc: Verify the GetSyncFolderMap function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetSyncFolderMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderMapTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        uint32_t syncFolderIndex1 = 123;
        SyncFolderValue value1;
        value1.bundleName = "com.test.example";
        value1.path = "/test123";
        syncFolder.AddSyncFolder(syncFolderIndex1, value1);

        uint32_t syncFolderIndex2 = 456;
        SyncFolderValue value2;
        value2.bundleName = "com.test.example";
        value2.path = "/test456";
        syncFolder.AddSyncFolder(syncFolderIndex2, value2);

        auto folderMap = syncFolder.GetSyncFolderMap();
        EXPECT_EQ(folderMap.size(), 2);

        auto it1 = folderMap.find(syncFolderIndex1);
        EXPECT_TRUE(it1 != folderMap.end());
        EXPECT_EQ(it1->second.path, "/test123");

        auto it2 = folderMap.find(syncFolderIndex2);
        EXPECT_TRUE(it2 != folderMap.end());
        EXPECT_EQ(it2->second.path, "/test456");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderMapTest001 end";
}

/**
 * @tc.name: RemoveXattrTest001
 * @tc.desc: Verify the RemoveXattr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, RemoveXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveXattrTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        string path = "/valid/path";
        string attrName = "user.test_attr";
        syncFolder.RemoveXattr(path, attrName);
        Mock::VerifyAndClearExpectations(insMock.get());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveXattrTest001 failed";
    }
    GTEST_LOG_(INFO) << "RemoveXattrTest001 end";
}

/**
 * @tc.name: RemoveXattrTest002
 * @tc.desc: Verify the RemoveXattr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, RemoveXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveXattrTest002 start";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("touch /data/test_tdd/test.txt");
        EXPECT_CALL(*insMock, removexattr(_, _)).WillOnce(Return(-1));
        Assistant::mockErrno = 1000;

        const char *realOutputPath = "/data/test_tdd/test.txt";
        EXPECT_CALL(*syncFolderMock, realpath(_, _))
        .WillOnce(DoAll(
            SetArrayArgument<1>(realOutputPath, realOutputPath + strlen(realOutputPath) + 1),
            Return(const_cast<char*>(realOutputPath))
        ));

        string path = "/data/test_tdd";
        string attrName = "user.test_attr";
        CloudDiskSyncFolder::GetInstance().RemoveXattr(path, attrName);
        EXPECT_EQ(errno, 1000);
        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveXattrTest002 failed";
    }
    GTEST_LOG_(INFO) << "RemoveXattrTest002 end";
}

/**
 * @tc.name: RemoveXattrTest003
 * @tc.desc: Verify the RemoveXattr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, RemoveXattrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveXattrTest003 start";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("touch /data/test_tdd/test.txt");
        EXPECT_CALL(*insMock, removexattr(_, _)).WillOnce(Return(-1));
        Assistant::mockErrno = ENODATA;

        const char *realOutputPath = "/data/test_tdd/test.txt";
        EXPECT_CALL(*syncFolderMock, realpath(_, _))
        .WillOnce(DoAll(
            SetArrayArgument<1>(realOutputPath, realOutputPath + strlen(realOutputPath) + 1),
            Return(const_cast<char*>(realOutputPath))
        ));

        string path = "/data/test_tdd";
        string attrName = "user.test_attr";
        CloudDiskSyncFolder::GetInstance().RemoveXattr(path, attrName);
        EXPECT_EQ(errno, ENODATA);
        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveXattrTest003 failed";
    }
    GTEST_LOG_(INFO) << "RemoveXattrTest003 end";
}

/**
 * @tc.name: RemoveXattrTest004
 * @tc.desc: Verify the RemoveXattr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, RemoveXattrTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveXattrTest004 start";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("touch /data/test_tdd/test.txt");
        EXPECT_CALL(*insMock, removexattr(_, _)).WillOnce(Return(0));
        Assistant::mockErrno = 1000;

        const char *realOutputPath = "/data/test_tdd/test.txt";
        EXPECT_CALL(*syncFolderMock, realpath(_, _))
        .WillOnce(DoAll(
            SetArrayArgument<1>(realOutputPath, realOutputPath + strlen(realOutputPath) + 1),
            Return(const_cast<char*>(realOutputPath))
        ));

        string path = "/data/test_tdd";
        string attrName = "user.test_attr";
        CloudDiskSyncFolder::GetInstance().RemoveXattr(path, attrName);
        EXPECT_EQ(errno, 1000);
        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveXattrTest004 failed";
    }
    GTEST_LOG_(INFO) << "RemoveXattrTest004 end";
}

/**
 * @tc.name: RemoveXattrTest005
 * @tc.desc: Verify the RemoveXattr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, RemoveXattrTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveXattrTest005 start";
    try {
        std::system("mkdir -p /data/test_tdd");
        std::system("touch /data/test_tdd/test.txt");
        EXPECT_CALL(*insMock, removexattr(_, _)).WillOnce(Return(0));
        Assistant::mockErrno = ENODATA;

        const char *realOutputPath = "/data/test_tdd/test.txt";
        EXPECT_CALL(*syncFolderMock, realpath(_, _))
        .WillOnce(DoAll(
            SetArrayArgument<1>(realOutputPath, realOutputPath + strlen(realOutputPath) + 1),
            Return(const_cast<char*>(realOutputPath))
        ));

        string path = "/data/test_tdd";
        string attrName = "user.test_attr";
        CloudDiskSyncFolder::GetInstance().RemoveXattr(path, attrName);
        EXPECT_EQ(errno, ENODATA);
        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveXattrTest005 failed";
    }
    GTEST_LOG_(INFO) << "RemoveXattrTest005 end";
}

/**
 * @tc.name: PathToPhysicalPathTest001
 * @tc.desc: Verify the PathToPhysicalPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToPhysicalPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToPhysicalPathTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/storage/Users/currentUser/testfile.txt";
        string userId = "100";
        string realpath;
        EXPECT_CALL(*syncFolderMock, realpath(_, _)).WillOnce(Return(nullptr));
        bool ret = syncFolder.PathToPhysicalPath(path, userId, realpath);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToPhysicalPathTest001 failed";
    }
    GTEST_LOG_(INFO) << "PathToPhysicalPathTest001 end";
}

/**
 * @tc.name: PathToPhysicalPathTest002
 * @tc.desc: Verify the PathToPhysicalPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToPhysicalPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToPhysicalPathTest002 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/storage/Users/otherUser/testfile.txt";
        string userId = "100";
        string realpath;
        bool ret = syncFolder.PathToPhysicalPath(path, userId, realpath);
        EXPECT_EQ(ret, false);
        EXPECT_TRUE(realpath.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToPhysicalPathTest002 failed";
    }
    GTEST_LOG_(INFO) << "PathToPhysicalPathTest002 end";
}

/**
 * @tc.name: PathToPhysicalPathTest003
 * @tc.desc: Verify the PathToPhysicalPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToPhysicalPathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToPhysicalPathTest003 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/storage/Users/currentUser/testfile.txt";
        string userId = "100";
        string realpath;
        const char *resolvedPath = "/data/service/el2/100/hmdfs/account/files/Docs/testfile.txt";
        EXPECT_CALL(*syncFolderMock, realpath(_, _))
            .WillOnce(DoAll(
                SetArrayArgument<1>(resolvedPath, resolvedPath + strlen(resolvedPath) + 1),
                Return(const_cast<char*>(resolvedPath))
            ));
        bool ret = syncFolder.PathToPhysicalPath(path, userId, realpath);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(realpath, resolvedPath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToPhysicalPathTest003 failed";
    }
    GTEST_LOG_(INFO) << "PathToPhysicalPathTest003 end";
}

/**
 * @tc.name: PathToMntPathBySandboxPathTest001
 * @tc.desc: Verify the PathToMntPathBySandboxPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToMntPathBySandboxPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/storage/Users/currentUser/testfile.txt";
        string userId = "100";
        string realpath;
        const char *resolvedPath = "/mnt/hmdfs/100/account/device_view/local/files/Docs/testfile.txt";
        EXPECT_CALL(*syncFolderMock, realpath(_, _))
            .WillOnce(DoAll(
                SetArrayArgument<1>(resolvedPath, resolvedPath + strlen(resolvedPath) + 1),
                Return(const_cast<char*>(resolvedPath))
            ));
        bool ret = syncFolder.PathToMntPathBySandboxPath(path, userId, realpath);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(realpath, resolvedPath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest001 failed";
    }
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest001 end";
}

/**
 * @tc.name: PathToMntPathBySandboxPathTest002
 * @tc.desc: Verify the PathToMntPathBySandboxPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToMntPathBySandboxPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest002 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/storage/Users/otherUser/testfile.txt";
        string userId = "100";
        string realpath;
        bool ret = syncFolder.PathToMntPathBySandboxPath(path, userId, realpath);
        EXPECT_EQ(ret, false);
        EXPECT_TRUE(realpath.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest002 failed";
    }
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest002 end";
}

/**
 * @tc.name: PathToMntPathBySandboxPathTest003
 * @tc.desc: Verify the PathToMntPathBySandboxPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToMntPathBySandboxPathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest003 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/storage/Users/currentUser/testfile.txt";
        string userId = "100";
        string realpath;
        EXPECT_CALL(*syncFolderMock, realpath(_, _)).WillOnce(Return(nullptr));
        bool ret = syncFolder.PathToMntPathBySandboxPath(path, userId, realpath);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest003 failed";
    }
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest003 end";
}

/**
 * @tc.name: PathToMntPathByPhysicalPathTest001
 * @tc.desc: Verify the PathToMntPathByPhysicalPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToMntPathByPhysicalPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToMntPathByPhysicalPathTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/data/service/el2/100/hmdfs/account/files/Docs/testfile.txt";
        string userId = "100";
        string realpath;
        bool ret = syncFolder.PathToMntPathByPhysicalPath(path, userId, realpath);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(realpath, "/mnt/hmdfs/100/account/device_view/local/files/Docs/testfile.txt");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToMntPathByPhysicalPathTest001 failed";
    }
    GTEST_LOG_(INFO) << "PathToMntPathByPhysicalPathTest001 end";
}

/**
 * @tc.name: PathToMntPathByPhysicalPathTest002
 * @tc.desc: Verify the PathToMntPathByPhysicalPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToMntPathByPhysicalPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToMntPathByPhysicalPathTest002 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/data/service/el2/123456/hmdfs/account/files/Docs/testfile.txt";
        string userId = "100";
        string realpath;
        bool ret = syncFolder.PathToMntPathByPhysicalPath(path, userId, realpath);
        EXPECT_EQ(ret, false);
        EXPECT_TRUE(realpath.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToMntPathByPhysicalPathTest002 failed";
    }
    GTEST_LOG_(INFO) << "PathToMntPathByPhysicalPathTest002 end";
}

/**
 * @tc.name: PathToSandboxPathByPhysicalPathTest001
 * @tc.desc: Verify the PathToSandboxPathByPhysicalPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToSandboxPathByPhysicalPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToSandboxPathByPhysicalPathTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/data/service/el2/100/hmdfs/account/files/Docs/testfile.txt";
        string userId = "100";
        string realpath;
        bool ret = syncFolder.PathToSandboxPathByPhysicalPath(path, userId, realpath);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(realpath, "/storage/Users/currentUser/testfile.txt");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToSandboxPathByPhysicalPathTest001 failed";
    }
    GTEST_LOG_(INFO) << "PathToSandboxPathByPhysicalPathTest001 end";
}

/**
 * @tc.name: PathToSandboxPathByPhysicalPathTest002
 * @tc.desc: Verify the PathToSandboxPathByPhysicalPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, PathToSandboxPathByPhysicalPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToSandboxPathByPhysicalPathTest002 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        string path = "/data/service/el2/123456/hmdfs/account/files/Docs/testfile.txt";
        string userId = "100";
        string realpath;
        bool ret = syncFolder.PathToSandboxPathByPhysicalPath(path, userId, realpath);
        EXPECT_EQ(ret, false);
        EXPECT_TRUE(realpath.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToSandboxPathByPhysicalPathTest002 failed";
    }
    GTEST_LOG_(INFO) << "PathToSandboxPathByPhysicalPathTest002 end";
}

/**
 * @tc.name: ClearMapTest001
 * @tc.desc: Verify the ClearMap function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, ClearMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearMapTest001 start";
    try {
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        syncFolder.ClearMap();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "ClearMapTest001 end";
}

/**
 * @tc.name: ReplacePathPrefixTest001
 * @tc.desc: Verify the ReplacePathPrefix function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, ReplacePathPrefixTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest001 start";
    try {
        string oldPrefix = "/storage/Users/currentUser";
        string newPrefix = "/data/service/el2/100/hmdfs/account/files/Docs";
        string inputPath = "/storage/Users/currentUser/a/b/file.txt";
        const char *outputRealPath = "/data/service/el2/100/hmdfs/account/files/Docs/a/b/file.txt";
        string outputPath = "";
        string outputPathTrue = "/data/service/el2/100/hmdfs/account/files/Docs/a/b/file.txt";
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        EXPECT_CALL(*syncFolderMock, realpath(_, _))
            .WillOnce(DoAll(
                SetArrayArgument<1>(outputRealPath, outputRealPath + strlen(outputRealPath) + 1),
                Return(const_cast<char*>(outputRealPath))
            ));
        bool ret = syncFolder.ReplacePathPrefix(oldPrefix, newPrefix, inputPath, outputPath);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(outputPath, outputPathTrue);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReplacePathPrefixTest001 failed";
    }
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest001 end";
}

/**
 * @tc.name: ReplacePathPrefixTest002
 * @tc.desc: Verify the ReplacePathPrefix function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, ReplacePathPrefixTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest002 start";
    try {
        string oldPrefix = "/storage/Users/currentUser";
        string newPrefix = "/data/service/el2/100/hmdfs/account/files/Docs";
        string inputPath = "/storage/Users/currentUser/a/../../file.txt";
        const char *outputRealPath = "/data/service/el2/100/hmdfs/account/files/file.txt";
        string outputPath = "";
        string outputPathTrue = "";
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        EXPECT_CALL(*syncFolderMock, realpath(_, _))
            .WillOnce(DoAll(
                SetArrayArgument<1>(outputRealPath, outputRealPath + strlen(outputRealPath) + 1),
                Return(const_cast<char*>(outputRealPath))
            ));
        bool ret = syncFolder.ReplacePathPrefix(oldPrefix, newPrefix, inputPath, outputPath);
        EXPECT_EQ(ret, false);
        EXPECT_EQ(outputPath, outputPathTrue);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReplacePathPrefixTest002 failed";
    }
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest002 end";
}
} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS