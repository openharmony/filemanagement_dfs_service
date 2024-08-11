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
    CloudDiskFuseData* data = nullptr;
    FindCloudDiskInodeFunc func;
    fuse_ino_t ino = 1;
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, ino, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
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
    CloudDiskFuseData* data = nullptr;
    FindCloudDiskInodeFunc func;
    shared_ptr<CloudDiskInode> inoPtr = nullptr;
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, inoPtr, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
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
    CloudDiskFuseData* data = nullptr;
    FindCloudDiskInodeFunc func;
    shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
    inoPtr->parent = 1;
    NotifyData notifyData;
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, inoPtr, notifyData);
    EXPECT_EQ(ret, E_OK);
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
    CloudDiskFuseData* data = nullptr;
    FindCloudDiskInodeFunc func;
    shared_ptr<CloudDiskInode> inoPtr = nullptr;
    NotifyData notifyData;
    string name = "test";
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, inoPtr, name, notifyData);
    EXPECT_EQ(ret, E_INVAL_ARG);
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
    CloudDiskFuseData* data = nullptr;
    FindCloudDiskInodeFunc func;
    shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
    inoPtr->parent = 1;
    inoPtr->fileName = "test";
    NotifyData notifyData;
    string name = "test";
    int ret = CloudDiskNotifyUtils.GetNotifyData(data, func, inoPtr, name, notifyData);
    EXPECT_EQ(ret, E_OK);
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
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS