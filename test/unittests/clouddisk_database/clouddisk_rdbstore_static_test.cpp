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

#include "directory_ex.h"

static bool g_forceCreateDirectoryShouldFail = false;

static bool MockForceCreateDirectory(const std::string& path)
{
    if (g_forceCreateDirectoryShouldFail) {
        errno = EACCES;
        return false;
    }
    return OHOS::ForceCreateDirectory(path);
}

#define ForceCreateDirectory MockForceCreateDirectory
#include "clouddisk_rdbstore.cpp"
#undef ForceCreateDirectory
#include "rdb_assistant.h"
#include "result_set_mock.h"
#include "transaction_mock.h"

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
    shared_ptr<AssistantMock> insMock = nullptr;
};

void CloudDiskRdbStoreStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskRdbStoreStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskRdbStoreStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void CloudDiskRdbStoreStaticTest::TearDown(void)
{
    Assistant::ins = nullptr;
    insMock = nullptr;
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
 * @tc.name: ExtAttributeSetValueTest005
 * @tc.desc: Verify the ExtAttributeSetValue
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, ExtAttributeSetValueTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest005 start";
    try {
        std::string jsonValue;
        const std::string key = "testKey";
        std::string value = "file://com.test.hap/data/storage/el2/cloud/\xD6\xF8\xD0\xA1/text.txt";
        std::string xattrList = "{}";

        int32_t ret = ExtAttributeSetValue(jsonValue, key, value, xattrList);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ExtAttributeSetValueTest005 failed";
    }
    GTEST_LOG_(INFO) << "ExtAttributeSetValueTest005 end";
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
        const int64_t localFlag = 0;
        const bool isWrite = false;

        HandleWriteValue(write, position, statInfo, isWrite, localFlag);
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
        const int64_t localFlag = 0;
        const bool isWrite = true;

        HandleWriteValue(write, position, statInfo, isWrite, localFlag);
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
        const int64_t localFlag = 0;
        const bool isWrite = false;
        statInfo.st_size = 100;

        HandleWriteValue(write, position, statInfo, isWrite, localFlag);

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
 * @tc.name: GetLocalFlagTest001
 * @tc.desc: Verify the GetLocalFlag
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetLocalFlagTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalFlagTest001 start";
    try {
        auto transaction = make_shared<TransactionMock>();
        const std::string cloudId = "100";
        int64_t localFlag = 0;
        bool preCount = true;

        EXPECT_CALL(*transaction, QueryByStep(An<const AbsRdbPredicates &>(),
        An<const std::vector<std::string> &>(), preCount)).WillOnce(Return(nullptr));

        int32_t ret = GetLocalFlag(transaction, cloudId, localFlag);

        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalFlagTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetLocalFlagTest001 end";
}

/**
 * @tc.name: GetLocalFlagTest002
 * @tc.desc: Verify the GetLocalFlag
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetLocalFlagTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalFlagTest002 start";
    try {
        auto transaction = make_shared<TransactionMock>();
        const std::string cloudId = "100";
        int64_t localFlag = 0;
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        bool preCount = true;

        EXPECT_CALL(*transaction, QueryByStep(An<const AbsRdbPredicates &>(),
        An<const std::vector<std::string> &>(), preCount)).WillOnce(Return(ByMove(rset)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(E_RDB));

        int32_t ret = GetLocalFlag(transaction, cloudId, localFlag);

        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalFlagTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetLocalFlagTest002 end";
}

/**
 * @tc.name: GetLocalFlagTest003
 * @tc.desc: Verify the GetLocalFlag
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetLocalFlagTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalFlagTest003 start";
    try {
        auto transaction = make_shared<TransactionMock>();
        const std::string cloudId = "100";
        int64_t localFlag = -1;
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        bool preCount = true;

        EXPECT_CALL(*transaction, QueryByStep(An<const AbsRdbPredicates &>(),
        An<const std::vector<std::string> &>(), preCount)).WillOnce(Return(ByMove(rset)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(E_OK));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(E_RDB));

        int32_t ret = GetLocalFlag(transaction, cloudId, localFlag);

        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalFlagTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetLocalFlagTest003 end";
}

/**
 * @tc.name: GetLocalFlagTest004
 * @tc.desc: Verify the GetLocalFlag
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetLocalFlagTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalFlagTest004 start";
    try {
        auto transaction = make_shared<TransactionMock>();
        const std::string cloudId = "100";
        int64_t localFlag = 0;
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        bool preCount = true;

        EXPECT_CALL(*transaction, QueryByStep(An<const AbsRdbPredicates &>(),
        An<const std::vector<std::string> &>(), preCount)).WillOnce(Return(ByMove(rset)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(E_OK));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(E_OK));

        int32_t ret = GetLocalFlag(transaction, cloudId, localFlag);

        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalFlagTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetLocalFlagTest004 end";
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

        g_forceCreateDirectoryShouldFail = true;
        int32_t ret = UpdateParent(userId, bundleName, srcPath, parentCloudId);
        g_forceCreateDirectoryShouldFail = false;

        EXPECT_NE(ret, E_OK) << "ForceCreateDirectory should fail and return errno.";
    } catch (...) {
        g_forceCreateDirectoryShouldFail = false;
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

/**
 * @tc.name: VersionFixFileTriggerTest002
 * @tc.desc: Verify the VersionFixFileTrigger with drop trigger failure
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixFileTriggerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixFileTriggerTest002 start";
    try {
        RdbStoreMock rdbStoreMock;

        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));

        VersionFixFileTrigger(rdbStoreMock);

        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixFileTriggerTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixFileTriggerTest002 end";
}

/**
 * @tc.name: VersionFixFileTriggerTest003
 * @tc.desc: Verify the VersionFixFileTrigger with add update trigger failure
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixFileTriggerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixFileTriggerTest003 start";
    try {
        RdbStoreMock rdbStoreMock;

        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));

        VersionFixFileTrigger(rdbStoreMock);

        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixFileTriggerTest003 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixFileTriggerTest003 end";
}

/**
 * @tc.name: VersionFixFileTriggerTest004
 * @tc.desc: Verify the VersionFixFileTrigger with add delete trigger failure
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixFileTriggerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixFileTriggerTest004 start";
    try {
        RdbStoreMock rdbStoreMock;

        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));

        VersionFixFileTrigger(rdbStoreMock);

        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixFileTriggerTest004 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixFileTriggerTest004 end";
}

/**
 * @tc.name: VersionFixFileTriggerTest005
 * @tc.desc: Verify the VersionFixFileTrigger with add local trigger failure
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixFileTriggerTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixFileTriggerTest005 start";
    try {
        RdbStoreMock rdbStoreMock;

        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR));

        VersionFixFileTrigger(rdbStoreMock);

        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixFileTriggerTest005 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixFileTriggerTest005 end";
}

/**
 * @tc.name: VersionFixCreateAndLocalTriggerTest001
 * @tc.desc: Trigger drop files_new_cloud_sync_trigger failure in VersionFixCreateAndLocalTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixCreateAndLocalTriggerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionFixCreateAndLocalTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest001 end";
}

/**
 * @tc.name: VersionFixCreateAndLocalTriggerTest002
 * @tc.desc: Trigger drop files_local_cloud_sync_trigger failure in VersionFixCreateAndLocalTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixCreateAndLocalTriggerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest002 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionFixCreateAndLocalTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest002 end";
}

/**
 * @tc.name: VersionFixCreateAndLocalTriggerTest003
 * @tc.desc: Trigger add local file trigger failure in VersionFixCreateAndLocalTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixCreateAndLocalTriggerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest003 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionFixCreateAndLocalTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest003 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest003 end";
}

/**
 * @tc.name: VersionFixCreateAndLocalTriggerTest004
 * @tc.desc: Trigger add create folder trigger failure in VersionFixCreateAndLocalTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixCreateAndLocalTriggerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest004 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionFixCreateAndLocalTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest004 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixCreateAndLocalTriggerTest004 end";
}

/**
 * @tc.name: VersionAddFileStatusAndErrorCodeTest001
 * @tc.desc: Trigger add is_favorite failure in VersionAddFileStatusAndErrorCode
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddFileStatusAndErrorCodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddFileStatusAndErrorCodeTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddFileStatusAndErrorCode(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddFileStatusAndErrorCodeTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddFileStatusAndErrorCodeTest001 end";
}

/**
 * @tc.name: VersionAddFileStatusTest001
 * @tc.desc: Trigger add file_status failure in VersionAddFileStatus
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddFileStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddFileStatusTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddFileStatus(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddFileStatusTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddFileStatusTest001 end";
}

/**
 * @tc.name: VersionSetFileStatusDefaultTest001
 * @tc.desc: Trigger set file_status failure in VersionSetFileStatusDefault
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionSetFileStatusDefaultTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionSetFileStatusDefaultTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionSetFileStatusDefault(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionSetFileStatusDefaultTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionSetFileStatusDefaultTest001 end";
}

/**
 * @tc.name: VersionFixSyncMetatimeTriggerTest001
 * @tc.desc: Trigger drop files_update_cloud_sync_trigger failure in VersionFixSyncMetatimeTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixSyncMetatimeTriggerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixSyncMetatimeTriggerTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionFixSyncMetatimeTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixSyncMetatimeTriggerTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixSyncMetatimeTriggerTest001 end";
}

/**
 * @tc.name: VersionFixSyncMetatimeTriggerTest002
 * @tc.desc: Trigger add update file trigger failure in VersionFixSyncMetatimeTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixSyncMetatimeTriggerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixSyncMetatimeTriggerTest002 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionFixSyncMetatimeTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixSyncMetatimeTriggerTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixSyncMetatimeTriggerTest002 end";
}

/**
 * @tc.name: VersionFixRetryTriggerTest001
 * @tc.desc: Trigger drop local file trigger failure in VersionFixRetryTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixRetryTriggerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixRetryTriggerTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionFixRetryTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixRetryTriggerTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixRetryTriggerTest001 end";
}

/**
 * @tc.name: VersionFixRetryTriggerTest002
 * @tc.desc: Trigger add local file trigger failure in VersionFixRetryTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionFixRetryTriggerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionFixRetryTriggerTest002 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionFixRetryTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionFixRetryTriggerTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionFixRetryTriggerTest002 end";
}

/**
 * @tc.name: VersionRemoveCloudSyncFuncTriggerTest001
 * @tc.desc: Trigger drop folders_new_cloud_sync_trigger failure in VersionRemoveCloudSyncFuncTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionRemoveCloudSyncFuncTriggerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionRemoveCloudSyncFuncTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest001 end";
}

/**
 * @tc.name: VersionRemoveCloudSyncFuncTriggerTest002
 * @tc.desc: Trigger drop files_update_cloud_sync_trigger failure in VersionRemoveCloudSyncFuncTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionRemoveCloudSyncFuncTriggerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest002 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionRemoveCloudSyncFuncTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest002 end";
}

/**
 * @tc.name: VersionRemoveCloudSyncFuncTriggerTest003
 * @tc.desc: Trigger drop files_delete_cloud_sync_trigger failure in VersionRemoveCloudSyncFuncTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionRemoveCloudSyncFuncTriggerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest003 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionRemoveCloudSyncFuncTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest003 failed";
    }
    GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest003 end";
}

/**
 * @tc.name: VersionRemoveCloudSyncFuncTriggerTest004
 * @tc.desc: Trigger drop files_local_cloud_sync_trigger failure in VersionRemoveCloudSyncFuncTrigger
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionRemoveCloudSyncFuncTriggerTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest004 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionRemoveCloudSyncFuncTrigger(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest004 failed";
    }
    GTEST_LOG_(INFO) << "VersionRemoveCloudSyncFuncTriggerTest004 end";
}

/**
 * @tc.name: VersionAddThmFlagTest001
 * @tc.desc: Trigger add thm_flag failure in VersionAddThmFlag
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddThmFlagTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddThmFlagTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddThmFlag(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddThmFlagTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddThmFlagTest001 end";
}

/**
 * @tc.name: VersionAddThmFlagTest002
 * @tc.desc: Trigger add lcd_flag failure in VersionAddThmFlag
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddThmFlagTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddThmFlagTest002 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddThmFlag(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddThmFlagTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddThmFlagTest002 end";
}

/**
 * @tc.name: VersionAddThmFlagTest003
 * @tc.desc: Trigger add upload_flag failure in VersionAddThmFlag
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddThmFlagTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddThmFlagTest003 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddThmFlag(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddThmFlagTest003 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddThmFlagTest003 end";
}

/**
 * @tc.name: VersionAddSrcCloudIdTest001
 * @tc.desc: Trigger add src_cloud_id failure in VersionAddSrcCloudId
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddSrcCloudIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddSrcCloudIdTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddSrcCloudId(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddSrcCloudIdTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddSrcCloudIdTest001 end";
}

/**
 * @tc.name: VersionAddThmSizeTest001
 * @tc.desc: Trigger add thm_size failure in VersionAddThmSize
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddThmSizeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddThmSizeTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddThmSize(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddThmSizeTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddThmSizeTest001 end";
}

/**
 * @tc.name: VersionAddThmSizeTest002
 * @tc.desc: Trigger add lcd_size failure in VersionAddThmSize
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddThmSizeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddThmSizeTest002 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddThmSize(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddThmSizeTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddThmSizeTest002 end";
}

/**
 * @tc.name: VersionAddThmSizeTest003
 * @tc.desc: Trigger add source_cloud_id failure in VersionAddThmSize
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddThmSizeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddThmSizeTest003 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_OK))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddThmSize(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddThmSizeTest003 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddThmSizeTest003 end";
}

/**
 * @tc.name: VersionAddLocalFlagTest001
 * @tc.desc: Trigger add local_flag failure in VersionAddLocalFlag
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddLocalFlagTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddLocalFlagTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddLocalFlag(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddLocalFlagTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddLocalFlagTest001 end";
}

/**
 * @tc.name: GetUserIdAndBundleNameTest001
 * @tc.desc: Trigger invalid user Id failure in GetUserIdAndBundleName
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GetUserIdAndBundleNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUserIdAndBundleNameTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, GetPath()).WillOnce(testing::Return("/data/cloudfile/abc123/com.example.app/rdb"));
        uint32_t userId;
        std::string bundleName;
        int32_t result = GetUserIdAndBundleName(rdbStoreMock, userId, bundleName);
        EXPECT_EQ(result, E_INVAL_ARG);
        EXPECT_EQ(bundleName, "com.example.app");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetUserIdAndBundleNameTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetUserIdAndBundleNameTest001 end";
}

/**
 * @tc.name: VersionAddAttributeTest001
 * @tc.desc: Trigger add attribute failure in VersionAddAttribute
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddAttributeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddAttributeTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_))
            .WillOnce(testing::Return(NativeRdb::E_ERROR))
            .WillRepeatedly(testing::Return(NativeRdb::E_OK));
        VersionAddAttribute(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddAttributeTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddAttributeTest001 end";
}

/**
 * @tc.name: VersionAddCheckFlagTest001
 * @tc.desc: ExecuteSql trigger CLOUD_FILE_FAULT_REPORT
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddCheckFlagTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddCheckFlagTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_)).WillOnce(testing::Return(NativeRdb::E_ERROR));
        EXPECT_CALL(rdbStoreMock, GetPath())
            .WillRepeatedly(testing::Return("/data/service/el2/100/hmdfs/cloudfile/100/com.example.app/rdb/cloud.db"));
        EXPECT_CALL(rdbStoreMock, Query(testing::_, testing::_)).WillRepeatedly(testing::Return(nullptr));
        VersionAddCheckFlag(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddCheckFlagTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddCheckFlagTest001 end";
}

/**
 * @tc.name: VersionAddCheckFlagTest002
 * @tc.desc: GenerateDentryRecursively trigger CLOUD_FILE_FAULT_REPORT
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddCheckFlagTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddCheckFlagTest002 start";
    try {
        RdbStoreMock rdbStoreMock;
        auto resultSetMock = std::make_shared<NiceMock<AbsSharedResultSetMock>>();
        // ExecuteSql 成功
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_)).WillOnce(testing::Return(NativeRdb::E_OK));
        EXPECT_CALL(rdbStoreMock, GetPath())
            .WillRepeatedly(testing::Return("/data/service/el2/100/hmdfs/cloudfile/100/com.example.app/rdb/cloud.db"));
        EXPECT_CALL(*resultSetMock, GetRowCount(testing::_))
            .WillOnce(testing::DoAll(testing::SetArgReferee<0>(1), testing::Return(E_OK)));
        EXPECT_CALL(*resultSetMock, GoToNextRow()).WillOnce(testing::Return(0));
        EXPECT_CALL(*resultSetMock, GetColumnIndex(testing::_, testing::_)).WillRepeatedly(testing::Return(E_RDB));
        auto resultSetZero = std::make_shared<NiceMock<AbsSharedResultSetMock>>();
        EXPECT_CALL(*resultSetZero, GetRowCount(testing::_))
            .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(0), testing::Return(E_OK)));
        EXPECT_CALL(rdbStoreMock, Query(testing::_, testing::_))
            .WillOnce(testing::Return(resultSetMock))
            .WillOnce(testing::Return(resultSetZero));
        VersionAddCheckFlag(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddCheckFlagTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddCheckFlagTest002 end";
}

/**
 * @tc.name: VersionAddCheckFlagTest003
 * @tc.desc: GenerateRecycleDentryRecursively trigger CLOUD_FILE_FAULT_REPORT
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddCheckFlagTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddCheckFlagTest003 start";
    try {
        RdbStoreMock rdbStoreMock;
        auto resultSetMock = std::make_shared<NiceMock<AbsSharedResultSetMock>>();
        auto resultSetZero = std::make_shared<NiceMock<AbsSharedResultSetMock>>();
        auto recycleResultSetMock = std::make_shared<NiceMock<AbsSharedResultSetMock>>();
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_)).WillOnce(testing::Return(NativeRdb::E_OK));
        EXPECT_CALL(rdbStoreMock, GetPath())
            .WillRepeatedly(testing::Return("/data/service/el2/100/hmdfs/cloudfile/100/com.ohos.photos/rdb/cloud.db"));
        EXPECT_CALL(*resultSetZero, GetRowCount(testing::_))
            .WillRepeatedly(testing::DoAll(testing::SetArgReferee<0>(0), testing::Return(E_OK)));
        EXPECT_CALL(*recycleResultSetMock, GetRowCount(testing::_))
            .WillOnce(testing::DoAll(testing::SetArgReferee<0>(1), testing::Return(E_OK)));
 	    EXPECT_CALL(*recycleResultSetMock, GoToNextRow())
 	        .WillOnce(testing::Return(0));
        EXPECT_CALL(*recycleResultSetMock, GetColumnIndex(testing::_, testing::_))
            .WillRepeatedly(testing::Return(E_RDB));
        EXPECT_CALL(rdbStoreMock, Query(testing::_, testing::_))
            .WillOnce(testing::Return(resultSetZero))
            .WillOnce(testing::Return(recycleResultSetMock));
        VersionAddCheckFlag(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddCheckFlagTest003 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddCheckFlagTest003 end";
}

/**
 * @tc.name: VersionAddRootDirectoryTest001
 * @tc.desc: ExecuteSql trigger CLOUD_FILE_FAULT_REPORT
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddRootDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddRootDirectoryTest001 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_)).WillOnce(testing::Return(NativeRdb::E_ERROR));
        EXPECT_CALL(rdbStoreMock, Update(testing::Matcher<int &>(testing::_), testing::_, testing::_, testing::_,
                                         testing::Matcher<const std::vector<ValueObject> &>(testing::_)))
            .WillOnce(testing::Return(NativeRdb::E_OK));
        VersionAddRootDirectory(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddRootDirectoryTest001 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddRootDirectoryTest001 end";
}

/**
 * @tc.name: VersionAddRootDirectoryTest002
 * @tc.desc: Update trigger CLOUD_FILE_FAULT_REPORT
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, VersionAddRootDirectoryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddRootDirectoryTest002 start";
    try {
        RdbStoreMock rdbStoreMock;
        EXPECT_CALL(rdbStoreMock, ExecuteSql(testing::_, testing::_)).WillOnce(testing::Return(NativeRdb::E_OK));
        EXPECT_CALL(rdbStoreMock, Update(testing::Matcher<int &>(testing::_), testing::_, testing::_, testing::_,
                                         testing::Matcher<const std::vector<ValueObject> &>(testing::_)))
            .WillOnce(testing::Return(NativeRdb::E_ERROR));
        VersionAddRootDirectory(rdbStoreMock);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddRootDirectoryTest002 failed";
    }
    GTEST_LOG_(INFO) << "VersionAddRootDirectoryTest002 end";
}

/**
 * @tc.name: GenerateDentryRecursivelyInvalidPathTest
 * @tc.desc: Trigger userId/bundleName failure
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenerateDentryRecursivelyInvalidPathTest, TestSize.Level1)
{
    RdbStoreMock store;
    EXPECT_CALL(store, GetPath()).Times(AtLeast(1)).WillRepeatedly(Return("cloudfile/100/invalid/rdb"));
    EXPECT_CALL(store, Query(_, _)).WillOnce(Return(nullptr));
    auto ret = GenerateDentryRecursively(store, "parent");
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GenerateDentryRecursivelyNullResultSetTest
 * @tc.desc: Trigger resultSet is null
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenerateDentryRecursivelyNullResultSetTest, TestSize.Level1)
{
    RdbStoreMock store;
    ON_CALL(store, GetPath()).WillByDefault(Return("cloudfile/100/bundleName/rdb"));
    EXPECT_CALL(store, Query(_, _)).WillOnce(Return(nullptr));
    auto ret = GenerateDentryRecursively(store, "parent");
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GenerateDentryRecursivelyNameMockTest
 * @tc.desc: Trigger metaFile->DoLookupAndUpdate(info.name, callback)
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenerateDentryRecursivelyNameMockTest, TestSize.Level1)
{
    RdbStoreMock store;
    ON_CALL(store, GetPath()).WillByDefault(
        Return("cloudfile/100/com.ohos.photos/rdb/cloud.db"));

    auto resultSet = std::make_shared<NiceMock<AbsSharedResultSetMock>>();
    EXPECT_CALL(store, Query(_, _)).WillOnce(Return(ByMove(resultSet)));
    EXPECT_CALL(*resultSet, GetRowCount(_))
        .WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));

    EXPECT_CALL(*resultSet, GoToNextRow())
        .WillOnce(Return(0))
        .WillRepeatedly(Return(E_RDB));

    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));
    EXPECT_CALL(*resultSet, GetString(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(std::string("mock")), Return(E_OK)));
    EXPECT_CALL(*resultSet, GetInt(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));
    EXPECT_CALL(*resultSet, GetLong(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));

    int32_t ret = GenerateDentryRecursively(store, "parentCloudId");
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GenerateRecycleDentryRecursivelyNullResultSetTest
 * @tc.desc: Trigger resultSet is null
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenerateRecycleDentryRecursivelyNullResultSetTest, TestSize.Level1)
{
    RdbStoreMock store;
    ON_CALL(store, GetPath()).WillByDefault(Return("cloudfile/100/bundleName/rdb"));
    EXPECT_CALL(store, Query(_, _)).WillOnce(Return(nullptr));
    auto ret = GenerateRecycleDentryRecursively(store);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GenerateRecycleDentryRecursivelyRowCountFailTest
 * @tc.desc: Trigger rowCount fault
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenerateRecycleDentryRecursivelyRowCountFailTest, TestSize.Level1)
{
    RdbStoreMock store;
    ON_CALL(store, GetPath()).WillByDefault(Return("cloudfile/100/bundleName/rdb"));

    auto resultSet = std::make_shared<AbsSharedResultSetMock>();
    EXPECT_CALL(store, Query(_, _)).WillOnce(Return(ByMove(resultSet)));
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(Return(E_RDB));

    auto ret = GenerateRecycleDentryRecursively(store);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GenerateRecycleDentryRecursivelyNameMockTest
 * @tc.desc: Trigger metaFile->DoLookupAndUpdate(uniqueName, callback)
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreStaticTest, GenerateRecycleDentryRecursivelyNameMockTest, TestSize.Level1)
{
    RdbStoreMock store;
    ON_CALL(store, GetPath()).WillByDefault(
        Return("cloudfile/100/com.ohos.photos/rdb/cloud.db"));

    auto resultSet = std::make_shared<NiceMock<AbsSharedResultSetMock>>();
    EXPECT_CALL(store, Query(_, _)).WillOnce(Return(ByMove(resultSet)));
    EXPECT_CALL(*resultSet, GetRowCount(_))
        .WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));

    EXPECT_CALL(*resultSet, GoToNextRow())
        .WillOnce(Return(0))
        .WillRepeatedly(Return(E_RDB));

    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));
    EXPECT_CALL(*resultSet, GetString(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(std::string("mock")), Return(E_OK)));
    EXPECT_CALL(*resultSet, GetInt(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));
    EXPECT_CALL(*resultSet, GetLong(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));

    int32_t ret = GenerateRecycleDentryRecursively(store);
    EXPECT_EQ(ret, E_RDB);
}
} // namespace OHOS::FileManagement::CloudDisk::Test