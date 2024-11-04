/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cloud_download_uri_manager.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDownloadUriManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDownloadUriManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDownloadUriManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDownloadUriManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudDownloadUriManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadUriManagerTest, GetInstanceTest, TestSize.Level1)
{
    try {
        CloudDownloadUriManager::GetInstance();
    } catch (...) {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: AddPathToUriTest001
 * @tc.desc: Verify the AddPathToUri function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadUriManagerTest, AddPathToUriTest001, TestSize.Level1)
{
    const std::string path = "file://data/file";
    const std::string uri = "file://data/file/test";
    CloudDownloadUriManager mUriMgr;
    mUriMgr.AddPathToUri(path, uri);
    EXPECT_EQ(mUriMgr.pathUriMap_[path], uri);
}

/**
 * @tc.name: AddPathToUriTest002
 * @tc.desc: Verify the AddPathToUri function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudDownloadUriManagerTest, AddPathToUriTest002, TestSize.Level1)
{
    const std::string path = "file://data/file";
    const std::string uri = "file://data/file/test";
    CloudDownloadUriManager mUriMgr;
    mUriMgr.pathUriMap_[path] = uri;
    auto ret = mUriMgr.AddPathToUri(path, uri);
    EXPECT_EQ(ret, E_STOP);
}

/**
 * @tc.name: RemoveUriTest001
 * @tc.desc: Verify the RemoveUri function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadUriManagerTest, RemoveUriTest001, TestSize.Level1)
{
    const std::string path = "file://data/file";
    const std::string uri = "file://data/file/test1";
    CloudDownloadUriManager mUriMgr;
    mUriMgr.AddPathToUri(path, uri);
    EXPECT_EQ(mUriMgr.pathUriMap_[path], uri);
    mUriMgr.RemoveUri(path);
    auto ret = mUriMgr.pathUriMap_.find(path);
    EXPECT_EQ(ret, mUriMgr.pathUriMap_.end());
}

/**
 * @tc.name: RemoveUriTest002
 * @tc.desc: Verify the RemoveUri function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudDownloadUriManagerTest, RemoveUriTest002, TestSize.Level1)
{
    const std::string path = "file://data/file";
    CloudDownloadUriManager mUriMgr;
    mUriMgr.RemoveUri(path);
    auto ret = mUriMgr.pathUriMap_.find(path);
    EXPECT_EQ(ret, mUriMgr.pathUriMap_.end());
}

/**
 * @tc.name: RemoveUriTest003
 * @tc.desc: Verify the RemoveUri function.
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(CloudDownloadUriManagerTest, RemoveUriTest003, TestSize.Level1)
{
    int64_t downloadId = 3;
    const std::string path = "file://data/file";
    CloudDownloadUriManager mUriMgr;
    (mUriMgr.downloadIdPathMap_)[3].push_back(path);
    (mUriMgr.pathUriMap_)[path] = "test2";
    mUriMgr.RemoveUri(downloadId);
    auto ret1 = mUriMgr.pathUriMap_.find(path);
    EXPECT_EQ(ret1, mUriMgr.pathUriMap_.end());
    auto ret2 = mUriMgr.downloadIdPathMap_.find(downloadId);
    EXPECT_EQ(ret2, mUriMgr.downloadIdPathMap_.end());
}

/**
 * @tc.name: GetUriTest001
 * @tc.desc: Verify the GetUri function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadUriManagerTest, GetUriTest001, TestSize.Level1)
{
    const std::string path = "";
    const std::string uri = "";
    CloudDownloadUriManager mUriMgr;
    string uriStr = mUriMgr.GetUri(path);
    EXPECT_EQ(uriStr, uri);
}

/**
 * @tc.name: GetUriTest002
 * @tc.desc: Verify the GetUri function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadUriManagerTest, GetUriTest002, TestSize.Level1)
{
    const std::string path = "file://data/file";
    const std::string uri = "file://data/file/test2";
    CloudDownloadUriManager mUriMgr;
    mUriMgr.AddPathToUri(path, uri);
    EXPECT_EQ(mUriMgr.pathUriMap_[path], uri);
    string uriStr = mUriMgr.GetUri(path);
    EXPECT_EQ(uriStr, uri);
}

/**
 * @tc.name: CheckDownloadIdPathMap001
 * @tc.desc: Verify the CheckDownloadIdPathMap function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadUriManagerTest, CheckDownloadIdPathMap001, TestSize.Level1)
{
    int64_t downloadId = 1;
    const std::string path = "file://data/file";
    CloudDownloadUriManager mUriMgr;
    (mUriMgr.downloadIdPathMap_)[1].push_back(path);
    mUriMgr.CheckDownloadIdPathMap(downloadId);
    auto ret = mUriMgr.downloadIdPathMap_.find(downloadId);
    EXPECT_EQ(ret, mUriMgr.downloadIdPathMap_.end());
}

/**
 * @tc.name: CheckDownloadIdPathMap002
 * @tc.desc: Verify the CheckDownloadIdPathMap function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDownloadUriManagerTest, CheckDownloadIdPathMap002, TestSize.Level1)
{
    int64_t downloadId = 1;
    const std::string path = "fild://data/file";
    CloudDownloadUriManager mUriMgr;
    (mUriMgr.downloadIdPathMap_)[1].push_back(path);
    (mUriMgr.pathUriMap_)[path] = "test2";
    mUriMgr.CheckDownloadIdPathMap(downloadId);
    auto ret1 = mUriMgr.downloadIdPathMap_.find(downloadId);
    EXPECT_NE(ret1, mUriMgr.downloadIdPathMap_.end());
    auto ret2 = mUriMgr.pathUriMap_.find(path);
    EXPECT_NE(ret2, mUriMgr.pathUriMap_.end());
}
} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
