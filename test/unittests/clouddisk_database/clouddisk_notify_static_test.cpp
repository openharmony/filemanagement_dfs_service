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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <memory>

#include "dfs_error.h"
#include "clouddisk_rdbstore_mock.cpp"
#include "clouddisk_notify.cpp"
#include "rdb_assistant.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;

constexpr int32_t USER_ID = 100;
class CloudDiskNotifyStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskNotifyStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskNotifyStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskNotifyStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskNotifyStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: HandleUpdateTest001
 * @tc.desc: Verify the HandleUpdate function
 * @tc.type: FUNC
 * @tc.require: issues2599
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUpdateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUpdateTest001 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "testCloudId";
        paramService.notifyType = NotifyType::NOTIFY_NONE;
        paramService.oldUri = "oldUri";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;
        
        auto rdbStore = std::make_shared<CloudDiskRdbStore>(paramOthers.bundleName, paramOthers.userId);
        CacheNode curNode;
        string uri = "";
        int32_t ret = rdbStore->GetCurNode(paramService.cloudId, curNode);
        EXPECT_EQ(ret, E_OK);
        ret = rdbStore->GetNotifyUri(curNode, uri);
        EXPECT_EQ(ret, E_OK);
        HandleUpdate(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUpdateTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUpdateTest001 End";
}

/**
 * @tc.name: HandleUpdateTest002
 * @tc.desc: Verify the HandleUpdate function
 * @tc.type: FUNC
 * @tc.require: issues2599
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUpdateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUpdateTest002 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "testCloudId";
        paramService.notifyType = NotifyType::NOTIFY_MODIFIED;
        paramService.oldUri = "oldUri";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;
        
        auto rdbStore = std::make_shared<CloudDiskRdbStore>(paramOthers.bundleName, paramOthers.userId);
        CacheNode curNode;
        string uri = "newUri";
        int32_t ret = rdbStore->GetCurNode(paramService.cloudId, curNode);
        EXPECT_EQ(ret, E_OK);
        ret = rdbStore->GetNotifyUri(curNode, uri);
        EXPECT_EQ(ret, E_OK);
        HandleUpdate(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUpdateTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUpdateTest002 End";
}

/**
 * @tc.name: HandleUpdateTest003
 * @tc.desc: Verify the HandleUpdate function
 * @tc.type: FUNC
 * @tc.require: issues2599
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUpdateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUpdateTest003 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "testCloudId";
        paramService.notifyType = NotifyType::NOTIFY_NONE;
        paramService.oldUri = "oldUri";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;
        
        auto rdbStore = std::make_shared<CloudDiskRdbStore>(paramOthers.bundleName, paramOthers.userId);
        CacheNode curNode;
        string uri = "oldUri";
        int32_t ret = rdbStore->GetCurNode(paramService.cloudId, curNode);
        EXPECT_EQ(ret, E_OK);
        ret = rdbStore->GetNotifyUri(curNode, uri);
        EXPECT_EQ(ret, E_OK);
        HandleUpdate(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUpdateTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUpdateTest003 End";
}

/**
 * @tc.name: HandleInsertNullStoreTest001
 * @tc.desc: rdbStore is nullptr
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleInsertNullStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleInsertNullStoreTest001 Start";
    try {
        NotifyParamService paramService;
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;

        cacheRdbStore.first = paramOthers.bundleName + std::to_string(paramOthers.userId);
        cacheRdbStore.second = nullptr;

        HandleInsert(paramService, paramOthers);
        EXPECT_EQ(cacheRdbStore.second, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleInsertNullStoreTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleInsertNullStoreTest001 End";
}

/**
 * @tc.name: HandleRecycleRestoreNullStoreTest001
 * @tc.desc: rdbStore is nullptr
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleRecycleRestoreNullStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRecycleRestoreNullStoreTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "bundleName";
        paramDisk.inoPtr->cloudId = "cloudId";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID;
        paramDisk.opsType = NotifyOpsType::DAEMON_RECYCLE;

        cacheRdbStore.first = paramDisk.inoPtr->bundleName + std::to_string(paramDisk.data->userId);
        cacheRdbStore.second = nullptr;

        HandleRecycleRestore(paramDisk);
        EXPECT_EQ(cacheRdbStore.second, nullptr);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleRecycleRestoreNullStoreTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleRecycleRestoreNullStoreTest001 End";
}

/**
 * @tc.name: HandleUpdateNullStoreTest001
 * @tc.desc: rdbStore is nullptr
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUpdateNullStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUpdateNullStoreTest001 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "cloudId";
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;

        cacheRdbStore.first = paramOthers.bundleName + std::to_string(paramOthers.userId);
        cacheRdbStore.second = nullptr;

        HandleUpdate(paramService, paramOthers);
        EXPECT_EQ(cacheRdbStore.second, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUpdateNullStoreTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUpdateNullStoreTest001 End";
}

/**
 * @tc.name: HandleUpdateRecycleNullStoreTest001
 * @tc.desc: rdbStore is nullptr
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUpdateRecycleNullStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUpdateRecycleNullStoreTest001 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "cloudId";
        paramService.node.isDir = TYPE_DIR_STR;
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;

        cacheRdbStore.first = paramOthers.bundleName + std::to_string(paramOthers.userId);
        cacheRdbStore.second = nullptr;

        HandleUpdateRecycle(paramService, paramOthers);
        EXPECT_EQ(cacheRdbStore.second, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUpdateRecycleNullStoreTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUpdateRecycleNullStoreTest001 End";
}

/**
 * @tc.name: GetDeleteNotifyDataNullStoreTest001
 * @tc.desc: rdbStore is nullptr
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetDeleteNotifyDataNullStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeleteNotifyDataNullStoreTest001 Start";
    try {
        vector<NativeRdb::ValueObject> deleteIds;
        deleteIds.emplace_back(std::string("cloudId"));
        vector<NotifyData> notifyList;
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "bundleName";
        paramOthers.userId = USER_ID;

        cacheRdbStore.first = paramOthers.bundleName + std::to_string(paramOthers.userId);
        cacheRdbStore.second = nullptr;

        auto ret = CloudDiskNotify::GetInstance().GetDeleteNotifyData(deleteIds, notifyList, paramOthers);
        EXPECT_EQ(ret, E_RDB);
        EXPECT_TRUE(notifyList.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDeleteNotifyDataNullStoreTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDeleteNotifyDataNullStoreTest001 End";
}

/**
 * @tc.name: GetRdbStoreTest001
 * @tc.desc: Verify the GetRdbStore function with cache hit
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetRdbStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest001 Start";
    try {
        string bundleName = "testBundle";
        int32_t userId = USER_ID;
        string storeKey = bundleName + to_string(userId);
        
        cacheRdbStore.first = storeKey;
        cacheRdbStore.second = make_shared<CloudDiskRdbStore>(bundleName, userId);
        
        auto rdbStore = GetRdbStore(bundleName, userId);
        EXPECT_NE(rdbStore, nullptr);
        EXPECT_EQ(cacheRdbStore.first, storeKey);
        EXPECT_EQ(cacheRdbStore.second, rdbStore);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest001 End";
}

/**
 * @tc.name: GetRdbStoreTest002
 * @tc.desc: Verify the GetRdbStore function with cache miss
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetRdbStoreTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest002 Start";
    try {
        string bundleName = "testBundle";
        int32_t userId = USER_ID;
        string storeKey = bundleName + to_string(userId);
        
        cacheRdbStore.first = "differentKey";
        cacheRdbStore.second = nullptr;
        
        auto rdbStore = GetRdbStore(bundleName, userId);
        EXPECT_NE(rdbStore, nullptr);
        EXPECT_EQ(cacheRdbStore.first, storeKey);
        EXPECT_EQ(cacheRdbStore.second, rdbStore);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest002 End";
}

/**
 * @tc.name: GetTrashNotifyDataTest001
 * @tc.desc: Verify the GetTrashNotifyData function with NotifyParamDisk
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetTrashNotifyDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTrashNotifyDataTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.inoPtr->fileName = "testFile.txt";
        
        NotifyData notifyData;
        int32_t ret = GetTrashNotifyData(paramDisk, notifyData);
        EXPECT_EQ(ret, E_OK);
        EXPECT_FALSE(notifyData.uri.empty());
        EXPECT_TRUE(notifyData.uri.find(".trash") != string::npos);
        EXPECT_TRUE(notifyData.uri.find("testFile.txt") != string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetTrashNotifyDataTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetTrashNotifyDataTest001 End";
}

/**
 * @tc.name: GetTrashNotifyDataTest002
 * @tc.desc: Verify the GetTrashNotifyData function with null inoPtr
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetTrashNotifyDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTrashNotifyDataTest002 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = nullptr;
        
        NotifyData notifyData;
        int32_t ret = GetTrashNotifyData(paramDisk, notifyData);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetTrashNotifyDataTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetTrashNotifyDataTest002 End";
}

/**
 * @tc.name: GetTrashNotifyDataTest003
 * @tc.desc: Verify the GetTrashNotifyData function with CacheNode
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetTrashNotifyDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTrashNotifyDataTest003 Start";
    try {
        CacheNode cacheNode;
        cacheNode.fileName = "testFile.txt";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "testBundle";
        
        NotifyData notifyData;
        int32_t ret = GetTrashNotifyData(cacheNode, paramOthers, notifyData);
        EXPECT_EQ(ret, E_OK);
        EXPECT_FALSE(notifyData.uri.empty());
        EXPECT_TRUE(notifyData.uri.find(".trash") != string::npos);
        EXPECT_TRUE(notifyData.uri.find("testFile.txt") != string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetTrashNotifyDataTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetTrashNotifyDataTest003 End";
}

/**
 * @tc.name: TrashUriAddRowIdTest001
 * @tc.desc: Verify the TrashUriAddRowId function success
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, TrashUriAddRowIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TrashUriAddRowIdTest001 Start";
    try {
        string bundleName = "testBundle";
        int32_t userId = USER_ID;
        auto rdbStore = make_shared<CloudDiskRdbStore>(bundleName, userId);
        
        string cloudId = "testCloudId";
        string uri = "file://testBundle/data/storage/el2/cloud/.trash/testFile.txt";
        
        int32_t ret = TrashUriAddRowId(rdbStore, cloudId, uri);
        if (ret == E_OK) {
            EXPECT_FALSE(uri.empty());
            EXPECT_TRUE(uri.find("_") != string::npos);
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TrashUriAddRowIdTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "TrashUriAddRowIdTest001 End";
}

/**
 * @tc.name: GetDataInnerTest001
 * @tc.desc: Verify the GetDataInner function with inoPtr
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetDataInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDataInnerTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        NotifyData notifyData;
        int32_t ret = GetDataInner(paramDisk, notifyData);
        EXPECT_EQ(ret, E_OK);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDataInnerTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDataInnerTest001 End";
}

/**
 * @tc.name: GetDataInnerTest002
 * @tc.desc: Verify the GetDataInner function with ino
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetDataInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDataInnerTest002 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = nullptr;
        paramDisk.ino = 1;
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        NotifyData notifyData;
        int32_t ret = GetDataInner(paramDisk, notifyData);
        EXPECT_EQ(ret, E_OK);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDataInnerTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDataInnerTest002 End";
}

/**
 * @tc.name: GetDataInnerWithNameTest001
 * @tc.desc: Verify the GetDataInnerWithName function with empty name
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetDataInnerWithNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDataInnerWithNameTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.name = "";
        
        NotifyData notifyData;
        int32_t ret = GetDataInnerWithName(paramDisk, notifyData);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDataInnerWithNameTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDataInnerWithNameTest001 End";
}

/**
 * @tc.name: GetDataInnerWithNameTest002
 * @tc.desc: Verify the GetDataInnerWithName function with inoPtr
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetDataInnerWithNameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDataInnerWithNameTest002 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.name = "testName";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        NotifyData notifyData;
        int32_t ret = GetDataInnerWithName(paramDisk, notifyData);
        EXPECT_EQ(ret, E_OK);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDataInnerWithNameTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDataInnerWithNameTest002 End";
}

/**
 * @tc.name: GetDataInnerWithNameTest003
 * @tc.desc: Verify the GetDataInnerWithName function with ino
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, GetDataInnerWithNameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDataInnerWithNameTest003 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = nullptr;
        paramDisk.ino = 1;
        paramDisk.name = "testName";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        NotifyData notifyData;
        int32_t ret = GetDataInnerWithName(paramDisk, notifyData);
        EXPECT_EQ(ret, E_OK);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDataInnerWithNameTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDataInnerWithNameTest003 End";
}

/**
 * @tc.name: HandleSetAttrTest001
 * @tc.desc: Verify the HandleSetAttr function
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleSetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleSetAttrTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        HandleSetAttr(paramDisk);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleSetAttrTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleSetAttrTest001 End";
}

/**
 * @tc.name: HandleWriteTest001
 * @tc.desc: Verify the HandleWrite function with TYPE_NO_NEED_UPLOAD
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleWriteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleWriteTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        ParamDiskOthers paramOthers;
        paramOthers.dirtyType = static_cast<int32_t>(DirtyType::TYPE_NO_NEED_UPLOAD);
        paramOthers.isWrite = false;
        
        HandleWrite(paramDisk, paramOthers);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleWriteTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleWriteTest001 End";
}

/**
 * @tc.name: HandleWriteTest002
 * @tc.desc: Verify the HandleWrite function with isWrite
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleWriteTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleWriteTest002 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        ParamDiskOthers paramOthers;
        paramOthers.dirtyType = static_cast<int32_t>(DirtyType::TYPE_FDIRTY);
        paramOthers.isWrite = true;
        
        HandleWrite(paramDisk, paramOthers);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleWriteTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleWriteTest002 End";
}

/**
 * @tc.name: HandleMkdirTest001
 * @tc.desc: Verify the HandleMkdir function
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleMkdirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleMkdirTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.name = "testDir";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        HandleMkdir(paramDisk);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleMkdirTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleMkdirTest001 End";
}

/**
 * @tc.name: HandleUnlinkTest001
 * @tc.desc: Verify the HandleUnlink function with DAEMON_UNLINK
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUnlinkTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUnlinkTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.name = "testFile";
        paramDisk.opsType = NotifyOpsType::DAEMON_UNLINK;
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        HandleUnlink(paramDisk);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUnlinkTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUnlinkTest001 End";
}

/**
 * @tc.name: HandleUnlinkTest002
 * @tc.desc: Verify the HandleUnlink function with DAEMON_RMDIR
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleUnlinkTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleUnlinkTest002 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.name = "testDir";
        paramDisk.opsType = NotifyOpsType::DAEMON_RMDIR;
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        HandleUnlink(paramDisk);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleUnlinkTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleUnlinkTest002 End";
}

/**
 * @tc.name: HandleRenameTest001
 * @tc.desc: Verify the HandleRename function
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleRenameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRenameTest001 Start";
    try {
        NotifyParamDisk paramDisk;
        paramDisk.inoPtr = make_shared<CloudDiskInode>();
        paramDisk.inoPtr->bundleName = "testBundle";
        paramDisk.ino = 1;
        paramDisk.name = "oldName";
        paramDisk.newIno = 2;
        paramDisk.newName = "newName";
        paramDisk.data = new CloudDiskFuseData();
        paramDisk.data->userId = USER_ID + 1;
        paramDisk.func = [](CloudDiskFuseData*, int64_t) {
            return make_shared<CloudDiskInode>();
        };
        
        ParamDiskOthers paramOthers;
        paramOthers.isDir = false;
        
        HandleRename(paramDisk, paramOthers);
        delete paramDisk.data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleRenameTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleRenameTest001 End";
}

/**
 * @tc.name: HandleDeleteTest001
 * @tc.desc: Verify the HandleDelete function with empty cloudId
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleDeleteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDeleteTest001 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "testBundle";
        
        HandleDelete(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleDeleteTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDeleteTest001 End";
}

/**
 * @tc.name: HandleDeleteTest002
 * @tc.desc: Verify the HandleDelete function with cloudId
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleDeleteTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDeleteTest002 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "testCloudId";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "testBundle";
        NotifyData notifyData;
        notifyData.uri = "testUri";
        notifyData.type = NotifyType::NOTIFY_DELETED;
        paramOthers.notifyDataList.push_back(notifyData);
        
        HandleDelete(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleDeleteTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDeleteTest002 End";
}

/**
 * @tc.name: HandleDeleteBatchTest001
 * @tc.desc: Verify the HandleDeleteBatch function
 * @tc.type: FUNC
 * @tc.require: issues2755
 */
HWTEST_F(CloudDiskNotifyStaticTest, HandleDeleteBatchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDeleteBatchTest001 Start";
    try {
        NotifyParamService paramService;
        paramService.cloudId = "testCloudId";
        
        ParamServiceOther paramOthers;
        paramOthers.bundleName = "testBundle";
        NotifyData notifyData1;
        notifyData1.uri = "testUri1";
        notifyData1.type = NotifyType::NOTIFY_DELETED;
        NotifyData notifyData2;
        notifyData2.uri = "testUri2";
        notifyData2.type = NotifyType::NOTIFY_DELETED;
        paramOthers.notifyDataList.push_back(notifyData1);
        paramOthers.notifyDataList.push_back(notifyData2);
        
        HandleDeleteBatch(paramService, paramOthers);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleDeleteBatchTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleDeleteBatchTest001 End";
}
}