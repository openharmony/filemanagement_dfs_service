 /*
 * Copyright (C) 2025 Huawei Device Co., Ltd. All rights reserved.
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

#include "assistant.h"
#include "dfs_error.h"
#include "fuse_manager/fuse_manager.h"
#include "fuse_manager.cpp"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace CloudDisk;
using namespace testing;
using namespace testing::ext;
constexpr int32_t USER_ID = 100;
class FuseManagerStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<FuseManager> fuseManager_;
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void FuseManagerStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void FuseManagerStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    Assistant::ins = nullptr;
    insMock = nullptr;
}

void FuseManagerStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    fuseManager_ = std::make_shared<FuseManager>();
}

void FuseManagerStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: IsValidCachePathTest001
 * @tc.desc: Verify the IsValidCachePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, IsValidCachePathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidCachePathTest001 Begin";
    try {
        string path = "/./";
        auto ret = IsValidCachePath(path);

        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidCachePathTest001 Error";
    }
    GTEST_LOG_(INFO) << "IsValidCachePathTest001 End";
}

/**
 * @tc.name: IsValidCachePathTest002
 * @tc.desc: Verify the IsValidCachePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, IsValidCachePathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidCachePathTest002 Begin";
    try {
        string path = "/mnt/data/100/cloud_fuse/../";
        auto ret = IsValidCachePath(path);

        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidCachePathTest002 Error";
    }
    GTEST_LOG_(INFO) << "IsValidCachePathTest002 End";
}

/**
 * @tc.name: IsValidCachePathTest003
 * @tc.desc: Verify the IsValidCachePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, IsValidCachePathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidCachePathTest003 Begin";
    try {
        string path = "/mnt/data/100/cloud_fuse/..";
        auto ret = IsValidCachePath(path);

        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidCachePathTest003 Error";
    }
    GTEST_LOG_(INFO) << "IsValidCachePathTest003 End";
}

/**
 * @tc.name: IsValidCachePathTest004
 * @tc.desc: Verify the IsValidCachePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, IsValidCachePathTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidCachePathTest004 Begin";
    try {
        string path = "/mnt/data/100/cloud_fuse/";
        auto ret = IsValidCachePath(path);

        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidCachePathTest004 Error";
    }
    GTEST_LOG_(INFO) << "IsValidCachePathTest004 End";
}

/**
 * @tc.name: DoSessionInitTest001
 * @tc.desc: Verify the DoSessionInit function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, DoSessionInitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoSessionInitTest001 start";
    try {
        auto cInode = make_shared<CloudInode>();
        auto err = make_shared<CloudError>();
        auto openFinish = make_shared<bool>();
        auto cond = make_shared<ffrt::condition_variable>();
        cInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(0, "", "", "", "");
        cInode->mBase = make_shared<MetaBase>("test");
        cInode->mBase->fileType = FILE_TYPE_THUMBNAIL;

        DoSessionInit(cInode, err, openFinish, cond);
        EXPECT_TRUE(err);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoSessionInitTest failed";
    }
    GTEST_LOG_(INFO) << "DoSessionInitTest end";
}

/**
 * @tc.name: FindKeyInCloudFdCacheTest001
 * @tc.desc: Verify the FindKeyInCloudFdCache function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, FindKeyInCloudFdCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindKeyInCloudFdCacheTest001 start";
    try {
        auto cInode = make_shared<FuseData>();
        uint64_t key = 100;
        auto newCloudFdInfo = std::make_shared<struct CloudFdInfo>();
        cInode->cloudFdCache[key] = newCloudFdInfo;
        auto ret = FindKeyInCloudFdCache(cInode.get(), key);
        EXPECT_NE(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindKeyInCloudFdCacheTest failed";
    }
    GTEST_LOG_(INFO) << "FindKeyInCloudFdCacheTest end";
}

/**
 * @tc.name: FindKeyInCloudFdCacheTest002
 * @tc.desc: Verify the FindKeyInCloudFdCache function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, FindKeyInCloudFdCacheTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindKeyInCloudFdCacheTest002 start";
    try {
        auto cInode = make_shared<FuseData>();
        uint64_t key = 1;
        auto ret = FindKeyInCloudFdCache(cInode.get(), key);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindKeyInCloudFdCacheTest failed";
    }
    GTEST_LOG_(INFO) << "FindKeyInCloudFdCacheTest end";
}

/**
 * @tc.name: SaveAppIdTest001
 * @tc.desc: Verify the CloudRead function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, SaveAppIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppIdTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        struct fuse_file_info* fi = nullptr;
        const void* inBuf = nullptr;
        FuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        SaveAppId(req, ino, fi, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SaveAppIdTest001 failed";
    }
    GTEST_LOG_(INFO) << "SaveAppIdTest001 end";
}

/**
 * @tc.name: SaveAppIdTest002
 * @tc.desc: Verify the CloudRead function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, SaveAppIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppIdTest002 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        struct fuse_file_info* fi = nullptr;
        const void* inBuf = nullptr;
        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        SaveAppId(req, ino, fi, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SaveAppIdTest002 failed";
    }
    GTEST_LOG_(INFO) << "SaveAppIdTest002 end";
}

/**
 * @tc.name: SaveAppIdTest003
 * @tc.desc: Verify the CloudRead function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, SaveAppIdTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveAppIdTest003 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        struct fuse_file_info fi;
        HmdfsSaveAppId hsaId;
        const void* inBuf = &hsaId;
        FuseData data;
        fi.fh = 100;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        shared_ptr<CloudFdInfo> cloudFdInfo = make_shared<CloudFdInfo>();
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});
        data.cloudFdCache.insert({100, cloudFdInfo});
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_ioctl(_, _, _, _)).WillOnce(Return(E_OK));

        SaveAppId(req, ino, &fi, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SaveAppIdTest003 failed";
    }
    GTEST_LOG_(INFO) << "SaveAppIdTest003 end";
}

/**
 * @tc.name: DeleteFdsanTest001
 * @tc.desc: Verify the DeleteFdsan function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, DeleteFdsanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteFdsanTest001 start";
    try {
        auto cloudFdInfo = make_shared<struct CloudFdInfo>();
        cloudFdInfo->fdsan = UINT64_MAX;

        DeleteFdsan(nullptr);
        EXPECT_EQ(cloudFdInfo->fdsan, UINT64_MAX);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteFdsanTest001 failed";
    }
    GTEST_LOG_(INFO) << "DeleteFdsanTest001 end";
}

/**
 * @tc.name: DeleteFdsanTest002
 * @tc.desc: Verify the DeleteFdsan function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, DeleteFdsanTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteFdsanTest002 start";
    try {
        auto cloudFdInfo = make_shared<struct CloudFdInfo>();
        cloudFdInfo->fdsan = UINT64_MAX;

        DeleteFdsan(cloudFdInfo);
        EXPECT_EQ(cloudFdInfo->fdsan, UINT64_MAX);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteFdsanTest002 failed";
    }
    GTEST_LOG_(INFO) << "DeleteFdsanTest002 end";
}

/**
 * @tc.name: DeleteFdsanTest003
 * @tc.desc: Verify the DeleteFdsan function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, DeleteFdsanTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteFdsanTest003 start";
    try {
        auto cloudFdInfo = make_shared<struct CloudFdInfo>();
        cloudFdInfo->fdsan = reinterpret_cast<uint64_t>(new fdsan_fd());

        DeleteFdsan(cloudFdInfo);
        EXPECT_EQ(cloudFdInfo->fdsan, UINT64_MAX);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteFdsanTest003 failed";
    }
    GTEST_LOG_(INFO) << "DeleteFdsanTest003 end";
}

/**
 * @tc.name: IsHdc001
 * @tc.desc: Verify the IsHdc function
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, IsHdc001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsHdc001 Begin";
    try {
        struct FuseData data;
        data.activeBundle = CloudSync::GALLERY_BUNDLE_NAME;
        bool ret = IsHdc(&data);

        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsHdc001 Error";
    }
    GTEST_LOG_(INFO) << "IsHdc001 End";
}

/**
 * @tc.name: IsHdc002
 * @tc.desc: Verify the IsHdc function
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, IsHdc002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsHdc002 Begin";
    try {
        struct FuseData data;
        data.activeBundle = CloudSync::HDC_BUNDLE_NAME;
        bool ret = IsHdc(&data);

        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsHdc002 Error";
    }
    GTEST_LOG_(INFO) << "IsHdc002 End";
}

/**
 * @tc.name: UpdateReadStatInfoTest001
 * @tc.desc: Verify the UpdateReadStatInfo function
 * @tc.type: FUNC
 * @tc.require:ICTLI4
 */
HWTEST_F(FuseManagerStaticTest, UpdateReadStatInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateReadStatInfoTest001 Begin";
    try {
        CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
        readStat.bundleName_ = "";
        size_t size = 1;
        string name = "name";
        uint64_t readTime = 1;
        string bundleName = "test.bundle.name";
        UpdateReadStatInfo(size, name, 1, bundleName);
        EXPECT_EQ(readStat.bundleName_, "test.bundle.name");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateReadStatInfoTest001 Error";
    }
    GTEST_LOG_(INFO) << "UpdateReadStatInfoTest001 End";
}

/**
 * @tc.name: UpdateReadStatInfoTest002
 * @tc.desc: Verify the UpdateReadStatInfo function
 * @tc.type: FUNC
 * @tc.require:ICTLI4
 */
HWTEST_F(FuseManagerStaticTest, UpdateReadStatInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateReadStatInfoTest002 Begin";
    try {
        CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
        readStat.bundleName_ = "test.bundle.name";
        size_t size = 1;
        string name = "name";
        uint64_t readTime = 1;
        string bundleName = "new.bundle.name";
        UpdateReadStatInfo(size, name, 1, bundleName);
        EXPECT_EQ(readStat.bundleName_, "new.bundle.name");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateReadStatInfoTest002 Error";
    }
    GTEST_LOG_(INFO) << "UpdateReadStatInfoTest002 End";
}

/**
 * @tc.name: UpdateReadStatTest001
 * @tc.desc: Verify the UpdateReadStat function
 * @tc.type: FUNC
 * @tc.require:ICTLI4
 */
HWTEST_F(FuseManagerStaticTest, UpdateReadStatTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateReadStatTest001 Begin";
    try {
        CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
        readStat.bundleName_ = "";
        auto cInode = make_shared<CloudInode>();
        cInode->mBase = make_shared<MetaBase>("test");
        cInode->mBase->fileType = FILE_TYPE_THUMBNAIL;
        uint64_t startTime = 1;
        string bundleName = "test.bundle.name";
        UpdateReadStat(cInode, startTime, bundleName);
        EXPECT_EQ(readStat.bundleName_, "test.bundle.name");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateReadStatTest001 Error";
    }
    GTEST_LOG_(INFO) << "UpdateReadStatTest001 End";
}

/**
 * @tc.name: UpdateReadStatTest002
 * @tc.desc: Verify the UpdateReadStat function
 * @tc.type: FUNC
 * @tc.require:ICTLI4
 */
HWTEST_F(FuseManagerStaticTest, UpdateReadStatTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateReadStatTest002 Begin";
    try {
        CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
        readStat.bundleName_ = "test.bundle.name";
        auto cInode = make_shared<CloudInode>();
        cInode->mBase = make_shared<MetaBase>("test");
        cInode->mBase->fileType = FILE_TYPE_THUMBNAIL;
        uint64_t startTime = 1;
        string bundleName = "new.bundle.name";
        UpdateReadStat(cInode, startTime, bundleName);
        EXPECT_EQ(readStat.bundleName_, "new.bundle.name");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateReadStatTest002 Error";
    }
    GTEST_LOG_(INFO) << "UpdateReadStatTest002 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test