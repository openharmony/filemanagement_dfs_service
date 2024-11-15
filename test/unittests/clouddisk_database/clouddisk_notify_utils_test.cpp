/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "clouddisk_notify_utils.h"
#include "clouddisk_rdb_utils.h"
#include "file_column.h"
#include "cloud_pref_impl.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDisk {
namespace Test {
using namespace testing::ext;
using namespace std;
const int32_t MOCKUSERID = 0;
const int32_t MOCK1 = 1;
const int32_t MOCK2 = 2;
const int32_t MOCK3 = 3;

std::shared_ptr<CloudDiskInode> MockFunc(CloudDiskFuseData* data, int64_t inode)
{
    if (data->userId == MOCKUSERID) {
        return nullptr;
    }
    std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
    if (data->userId == MOCK1) {
        inoPtr->parent = FUSE_ROOT_ID;
    }
    if (data->userId == MOCK2) {
        inoPtr->parent = 0;
        inoPtr->fileName = "";
    }
    if (data->userId == MOCK3) {
        inoPtr->fileName = "MOCK3";
    }
    return inoPtr;
}

class CloudDiskNotifyUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskNotifyUtilsTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDiskNotifyUtilsTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDiskNotifyUtilsTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudDiskNotifyUtilsTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: GetNotifyDataTest001
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    FindCloudDiskInodeFunc func = MockFunc;
    fuse_ino_t ino = FUSE_ROOT_ID;
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, ino, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest002
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    data->userId = MOCKUSERID;
    FindCloudDiskInodeFunc func = MockFunc;
    fuse_ino_t ino = 0;
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, ino, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest003
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    data->userId = MOCK1;
    FindCloudDiskInodeFunc func = MockFunc;
    fuse_ino_t ino = 0;
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, ino, notifyData);
    EXPECT_EQ(ret, E_OK);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest004
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    data->userId = MOCK2;
    FindCloudDiskInodeFunc func = MockFunc;
    fuse_ino_t ino = 0;
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, ino, notifyData);
    EXPECT_EQ(ret, E_OK);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest005
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    data->userId = MOCKUSERID;
    FindCloudDiskInodeFunc func = MockFunc;
    fuse_ino_t parent = 0;
    string name = "test";
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, parent, name, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest006
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    data->userId = MOCK2;
    FindCloudDiskInodeFunc func = MockFunc;
    fuse_ino_t parent = 0;
    string name = "test";
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, parent, name, notifyData);
    EXPECT_EQ(ret, E_OK);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest01
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    data->userId = MOCK3;
    FindCloudDiskInodeFunc func = MockFunc;
    fuse_ino_t parent = 0;
    string name = "test";
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, parent, name, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest007
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    FindCloudDiskInodeFunc func = MockFunc;
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, nullptr, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest008
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    FindCloudDiskInodeFunc func = MockFunc;
    shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, inoPtr, notifyData);
    EXPECT_EQ(ret, E_OK);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest009
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    FindCloudDiskInodeFunc func = MockFunc;
    string name = "test";
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, nullptr, name, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest010
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    FindCloudDiskInodeFunc func = MockFunc;
    shared_ptr<CloudDiskInode> pInoPtr = make_shared<CloudDiskInode>();
    pInoPtr->fileName = "test";
    string name = "test";
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, pInoPtr, name, notifyData);
    EXPECT_EQ(ret, E_OK);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetNotifyDataTest011
 * @tc.desc: Verify the GetNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetNotifyDataTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNotifyData Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    CloudDiskFuseData* data = new CloudDiskFuseData();
    FindCloudDiskInodeFunc func = MockFunc;
    shared_ptr<CloudDiskInode> pInoPtr = make_shared<CloudDiskInode>();
    pInoPtr->fileName = "";
    string name = "test";
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, pInoPtr, name, notifyData);
    EXPECT_EQ(ret, E_OK);
    delete data;
    GTEST_LOG_(INFO) << "GetNotifyData End";
}

/**
 * @tc.name: GetCacheNodeTest001
 * @tc.desc: Verify the GetCacheNode function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetCacheNodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCacheNode Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    string cloudId = "100";
    CacheNode cacheNode;
    int ret = CloudDiskNotifyUtils.GetCacheNode(cloudId, cacheNode);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "GetCacheNode End";
}

/**
 * @tc.name: GetCacheNodeTest002
 * @tc.desc: Verify the GetCacheNode function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetCacheNodeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCacheNode Start";
    CacheNode cacheNode;
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    auto iter = (CloudDiskNotifyUtils.cacheList_)
        .insert(CloudDiskNotifyUtils.cacheList_.end(), std::make_pair("key", cacheNode));
    (CloudDiskNotifyUtils.cacheMap_).insert(std::make_pair("key", iter));
    string cloudId = "key";

    int ret = CloudDiskNotifyUtils.GetCacheNode(cloudId, cacheNode);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetCacheNode End";
}

/**
 * @tc.name: PutCacheNodeTest001
 * @tc.desc: Verify the PutCacheNode function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, PutCacheNodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCacheNode Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    string cloudId = "100";
    CacheNode cacheNode;
    cacheNode.isDir = "";
    CloudDiskNotifyUtils.PutCacheNode(cloudId, cacheNode);
    GTEST_LOG_(INFO) << "PutCacheNode End";
}

/**
 * @tc.name: PutCacheNodeTest002
 * @tc.desc: Verify the PutCacheNode function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, PutCacheNodeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCacheNode Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    string cloudId = "100";
    CacheNode cacheNode;
    cacheNode.isDir = "directory";
    CloudDiskNotifyUtils.PutCacheNode(cloudId, cacheNode);
    GTEST_LOG_(INFO) << "PutCacheNode End";
}

/**
 * @tc.name: PutCacheNodeTest003
 * @tc.desc: Verify the PutCacheNode function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, PutCacheNodeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCacheNode Start";
    CacheNode cacheNode;
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    auto iter = (CloudDiskNotifyUtils.cacheList_)
        .insert(CloudDiskNotifyUtils.cacheList_.end(), std::make_pair("key", cacheNode));
    (CloudDiskNotifyUtils.cacheMap_).insert(std::make_pair("key", iter));
    string cloudId = "key";
    cacheNode.isDir = TYPE_DIR_STR;

    CloudDiskNotifyUtils.PutCacheNode(cloudId, cacheNode);
    auto res = CloudDiskNotifyUtils.cacheMap_.find("key");
    EXPECT_EQ(cloudId, res->first);
    GTEST_LOG_(INFO) << "PutCacheNode End";
}

/**
 * @tc.name: PutCacheNodeTest004
 * @tc.desc: Verify the PutCacheNode function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, PutCacheNodeTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCacheNode Start";
    CacheNode cacheNode;
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    for (int i = 0; i < CloudDiskNotifyUtils::maxCacheCnt_ - 2; ++i) {
        std::stringstream ss;
        ss << i;
        string str = ss.str();
        auto iter = (CloudDiskNotifyUtils.cacheList_)
            .insert(CloudDiskNotifyUtils.cacheList_.end(), std::make_pair(str, cacheNode));
        (CloudDiskNotifyUtils.cacheMap_).insert(std::make_pair(str, iter));
    }
    string cloudId = "key";
    cacheNode.isDir = TYPE_DIR_STR;

    CloudDiskNotifyUtils.PutCacheNode(cloudId, cacheNode);
    EXPECT_EQ(CloudDiskNotifyUtils.cacheMap_.size(), CloudDiskNotifyUtils::maxCacheCnt_);
    GTEST_LOG_(INFO) << "PutCacheNode End";
}

/**
 * @tc.name: GetUriFromCacheTest001
 * @tc.desc: Verify the GetUriFromCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetUriFromCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUriFromCache Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    string bundleName = "com.ohos.photos";
    string rootId = "rootId";
    CacheNode cacheNode;
    cacheNode.isDir = "";
    cacheNode.cloudId = "";
    string uri = "";
    int ret = CloudDiskNotifyUtils.GetUriFromCache(bundleName, rootId, cacheNode, uri);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "GetUriFromCache End";
}

/**
 * @tc.name: GetUriFromCacheTest002
 * @tc.desc: Verify the GetUriFromCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyUtilsTest, GetUriFromCacheTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUriFromCache Start";
    CloudDiskNotifyUtils CloudDiskNotifyUtils;
    string bundleName = "com.ohos.photos";
    string rootId = "same";
    CacheNode cacheNode;
    cacheNode.isDir = "";
    cacheNode.parentCloudId = "same";
    string uri = "";
    int ret = CloudDiskNotifyUtils.GetUriFromCache(bundleName, rootId, cacheNode, uri);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetUriFromCache End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS