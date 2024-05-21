/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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

#include "cloud_file_utils.h"
#include "clouddisk_rdb_utils.h"
#include "clouddisk_rdbstore.h"
#include "clouddisk_rdbstore_mock.h"
#include "clouddisk_resultset_mock.h"
#include "dfs_error.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"
#include "sync_rule/cloud_status.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;

class CloudDiskRdbStoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskRdbStoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskRdbStoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskRdbStoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskRdbStoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RdbInit
 * @tc.desc: Verify the CloudDiskRdbStore::RdbInit function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RdbInitTest1, TestSize.Level1)
{
    const std::string bundleName = "RdbInitTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.RdbInit();
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetRaw
 * @tc.desc: Verify the CloudDiskRdbStore::GetRaw function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetRawTest1, TestSize.Level1)
{
    const std::string bundleName = "InitRootIdTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    EXPECT_TRUE(CloudDiskRdbStore.GetRaw());
}

/**
 * @tc.name: LookUp
 * @tc.desc: Verify the CloudDiskRdbStore::LookUp function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, LookUpTest1, TestSize.Level1)
{
    const std::string parentCloudId = "LookUpTest";
    const std::string fileName = "";
    const std::string bundleName = "InitRootIdTest";
    const int32_t userId = 123456789;
    CloudDiskFileInfo info;
    info.fileName = "test.txt";
    info.cloudId = "1223456";
    info.parentCloudId = "22222";
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.LookUp(parentCloudId, fileName, info);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: LookUp
 * @tc.desc: Verify the CloudDiskRdbStore::LookUp function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, LookUpTest2, TestSize.Level1)
{
    const std::string parentCloudId = "";
    const std::string fileName = "LookUpTest";
    const std::string bundleName = "InitRootIdTest";
    const int32_t userId = 123456789;
    CloudDiskFileInfo info;
    info.fileName = "test.txt";
    info.cloudId = "1223456";
    info.parentCloudId = "22222";
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.LookUp(parentCloudId, fileName, info);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetAttrTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string parentCloudId = "123456";
    const std::string fileName = "LookUpTest";
    const std::string bundleName = "InitRootIdTest";
    const int32_t userId = 123456789;
    CloudDiskFileInfo info;
    info.fileName = "test.txt";
    info.cloudId = "";
    info.parentCloudId = "22222";
    DriveKit::DKRecordId rootId_ = "22222";
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.GetAttr(cloudId, info);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetAttrTest2, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string parentCloudId = "123456";
    const std::string fileName = "LookUpTest";
    const std::string bundleName = "InitRootIdTest";
    const int32_t userId = 123456789;
    CloudDiskFileInfo info;
    info.fileName = "test.txt";
    info.cloudId = "rootId";
    info.parentCloudId = "22222";
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.GetAttr(cloudId, info);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string parentCloudId = "3246213";
    const std::string fileName = "test.txt";
    const std::string filePath = "/data/test";
    const std::string bundleName = "MkDirTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest2, TestSize.Level1)
{
    const std::string cloudId = "123123";
    const std::string parentCloudId = "";
    const std::string fileName = "test.txt";
    const std::string filePath = "/data/test";
    const std::string bundleName = "MkDirTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest3, TestSize.Level1)
{
    const std::string cloudId = "123123";
    const std::string parentCloudId = "123456";
    const std::string fileName = "";
    const std::string filePath = "/data/test";
    const std::string bundleName = "MkDirTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest4, TestSize.Level1)
{
    const std::string cloudId = "123123";
    const std::string parentCloudId = "123456";
    const std::string fileName = "1.txt";
    const std::string filePath = "/data/test";
    const std::string bundleName = "MkDirTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName);
    EXPECT_EQ(ret, E_PATH);
}

/**
 * @tc.name: MkDir
 * @tc.desc: Verify the CloudDiskRdbStore::MkDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, MkDirTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string parentCloudId = "123456";
    const std::string directoryName = "nullptr";
    const std::string bundleName = "MkDirTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.MkDir(cloudId, parentCloudId, directoryName);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: MkDir
 * @tc.desc: Verify the CloudDiskRdbStore::MkDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, MkDirTest2, TestSize.Level1)
{
    const std::string cloudId = "nullptr";
    const std::string parentCloudId = "";
    const std::string directoryName = "nullptr";
    const std::string bundleName = "MkDirTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.MkDir(cloudId, parentCloudId, directoryName);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: MkDir
 * @tc.desc: Verify the CloudDiskRdbStore::MkDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, MkDirTest3, TestSize.Level1)
{
    const std::string cloudId = "nullptr";
    const std::string parentCloudId = "123456";
    const std::string directoryName = "";
    const std::string bundleName = "MkDirTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.MkDir(cloudId, parentCloudId, directoryName);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: Write
 * @tc.desc: Verify the CloudDiskRdbStore::Write function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, WriteTest1, TestSize.Level1)
{
    const std::string cloudId = "nullptr";
    const std::string bundleName = "WriteTest";
    const std::string fileName = "file";
    const std::string parentCloudId = "rootId";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Write(fileName, parentCloudId, cloudId);
    EXPECT_EQ(ret, E_PATH);
}

/**
 * @tc.name: Write
 * @tc.desc: Verify the CloudDiskRdbStore::Write function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, WriteTest2, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string bundleName = "WriteTest";
    const std::string fileName = "file";
    const std::string parentCloudId = "rootId";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Write(fileName, parentCloudId, cloudId);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Write
 * @tc.desc: Verify the CloudDiskRdbStore::Write function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, WriteTest3, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string bundleName = "WriteTest";
    const int32_t userId = 123456789;
    const std::string fileName = "file";
    const std::string parentCloudId = "rootId";
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Write(fileName, parentCloudId, cloudId);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Write
 * @tc.desc: Verify the CloudDiskRdbStore::Write function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, WriteTest4, TestSize.Level1)
{
    const std::string cloudId = "123123";
    const std::string filePath = "/data/test";
    const std::string bundleName = "MkDirTest";
    const std::string fileName = "test";
    const std::string parentCloudId = "rootId";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Write(fileName, parentCloudId, cloudId);
    EXPECT_EQ(ret, E_PATH);
}

/**
 * @tc.name: GetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetXAttrTest1, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string bundleName = "WriteTest";
    const std::string key = "user.cloud.location";
    std::string value = "test";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.GetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetXAttrTest2, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string bundleName = "WriteTest";
    const std::string key = "user.cloud.location";
    std::string value = "test";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.GetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: SetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetXAttrTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string bundleName = "WriteTest";
    const std::string key = "11111";
    std::string value = "test";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: SetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetXAttrTest2, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string bundleName = "WriteTest";
    const std::string key = "1234";
    std::string value = "test";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: SetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetXAttrTest3, TestSize.Level1)
{
    const std::string cloudId = "nullptr";
    const std::string bundleName = "WriteTest";
    const std::string key = "user.cloud.location";
    std::string value = "test";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: SetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetXAttrTest4, TestSize.Level1)
{
    const std::string cloudId = "nullptr";
    const std::string bundleName = "WriteTest";
    const std::string key = "1231233";
    std::string value = "4";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: Rename
 * @tc.desc: Verify the CloudDiskRdbStore::Rename function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RenameTest1, TestSize.Level1)
{
    const std::string oldParentCloudId = "";
    const std::string oldFileName = "test.txt";
    const std::string newParentCloudId = "4461356";
    const std::string newFileName = "test1.txt";
    const std::string bundleName = "WriteTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Rename(oldParentCloudId, oldFileName, newParentCloudId, newFileName);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Rename
 * @tc.desc: Verify the CloudDiskRdbStore::Rename function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RenameTest2, TestSize.Level1)
{
    const std::string oldParentCloudId = "123456";
    const std::string oldFileName = "test.txt";
    const std::string newParentCloudId = "";
    const std::string newFileName = "test1.txt";
    const std::string bundleName = "WriteTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Rename(oldParentCloudId, oldFileName, newParentCloudId, newFileName);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Rename
 * @tc.desc: Verify the CloudDiskRdbStore::Rename function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RenameTest3, TestSize.Level1)
{
    const std::string oldParentCloudId = "123456";
    const std::string oldFileName = "test.txt";
    const std::string newParentCloudId = "4461356";
    const std::string newFileName = "";
    const std::string bundleName = "WriteTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Rename(oldParentCloudId, oldFileName, newParentCloudId, newFileName);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: Rename
 * @tc.desc: Verify the CloudDiskRdbStore::Rename function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RenameTest4, TestSize.Level1)
{
    const std::string oldParentCloudId = "123456";
    const std::string oldFileName = "";
    const std::string newParentCloudId = "4461356";
    const std::string newFileName = "test1.txt";
    const std::string bundleName = "WriteTest";
    const int32_t userId = 123456789;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Rename(oldParentCloudId, oldFileName, newParentCloudId, newFileName);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Unlink
 * @tc.desc: Verify the CloudDiskRdbStore::Unlink function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, UnlinkTest1, TestSize.Level1)
{
    string unlinkCloudId = "213546";
    const std::string bundleName = "InitRootIdTest";
    const int32_t userId = 123456789;
    int32_t position = LOCAL;
    CloudDiskRdbStore CloudDiskRdbStore(bundleName, userId);
    int32_t ret = CloudDiskRdbStore.Unlink(unlinkCloudId, position);
    EXPECT_EQ(ret, E_OK);
}

} // namespace OHOS::FileManagement::CloudDisk::Test