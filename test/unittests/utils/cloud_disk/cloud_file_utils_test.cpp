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
#include "fuse_assistant.h"
#include "parameters.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace std;
using namespace CloudFile;
using namespace OHOS;
using namespace testing;
using namespace testing::ext;

static const std::string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";

class CloudFileUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FuseAssistantMock> insMock_ = nullptr;
};

void CloudFileUtilsTest::SetUpTestCase(void) {}

void CloudFileUtilsTest::TearDownTestCase(void) {}

void CloudFileUtilsTest::SetUp(void)
{
    insMock_ = make_shared<FuseAssistantMock>();
    FuseAssistantMock::ins = insMock_;
    insMock_->EnableMock();
}

void CloudFileUtilsTest::TearDown(void)
{
    insMock_->DisableMock();
    FuseAssistantMock::ins = nullptr;
    insMock_ = nullptr;
}

/**
 * @tc.name: DfsService_CloudDisk_GetCloudId_001
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetCloudId_001, TestSize.Level0)
{
    string cloudId = CloudFileUtils::GetCloudId("");
    EXPECT_EQ((cloudId == ""), true);

    cloudId = CloudFileUtils::GetCloudId("test");
    EXPECT_EQ((cloudId == ""), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetLocalBucketPath_002, TestSize.Level0)
{
    string cloudId;
    string bundleName;
    int32_t userId = -1;
    string bucketPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName, userId);
    EXPECT_EQ((cloudId == ""), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetLocalBucketPath_003, TestSize.Level0)
{
    string cloudId = "testId";
    string bundleName = "testBundleName";
    int32_t userId = -1;
    string bucketPath = CloudFileUtils::GetLocalBucketPath(cloudId, bundleName, userId);
    EXPECT_EQ((cloudId == ""), false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_CheckIsCloud_004, TestSize.Level0)
{
    string key;
    EXPECT_EQ(CloudFileUtils::CheckIsCloud(key), false);

    key = CLOUD_CLOUD_ID_XATTR;
    EXPECT_EQ(CloudFileUtils::CheckIsCloud(key), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_CheckIsCloudLocation_005, TestSize.Level0)
{
    string key;
    EXPECT_EQ(CloudFileUtils::CheckIsCloudLocation(key), false);

    key = CLOUD_FILE_LOCATION;
    EXPECT_EQ(CloudFileUtils::CheckIsCloudLocation(key), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_CheckIsHmdfsPermission_006, TestSize.Level0)
{
    string key;
    EXPECT_EQ(CloudFileUtils::CheckIsHmdfsPermission(key), false);

    key = HMDFS_PERMISSION_XATTR;
    EXPECT_EQ(CloudFileUtils::CheckIsHmdfsPermission(key), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetLocalFilePath_007, TestSize.Level0)
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

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetPathWithoutTmp_008, TestSize.Level0)
{
    string path;
    string ret = CloudFileUtils::GetPathWithoutTmp(path);
    EXPECT_EQ(ret, path);

    path = "path/test";
    ret = CloudFileUtils::GetPathWithoutTmp(path);
    EXPECT_EQ(ret, path);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_LocalWriteOpen_009, TestSize.Level0)
{
    string dfsPath;
    EXPECT_EQ(CloudFileUtils::LocalWriteOpen(dfsPath), false);

    dfsPath = "testPath";
    EXPECT_EQ(CloudFileUtils::LocalWriteOpen(dfsPath), false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_EndsWith_010, TestSize.Level0)
{
    string fullString;
    string ending;
    EXPECT_EQ(CloudFileUtils::EndsWith(fullString, ending), true);

    fullString = "testPath";
    EXPECT_EQ(CloudFileUtils::EndsWith(fullString, ending), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_EndsWith_011, TestSize.Level0)
{
    string fullString = "testPath";
    string ending = "test@@text##";
    EXPECT_EQ(CloudFileUtils::EndsWith(fullString, ending), false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_DentryHash_012, TestSize.Level0)
{
    try {
        string inputStr = ".";
        EXPECT_EQ(CloudFileUtils::DentryHash(inputStr), 0);

        inputStr = "..";
        EXPECT_EQ(CloudFileUtils::DentryHash(inputStr), 0);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " DfsService_CloudDisk_DentryHash_012 ERROR";
    }
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_LocalWriteOpen_013, TestSize.Level0)
{
    string dfsPath = "./";
    EXPECT_EQ(CloudFileUtils::LocalWriteOpen(dfsPath), false);

    dfsPath = "testPath";
    EXPECT_EQ(CloudFileUtils::LocalWriteOpen(dfsPath), false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_GetCloudId_014, TestSize.Level1)
{
    string dfsPath = "/data/service/el1/public/cloudfile";
    string cloudId = CloudFileUtils::GetCloudId(dfsPath);
    EXPECT_EQ((cloudId == ""), true);

    dfsPath = "/data/service/el1/public/cloudfile/cloud_data_statistic";
    cloudId = CloudFileUtils::GetCloudId(dfsPath);
    EXPECT_EQ((cloudId == ""), true);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_ClearCache_01, TestSize.Level1)
{
    string dfsPath = "/mnt/hmdfs/100/cloud/data/" + system::GetParameter(FILEMANAGER_KEY, "");
    string cloudPath = "/mnt/data/100/cloud_fuse";
    bool res = CloudFileUtils::ClearCache(dfsPath, cloudPath);
#if CLOUD_ADAPTER_ENABLED
    EXPECT_EQ(res, true);
#else
    EXPECT_EQ(res, false);
#endif
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_ClearCache_02, TestSize.Level1)
{
    string dfsPath = "/mnt/hmdfs/100/cloud/data/" + system::GetParameter(FILEMANAGER_KEY, "");
    string cloudPath = "./";
    bool res = CloudFileUtils::ClearCache(dfsPath, cloudPath);
    EXPECT_EQ(res, false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_ClearCache_03, TestSize.Level1)
{
    string dfsPath = "/invalid/path";
    string cloudPath = "/mnt/data/100/cloud_fuse";
    bool res = CloudFileUtils::ClearCache(dfsPath, cloudPath);
    EXPECT_EQ(res, false);
}

HWTEST_F(CloudFileUtilsTest, DfsService_CloudDisk_ClearCache_04, TestSize.Level1)
{
    string dfsPath = "/mnt/hmdfs/100/cloud/data/" + system::GetParameter(FILEMANAGER_KEY, "");
    string cloudPath = "/invalid/path";
    bool res = CloudFileUtils::ClearCache(dfsPath, cloudPath);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: CloudFileUtils_ChangeUid
 * @tc.desc: Verify ChangeUid for bundle name.
 * @tc.type: FUNC
 * @tc.require: ICTBV2
 */
HWTEST_F(CloudFileUtilsTest, CloudFileUtils_ChangeUid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudFileUtils_ChangeUid Start.";
    try {
        int32_t userId = 100;
        string bundleName = "";
        uint32_t mode = 0771;
        string path = "";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);

        bundleName = "com.ohos.camera";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudFileUtils_ChangeUid ERROR.";
    }
}

/**
 * @tc.name: CloudFileUtils_ChangeUidByCloudId
 * @tc.desc: Verify ChangeUidByCloudId for path.
 * @tc.type: FUNC
 * @tc.require: ICTBV2
 */
HWTEST_F(CloudFileUtilsTest, CloudFileUtils_ChangeUidByCloudId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudFileUtils_ChangeUidByCloudId Start.";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0771;
        string cloudId = "";
        uid_t uid = 0;
        CloudFileUtils::ChangeUidByCloudId(userId, bundleName, cloudId, mode, uid);

        cloudId = "testCloudId";
        CloudFileUtils::ChangeUidByCloudId(userId, bundleName, cloudId, mode, uid);

        userId = 0;
        bundleName = "";
        CloudFileUtils::ChangeUidByCloudId(userId, bundleName, cloudId, mode, uid);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudFileUtils_ChangeUidByCloudId ERROR.";
    }
}

/**
 * @tc.name: CloudFileUtils_ChangeUidByPath
 * @tc.desc: Verify ChangeUidByPath for path.
 * @tc.type: FUNC
 * @tc.require: ICTBV2
 */
HWTEST_F(CloudFileUtilsTest, CloudFileUtils_ChangeUidByPath, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudFileUtils_ChangeUidByPath Start.";
    try {
        mode_t mode = 0;
        string path = "/data/testdir";
        uid_t uid = 0;
        CloudFileUtils::ChangeUidByPath(path, mode, uid);

        uid = 1;
        CloudFileUtils::ChangeUidByPath(path, mode, uid);

        path = "/data/test";
        EXPECT_CALL(*insMock_, chmod(_, _)).WillOnce(Return(-1));
        CloudFileUtils::ChangeUidByPath(path, mode, uid);

        EXPECT_CALL(*insMock_, chmod(_, _)).Times(1).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, chown(_, _, _)).WillOnce(Return(-1));
        CloudFileUtils::ChangeUidByPath(path, mode, uid);

        mode = 0755;
        EXPECT_CALL(*insMock_, chown(_, _, _)).Times(2).WillOnce(Return(0));
        CloudFileUtils::ChangeUidByPath(path, mode, uid);

        uid = 1000;
        CloudFileUtils::ChangeUidByPath(path, mode, uid);

        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudFileUtils_ChangeUidByPath ERROR.";
    }
}

/**
 * @tc.name: GetPathFromUri001
 * @tc.desc: Verify ChangeUidByPath for path.
 * @tc.type: FUNC
 * @tc.require: ICTBV2
 */
HWTEST_F(CloudFileUtilsTest, GetPathFromUri001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPathFromUri001 Start.";
    try {
    std::string uriString = "file://com.ohos.camera/data/test/file.txt";
    std::string expectedPath = "/data/test/file.txt";
    std::string actualPath = CloudFileUtils::GetPathFromUri(uriString);

    EXPECT_EQ(expectedPath, actualPath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetPathFromUri001 ERROR.";
    }
}
 
/**
 * @tc.name: GetPathFromUri002
 * @tc.desc: Verify ChangeUidByPath for path.
 * @tc.type: FUNC
 * @tc.require: ICTBV2
 */
HWTEST_F(CloudFileUtilsTest, GetPathFromUri002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPathFromUri002 Start.";
    try {
    std::string uriString = "file://com.ohos.camera/data/test/.txt";
    std::string expectedPath = "/data/test/.txt";
    std::string actualPath = CloudFileUtils::GetPathFromUri(uriString);

    EXPECT_EQ(expectedPath, actualPath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetPathFromUri002 ERROR.";
    }
}
 
/**
 * @tc.name: GetPathFromUri003
 * @tc.desc: Verify ChangeUidByPath for path.
 * @tc.type: FUNC
 * @tc.require: ICTBV2
 */
HWTEST_F(CloudFileUtilsTest, GetPathFromUri003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPathFromUri003 Start.";
    try {
    std::string uriString = "file:///data";
    std::string expectedPath = "/data";
    std::string actualPath = CloudFileUtils::GetPathFromUri(uriString);

    EXPECT_EQ(expectedPath, actualPath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetPathFromUri003 ERROR.";
    }
}

/**
 * @tc.name: GetPathFromUri004
 * @tc.desc: Verify ChangeUidByPath for path.
 * @tc.type: FUNC
 * @tc.require: ICTBV2
 */
HWTEST_F(CloudFileUtilsTest, GetPathFromUri004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPathFromUri004 Start.";
    try {
    std::string uriString = "file://com.ohos.camera/";
    std::string expectedPath = "/";
    std::string actualPath = CloudFileUtils::GetPathFromUri(uriString);

    EXPECT_EQ(expectedPath, actualPath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetPathFromUri004 ERROR.";
    }
}

/**
 * @tc.name: GetPathFromUri005
 * @tc.desc: Verify ChangeUidByPath for path.
 * @tc.type: FUNC
 * @tc.require: ICTBV2
 */
HWTEST_F(CloudFileUtilsTest, GetPathFromUri005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPathFromUri005 Start.";
    try {
    std::string uriString = "file://com.ohos.camera";
    std::string expectedPath = "";
    std::string actualPath = CloudFileUtils::GetPathFromUri(uriString);
 
    EXPECT_EQ(expectedPath, actualPath);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetPathFromUri005 ERROR.";
    }
}

/**
 * @tc.name: GenerateUuidTest001
 * @tc.desc: Verify the GenerateUuid function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GenerateUuidTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateUuidTest001 start";
    try {
        string uuid = CloudFileUtils::GenerateUuid();
        EXPECT_EQ(uuid.length(), 36);
        EXPECT_TRUE(uuid.find('-') != string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateUuidTest001 failed";
    }
    GTEST_LOG_(INFO) << "GenerateUuidTest001 end";
}
} // OHOS