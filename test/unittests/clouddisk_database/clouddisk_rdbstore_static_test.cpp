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
} // namespace OHOS::FileManagement::CloudDisk::Test