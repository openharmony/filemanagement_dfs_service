/*
* Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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
#include <memory>

#include "cloud_file_utils.h"
#include "cloud_status.h"
#include "clouddisk_notify_utils_mock.h"
#include "clouddisk_rdb_utils.h"
#include "clouddisk_rdbstore.cpp"
#include "dfs_error.h"
#include "rdb_assistant.h"
#include "result_set_mock.h"
#include "transaction_mock.cpp"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
using namespace OHOS::FileManagement::CloudDisk;

static const string BUNDLE_NAME = "com.ohos.photos";
static const int32_t USER_ID = 100;
class CloudDiskRdbStoreStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<CloudDiskRdbStore> clouddiskrdbStore_ = nullptr;
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void CloudDiskRdbStoreStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
    clouddiskrdbStore_ = make_shared<CloudDiskRdbStore>(BUNDLE_NAME, USER_ID);
}

void CloudDiskRdbStoreStaticTest::TearDownTestCase(void)
{
    Assistant::ins = nullptr;
    insMock = nullptr;
    clouddiskrdbStore_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskRdbStoreStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskRdbStoreStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FileRenameTest001
 * @tc.desc: Verify the FileRename
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileRenameTest001, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = CLOUD;
    const std::string newFileName = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);
 
    FileRename(values, position, newFileName, dirtyType);
    EXPECT_EQ(newFileName, "test");
}

/**
 * @tc.name: FileRenameTest002
 * @tc.desc: Verify the FileRename
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileRenameTest002, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = LOCAL;
    const std::string newFileName = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);
 
    FileRename(values, position, newFileName, dirtyType);
    EXPECT_EQ(newFileName, "test");
}

/**
 * @tc.name: FileRenameTest003
 * @tc.desc: Verify the FileRename
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileRenameTest003, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = CLOUD;
    const std::string newFileName = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_SYNCED);
 
    FileRename(values, position, newFileName, dirtyType);
    EXPECT_EQ(position, CLOUD);
}

/**
 * @tc.name: RecycleSetValue
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest1, TestSize.Level1)
{
    ValuesBucket setXAttr;
    int32_t position = CLOUD;
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);
 
    RecycleSetValue(TrashOptType::RECYCLE, setXAttr, position, dirtyType);
    EXPECT_EQ(position, CLOUD);
}

/**
 * @tc.name: FileMoveTest001
 * @tc.desc: Verify the FileMove
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileMoveTest001, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = CLOUD;
    const std::string newParentCloudId = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);
 
    FileMove(values, position, newParentCloudId, dirtyType);
    EXPECT_EQ(newParentCloudId, "test");
}

/**
 * @tc.name: FileMoveTest002
 * @tc.desc: Verify the FileMove
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileMoveTest002, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = LOCAL;
    const std::string newParentCloudId = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);
 
    FileMove(values, position, newParentCloudId, dirtyType);
    EXPECT_EQ(newParentCloudId, "test");
}

/**
 * @tc.name: FileMoveTest003
 * @tc.desc: Verify the FileMove
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileMoveTest003, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = CLOUD;
    const std::string newParentCloudId = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_SYNCED);
 
    FileMove(values, position, newParentCloudId, dirtyType);
    EXPECT_EQ(newParentCloudId, "test");
}

/**
 * @tc.name: FileMoveAndRenameTest001
 * @tc.desc: Verify the FileMoveAndRename
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileMoveAndRenameTest001, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = CLOUD;
    const std::string newParentCloudId = "test";
    const std::string newFileName = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);
 
    FileMoveAndRename(values, position, newParentCloudId, newFileName, dirtyType);
    EXPECT_EQ(newFileName, "test");
}

/**
 * @tc.name: FileMoveAndRenameTest002
 * @tc.desc: Verify the FileMoveAndRename
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileMoveAndRenameTest002, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = LOCAL;
    const std::string newParentCloudId = "test";
    const std::string newFileName = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);
 
    FileMoveAndRename(values, position, newParentCloudId, newFileName, dirtyType);
    EXPECT_EQ(newFileName, "test");
}

/**
 * @tc.name: FileMoveAndRenameTest003
 * @tc.desc: Verify the FileMoveAndRename
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FileMoveAndRenameTest003, TestSize.Level1)
{
    ValuesBucket values;
    const int32_t position = CLOUD;
    const std::string newParentCloudId = "test";
    const std::string newFileName = "test";
    int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_SYNCED);
 
    FileMoveAndRename(values, position, newParentCloudId, newFileName, dirtyType);
    EXPECT_EQ(newFileName, "test");
}

/**
 * @tc.name: ExtAttributeSetValueTest001
 * @tc.desc: Verify the ExtAttributeSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttributeSetValueTest1, TestSize.Level1)
{
    const std::string key = "testKey";
    std::string value = "testValue";
    std::string xattrList = "{}";
    std::string jsonValue = "";
 
    int32_t result = ExtAttributeSetValue(jsonValue, key, value, xattrList);
    EXPECT_EQ(result, E_OK);
}

/**
 * @tc.name: ConvertUriToSrcPathTest001
 * @tc.desc: Verify the ConvertUriToSrcPath
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ConvertUriToSrcPathTest1, TestSize.Level1)
{
    string uri = "file:///data/storage/el2/cloud/a/b/c";
    string expectResult = "/a/b";
    string filePath = ConvertUriToSrcPath(uri);
    EXPECT_EQ(filePath, expectResult);
}

/**
 * @tc.name: ConvertUriToSrcPathTest002
 * @tc.desc: Verify the ConvertUriToSrcPath
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ConvertUriToSrcPathTest2, TestSize.Level1)
{
    string uri = "files:///data/storage/el2/cloud/a/b/c";
    string expectResult = "/";
    string filePath = ConvertUriToSrcPath(uri);
    EXPECT_EQ(filePath, expectResult);
}

/**
 * @tc.name: ConvertUriToSrcPathTest003
 * @tc.desc: Verify the ConvertUriToSrcPath
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ConvertUriToSrcPathTest3, TestSize.Level1)
{
    string uri = "file:///a/b/c";
    string expectResult = "/";
    string filePath = ConvertUriToSrcPath(uri);
    EXPECT_EQ(filePath, expectResult);
}

/**
 * @tc.name: ConvertUriToSrcPathTest004
 * @tc.desc: Verify the ConvertUriToSrcPath
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ConvertUriToSrcPathTest4, TestSize.Level1)
{
    string uri = "file:///data/storage/el2/cloud/a";
    string expectResult = "/";
    string filePath = ConvertUriToSrcPath(uri);
    EXPECT_EQ(filePath, expectResult);
}

/**
 * @tc.name: CloudSyncTriggerFuncTest001
 * @tc.desc: Verify the CloudSyncTriggerFunc
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CloudSyncTriggerFuncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest001 start";
    try {
        std::vector<std::string> args = {"123", "com.example.app"};
        std::string result = CloudSyncTriggerFunc(args);
        EXPECT_EQ(result, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest001 failed";
    }
    GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest001 end";
}

/**
 * @tc.name: CloudSyncTriggerFuncTest002
 * @tc.desc: Verify the CloudSyncTriggerFunc
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CloudSyncTriggerFuncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest002 start";
    try {
        std::vector<std::string> args = {"123"};
        std::string result = CloudSyncTriggerFunc(args);
        EXPECT_EQ(result, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest002 failed";
    }
    GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest002 end";
}

/**
 * @tc.name: GetFileExtensionTest001
 * @tc.desc: Verify the GetFileExtension
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetFileExtensionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtensionTest001 start";
    try {
        std::string fileName = "example.txt";
        std::string extension;
        int32_t result = GetFileExtension(fileName, extension);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileExtensionTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetFileExtensionTest001 end";
}

/**
 * @tc.name: GetFileExtensionTest002
 * @tc.desc: Verify the GetFileExtension
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetFileExtensionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtensionTest002 start";
    try {
        std::string fileName = "example";
        std::string extension;
        int32_t result = GetFileExtension(fileName, extension);
        EXPECT_EQ(result, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileExtensionTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetFileExtensionTest002 end";
}

/**
 * @tc.name: GetFileExtensionTest003
 * @tc.desc: Verify the GetFileExtension
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetFileExtensionTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtensionTest003 start";
    try {
        std::string fileName = "file.name.with.multiple.dots";
        std::string extension;
        int32_t result = GetFileExtension(fileName, extension);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileExtensionTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetFileExtensionTest003 end";
}

/**
 * @tc.name: GetFileExtensionTest004
 * @tc.desc: Verify the GetFileExtension
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetFileExtensionTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtensionTest004 start";
    try {
        std::string fileName = ".bashrc";
        std::string extension;
        int32_t result = GetFileExtension(fileName, extension);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileExtensionTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetFileExtensionTest004 end";
}

/**
 * @tc.name: GetFileExtensionTest005
 * @tc.desc: Verify the GetFileExtension
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetFileExtensionTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtensionTest005 start";
    try {
        std::string fileName = "";
        std::string extension;
        int32_t result = GetFileExtension(fileName, extension);
        EXPECT_EQ(result, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileExtensionTest005 failed";
    }
    GTEST_LOG_(INFO) << "GetFileExtensionTest005 end";
}

/**
 * @tc.name: CheckNameForSpaceTest001
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest001 start";
    try {
        std::string fileName = "";
        int32_t isDir = DIRECTORY;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest001 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest001 end";
}

/**
 * @tc.name: CheckNameForSpaceTest002
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest002 start";
    try {
        std::string fileName = " fileName";
        int32_t isDir = DIRECTORY;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest002 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest002 end";
}

/**
 * @tc.name: CheckNameForSpaceTest003
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest003 start";
    try {
        std::string fileName = "fileName";
        int32_t isDir = DIRECTORY;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest003 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest003 end";
}

/**
 * @tc.name: CheckNameForSpaceTest004
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest004 start";
    try {
        std::string fileName = "fileName ";
        int32_t isDir = DIRECTORY;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest004 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest004 end";
}

/**
 * @tc.name: CheckNameForSpaceTest005
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest005 start";
    try {
        std::string fileName = ".trash";
        int32_t isDir = DIRECTORY;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest005 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest005 end";
}

/**
 * @tc.name: CheckNameForSpaceTest006
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest006 start";
    try {
        std::string fileName = "trash";
        int32_t isDir = 1;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest006 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest006 end";
}

/**
 * @tc.name: CheckNameTest001
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest001 start";
    try {
        std::string fileName = "";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest001 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest001 end";
}

/**
 * @tc.name: CheckNameTest002
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest002 start";
    try {
        std::string fileName = ".";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest002 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest002 end";
}

/**
 * @tc.name: CheckNameTest003
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest003 start";
    try {
        std::string fileName = "..";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest003 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest003 end";
}

/**
 * @tc.name: CheckNameTest004
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest004 start";
    try {
        std::string fileName =
            "fileNamefileNamefileNamefileNamefileNamefileNamefileNamefileName"
            "fileNamefileNamefileNamefileNamefileNamefileNamefileNamefileName"
            "fileNamefileNamefileNamefileNamefileNamefileNamefileNamefileName"
            "fileNamefileNamefileNamefileNamefileNamefileNamefileNamefileName";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest004 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest004 end";
}

/**
 * @tc.name: CheckNameTest005
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest005 start";
    try {
        std::string fileName = "file<Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest005 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest005 end";
}

/**
 * @tc.name: CheckNameTest006
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest006 start";
    try {
        std::string fileName = "file>Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest006 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest006 end";
}

/**
 * @tc.name: CheckNameTest007
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest007 start";
    try {
        std::string fileName = "file|Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest007 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest007 end";
}

/**
 * @tc.name: CheckNameTest008
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest008 start";
    try {
        std::string fileName = "file:Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest008 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest008 end";
}

/**
 * @tc.name: CheckNameTest009
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest009 start";
    try {
        std::string fileName = "file?Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest009 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest009 end";
}

/**
 * @tc.name: CheckNameTest010
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest010 start";
    try {
        std::string fileName = "file/Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest010 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest010 end";
}

/**
 * @tc.name: CheckNameTest011
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest011 start";
    try {
        std::string fileName = "file\\Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest011 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest011 end";
}

/**
 * @tc.name: CheckNameTest012
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest012 start";
    try {
        std::string fileName = "file\"Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest012 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest012 end";
}

/**
 * @tc.name: CheckNameTest013
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest013 start";
    try {
        std::string fileName = "file*Name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest013 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest013 end";
}

/**
 * @tc.name: CheckNameTest014
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest014 start";
    try {
        std::string fileName = "file_name";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest014 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest014 end";
}

/**
 * @tc.name: CreateFileTest001
 * @tc.desc: Verify the CreateFile
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CreateFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFileTest001 start";
    try {
        std::string fileName = "test.txt";
        std::string filePath = "";
        ValuesBucket fileInfo;
        struct stat statInfo{};
        int32_t result = CreateFile(fileName, filePath, fileInfo, &statInfo);
        EXPECT_EQ(result, E_PATH);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateFileTest001 failed";
    }
    GTEST_LOG_(INFO) << "CreateFileTest001 end";
}

} // namespace OHOS::FileManagement::CloudDisk::Test