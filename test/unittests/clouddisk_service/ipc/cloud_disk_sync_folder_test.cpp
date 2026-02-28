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

#include "cloud_disk_sync_folder.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assistant.h"
#include "cloud_disk_service_error.h"
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
    static inline shared_ptr<AssistantMock> assistantMock;
    static inline shared_ptr<SyncFolderAssistantMock> syncFolderAssistantMock;
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
    assistantMock = make_shared<AssistantMock>();
    Assistant::ins = assistantMock;
    assistantMock->EnableMock();
    syncFolderAssistantMock = make_shared<SyncFolderAssistantMock>();
    SyncFolderAssistant::ins = syncFolderAssistantMock;
}

void CloudDiskSyncFolderTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    assistantMock->DisableMock();
    Assistant::ins = nullptr;
    assistantMock = nullptr;
    SyncFolderAssistant::ins = nullptr;
    syncFolderAssistantMock = nullptr;
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
        CloudDiskSyncFolder& instanceRef1 = CloudDiskSyncFolder::GetInstance();
        CloudDiskSyncFolder& instanceRef2 = CloudDiskSyncFolder::GetInstance();
        EXPECT_EQ(&instanceRef1, &instanceRef2);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        EXPECT_EQ(syncFolderInstance.GetSyncFolderSize(), 1);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        EXPECT_TRUE(syncFolderInstance.CheckSyncFolder(testIndex));
        EXPECT_EQ(syncFolderInstance.GetSyncFolderSize(), 1);

        syncFolderInstance.DeleteSyncFolder(testIndex);
        EXPECT_FALSE(syncFolderInstance.CheckSyncFolder(testIndex));
        EXPECT_EQ(syncFolderInstance.GetSyncFolderSize(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "DeleteSyncFolderTest001 end";
}

/**
 * @tc.name: DeleteSyncFolderTest002
 * @tc.desc: Verify the DeleteSyncFolder function with non-existent index
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, DeleteSyncFolderTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteSyncFolderTest002 start";
    try {
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t nonExistentIndex = 999;
        syncFolderInstance.DeleteSyncFolder(nonExistentIndex);
        EXPECT_EQ(syncFolderInstance.GetSyncFolderSize(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteSyncFolderTest002 failed";
    }
    GTEST_LOG_(INFO) << "DeleteSyncFolderTest002 end";
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex1 = 12345;
        SyncFolderValue syncValue1;
        syncValue1.bundleName = "com.test.example";
        syncValue1.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex1, syncValue1);

        uint32_t testIndex2 = 67890;
        SyncFolderValue syncValue2;
        syncValue2.bundleName = "com.test.example";
        syncValue2.path = "/test456";
        syncFolderInstance.AddSyncFolder(testIndex2, syncValue2);
        EXPECT_EQ(syncFolderInstance.GetSyncFolderSize(), 2);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        bool checkResult = syncFolderInstance.CheckSyncFolder(testIndex);
        EXPECT_EQ(checkResult, true);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        uint32_t nonExistentIndex = 999;
        bool checkResult = syncFolderInstance.CheckSyncFolder(nonExistentIndex);
        EXPECT_EQ(checkResult, false);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        string folderPath;
        bool getResult = syncFolderInstance.GetSyncFolderByIndex(testIndex, folderPath);
        EXPECT_EQ(getResult, true);
        EXPECT_EQ(folderPath, "/test123");
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        string folderPath;
        uint32_t nonExistentIndex = 999;
        bool getResult = syncFolderInstance.GetSyncFolderByIndex(nonExistentIndex, folderPath);
        EXPECT_EQ(getResult, false);
        EXPECT_EQ(folderPath, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderByIndexTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderByIndexTest002 end";
}

/**
 * @tc.name: GetSyncFolderValueByIndexTest001
 * @tc.desc: Verify the GetSyncFolderValueByIndex function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetSyncFolderValueByIndexTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderValueByIndexTest001 start";
    try {
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        SyncFolderValue getValue;
        bool getResult = syncFolderInstance.GetSyncFolderValueByIndex(testIndex, getValue);
        EXPECT_EQ(getResult, true);
        EXPECT_EQ(getValue.bundleName, "com.test.example");
        EXPECT_EQ(getValue.path, "/test123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderValueByIndexTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderValueByIndexTest001 end";
}

/**
 * @tc.name: GetSyncFolderValueByIndexTest002
 * @tc.desc: Verify the GetSyncFolderValueByIndex function with non-existent index
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, GetSyncFolderValueByIndexTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderValueByIndexTest002 start";
    try {
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        SyncFolderValue getValue;
        uint32_t nonExistentIndex = 999;
        bool getResult = syncFolderInstance.GetSyncFolderValueByIndex(nonExistentIndex, getValue);
        EXPECT_EQ(getResult, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncFolderValueByIndexTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetSyncFolderValueByIndexTest002 end";
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        uint32_t foundIndex = 0;
        bool getResult = syncFolderInstance.GetIndexBySyncFolder(foundIndex, "/test123");
        EXPECT_EQ(getResult, true);
        EXPECT_EQ(foundIndex, testIndex);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex = 12345;
        SyncFolderValue syncValue;
        syncValue.bundleName = "com.test.example";
        syncValue.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex, syncValue);
        uint32_t foundIndex = 0;
        bool getResult = syncFolderInstance.GetIndexBySyncFolder(foundIndex, "/not_exists");
        EXPECT_EQ(getResult, false);
        EXPECT_EQ(foundIndex, 0);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex1 = 12345;
        SyncFolderValue syncValue1;
        syncValue1.bundleName = "com.test.example";
        syncValue1.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex1, syncValue1);

        uint32_t testIndex2 = 67890;
        SyncFolderValue syncValue2;
        syncValue2.bundleName = "com.test.example";
        syncValue2.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex2, syncValue2);

        uint32_t foundIndex = 0;
        bool getResult = syncFolderInstance.GetIndexBySyncFolder(foundIndex, "/test123");
        EXPECT_EQ(getResult, true);
        EXPECT_EQ(foundIndex, testIndex2);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        uint32_t testIndex1 = 123;
        SyncFolderValue syncValue1;
        syncValue1.bundleName = "com.test.example";
        syncValue1.path = "/test123";
        syncFolderInstance.AddSyncFolder(testIndex1, syncValue1);

        uint32_t testIndex2 = 456;
        SyncFolderValue syncValue2;
        syncValue2.bundleName = "com.test.example";
        syncValue2.path = "/test456";
        syncFolderInstance.AddSyncFolder(testIndex2, syncValue2);

        auto folderMap = syncFolderInstance.GetSyncFolderMap();
        EXPECT_EQ(folderMap.size(), 2);

        auto it1 = folderMap.find(testIndex1);
        EXPECT_TRUE(it1 != folderMap.end());
        EXPECT_EQ(it1->second.path, "/test123");

        auto it2 = folderMap.find(testIndex2);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
        string testPath = "/valid/path";
        string attrName = "user.test_attr";
        syncFolderInstance.RemoveXattr(testPath, attrName);
        Mock::VerifyAndClearExpectations(assistantMock.get());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveXattrTest001 failed";
    }
    GTEST_LOG_(INFO) << "RemoveXattrTest001 end";
}

/**
 * @tc.name: RemoveXattrTest006
 * @tc.desc: Verify the RemoveXattr function with opendir failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, RemoveXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveXattrTest002 start";
    try {
        string invalidPath = "/invalid/nonexistent/path";
        string attrName = "user.test_attr";
        CloudDiskSyncFolder::GetInstance().RemoveXattr(invalidPath, attrName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveXattrTest002 failed";
    }
    GTEST_LOG_(INFO) << "RemoveXattrTest002 end";
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/storage/Users/currentUser/testfile.txt";
        string testUserId = "100";
        string realPath;
        EXPECT_CALL(*syncFolderAssistantMock, realpath(_, _)).WillOnce(Return(nullptr));
        int32_t convertResult = syncFolderInstance.PathToPhysicalPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, E_SYNC_FOLDER_PATH_NOT_EXIST);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/storage/Users/otherUser/testfile.txt";
        string testUserId = "100";
        string realPath;
        int32_t convertResult = syncFolderInstance.PathToPhysicalPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, E_INVALID_ARG);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/storage/Users/currentUser/testfile.txt";
        string testUserId = "100";
        string realPath;
        const char *resolvedPath = "/data/service/el2/100/hmdfs/account/files/Docs/testfile.txt";
        EXPECT_CALL(*syncFolderAssistantMock, realpath(_, _))
            .WillOnce(DoAll(
                SetArrayArgument<1>(resolvedPath, resolvedPath + strlen(resolvedPath) + 1),
                Return(const_cast<char*>(resolvedPath))
            ));
        int32_t convertResult = syncFolderInstance.PathToPhysicalPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, E_OK);
        EXPECT_EQ(realPath, resolvedPath);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/storage/Users/currentUser/testfile.txt";
        string testUserId = "100";
        string realPath;
        const char *resolvedPath = "/mnt/hmdfs/100/account/device_view/local/files/Docs/testfile.txt";
        EXPECT_CALL(*syncFolderAssistantMock, realpath(_, _))
            .WillOnce(DoAll(
                SetArrayArgument<1>(resolvedPath, resolvedPath + strlen(resolvedPath) + 1),
                Return(const_cast<char*>(resolvedPath))
            ));
        int32_t convertResult = syncFolderInstance.PathToMntPathBySandboxPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, E_OK);
        EXPECT_EQ(realPath, resolvedPath);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/storage/Users/otherUser/testfile.txt";
        string testUserId = "100";
        string realPath;
        int32_t convertResult = syncFolderInstance.PathToMntPathBySandboxPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, E_INVALID_ARG);
        EXPECT_TRUE(realPath.empty());
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/storage/Users/currentUser/testfile.txt";
        string testUserId = "100";
        string realPath;
        EXPECT_CALL(*syncFolderAssistantMock, realpath(_, _)).WillOnce(Return(nullptr));
        int32_t convertResult = syncFolderInstance.PathToMntPathBySandboxPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, E_SYNC_FOLDER_PATH_NOT_EXIST);
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/data/service/el2/100/hmdfs/account/files/Docs/testfile.txt";
        string testUserId = "100";
        string realPath;
        bool convertResult = syncFolderInstance.PathToMntPathByPhysicalPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, true);
        EXPECT_EQ(realPath, "/mnt/hmdfs/100/account/device_view/local/files/Docs/testfile.txt");
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/data/service/el2/123456/hmdfs/account/files/Docs/testfile.txt";
        string testUserId = "100";
        string realPath;
        bool convertResult = syncFolderInstance.PathToMntPathByPhysicalPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, false);
        EXPECT_TRUE(realPath.empty());
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/data/service/el2/100/hmdfs/account/files/Docs/testfile.txt";
        string testUserId = "100";
        string realPath;
        bool convertResult = syncFolderInstance.PathToSandboxPathByPhysicalPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, true);
        EXPECT_EQ(realPath, "/storage/Users/currentUser/testfile.txt");
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        string testPath = "/data/service/el2/123456/hmdfs/account/files/Docs/testfile.txt";
        string testUserId = "100";
        string realPath;
        bool convertResult = syncFolderInstance.PathToSandboxPathByPhysicalPath(testPath, testUserId, realPath);
        EXPECT_EQ(convertResult, false);
        EXPECT_TRUE(realPath.empty());
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
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        syncFolderInstance.ClearMap();
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
        string expectedPath = "/data/service/el2/100/hmdfs/account/files/Docs/a/b/file.txt";
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        EXPECT_CALL(*syncFolderAssistantMock, realpath(_, _))
            .WillOnce(DoAll(
                SetArrayArgument<1>(outputRealPath, outputRealPath + strlen(outputRealPath) + 1),
                Return(const_cast<char*>(outputRealPath))
            ));
        int32_t replaceResult = syncFolderInstance.ReplacePathPrefix(oldPrefix, newPrefix, inputPath, outputPath);
        EXPECT_EQ(replaceResult, E_OK);
        EXPECT_EQ(outputPath, expectedPath);
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
        string expectedPath = "";
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        EXPECT_CALL(*syncFolderAssistantMock, realpath(_, _))
            .WillOnce(DoAll(
                SetArrayArgument<1>(outputRealPath, outputRealPath + strlen(outputRealPath) + 1),
                Return(const_cast<char*>(outputRealPath))
            ));
        int32_t replaceResult = syncFolderInstance.ReplacePathPrefix(oldPrefix, newPrefix, inputPath, outputPath);
        EXPECT_EQ(replaceResult, E_INVALID_ARG);
        EXPECT_EQ(outputPath, expectedPath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReplacePathPrefixTest002 failed";
    }
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest002 end";
}

/**
 * @tc.name: ReplacePathPrefixTest003
 * @tc.desc: Verify the ReplacePathPrefix function with invalid prefix
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, ReplacePathPrefixTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest003 start";
    try {
        string oldPrefix = "/storage/Users/currentUser";
        string newPrefix = "/data/service/el2/100/hmdfs/account/files/Docs";
        string inputPath = "/invalid/prefix/path";
        string outputPath = "";
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        int32_t replaceResult = syncFolderInstance.ReplacePathPrefix(oldPrefix, newPrefix, inputPath, outputPath);
        EXPECT_EQ(replaceResult, E_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReplacePathPrefixTest003 failed";
    }
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest003 end";
}

/**
 * @tc.name: ReplacePathPrefixTest004
 * @tc.desc: Verify the ReplacePathPrefix function with path shorter than prefix
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskSyncFolderTest, ReplacePathPrefixTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest004 start";
    try {
        string oldPrefix = "/storage/Users/currentUser";
        string newPrefix = "/data/service/el2/100/hmdfs/account/files/Docs";
        string inputPath = "/storage";
        string outputPath = "";
        CloudDiskSyncFolder &syncFolderInstance = CloudDiskSyncFolder::GetInstance();
        int32_t replaceResult = syncFolderInstance.ReplacePathPrefix(oldPrefix, newPrefix, inputPath, outputPath);
        EXPECT_EQ(replaceResult, E_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReplacePathPrefixTest004 failed";
    }
    GTEST_LOG_(INFO) << "ReplacePathPrefixTest004 end";
}

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS

