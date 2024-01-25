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

#include "cloud_file_utils.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace std;
using namespace OHOS;
using namespace testing::ext;

class CloudFileUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudFileUtilsTest::SetUpTestCase(void) {}

void CloudFileUtilsTest::TearDownTestCase(void) {}

void CloudFileUtilsTest::SetUp(void) {}

void CloudFileUtilsTest::TearDown(void) {}

/**
 * @tc.name: DfsService_CloudDisk_GetCloudId_001
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetCloudId_001, TestSize.Level1)
{
    string cloudId = CloudFileUtils::GetCloudId("");
    EXPECT_EQ((cloudId == ""), true);

    cloudId = CloudFileUtils::GetCloudId("test");
    EXPECT_EQ((cloudId == ""), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetLocalBucketPath_002, TestSize.Level1)
{
    string cloudId;
    string bundleName;
    int32_t userId = -1;
    string bucketPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName, userId);
    EXPECT_EQ((cloudId == ""), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetLocalBucketPath_003, TestSize.Level1)
{
    string cloudId = "testId";
    string bundleName = "testBundleName";
    int32_t userId = -1;
    string bucketPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName, userId);
    EXPECT_EQ((cloudId == ""), false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_CheckIsCloud_004, TestSize.Level1)
{
    string key;
    EXPECT_EQ(CloudFileUtils::CheckIsCloud(key), false);

    key = CLOUD_CLOUD_ID_XATTR;
    EXPECT_EQ(CloudFileUtils::CheckIsCloud(key), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_CheckIsCloudLocation_005, TestSize.Level1)
{
    string key;
    EXPECT_EQ(CloudFileUtils::CheckIsCloudLocation(key), false);

    key = CLOUD_FILE_LOCATION;
    EXPECT_EQ(CloudFileUtils::CheckIsCloudLocation(key), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_CheckIsHmdfsPermission_006, TestSize.Level1)
{
    string key;
    EXPECT_EQ(CloudFileUtils::CheckIsHmdfsPermission(key), false);

    key = HMDFS_PERMISSION_XATTR;
    EXPECT_EQ(CloudFileUtils::CheckIsHmdfsPermission(key), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetLocalFilePath_007, TestSize.Level1)
{
    string cloudId;
    string bundleName;
    int32_t userId = 0;
    string  ret = CloudFileUtils::GetLocalFilePath(cloudId, bundleName, userId);
    EXPECT_EQ((ret == ""), false);

    userId = -1;
    ret = CloudFileUtils::GetLocalFilePath(cloudId, bundleName, userId);
    EXPECT_EQ((ret == ""), false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetPathWithoutTmp_008, TestSize.Level1)
{
    string path;
    string ret = CloudFileUtils::GetPathWithoutTmp(path);
    EXPECT_EQ(ret, path);

    path = "path/test";
    ret = CloudFileUtils::GetPathWithoutTmp(path);
    EXPECT_EQ(ret, path);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_LocalWriteOpen_009, TestSize.Level1)
{
    string dfsPath;
    EXPECT_EQ(CloudFileUtils::LocalWriteOpen(dfsPath), false);

    dfsPath = "testPath";
    EXPECT_EQ(CloudFileUtils::LocalWriteOpen(dfsPath), false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_EndsWith_010, TestSize.Level1)
{
    string fullString;
    string ending;
    EXPECT_EQ(CloudFileUtils::EndsWith(fullString, ending), true);

    fullString = "testPath";
    EXPECT_EQ(CloudFileUtils::EndsWith(fullString, ending), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_EndsWith_011, TestSize.Level1)
{
    string fullString = "testPath";
    string ending = "test@@text##";
    EXPECT_EQ(CloudFileUtils::EndsWith(fullString, ending), false);
}
} // OHOS