/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "cloud_status.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;

static const string BUNDLE_NAME = "com.ohos.photos";
static const int32_t USER_ID = 100;
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
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
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
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
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
    const std::string parentCloudId = "";
    const std::string fileName = "";
    CloudDiskFileInfo info;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
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
    const std::string parentCloudId = "rootId";
    const std::string fileName = "test";
    CloudDiskFileInfo info;
    info.name = "mock";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.LookUp(parentCloudId, fileName, info);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: LookUp
 * @tc.desc: Verify the CloudDiskRdbStore::LookUp function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, LookUpTest3, TestSize.Level1)
{
    const std::string parentCloudId = "rootId";
    const std::string fileName = "test";
    CloudDiskFileInfo info;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.LookUp(parentCloudId, fileName, info);
    EXPECT_EQ(ret, E_OK);
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
    const std::string fileName = "test";
    CloudDiskFileInfo info;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
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
    const std::string cloudId = "100";
    CloudDiskFileInfo info;
    info.name = "mock";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetAttr(cloudId, info);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetAttrTest3, TestSize.Level1)
{
    const std::string cloudId = "100";
    CloudDiskFileInfo info;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetAttr(cloudId, info);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: SetAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetAttrTest1, TestSize.Level1)
{
    const std::string fileName = "Test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "";
    const unsigned long long size = 0;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetAttr(fileName, parentCloudId, cloudId, size);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: SetAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetAttrTest2, TestSize.Level1)
{
    const std::string fileName = "Test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "rootId";
    const unsigned long long size = 0;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetAttr(fileName, parentCloudId, cloudId, size);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: SetAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetAttrTest3, TestSize.Level1)
{
    const std::string fileName = "mock";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const unsigned long long size = 0;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetAttr(fileName, parentCloudId, cloudId, size);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: SetAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetAttrTest4, TestSize.Level1)
{
    const std::string fileName = "test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const unsigned long long size = 0;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetAttr(fileName, parentCloudId, cloudId, size);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: ReadDir
 * @tc.desc: Verify the CloudDiskRdbStore::ReadDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, ReadDirTest1, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    vector<CloudDiskFileInfo> infos;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.ReadDir(cloudId, infos);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: ReadDir
 * @tc.desc: Verify the CloudDiskRdbStore::ReadDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, ReadDirTest2, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    CloudDiskFileInfo info;
    vector<CloudDiskFileInfo> infos;
    infos.push_back(info);
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.ReadDir(cloudId, infos);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest1, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string parentCloudId = "100";
    const std::string fileName = "";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName, noNeedUpload);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest2, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string parentCloudId = "100";
    const std::string fileName = "<|>";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName, noNeedUpload);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest3, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string parentCloudId = "100";
    const std::string fileName = " test";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName, noNeedUpload);
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
    const std::string cloudId = "";
    const std::string parentCloudId = "";
    const std::string fileName = "test";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName, noNeedUpload);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest5, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string parentCloudId = "100";
    const std::string fileName = "test";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName, noNeedUpload);
    EXPECT_EQ(ret, E_PATH);
}

/**
 * @tc.name: Create
 * @tc.desc: Verify the CloudDiskRdbStore::Create function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, CreateTest6, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string parentCloudId = "100";
    const std::string fileName = "test";
    bool noNeedUpload = true;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Create(cloudId, parentCloudId, fileName, noNeedUpload);
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
    const std::string cloudId = "100";
    const std::string parentCloudId = "100";
    const std::string directoryName = "";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.MkDir(cloudId, parentCloudId, directoryName, noNeedUpload);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: MkDir
 * @tc.desc: Verify the CloudDiskRdbStore::MkDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, MkDirTest2, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string parentCloudId = "100";
    const std::string directoryName = ".trash";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.MkDir(cloudId, parentCloudId, directoryName, noNeedUpload);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: MkDir
 * @tc.desc: Verify the CloudDiskRdbStore::MkDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, MkDirTest3, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string parentCloudId = "";
    const std::string directoryName = "test";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.MkDir(cloudId, parentCloudId, directoryName, noNeedUpload);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: MkDir
 * @tc.desc: Verify the CloudDiskRdbStore::MkDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, MkDirTest4, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string parentCloudId = "100";
    const std::string directoryName = "mock";
    bool noNeedUpload = false;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.MkDir(cloudId, parentCloudId, directoryName, noNeedUpload);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: MkDir
 * @tc.desc: Verify the CloudDiskRdbStore::MkDir function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, MkDirTest5, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string parentCloudId = "100";
    const std::string directoryName = "test";
    bool noNeedUpload = true;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.MkDir(cloudId, parentCloudId, directoryName, noNeedUpload);
}

/**
 * @tc.name: Write
 * @tc.desc: Verify the CloudDiskRdbStore::Write function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, WriteTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string parentCloudId = "100";
    const std::string fileName = "file";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Write(fileName, parentCloudId, cloudId);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Write
 * @tc.desc: Verify the CloudDiskRdbStore::Write function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, WriteTest2, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string fileName = "file";
    const std::string parentCloudId = "rootId";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Write(fileName, parentCloudId, cloudId);
    EXPECT_EQ(ret, E_PATH);
}

/**
 * @tc.name: LocationSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::LocationSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, LocationSetXattrTest1, TestSize.Level1)
{
    const std::string name = "test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const std::string value = "4";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.LocationSetXattr(name, parentCloudId, cloudId, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: LocationSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::LocationSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, LocationSetXattrTest2, TestSize.Level1)
{
    const std::string name = "mock";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const std::string value = "1";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.LocationSetXattr(name, parentCloudId, cloudId, value);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: LocationSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::LocationSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, LocationSetXattrTest3, TestSize.Level1)
{
    const std::string name = "test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const std::string value = "1";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.LocationSetXattr(name, parentCloudId, cloudId, value);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetRowId
 * @tc.desc: Verify the CloudDiskRdbStore::GetRowId function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetRowIdTest1, TestSize.Level1)
{
    const std::string cloudId = "100";
    int64_t rowId = 100;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetRowId(cloudId, rowId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetParentCloudId
 * @tc.desc: Verify the CloudDiskRdbStore::GetParentCloudId function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetParentCloudIdTest1, TestSize.Level1)
{
    const std::string cloudId = "100";
    std::string parentCloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetParentCloudId(cloudId, parentCloudId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetParentCloudId
 * @tc.desc: Verify the CloudDiskRdbStore::GetParentCloudId function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetParentCloudIdTest2, TestSize.Level1)
{
    const std::string cloudId = "100";
    std::string parentCloudId = "mock";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetParentCloudId(cloudId, parentCloudId);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: RecycleSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::RecycleSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RecycleSetXattrTest1, TestSize.Level1)
{
    const std::string name = "test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const std::string value = "notnum";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.RecycleSetXattr(name, parentCloudId, cloudId, value);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: RecycleSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::RecycleSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RecycleSetXattrTest2, TestSize.Level1)
{
    const std::string name = "test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const std::string value = "2";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.RecycleSetXattr(name, parentCloudId, cloudId, value);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: RecycleSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::RecycleSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RecycleSetXattrTest3, TestSize.Level1)
{
    const std::string name = "test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const std::string value = "0";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.RecycleSetXattr(name, parentCloudId, cloudId, value);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: RecycleSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::RecycleSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RecycleSetXattrTest4, TestSize.Level1)
{
    const std::string name = "test";
    const std::string parentCloudId = "100";
    const std::string cloudId = "100";
    const std::string value = "1";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.RecycleSetXattr(name, parentCloudId, cloudId, value);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: FavoriteSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::FavoriteSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, FavoriteSetXattrTest1, TestSize.Level1)
{
    const std::string cloudId = "rootId";
    const std::string value = "notnum";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.FavoriteSetXattr(cloudId, value);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: FavoriteSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::FavoriteSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, FavoriteSetXattrTest2, TestSize.Level1)
{
    const std::string cloudId = "root";
    const std::string value = "0";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.FavoriteSetXattr(cloudId, value);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: FavoriteSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::FavoriteSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, FavoriteSetXattrTest3, TestSize.Level1)
{
    const std::string cloudId = "root";
    const std::string value = "1";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.FavoriteSetXattr(cloudId, value);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: FavoriteSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::FavoriteSetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, FavoriteSetXattrTest4, TestSize.Level1)
{
    const std::string cloudId = "root";
    const std::string value = "2";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.FavoriteSetXattr(cloudId, value);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: LocationGetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::LocationGetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, LocationGetXattrTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = IS_FAVORITE_XATTR;
    std::string value = "";
    const std::string name = "test";
    const std::string parentCloudId = "rootId";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.LocationGetXattr(name, key, value, parentCloudId);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: LocationGetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::LocationGetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, LocationGetXattrTest2, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = CLOUD_FILE_LOCATION;
    std::string value = "";
    const std::string name = "test";
    const std::string parentCloudId = "rootId";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.LocationGetXattr(name, key, value, parentCloudId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: FavoriteGetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::FavoriteGetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, FavoriteGetXattrTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = CLOUD_FILE_LOCATION;
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.FavoriteGetXattr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: FavoriteGetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::FavoriteGetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, FavoriteGetXattrTest2, TestSize.Level1)
{
    const std::string cloudId = "cloudId";
    const std::string key = IS_FAVORITE_XATTR;
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.FavoriteGetXattr(cloudId, key, value);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: FileStatusGetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::FileStatusGetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, FileStatusGetXattrTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = IS_FAVORITE_XATTR;
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.FileStatusGetXattr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: FileStatusGetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::FileStatusGetXattr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, FileStatusGetXattrTest2, TestSize.Level1)
{
    const std::string cloudId = "cloudId";
    const std::string key = IS_FILE_STATUS_XATTR;
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.FileStatusGetXattr(cloudId, key, value);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetExtAttrValue
 * @tc.desc: Verify the CloudDiskRdbStore::ExtAttributeSetXattr function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreTest, GetExtAttrValueTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = "";
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetExtAttrValue(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetExtAttrValue
 * @tc.desc: Verify the CloudDiskRdbStore::ExtAttributeSetXattr function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreTest, GetExtAttrValueTest2, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string key = "100";
    std::string value = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetExtAttrValue(cloudId, key, value);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetExtAttr
 * @tc.desc: Verify the CloudDiskRdbStore::ExtAttributeSetXattr function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreTest, GetExtAttrTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    std::string value = "";
    int32_t pos = 0;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetExtAttr(cloudId, value, pos);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetExtAttr
 * @tc.desc: Verify the CloudDiskRdbStore::ExtAttributeSetXattr function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreTest, GetExtAttrTest2, TestSize.Level1)
{
    const std::string cloudId = "100";
    std::string value = "mock";
    int32_t pos = 0;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetExtAttr(cloudId, value, pos);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetExtAttr
 * @tc.desc: Verify the CloudDiskRdbStore::ExtAttributeSetXattr function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreTest, GetExtAttrTest3, TestSize.Level1)
{
    const std::string cloudId = "100";
    std::string value = "";
    int32_t pos = 0;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetExtAttr(cloudId, value, pos);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetXAttrTest1, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = CLOUD_FILE_LOCATION;
    std::string value = "";
    CacheNode node;
    node.fileName = "test";
    node.parentCloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_OK);
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
    const std::string key = IS_FAVORITE_XATTR;
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetXAttrTest3, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = IS_FILE_STATUS_XATTR;
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetXAttrTest4, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = CLOUD_EXT_ATTR;
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::GetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetXAttrTest5, TestSize.Level1)
{
    const std::string cloudId = "";
    const std::string key = CLOUD_CLOUD_RECYCLE_XATTR;
    std::string value = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetXAttr(cloudId, key, value);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: ExtAttributeSetXattr
 * @tc.desc: Verify the CloudDiskRdbStore::ExtAttributeSetXattr function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDiskRdbStoreTest, ExtAttributeSetXattrTest1, TestSize.Level1)
{
    const std::string cloudId = "root";
    const std::string key = "user.cloud.test1";
    const std::string value = "1";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.ExtAttributeSetXattr(cloudId, value, key);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: SetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetXAttrTest1, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string key = CLOUD_FILE_LOCATION;
    const std::string value = "notnum";
    const std::string name = "test";
    const std::string parentCloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value, name, parentCloudId);
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
    const std::string cloudId = "100";
    const std::string key = CLOUD_CLOUD_RECYCLE_XATTR;
    const std::string value = "notnum";
    const std::string name = "test";
    const std::string parentCloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value, name, parentCloudId);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: SetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetXAttrTest3, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string key = IS_FAVORITE_XATTR;
    const std::string value = "notnum";
    const std::string name = "test";
    const std::string parentCloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value, name, parentCloudId);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: SetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetXAttrTest4, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string key = CLOUD_EXT_ATTR;
    const std::string value = "notnum";
    const std::string name = "test";
    const std::string parentCloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value, name, parentCloudId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: SetXAttr
 * @tc.desc: Verify the CloudDiskRdbStore::SetXAttr function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, SetXAttrTest5, TestSize.Level1)
{
    const std::string cloudId = "100";
    const std::string key = IS_FILE_STATUS_XATTR;
    const std::string value = "notnum";
    const std::string name = "test";
    const std::string parentCloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.SetXAttr(cloudId, key, value, name, parentCloudId);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Rename
 * @tc.desc: Verify the CloudDiskRdbStore::Rename function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RenameTest1, TestSize.Level1)
{
    const std::string oldParentCloudId = "100";
    const std::string oldFileName = "test";
    const std::string newParentCloudId = "100";
    const std::string newFileName = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Rename(oldParentCloudId, oldFileName, newParentCloudId, newFileName);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: Rename
 * @tc.desc: Verify the CloudDiskRdbStore::Rename function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, RenameTest2, TestSize.Level1)
{
    const std::string oldParentCloudId = "";
    const std::string oldFileName = "";
    const std::string newParentCloudId = "";
    const std::string newFileName = "test";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
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
    const std::string oldParentCloudId = "100";
    const std::string oldFileName = "test";
    const std::string newParentCloudId = "100";
    const std::string newFileName = " test";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Rename(oldParentCloudId, oldFileName, newParentCloudId, newFileName);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: GetHasChild
 * @tc.desc: Verify the CloudDiskRdbStore::GetHasChild function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetHasChildTest1, TestSize.Level1)
{
    const std::string cloudId = "100";
    bool hasChild = true;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetHasChild(cloudId, hasChild);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: UnlinkSynced
 * @tc.desc: Verify the CloudDiskRdbStore::UnlinkSynced function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, UnlinkSyncedTest1, TestSize.Level1)
{
    const std::string cloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.UnlinkSynced(cloudId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: UnlinkLocal
 * @tc.desc: Verify the CloudDiskRdbStore::UnlinkLocal function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, UnlinkLocalTest1, TestSize.Level1)
{
    const std::string cloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.UnlinkLocal(cloudId);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: Unlink
 * @tc.desc: Verify the CloudDiskRdbStore::Unlink function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, UnlinkTest1, TestSize.Level1)
{
    std::string cloudId = "";
    const int32_t position = LOCAL;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Unlink(cloudId, position);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: Unlink
 * @tc.desc: Verify the CloudDiskRdbStore::Unlink function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, UnlinkTest2, TestSize.Level1)
{
    const int32_t position = LOCAL;
    std::string cloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Unlink(cloudId, position);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: Unlink
 * @tc.desc: Verify the CloudDiskRdbStore::Unlink function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, UnlinkTest3, TestSize.Level1)
{
    const int32_t position = CLOUD;
    std::string cloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.Unlink(cloudId, position);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetDirtyType
 * @tc.desc: Verify the CloudDiskRdbStore::GetDirtyType function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetDirtyTypeTest1, TestSize.Level1)
{
    int32_t dirtyType = 0;
    std::string cloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetDirtyType(cloudId, dirtyType);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetCurNode
 * @tc.desc: Verify the CloudDiskRdbStore::GetCurNode function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetCurNodeTest1, TestSize.Level1)
{
    CacheNode curNode;
    std::string cloudId = "";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetCurNode(cloudId, curNode);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetCurNode
 * @tc.desc: Verify the CloudDiskRdbStore::GetCurNode function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetCurNodeTest2, TestSize.Level1)
{
    CacheNode curNode;
    std::string cloudId = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetCurNode(cloudId, curNode);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetParentNode
 * @tc.desc: Verify the CloudDiskRdbStore::GetParentNode function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetParentNodeTest1, TestSize.Level1)
{
    const std::string parentCloudId = "";
    std::string nextCloudId = "100";
    std::string fileName = "test";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetParentNode(parentCloudId, nextCloudId, fileName);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetParentNode
 * @tc.desc: Verify the CloudDiskRdbStore::GetParentNode function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetParentNodeTest2, TestSize.Level1)
{
    const std::string parentCloudId = "100";
    std::string nextCloudId = "100";
    std::string fileName = "test";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetParentNode(parentCloudId, nextCloudId, fileName);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetUriFromDB
 * @tc.desc: Verify the CloudDiskRdbStore::GetUriFromDB function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetUriFromDBTest1, TestSize.Level1)
{
    const std::string parentCloudId = "";
    std::string uri = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetUriFromDB(parentCloudId, uri);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: GetUriFromDB
 * @tc.desc: Verify the CloudDiskRdbStore::GetUriFromDB function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetUriFromDBTest2, TestSize.Level1)
{
    const std::string parentCloudId = "100";
    std::string uri = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetUriFromDB(parentCloudId, uri);
    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: GetNotifyUri
 * @tc.desc: Verify the CloudDiskRdbStore::GetNotifyUri function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetNotifyUriTest1, TestSize.Level1)
{
    CacheNode cacheNode;
    std::string uri = "100";
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetNotifyUri(cacheNode, uri);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: GetNotifyData
 * @tc.desc: Verify the CloudDiskRdbStore::GetNotifyData function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, GetNotifyDataTest1, TestSize.Level1)
{
    CacheNode cacheNode;
    NotifyData notifyData;
    CloudDiskRdbStore CloudDiskRdbStore(BUNDLE_NAME, USER_ID);
    int32_t ret = CloudDiskRdbStore.GetNotifyData(cacheNode, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
}

/**
 * @tc.name: OnUpgrade
 * @tc.desc: Verify the CloudDiskRdbStore::OnUpgrade function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskRdbStoreTest, OnUpgradeTest1, TestSize.Level1)
{
    RdbStoreMock store;
    int32_t oldVersion = 13;
    int32_t newVersion = 14;
    CloudDiskDataCallBack CloudDiskDataCallBack;
    int32_t ret = CloudDiskDataCallBack.OnUpgrade(store, oldVersion, newVersion);
    EXPECT_EQ(ret, E_OK);
}
} // namespace OHOS::FileManagement::CloudDisk::Test