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

#define TEST_LOCAL_PATH_MNT_HMDFS "/test/mnt/hmdfs/"
#include "clouddisk_rdbstore.cpp"
#undef TEST_LOCAL_PATH_MNT_HMDFS
#include "rdb_assistant.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
using namespace OHOS::FileManagement::CloudDisk;

class CloudDiskRdbStoreStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void CloudDiskRdbStoreStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void CloudDiskRdbStoreStaticTest::TearDownTestCase(void)
{
    Assistant::ins = nullptr;
    insMock = nullptr;
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
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttributeSetValueTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest001 start";
    try {
        std::string key = "testKey";
        std::string value = "testValue";
        std::string xattrList = "{}";
        std::string jsonValue = "";

        int32_t result = ExtAttributeSetValue(jsonValue, key, value, xattrList);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtAttributeSetValueTest001 failed";
    }
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest001 end";
}

/**
 * @tc.name: ExtAttributeSetValueTest002
 * @tc.desc: Verify the ExtAttributeSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttributeSetValueTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest002 start";
    try {
        std::string key = "testKey";
        std::string value = "testValue";
        std::string xattrList = "{}";
        std::string jsonValue = "invalidJson";

        int32_t result = ExtAttributeSetValue(jsonValue, key, value, xattrList);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtAttributeSetValueTest002 failed";
    }
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest002 end";
}

/**
 * @tc.name: ExtAttributeSetValueTest003
 * @tc.desc: Verify the ExtAttributeSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttributeSetValueTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest003 start";
    try {
        std::string key = "existingKey";
        std::string value = "testValue";
        std::string xattrList = "{\"existingKey\":\"existingValue\"}";
        std::string jsonValue = "";

        int32_t result = ExtAttributeSetValue(jsonValue, key, value, xattrList);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtAttributeSetValueTest003 failed";
    }
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest003 end";
}

/**
 * @tc.name: ExtAttributeSetValueTest004
 * @tc.desc: Verify the ExtAttributeSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttributeSetValueTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest004 start";
    try {
        std::string key = "testKey";
        std::string value = "testValue";
        std::string xattrList = "{\"existingKey\":\"existingValue\"}";
        std::string jsonValue = "";

        int32_t result = ExtAttributeSetValue(jsonValue, key, value, xattrList);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtAttributeSetValueTest004 failed";
    }
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest004 end";
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
 * @tc.name: ConvertUriToSrcPathTest005
 * @tc.desc: Verify the ConvertUriToSrcPath
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ConvertUriToSrcPathTest005, TestSize.Level1)
{
    string uri = "/data/storage/el2/cloud/a";
    string expectResult = "/";
    string filePath = ConvertUriToSrcPath(uri);
    EXPECT_EQ(filePath, expectResult);
}

/**
 * @tc.name: ConvertUriToSrcPathTest006
 * @tc.desc: Verify the ConvertUriToSrcPath
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ConvertUriToSrcPathTest006, TestSize.Level1)
{
    string uri = "";
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
 * @tc.name: CloudSyncTriggerFuncTest003
 * @tc.desc: Verify the CloudSyncTriggerFunc
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CloudSyncTriggerFuncTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest003 start";
    try {
        std::vector<std::string> args = {};
        std::string result = CloudSyncTriggerFunc(args);
        EXPECT_EQ(result, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest003 failed";
    }
    GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest003 end";
}

/**
 * @tc.name: CloudSyncTriggerFuncTest004
 * @tc.desc: Verify the CloudSyncTriggerFunc
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CloudSyncTriggerFuncTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest004 start";
    try {
        std::vector<std::string> args = {"123", "com.example.app", "extra"};
        std::string result = CloudSyncTriggerFunc(args);
        EXPECT_EQ(result, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest004 failed";
    }
    GTEST_LOG_(INFO) << "CloudSyncTriggerFuncTest004 end";
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
 * @tc.name: GetFileExtensionTest006
 * @tc.desc: Verify the GetFileExtension
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetFileExtensionTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtensionTest006 start";
    try {
        std::string fileName = ".";
        std::string extension;
        int32_t result = GetFileExtension(fileName, extension);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileExtensionTest006 failed";
    }
    GTEST_LOG_(INFO) << "GetFileExtensionTest006 end";
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
 * @tc.name: CheckNameForSpaceTest007
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest007 start";
    try {
        std::string fileName = "fileName ";
        int32_t isDir = FILE;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest007 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest007 end";
}

/**
 * @tc.name: CheckNameForSpaceTest008
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest008 start";
    try {
        std::string fileName = ".trash";
        int32_t isDir = FILE;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest008 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest008 end";
}

/**
 * @tc.name: CheckNameForSpaceTest009
 * @tc.desc: Verify the CheckNameForSpace
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameForSpaceTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest009 start";
    try {
        std::string fileName = ".Trash";
        int32_t isDir = DIRECTORY;
        int32_t result = CheckNameForSpace(fileName, isDir);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameForSpaceTest009 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameForSpaceTest009 end";
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
 * @tc.name: CheckNameTest015
 * @tc.desc: Verify the CheckName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CheckNameTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNameTest015 start";
    try {
        std::string fileName = "file-name_test (1)";
        int32_t result = CheckName(fileName);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckNameTest015 failed";
    }
    GTEST_LOG_(INFO) << "CheckNameTest015 end";
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

/**
 * @tc.name: CreateFileTest002
 * @tc.desc: Verify the CloudSyncTriggerFunc
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CreateFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFileTest002 start";
    try {
        std::string fileName = "test.txt";
        std::string filePath = "/data/service/el2/100/hmdfs/cloud/";
        ValuesBucket fileInfo;
        struct stat statInfo{};
        int32_t result = CreateFile(fileName, filePath, fileInfo, &statInfo);

        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateFileTest002 failed";
    }
    GTEST_LOG_(INFO) << "CreateFileTest002 end";
}

/**
 * @tc.name: CreateFileTest003
 * @tc.desc: Verify the CreateFile
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CreateFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFileTest003 start";
    try {
        std::string fileName = "test.txt";
        std::string filePath = "/invalid/path/";
        ValuesBucket fileInfo;
        struct stat statInfo{};
        int32_t result = CreateFile(fileName, filePath, fileInfo, &statInfo);

        EXPECT_EQ(result, E_PATH);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateFileTest003 failed";
    }
    GTEST_LOG_(INFO) << "CreateFileTest003 end";
}

/**
 * @tc.name: FillFileTypeTest001
 * @tc.desc: Verify the FillFileType
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, FillFileTypeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FillFileTypeTest001 start";
    try {
        std::string fileName = "test.txt";
        ValuesBucket fileInfo;
        FillFileType(fileName, fileInfo);

        std::string extension;
        int32_t result = GetFileExtension(fileName, extension);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FillFileTypeTest001 failed";
    }
    GTEST_LOG_(INFO) << "FillFileTypeTest001 end";
}

/**
 * @tc.name: CreateDentryTest001
 * @tc.desc: Verify the CreateDentry
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CreateDentryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateDentryTest001 start";
    try {
        std::string fileName = "mock";
        std::string cloudId;
        uint32_t userId = 1;
        std::string bundleName;
        std::string parentCloudId;
        MetaBase metaBase(fileName, cloudId);

        int32_t result = CreateDentry(metaBase, userId, bundleName, fileName, parentCloudId);
        EXPECT_EQ(result, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateDentryTest001 failed";
    }
    GTEST_LOG_(INFO) << "CreateDentryTest001 end";
}

/**
 * @tc.name: CreateDentryTest002
 * @tc.desc: Verify the CreateDentry
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CreateDentryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateDentryTest002 start";
    try {
        std::string fileName = "filename";
        std::string cloudId;
        uint32_t userId = 1;
        std::string bundleName;
        std::string parentCloudId;
        MetaBase metaBase(fileName, cloudId);

        int32_t result = CreateDentry(metaBase, userId, bundleName, fileName, parentCloudId);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateDentryTest002 failed";
    }
    GTEST_LOG_(INFO) << "CreateDentryTest002 end";
}

/**
 * @tc.name: HandleWriteValueTest001
 * @tc.desc: Verify the HandleWriteValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, HandleWriteValueTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleWriteValueTest001 start";
    try {
        ValuesBucket write;
        int32_t position = LOCAL;
        struct stat statInfo;

        HandleWriteValue(write, position, statInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleWriteValueTest001 failed";
    }
    GTEST_LOG_(INFO) << "HandleWriteValueTest001 end";
}

/**
 * @tc.name: HandleWriteValueTest002
 * @tc.desc: Verify the HandleWriteValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, HandleWriteValueTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleWriteValueTest002 start";
    try {
        ValuesBucket write;
        int32_t position = 1;
        struct stat statInfo;

        HandleWriteValue(write, position, statInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleWriteValueTest002 failed";
    }
    GTEST_LOG_(INFO) << "HandleWriteValueTest002 end";
}

/**
 * @tc.name: HandleWriteValueTest003
 * @tc.desc: Verify the HandleWriteValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, HandleWriteValueTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleWriteValueTest003 start";
    try {
        ValuesBucket write;
        int32_t position = CLOUD;
        struct stat statInfo;
        statInfo.st_size = 100;

        HandleWriteValue(write, position, statInfo);

        ValueObject obj;
        write.GetObject(FileColumn::FILE_SIZE, obj);
        int64_t fileSize;
        obj.GetLong(fileSize);
        EXPECT_EQ(fileSize, 100);

        write.GetObject(FileColumn::DIRTY_TYPE, obj);
        int32_t dirtyType;
        obj.GetInt(dirtyType);
        EXPECT_EQ(dirtyType, static_cast<int32_t>(DirtyType::TYPE_FDIRTY));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleWriteValueTest003 failed";
    }
    GTEST_LOG_(INFO) << "HandleWriteValueTest003 end";
}


/**
 * @tc.name: WriteUpdateDentryTest001
 * @tc.desc: Verify the WriteUpdateDentry
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, WriteUpdateDentryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteUpdateDentryTest001 start";
    try {
        std::string fileName = "mock";
        std::string cloudId;
        uint32_t userId = 1;
        std::string bundleName;
        std::string parentCloudId;
        MetaBase metaBase(fileName, cloudId);

        int32_t result = WriteUpdateDentry(metaBase, userId, bundleName, fileName, parentCloudId);
        EXPECT_EQ(result, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteUpdateDentryTest001 failed";
    }
    GTEST_LOG_(INFO) << "WriteUpdateDentryTest001 end";
}

/**
 * @tc.name: WriteUpdateDentryTest002
 * @tc.desc: Verify the WriteUpdateDentry
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, WriteUpdateDentryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteUpdateDentryTest002 start";
    try {
        std::string fileName = "filename";
        std::string cloudId;
        uint32_t userId = 1;
        std::string bundleName;
        std::string parentCloudId;
        MetaBase metaBase(fileName, cloudId);

        int32_t result = WriteUpdateDentry(metaBase, userId, bundleName, fileName, parentCloudId);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteUpdateDentryTest002 failed";
    }
    GTEST_LOG_(INFO) << "WriteUpdateDentryTest002 end";
}

/**
 * @tc.name: UpdateValueBucketsTest001
 * @tc.desc: Verify the UpdateValueBuckets
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateValueBucketsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateValueBucketsTest001 start";
    try {
        MetaBase metaBase;
        std::string filePath = "";
        int32_t status = 1;
        ValuesBucket fileInfo;
        int64_t fileSize = 0;
        struct stat statInfo{};
        int32_t result = UpdateValueBuckets(metaBase, filePath, status, fileInfo, fileSize);
        EXPECT_NE(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateValueBucketsTest001 failed";
    }
    GTEST_LOG_(INFO) << "UpdateValueBucketsTest001 end";
}

/**
 * @tc.name: UpdateValueBucketsTest002
 * @tc.desc: Verify the UpdateValueBuckets
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateValueBucketsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateValueBucketsTest002 start";
    try {
        MetaBase metaBase;
        std::string filePath = "/data/service/el2/100/hmdfs/cloud/";
        int32_t status = 1;
        ValuesBucket fileInfo;
        int64_t fileSize = 0;
        struct stat statInfo{};
        int32_t result = UpdateValueBuckets(metaBase, filePath, status, fileInfo, fileSize);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateValueBucketsTest002 failed";
    }
    GTEST_LOG_(INFO) << "UpdateValueBucketsTest002 end";
}

/**
 * @tc.name: RecycleSetValueTest001
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleSetValueTest001 start";
    try {
        TrashOptType val = TrashOptType::RESTORE;
        ValuesBucket setXAttr;
        int32_t position = -1;
        int32_t dirtyType = -1;

        int32_t result = RecycleSetValue(val, setXAttr, position, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RecycleSetValueTest001 failed";
    }
    GTEST_LOG_(INFO) << "RecycleSetValueTest001 end";
}

/**
 * @tc.name: RecycleSetValueTest002
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleSetValueTest002 start";
    try {
        TrashOptType val = TrashOptType::RESTORE;
        ValuesBucket setXAttr;
        int32_t position = -1;
        int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);

        int32_t result = RecycleSetValue(val, setXAttr, position, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RecycleSetValueTest002 failed";
    }
    GTEST_LOG_(INFO) << "RecycleSetValueTest002 end";
}

/**
 * @tc.name: RecycleSetValueTest003
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleSetValueTest003 start";
    try {
        TrashOptType val = TrashOptType::RESTORE;
        ValuesBucket setXAttr;
        int32_t position = LOCAL;
        int32_t dirtyType = -1;

        int32_t result = RecycleSetValue(val, setXAttr, position, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RecycleSetValueTest003 failed";
    }
    GTEST_LOG_(INFO) << "RecycleSetValueTest003 end";
}

/**
 * @tc.name: RecycleSetValueTest004
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleSetValueTest004 start";
    try {
        TrashOptType val = TrashOptType::RECYCLE;
        ValuesBucket setXAttr;
        int32_t position = LOCAL;
        int32_t dirtyType = -1;

        int32_t result = RecycleSetValue(val, setXAttr, position, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RecycleSetValueTest004 failed";
    }
    GTEST_LOG_(INFO) << "RecycleSetValueTest004 end";
}

/**
 * @tc.name: RecycleSetValueTest005
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleSetValueTest005 start";
    try {
        int intValue = -1;
        TrashOptType val = static_cast<TrashOptType>(intValue);
        ValuesBucket setXAttr;
        int32_t position = LOCAL;
        int32_t dirtyType = -1;

        int32_t result = RecycleSetValue(val, setXAttr, position, dirtyType);
        EXPECT_EQ(result, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RecycleSetValueTest005 failed";
    }
    GTEST_LOG_(INFO) << "RecycleSetValueTest005 end";
}

/**
 * @tc.name: RecycleSetValueTest006
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleSetValueTest006 start";
    try {
        TrashOptType val = TrashOptType::RECYCLE;
        ValuesBucket setXAttr;
        int32_t position = CLOUD;
        int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_SYNCED);

        int32_t result = RecycleSetValue(val, setXAttr, position, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RecycleSetValueTest006 failed";
    }
    GTEST_LOG_(INFO) << "RecycleSetValueTest006 end";
}

/**
 * @tc.name: RecycleSetValueTest007
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleSetValueTest007 start";
    try {
        TrashOptType val = TrashOptType::RESTORE;
        ValuesBucket setXAttr;
        int32_t position = LOCAL;
        int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_SYNCED);

        int32_t result = RecycleSetValue(val, setXAttr, position, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RecycleSetValueTest007 failed";
    }
    GTEST_LOG_(INFO) << "RecycleSetValueTest007 end";
}

/**
 * @tc.name: RecycleSetValueTest008
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, RecycleSetValueTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleSetValueTest008 start";
    try {
        TrashOptType val = TrashOptType::RECYCLE;
        ValuesBucket setXAttr;
        int32_t position = CLOUD;
        int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);

        int32_t result = RecycleSetValue(val, setXAttr, position, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RecycleSetValueTest008 failed";
    }
    GTEST_LOG_(INFO) << "RecycleSetValueTest008 end";
}

/**
 * @tc.name: UpdateParentTest001
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateParentTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateParentTest001 start";
    try {
        int32_t userId = 100;
        string bundleName;
        string srcPath = "";
        string parentCloudId;

        int32_t result = UpdateParent(userId, bundleName, srcPath, parentCloudId);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateParentTest001 failed";
    }
    GTEST_LOG_(INFO) << "UpdateParentTest001 end";
}

/**
 * @tc.name: UpdateParentTest002
 * @tc.desc: Verify the RecycleSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateParentTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateParentTest002 start";
    try {
        int32_t userId = 100;
        string bundleName;
        string srcPath = "/";
        string parentCloudId;

        int32_t result = UpdateParent(userId, bundleName, srcPath, parentCloudId);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateParentTest002 failed";
    }
    GTEST_LOG_(INFO) << "UpdateParentTest002 end";
}

/**
 * @tc.name: UpdateParentTest003
 * @tc.desc: Verify the UpdateParent
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateParentTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateParentTest003 start";
    try {
        string parentCloudId;
        int32_t userId = 0;
        string bundleName = "test";
        string srcPath = "/should_fail";

        int32_t ret = UpdateParent(userId, bundleName, srcPath, parentCloudId);

        EXPECT_NE(ret, E_OK) << "ForceCreateDirectory should fail and return errno.";
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateParentTest003 failed";
    }
    GTEST_LOG_(INFO) << "UpdateParentTest003 end";
}

/**
 * @tc.name: UTCTimeMilliSecondsTest001
 * @tc.desc: Verify the UTCTimeMilliSeconds
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UTCTimeMilliSecondsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UTCTimeMilliSecondsTest001 start";
    try {
        int64_t time1 = UTCTimeMilliSeconds();
        EXPECT_GT(time1, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UTCTimeMilliSecondsTest001 failed";
    }
    GTEST_LOG_(INFO) << "UTCTimeMilliSecondsTest001 end";
}

/**
 * @tc.name: UTCTimeMilliSecondsTest002
 * @tc.desc: Verify the UTCTimeMilliSeconds
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UTCTimeMilliSecondsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UTCTimeMilliSecondsTest002 start";
    try {
        int64_t time1 = UTCTimeMilliSeconds();
        int64_t time2 = UTCTimeMilliSeconds();
        EXPECT_GE(time2, time1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UTCTimeMilliSecondsTest002 failed";
    }
    GTEST_LOG_(INFO) << "UTCTimeMilliSecondsTest002 end";
}

/**
 * @tc.name: UpdateMetabaseTest001
 * @tc.desc: Verify the UpdateMetabase
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateMetabaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateMetabaseTest001 start";
    try {
        MetaBase metaBase;
        int64_t fileTimeAdded = 1234567890;
        struct stat statInfo{};
        statInfo.st_mtim.tv_sec = 1000;
        statInfo.st_mtim.tv_nsec = 500000000;
        statInfo.st_mode = 0644;
        statInfo.st_size = 1024;

        UpdateMetabase(metaBase, fileTimeAdded, &statInfo);

        EXPECT_EQ(metaBase.atime, static_cast<uint64_t>(fileTimeAdded));
        EXPECT_EQ(metaBase.mode, statInfo.st_mode);
        EXPECT_EQ(metaBase.size, static_cast<uint64_t>(statInfo.st_size));
        EXPECT_EQ(metaBase.position, LOCAL);
        EXPECT_EQ(metaBase.fileType, FILE_TYPE_CONTENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateMetabaseTest001 failed";
    }
    GTEST_LOG_(INFO) << "UpdateMetabaseTest001 end";
}

/**
 * @tc.name: UpdateMetabaseTest002
 * @tc.desc: Verify the UpdateMetabase
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateMetabaseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateMetabaseTest002 start";
    try {
        MetaBase metaBase;
        int64_t fileTimeAdded = 0;
        struct stat statInfo{};

        UpdateMetabase(metaBase, fileTimeAdded, &statInfo);

        EXPECT_EQ(metaBase.atime, 0);
        EXPECT_EQ(metaBase.position, LOCAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateMetabaseTest002 failed";
    }
    GTEST_LOG_(INFO) << "UpdateMetabaseTest002 end";
}

/**
 * @tc.name: HandleCreateValueTest001
 * @tc.desc: Verify the HandleCreateValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, HandleCreateValueTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCreateValueTest001 start";
    try {
        ValuesBucket fileInfo;
        std::string cloudId = "testCloudId";
        std::string parentCloudId = "testParentCloudId";
        std::string fileName = "testFile.txt";
        std::string bundleName = "com.example.app";

        HandleCreateValue(fileInfo, cloudId, parentCloudId, fileName, bundleName);

        ValueObject valueObj;
        fileInfo.GetObject(FileColumn::CLOUD_ID, valueObj);
        std::string resultCloudId;
        valueObj.GetString(resultCloudId);
        EXPECT_EQ(resultCloudId, cloudId);

        fileInfo.GetObject(FileColumn::FILE_NAME, valueObj);
        std::string resultFileName;
        valueObj.GetString(resultFileName);
        EXPECT_EQ(resultFileName, fileName);

        fileInfo.GetObject(FileColumn::PARENT_CLOUD_ID, valueObj);
        std::string resultParentCloudId;
        valueObj.GetString(resultParentCloudId);
        EXPECT_EQ(resultParentCloudId, parentCloudId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCreateValueTest001 failed";
    }
    GTEST_LOG_(INFO) << "HandleCreateValueTest001 end";
}

/**
 * @tc.name: HandleCreateValueTest002
 * @tc.desc: Verify the HandleCreateValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, HandleCreateValueTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCreateValueTest002 start";
    try {
        ValuesBucket fileInfo;
        std::string cloudId = "";
        std::string parentCloudId = "";
        std::string fileName = "";
        std::string bundleName = "";

        HandleCreateValue(fileInfo, cloudId, parentCloudId, fileName, bundleName);

        ValueObject valueObj;
        fileInfo.GetObject(FileColumn::CLOUD_ID, valueObj);
        std::string resultCloudId;
        valueObj.GetString(resultCloudId);
        EXPECT_EQ(resultCloudId, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCreateValueTest002 failed";
    }
    GTEST_LOG_(INFO) << "HandleCreateValueTest002 end";
}

/**
 * @tc.name: UpdateFileTriggerSyncTest001
 * @tc.desc: Verify the UpdateFileTriggerSync
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateFileTriggerSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateFileTriggerSyncTest001 start";
    try {
        RdbStoreMock store;
        EXPECT_CALL(store, GetPath()).WillOnce(Return("/path/to/cloudfile/user123/app123/rdb"));

        std::string result = UpdateFileTriggerSync(store);
        EXPECT_FALSE(result.empty());
        EXPECT_NE(result.find("files_update_cloud_sync_trigger"), std::string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateFileTriggerSyncTest001 failed";
    }
    GTEST_LOG_(INFO) << "UpdateFileTriggerSyncTest001 end";
}

/**
 * @tc.name: UpdateFileTriggerSyncTest002
 * @tc.desc: Verify the UpdateFileTriggerSync
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, UpdateFileTriggerSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateFileTriggerSyncTest002 start";
    try {
        RdbStoreMock store;
        EXPECT_CALL(store, GetPath()).WillOnce(Return(""));

        std::string result = UpdateFileTriggerSync(store);
        EXPECT_FALSE(result.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateFileTriggerSyncTest002 failed";
    }
    GTEST_LOG_(INFO) << "UpdateFileTriggerSyncTest002 end";
}

/**
 * @tc.name: DeleteFileTriggerSyncTest001
 * @tc.desc: Verify the DeleteFileTriggerSync
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, DeleteFileTriggerSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteFileTriggerSyncTest001 start";
    try {
        RdbStoreMock store;
        EXPECT_CALL(store, GetPath()).WillOnce(Return("/path/to/cloudfile/user123/app123/rdb"));

        std::string result = DeleteFileTriggerSync(store);
        EXPECT_FALSE(result.empty());
        EXPECT_NE(result.find("files_delete_cloud_sync_trigger"), std::string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteFileTriggerSyncTest001 failed";
    }
    GTEST_LOG_(INFO) << "DeleteFileTriggerSyncTest001 end";
}

/**
 * @tc.name: DeleteFileTriggerSyncTest002
 * @tc.desc: Verify the DeleteFileTriggerSync
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, DeleteFileTriggerSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteFileTriggerSyncTest002 start";
    try {
        RdbStoreMock store;
        EXPECT_CALL(store, GetPath()).WillOnce(Return(""));

        std::string result = DeleteFileTriggerSync(store);
        EXPECT_FALSE(result.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteFileTriggerSyncTest002 failed";
    }
    GTEST_LOG_(INFO) << "DeleteFileTriggerSyncTest002 end";
}

/**
 * @tc.name: LocalFileTriggerSyncTest001
 * @tc.desc: Verify the LocalFileTriggerSync
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, LocalFileTriggerSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalFileTriggerSyncTest001 start";
    try {
        RdbStoreMock store;
        EXPECT_CALL(store, GetPath()).WillOnce(Return("/path/to/cloudfile/user123/app123/rdb"));

        std::string result = LocalFileTriggerSync(store);
        EXPECT_FALSE(result.empty());
        EXPECT_NE(result.find("files_local_cloud_sync_trigger"), std::string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalFileTriggerSyncTest001 failed";
    }
    GTEST_LOG_(INFO) << "LocalFileTriggerSyncTest001 end";
}

/**
 * @tc.name: LocalFileTriggerSyncTest002
 * @tc.desc: Verify the LocalFileTriggerSync
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, LocalFileTriggerSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalFileTriggerSyncTest002 start";
    try {
        RdbStoreMock store;
        EXPECT_CALL(store, GetPath()).WillOnce(Return(""));

        std::string result = LocalFileTriggerSync(store);
        EXPECT_FALSE(result.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalFileTriggerSyncTest002 failed";
    }
    GTEST_LOG_(INFO) << "LocalFileTriggerSyncTest002 end";
}

/**
 * @tc.name: ExtAttrJsonValueTest001
 * @tc.desc: Verify the ExtAttrJsonValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttrJsonValueTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtAttrJsonValueTest001 start";
    try {
        std::string jsonValue;
        ValuesBucket setXAttr;
        int32_t pos = 0;
        int32_t dirtyType = 0;

        int32_t result = ExtAttrJsonValue(jsonValue, setXAttr, pos, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtAttrJsonValueTest001 failed";
    }
    GTEST_LOG_(INFO) << "ExtAttrJsonValueTest001 end";
}

/**
 * @tc.name: ExtAttrJsonValueTest002
 * @tc.desc: Verify the ExtAttrJsonValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttrJsonValueTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtAttrJsonValueTest002 start";
    try {
        std::string jsonValue;
        ValuesBucket setXAttr;
        int32_t pos = 0;
        int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);

        int32_t result = ExtAttrJsonValue(jsonValue, setXAttr, pos, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtAttrJsonValueTest002 failed";
    }
    GTEST_LOG_(INFO) << "ExtAttrJsonValueTest002 end";
}

/**
 * @tc.name: ExtAttrJsonValueTest003
 * @tc.desc: Verify the ExtAttrJsonValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttrJsonValueTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtAttrJsonValueTest003 start";
    try {
        std::string jsonValue;
        ValuesBucket setXAttr;
        int32_t pos = 1;
        int32_t dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);

        int32_t result = ExtAttrJsonValue(jsonValue, setXAttr, pos, dirtyType);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtAttrJsonValueTest003 failed";
    }
    GTEST_LOG_(INFO) << "ExtAttrJsonValueTest003 end";
}

/**
 * @tc.name: GenCloudSyncTriggerFuncParamsTest001
 * @tc.desc: Verify the GenCloudSyncTriggerFuncParams
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenCloudSyncTriggerFuncParamsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest001 start";
    try {
        std::string userId;
        std::string bundleName;
        RdbStoreMock store;
        RdbStoreMock *rdb = &store;
        std::string path = "/path/to/cloudfile/user123/app123/rdb";
        EXPECT_CALL(*rdb, GetPath()).WillOnce(Return(path));

        GenCloudSyncTriggerFuncParams(store, userId, bundleName);
        EXPECT_EQ(userId, "user123");
        EXPECT_EQ(bundleName, "app123");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest001 failed";
    }
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest001 end";
}

/**
 * @tc.name: GenCloudSyncTriggerFuncParamsTest002
 * @tc.desc: Verify the GenCloudSyncTriggerFuncParams
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenCloudSyncTriggerFuncParamsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest002 start";
    try {
        std::string userId;
        std::string bundleName;
        RdbStoreMock store;
        RdbStoreMock *rdb = &store;
        std::string path = "/path/to/user123/app123/rdb";
        EXPECT_CALL(*rdb, GetPath()).WillOnce(Return(path));

        GenCloudSyncTriggerFuncParams(store, userId, bundleName);
        EXPECT_TRUE(userId.empty());
        EXPECT_TRUE(bundleName.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest002 failed";
    }
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest002 end";
}

/**
 * @tc.name: GenCloudSyncTriggerFuncParamsTest003
 * @tc.desc: Verify the GenCloudSyncTriggerFuncParams
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenCloudSyncTriggerFuncParamsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest003 start";
    try {
        std::string userId;
        std::string bundleName;
        RdbStoreMock store;
        RdbStoreMock *rdb = &store;
        std::string path = "/path/to/cloudfile/user123/app123";
        EXPECT_CALL(*rdb, GetPath()).WillOnce(Return(path));

        GenCloudSyncTriggerFuncParams(store, userId, bundleName);
        EXPECT_TRUE(userId.empty());
        EXPECT_TRUE(bundleName.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest003 failed";
    }
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest003 end";
}

/**
 * @tc.name: GenCloudSyncTriggerFuncParamsTest004
 * @tc.desc: Verify the GenCloudSyncTriggerFuncParams
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenCloudSyncTriggerFuncParamsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest004 start";
    try {
        std::string userId;
        std::string bundleName;
        RdbStoreMock store;
        RdbStoreMock *rdb = &store;
        std::string path = "/path/to/cloudfile/user123app123/rdb";
        EXPECT_CALL(*rdb, GetPath()).WillOnce(Return(path));

        GenCloudSyncTriggerFuncParams(store, userId, bundleName);
        EXPECT_TRUE(userId.empty());
        EXPECT_TRUE(bundleName.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest004 failed";
    }
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest004 end";
}

/**
 * @tc.name: GenCloudSyncTriggerFuncParamsTest005
 * @tc.desc: Verify the GenCloudSyncTriggerFuncParams
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenCloudSyncTriggerFuncParamsTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest005 start";
    try {
        std::string userId;
        std::string bundleName;
        RdbStoreMock store;
        RdbStoreMock *rdb = &store;
        std::string path = "";
        EXPECT_CALL(*rdb, GetPath()).WillOnce(Return(path));

        GenCloudSyncTriggerFuncParams(store, userId, bundleName);
        EXPECT_TRUE(userId.empty());
        EXPECT_TRUE(bundleName.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest005 failed";
    }
    GTEST_LOG_(INFO) << "GenCloudSyncTriggerFuncParamsTest005 end";
}

/**
 * @tc.name: CreateFolderTriggerSyncTest001
 * @tc.desc: Verify the CreateFolderTriggerSync
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, CreateFolderTriggerSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFolderTriggerSyncTest001 start";
    try {
        RdbStoreMock store;
        static const string CREATE_FOLDERS_NEW_CLOUD_SYNC =
            "CREATE TRIGGER folders_new_cloud_sync_trigger AFTER INSERT ON CloudDisk FOR EACH ROW WHEN new.isDirectory "
            "== 1 AND new.dirty_type == 1 BEGIN SELECT cloud_sync_func('', ''); END;";
        std::string result = CreateFolderTriggerSync(store);
        EXPECT_EQ(result, CREATE_FOLDERS_NEW_CLOUD_SYNC);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateFolderTriggerSyncTest001 failed";
    }
    GTEST_LOG_(INFO) << "CreateFolderTriggerSyncTest001 end";
}

} // namespace OHOS::FileManagement::CloudDisk::Test