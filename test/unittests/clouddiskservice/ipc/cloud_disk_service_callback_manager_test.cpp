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

#include "cloud_disk_service_callback_manager.h"

#include "cloud_disk_service_callback_mock.h"
#include "cloud_disk_sync_folder.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper_mock.h"
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
    static inline shared_ptr<DfsuAccessTokenMock> dfsuAccessTokenMock_ = nullptr;
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
    dfsuAccessTokenMock_ = make_shared<DfsuAccessTokenMock>();
    DfsuAccessTokenMock::dfsuAccessToken = dfsuAccessTokenMock_;
}

void CloudDiskServiceCallbackManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    dfsuAccessTokenMock_ = nullptr;
}

/**
 * @tc.name: GetCloudDiskServiceCallbackManagerInstanceTest001
 * @tc.desc: Verify the GetCloudDiskServiceCallbackManagerInstance function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, GetCloudDiskServiceCallbackManagerInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskServiceCallbackManagerInstanceTest001 start";
    try {
        CloudDiskServiceCallbackManager& instance1 = CloudDiskServiceCallbackManager::GetInstance();
        CloudDiskServiceCallbackManager& instance2 = CloudDiskServiceCallbackManager::GetInstance();
        EXPECT_EQ(&instance1, &instance2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskServiceCallbackManagerInstanceTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskServiceCallbackManagerInstanceTest001 end";
}

/**
 * @tc.name: AddCallbackTest001
 * @tc.desc: Verify the AddCallback function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, AddCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string bundleName = "com.test.example";
        uint32_t syncFolderIndex = 1;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool result = callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        EXPECT_TRUE(result);
        callbackManager.AddCallback(bundleName, callback);
        callbackManager.AddCallback(bundleName, callback);
        EXPECT_EQ(callbackManager.callbackAppMap_[bundleName].callback, callback);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest001 failed";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest001 end";
}

/**
 * @tc.name: AddCallbackTest002
 * @tc.desc: Verify the AddCallback function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, AddCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string bundleName = "com.test.example";
        callbackManager.AddCallback(bundleName, nullptr);
        EXPECT_EQ(callbackManager.callbackAppMap_[bundleName].callback, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest002 failed";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest002 end";
}

/**
 * @tc.name: OnChangeDataTest001
 * @tc.desc: Verify the OnChangeData function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t syncFolderIndex = 1;
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "fileId123";
        vector<ChangeData> changeData = {data};
        callbackManager.OnChangeData(syncFolderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest001 end";
}

/**
 * @tc.name: OnChangeDataTest002
 * @tc.desc: Verify the OnChangeData function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest002 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t syncFolderIndex = 2;
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "fileId123";
        vector<ChangeData> changeData = {data};
        string bundleName = "com.test.example";
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        callbackManager.OnChangeData(syncFolderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest002 end";
}

/**
 * @tc.name: OnChangeDataTest003
 * @tc.desc: Verify the OnChangeData function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest003 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t syncFolderIndex = 3;
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "fileId123";
        vector<ChangeData> changeData = {data};
        string bundleName = "com.test.example";
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        EXPECT_CALL(*dfsuAccessTokenMock_, GetUserId()).WillOnce(Return(123));
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        SyncFolderValue value;
        value.bundleName = bundleName;
        value.path = "/data/service/el2/123/hmdfs/account/files/Docs/testfile.txt";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        callbackManager.OnChangeData(syncFolderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest003 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest003 end";
}

/**
 * @tc.name: OnChangeDataTest004
 * @tc.desc: Verify the OnChangeData function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, OnChangeDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnChangeDataTest004 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        uint32_t syncFolderIndex = 1;
        ChangeData data;
        data.updateSequenceNumber = 1;
        data.fileId = "fileId123";
        vector<ChangeData> changeData = {data};
        string bundleName = "com.test.example";
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        EXPECT_CALL(*dfsuAccessTokenMock_, GetUserId()).WillOnce(Return(0));
        CloudDiskSyncFolder &syncFolder = CloudDiskSyncFolder::GetInstance();
        for (const auto &item : syncFolder.GetSyncFolderMap()) {
            syncFolder.DeleteSyncFolder(item.first);
        }
        SyncFolderValue value;
        value.bundleName = bundleName;
        value.path = "/data/service/el2/123/Docs/testfile.txt";
        syncFolder.AddSyncFolder(syncFolderIndex, value);
        callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        callbackManager.OnChangeData(syncFolderIndex, changeData);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnChangeDataTest004 failed";
    }
    GTEST_LOG_(INFO) << "OnChangeDataTest004 end";
}

/**
 * @tc.name: RegisterSyncFolderMapTest001
 * @tc.desc: Verify the RegisterSyncFolderMap function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, RegisterSyncFolderMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string bundleName = "com.test.example";
        uint32_t syncFolderIndex = 1;
        uint32_t syncFolderIndexNew = 2;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool result = callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        EXPECT_TRUE(result);
        result = callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        EXPECT_FALSE(result);
        result = callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndexNew, callback);
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "RegisterSyncFolderMapTest001 end";
}

/**
 * @tc.name: UnregisterSyncFolderMapTest001
 * @tc.desc: Verify the UnregisterSyncFolderMap function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string bundleName = "com.test.example";
        uint32_t syncFolderIndex = 3;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool result = callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        EXPECT_TRUE(result);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        callbackManager.UnregisterSyncFolderMap(bundleName, syncFolderIndex);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderMapTest001 end";
}

/**
 * @tc.name: UnregisterSyncFolderForChangesMapTest001
 * @tc.desc: Verify the UnregisterSyncFolderForChangesMap function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderForChangesMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string bundleName = "com.test.example";
        uint32_t syncFolderIndex = 4;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool result = callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        EXPECT_TRUE(result);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        auto ret = callbackManager.UnregisterSyncFolderForChangesMap(bundleName, syncFolderIndex);
        EXPECT_TRUE(ret);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest001 end";
}

/**
 * @tc.name: UnregisterSyncFolderForChangesMapTest002
 * @tc.desc: Verify the UnregisterSyncFolderForChangesMap function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, UnregisterSyncFolderForChangesMapTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest002 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        string bundleName = "com.test.example";
        string bundleName_notExists = "com.test.kk";
        uint32_t syncFolderIndex = 5;
        uint32_t syncFolderIndex_notExists = 6;
        sptr<ICloudDiskServiceCallback> callback = sptr(new CloudDiskServiceCallbackMock());
        bool result = callbackManager.RegisterSyncFolderMap(bundleName, syncFolderIndex, callback);
        EXPECT_TRUE(result);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        auto ret = callbackManager.UnregisterSyncFolderForChangesMap(bundleName, syncFolderIndex_notExists);
        EXPECT_FALSE(ret);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 1);
        ret = callbackManager.UnregisterSyncFolderForChangesMap(bundleName_notExists, syncFolderIndex);
        EXPECT_FALSE(ret);
        EXPECT_EQ(callbackManager.callbackIndexMap_.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest002 failed";
    }
    GTEST_LOG_(INFO) << "UnregisterSyncFolderForChangesMapTest002 end";
}

/**
 * @tc.name: ClearMapTest001
 * @tc.desc: Verify the ClearMap function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackManagerTest, ClearMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearMapTest001 start";
    try {
        CloudDiskServiceCallbackManager callbackManager;
        callbackManager.ClearMap();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearMapTest001 failed";
    }
    GTEST_LOG_(INFO) << "ClearMapTest001 end";
}
} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS