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

/**
 * @tc.name: GetRealPathTest001
 * @tc.desc: Verify GetRealPath function with simple path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GetRealPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPathTest001 start";
    try {
        string path = "/data/test";
        string realPath = CloudFileUtils::GetRealPath(path);
        EXPECT_EQ(realPath, "/data/test");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRealPathTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetRealPathTest001 end";
}

/**
 * @tc.name: GetRealPathTest002
 * @tc.desc: Verify GetRealPath function with current directory in path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GetRealPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPathTest002 start";
    try {
        string path = "/data/./test";
        string realPath = CloudFileUtils::GetRealPath(path);
        EXPECT_EQ(realPath, "/data/test");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRealPathTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetRealPathTest002 end";
}

/**
 * @tc.name: GetRealPathTest003
 * @tc.desc: Verify GetRealPath function with parent directory in path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GetRealPathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPathTest003 start";
    try {
        string path = "/data/test/../file";
        string realPath = CloudFileUtils::GetRealPath(path);
        EXPECT_EQ(realPath, "/data/file");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRealPathTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetRealPathTest003 end";
}

/**
 * @tc.name: GetRealPathTest004
 * @tc.desc: Verify GetRealPath function with multiple dots
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GetRealPathTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPathTest004 start";
    try {
        string path = "/data/././test";
        string realPath = CloudFileUtils::GetRealPath(path);
        EXPECT_EQ(realPath, "/data/test");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRealPathTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetRealPathTest004 end";
}

/**
 * @tc.name: GetRealPathTest005
 * @tc.desc: Verify GetRealPath function with multiple parent directories
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GetRealPathTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPathTest005 start";
    try {
        string path = "/data/test/subdir/../../file";
        string realPath = CloudFileUtils::GetRealPath(path);
        EXPECT_EQ(realPath, "/data/file");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRealPathTest005 failed";
    }
    GTEST_LOG_(INFO) << "GetRealPathTest005 end";
}

/**
 * @tc.name: GetRealPathTest006
 * @tc.desc: Verify GetRealPath function with root path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GetRealPathTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPathTest006 start";
    try {
        string path = "/";
        string realPath = CloudFileUtils::GetRealPath(path);
        EXPECT_EQ(realPath, "/");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRealPathTest006 failed";
    }
    GTEST_LOG_(INFO) << "GetRealPathTest006 end";
}

/**
 * @tc.name: GetRealPathTest007
 * @tc.desc: Verify GetRealPath function with mixed dots
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GetRealPathTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPathTest007 start";
    try {
        string path = "/data/./test/../subdir/./file";
        string realPath = CloudFileUtils::GetRealPath(path);
        EXPECT_EQ(realPath, "/data/subdir/file");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRealPathTest007 failed";
    }
    GTEST_LOG_(INFO) << "GetRealPathTest007 end";
}

/**
 * @tc.name: GetRealPathTest008
 * @tc.desc: Verify GetRealPath function with relative path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, GetRealPathTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPathTest008 start";
    try {
        string path = "data/test";
        string realPath = CloudFileUtils::GetRealPath(path);
        EXPECT_EQ(realPath, "data/test");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRealPathTest008 failed";
    }
    GTEST_LOG_(INFO) << "GetRealPathTest008 end";
}

/**
 * @tc.name: ChangeUidTest001
 * @tc.desc: Verify ChangeUid function with empty bundle name
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest001 start";
    try {
        int32_t userId = 100;
        string bundleName = "";
        uint32_t mode = 0755;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest001 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest001 end";
}

/**
 * @tc.name: ChangeUidTest002
 * @tc.desc: Verify ChangeUid function with valid parameters
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest002 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0755;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest002 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest002 end";
}

/**
 * @tc.name: ChangeUidTest003
 * @tc.desc: Verify ChangeUid function with different user id
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest003 start";
    try {
        int32_t userId = 0;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0771;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest003 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest003 end";
}

/**
 * @tc.name: ChangeUidTest004
 * @tc.desc: Verify ChangeUid function with different mode
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest004 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0644;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest004 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest004 end";
}

/**
 * @tc.name: ChangeUidTest005
 * @tc.desc: Verify ChangeUid function with empty path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest005 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0755;
        string path = "";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest005 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest005 end";
}

/**
 * @tc.name: ChangeUidTest006
 * @tc.desc: Verify ChangeUid function with negative user id
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest006 start";
    try {
        int32_t userId = -1;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0755;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest006 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest006 end";
}

/**
 * @tc.name: ChangeUidTest007
 * @tc.desc: Verify ChangeUid function with maximum mode
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest007 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0777;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest007 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest007 end";
}

/**
 * @tc.name: ChangeUidTest008
 * @tc.desc: Verify ChangeUid function with minimum mode
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest008 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0000;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest008 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest008 end";
}

/**
 * @tc.name: ChangeUidTest009
 * @tc.desc: Verify ChangeUid function with long bundle name
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest009 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera.long.bundle.name.test";
        uint32_t mode = 0755;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest009 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest009 end";
}

/**
 * @tc.name: ChangeUidTest010
 * @tc.desc: Verify ChangeUid function with special characters in bundle name
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest010 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.test_bundle";
        uint32_t mode = 0755;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest010 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest010 end";
}

/**
 * @tc.name: ChangeUidTest011
 * @tc.desc: Verify ChangeUid function with deep path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest011 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0755;
        string path = "/data/test/deep/path/to/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest011 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest011 end";
}

/**
 * @tc.name: ChangeUidTest012
 * @tc.desc: Verify ChangeUid function with executable mode
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest012 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0755;
        string path = "/data/test/executable";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest012 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest012 end";
}

/**
 * @tc.name: ChangeUidTest013
 * @tc.desc: Verify ChangeUid function with directory mode
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest013 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0771;
        string path = "/data/test/directory";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest013 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest013 end";
}

/**
 * @tc.name: ChangeUidTest014
 * @tc.desc: Verify ChangeUid function with read-only mode
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest014 start";
    try {
        int32_t userId = 100;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0444;
        string path = "/data/test/readonly";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest014 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest014 end";
}

/**
 * @tc.name: ChangeUidTest015
 * @tc.desc: Verify ChangeUid function with large user id
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, ChangeUidTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeUidTest015 start";
    try {
        int32_t userId = 99999;
        string bundleName = "com.ohos.camera";
        uint32_t mode = 0755;
        string path = "/data/test/file";
        CloudFileUtils::ChangeUid(userId, bundleName, mode, path);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeUidTest015 failed";
    }
    GTEST_LOG_(INFO) << "ChangeUidTest015 end";
}

/**
 * @tc.name: LocalWriteOpenTest001
 * @tc.desc: Verify LocalWriteOpen function with empty path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest001 start";
    try {
        string dfsPath = "";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest001 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest001 end";
}

/**
 * @tc.name: LocalWriteOpenTest002
 * @tc.desc: Verify LocalWriteOpen function with invalid path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest002 start";
    try {
        string dfsPath = "/invalid/nonexistent/path";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest002 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest002 end";
}

/**
 * @tc.name: LocalWriteOpenTest003
 * @tc.desc: Verify LocalWriteOpen function with relative path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest003 start";
    try {
        string dfsPath = "./relative/path";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest003 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest003 end";
}

/**
 * @tc.name: LocalWriteOpenTest004
 * @tc.desc: Verify LocalWriteOpen function with absolute path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest004 start";
    try {
        string dfsPath = "/data/service/el1/public/cloudfile";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest004 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest004 end";
}

/**
 * @tc.name: LocalWriteOpenTest005
 * @tc.desc: Verify LocalWriteOpen function with directory path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest005 start";
    try {
        string dfsPath = "/data/test/directory";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest005 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest005 end";
}

/**
 * @tc.name: LocalWriteOpenTest006
 * @tc.desc: Verify LocalWriteOpen function with special characters in path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest006 start";
    try {
        string dfsPath = "/data/test/file with spaces";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest006 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest006 end";
}

/**
 * @tc.name: LocalWriteOpenTest007
 * @tc.desc: Verify LocalWriteOpen function with very long path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest007 start";
    try {
        string dfsPath = "/data/test/very/long/path/that/exceeds/normal/length/for/testing/purposes/file.txt";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest007 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest007 end";
}

/**
 * @tc.name: LocalWriteOpenTest008
 * @tc.desc: Verify LocalWriteOpen function with symbolic link path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest008 start";
    try {
        string dfsPath = "/data/test/symlink";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest008 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest008 end";
}

/**
 * @tc.name: LocalWriteOpenTest009
 * @tc.desc: Verify LocalWriteOpen function with root directory
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest009 start";
    try {
        string dfsPath = "/";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest009 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest009 end";
}

/**
 * @tc.name: LocalWriteOpenTest010
 * @tc.desc: Verify LocalWriteOpen function with path containing dots
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudFileUtilsTest, LocalWriteOpenTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LocalWriteOpenTest010 start";
    try {
        string dfsPath = "/data/test/./file.txt";
        bool result = CloudFileUtils::LocalWriteOpen(dfsPath);
        EXPECT_EQ(result, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LocalWriteOpenTest010 failed";
    }
    GTEST_LOG_(INFO) << "LocalWriteOpenTest010 end";
}

/**
 * @tc.name: CheckIsSourcePathTest001
 * @tc.desc: Verify the CheckIsSourcePath function
 * @tc.type: FUNC
 * @tc.require: #2971
 */
HWTEST_F(CloudFileUtilsTest, CheckIsSourcePathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckIsSourcePathTest001 start";
    try {
        std::string key = "test";
        bool res = CloudFileUtils::CheckIsSourcePath(key);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckIsSourcePathTest001 failed";
    }
    GTEST_LOG_(INFO) << "CheckIsSourcePathTest001 end";
}
} // OHOS