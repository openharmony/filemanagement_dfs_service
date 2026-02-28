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

#include "cloud_disk_service_callback_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_disk_service_callback_mock.h"
#include "cloud_disk_service_access_token_mock.h"
#include "cloud_disk_sync_folder.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace std;
using namespace testing;
using namespace testing::ext;

class CloudDiskServiceCallbackManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<CloudDiskServiceAccessTokenMock> dfsuAccessTokenMock_ = nullptr;
};

void CloudDiskServiceCallbackManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskServiceCallbackManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskServiceCallbackManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    dfsuAccessTokenMock_ = make_shared<CloudDiskServiceAccessTokenMock>();
    CloudDiskServiceAccessTokenMock::dfsuAccessToken = dfsuAccessTokenMock_;
}

void CloudDiskServiceCallbackManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    dfsuAccessTokenMock_ = nullptr;
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify the GetInstance function returns same instance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 start";
    try {
        CloudDiskServiceCallbackManager& instance1 = CloudDiskServiceCallbackManager::GetInstance();
        CloudDiskServiceCallbackManager& instance2 = CloudDiskServiceCallbackManager::GetInstance();
        EXPECT_EQ(&instance1, &instance2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 end";
}

/**
 * @tc.name: AddCallbackTest001
 * @tc.desc: Verify the AddCallback function with valid callback
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, AddCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex = 1;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool ret = callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        EXPECT_TRUE(ret);
        callbackManager.AddCallback(name, callback);
        callbackManager.AddCallback(name, callback);
        EXPECT_EQ(callbackManager.callbackAppMap_[name].callback, callback);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest001 failed";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest001 end";
}

/**
 * @tc.name: AddCallbackTest002
 * @tc.desc: Verify the AddCallback function with null callback
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, AddCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest002 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        callbackManager.AddCallback(name, nullptr);
        EXPECT_EQ(callbackManager.callbackAppMap_[name].callback, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest002 failed";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest002 end";
}

/**
 * @tc.name: AddCallbackTest003
 * @tc.desc: Verify the AddCallback function with existing callback does not replace
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, AddCallbackTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest003 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex = 1;
        sptr<ICloudDiskServiceCallback> callback1 = sptr(new CloudDiskServiceCallbackMock());
        sptr<ICloudDiskServiceCallback> callback2 = sptr(new CloudDiskServiceCallbackMock());
        callbackManager.RegisterSyncFolderMap(name, folderIndex, callback1);
        callbackManager.AddCallback(name, callback1);
        callbackManager.AddCallback(name, callback2);
        EXPECT_EQ(callbackManager.callbackAppMap_[name].callback, callback1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest003 failed";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest003 end";
}

/**
 * @tc.name: OnChangeDataTest001
 * @tc.desc: Verify the OnChangeData function with invalid sync folder index
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t folderIndex = 999;
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "fileId321";
        vector<ChangeData> changeData = {data};
        callbackManager.OnChangeData(folderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest001 end";
}

/**
 * @tc.name: OnChangeDataTest002
 * @tc.desc: Verify the OnChangeData function with valid sync folder index
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest002 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t folderIndex = 2;
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "fileId321";
        vector<ChangeData> changeData = {data};
        string name = "com.test.example";
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        callbackManager.OnChangeData(folderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest002 end";
}

/**
 * @tc.name: OnChangeDataTest003
 * @tc.desc: Verify the OnChangeData function with valid path and non-zero user id
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest003 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t folderIndex = 3;
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "fileId321";
        vector<ChangeData> changeData = {data};
        string name = "com.test.example";
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        EXPECT_CALL(*dfsuAccessTokenMock_, GetUserId()).WillOnce(Return(123));
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        SyncFolderValue value;
        value.bundleName = name;
        value.path = "/data/service/el2/123/hmdfs/account/files/Docs/testfile.txt";
        syncFolder.AddSyncFolder(folderIndex, value);
        callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        callbackManager.OnChangeData(folderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest003 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest003 end";
}

/**
 * @tc.name: OnChangeDataTest004
 * @tc.desc: Verify the OnChangeData function with zero user id and get account id
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest004 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t folderIndex = 4;
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "fileId321";
        vector<ChangeData> changeData = {data};
        string name = "com.test.example";
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        EXPECT_CALL(*dfsuAccessTokenMock_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessTokenMock_, GetAccountId(_)).WillOnce(Return(E_OK));
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        SyncFolderValue value;
        value.bundleName = name;
        value.path = "/data/service/el2/0/Docs/testfile.txt";
        syncFolder.AddSyncFolder(folderIndex, value);
        callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        callbackManager.OnChangeData(folderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest004 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest004 end";
}

/**
 * @tc.name: OnChangeDataTest005
 * @tc.desc: Verify the OnChangeData function with empty change data
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest005 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t folderIndex = 5;
        vector<ChangeData> changeData;
        string name = "com.test.example";
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        callbackManager.OnChangeData(folderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest005 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest005 end";
}

/**
 * @tc.name: OnChangeDataTest006
 * @tc.desc: Verify the OnChangeData function with multiple change data items
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest006 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t folderIndex = 6;
        ChangeData data1;
        data1.updateSequenceNumber = 1;
        data1.fileId = "file1";
        ChangeData data2;
        data2.updateSequenceNumber = 2;
        data2.fileId = "file2";
        ChangeData data3;
        data3.updateSequenceNumber = 3;
        data3.fileId = "file3";
        vector<ChangeData> changeData = {data1, data2, data3};
        string name = "com.test.example";
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        callbackManager.OnChangeData(folderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest006 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest006 end";
}

/**
 * @tc.name: RegisterSyncFolderMapTest001
 * @tc.desc: Verify the RegisterSyncFolderMap function with new sync folder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, RegisterSyncFolderMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex = 10;
        uint32_t folderIndexNew = 11;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool ret = callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        EXPECT_TRUE(ret);
        ret = callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        EXPECT_FALSE(ret);
        ret = callbackManager.RegisterSyncFolderMap(name, folderIndexNew, callback);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest001 end";
}

/**
 * @tc.name: RegisterSyncFolderMapTest002
 * @tc.desc: Verify the RegisterSyncFolderMap function with different bundle names
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, RegisterSyncFolderMapTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest002 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name1 = "com.test.example1";
        string name2 = "com.test.example2";
        uint32_t folderIndex = 12;
        sptr<ICloudDiskServiceCallback> callback1 = sptr(new CloudDiskServiceCallbackMock());
        sptr<ICloudDiskServiceCallback> callback2 = sptr(new CloudDiskServiceCallbackMock());
        bool ret1 = callbackManager.RegisterSyncFolderMap(name1, folderIndex, callback1);
        EXPECT_TRUE(ret1);
        bool ret2 = callbackManager.RegisterSyncFolderMap(name2, folderIndex, callback2);
        EXPECT_FALSE(ret2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest002 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest002 end";
}

/**
 * @tc.name: RegisterSyncFolderMapTest003
 * @tc.desc: Verify the RegisterSyncFolderMap function with multiple sync folders for same bundle
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, RegisterSyncFolderMapTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest003 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex1 = 13;
        uint32_t folderIndex2 = 14;
        uint32_t folderIndex3 = 15;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool ret1 = callbackManager.RegisterSyncFolderMap(name, folderIndex1, callback);
        EXPECT_TRUE(ret1);
        bool ret2 = callbackManager.RegisterSyncFolderMap(name, folderIndex2, callback);
        EXPECT_TRUE(ret2);
        bool ret3 = callbackManager.RegisterSyncFolderMap(name, folderIndex3, callback);
        EXPECT_TRUE(ret3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest003 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest003 end";
}

/**
 * @tc.name: UnregisterSyncFolderMapTest001
 * @tc.desc: Verify the UnregisterSyncFolderMap function with existing sync folder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex = 16;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool ret = callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        EXPECT_TRUE(ret);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        callbackManager.UnregisterSyncFolderMap(name, folderIndex);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest001 end";
}

/**
 * @tc.name: UnregisterSyncFolderMapTest002
 * @tc.desc: Verify the UnregisterSyncFolderMap function with non-existent sync folder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderMapTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest002 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex = 17;
        callbackManager.UnregisterSyncFolderMap(name, folderIndex);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest002 end";
}

/**
 * @tc.name: UnregisterSyncFolderMapTest003
 * @tc.desc: Verify the UnregisterSyncFolderMap function with multiple sync folders
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderMapTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest003 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex1 = 18;
        uint32_t folderIndex2 = 19;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        callbackManager.RegisterSyncFolderMap(name, folderIndex1, callback);
        callbackManager.RegisterSyncFolderMap(name, folderIndex2, callback);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 2);
        callbackManager.UnregisterSyncFolderMap(name, folderIndex1);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest003 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest003 end";
}

/**
 * @tc.name: UnregisterSyncFolderForChangesMapTest001
 * @tc.desc: Verify the UnregisterSyncFolderForChangesMap function with existing sync folder
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderForChangesMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex = 20;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool ret = callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        EXPECT_TRUE(ret);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        auto res = callbackManager.UnregisterSyncFolderForChangesMap(name, folderIndex);
        EXPECT_TRUE(res);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest001 end";
}

/**
 * @tc.name: UnregisterSyncFolderForChangesMapTest002
 * @tc.desc: Verify the UnregisterSyncFolderForChangesMap function with non-existent sync folder index
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderForChangesMapTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest002 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        string name_notExists = "com.test.kk";
        uint32_t folderIndex = 21;
        uint32_t nonExistentIndex = 999;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool ret = callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        EXPECT_TRUE(ret);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        auto res = callbackManager.UnregisterSyncFolderForChangesMap(name, nonExistentIndex);
        EXPECT_FALSE(res);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        res = callbackManager.UnregisterSyncFolderForChangesMap(name_notExists, folderIndex);
        EXPECT_FALSE(res);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest002 end";
}

/**
 * @tc.name: UnregisterSyncFolderForChangesMapTest003
 * @tc.desc: Verify the UnregisterSyncFolderForChangesMap function with non-existent bundle name
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderForChangesMapTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest003 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        string nonExistentBundle = "com.test.nonexistent";
        uint32_t folderIndex = 22;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool ret = callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        EXPECT_TRUE(ret);
        auto res = callbackManager.UnregisterSyncFolderForChangesMap(nonExistentBundle, folderIndex);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest003 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest003 end";
}

/**
 * @tc.name: ClearMapTest001
 * @tc.desc: Verify the ClearMap function clears all maps
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, ClearMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearMapTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name = "com.test.example";
        uint32_t folderIndex = 23;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        callbackManager.RegisterSyncFolderMap(name, folderIndex, callback);
        callbackManager.AddCallback(name, callback);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        EXPECT_EQ(callbackManager.callbackAppMap_.size(), 1);
        callbackManager.ClearMap();
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
        EXPECT_EQ(callbackManager.callbackAppMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "ClearMapTest001 end";
}

/**
 * @tc.name: ClearMapTest002
 * @tc.desc: Verify the ClearMap function with empty maps
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, ClearMapTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearMapTest002 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        callbackManager.ClearMap();
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
        EXPECT_EQ(callbackManager.callbackAppMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearMapTest002 failed";
    }
    GTEST_LOG_(INFO) << "ClearMapTest002 end";
}

/**
 * @tc.name: ClearMapTest003
 * @tc.desc: Verify the ClearMap function with multiple entries
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, ClearMapTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearMapTest003 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string name1 = "com.test.example1";
        string name2 = "com.test.example2";
        uint32_t folderIndex1 = 24;
        uint32_t folderIndex2 = 25;
        sptr<ICloudDiskServiceCallback> callback1 = sptr(new CloudDiskServiceCallbackMock());
        sptr<ICloudDiskServiceCallback> callback2 = sptr(new CloudDiskServiceCallbackMock());
        callbackManager.RegisterSyncFolderMap(name1, folderIndex1, callback1);
        callbackManager.RegisterSyncFolderMap(name2, folderIndex2, callback2);
        callbackManager.AddCallback(name1, callback1);
        callbackManager.AddCallback(name2, callback2);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 2);
        EXPECT_EQ(callbackManager.callbackAppMap_.size(), 2);
        callbackManager.ClearMap();
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
        EXPECT_EQ(callbackManager.callbackAppMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearMapTest003 failed";
    }
    GTEST_LOG_(INFO) << "ClearMapTest003 end";
}
} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS
