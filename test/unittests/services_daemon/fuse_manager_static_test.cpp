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

#define open(file, flag) MyOpen(file, flag)
#define pread(fd, buf, size, off) MyPread(fd, buf, size, off)
#include "fuse_manager.cpp"
#define MyOpen(file, flag) open(file, flag)
#define MyPread(fd, buf, size, off) pread(fd, buf, size, off)

#include "utils_log.h"
#include "cloud_asset_read_session_mock.h"

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
    AssistantMock::EnableMock();
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
    fuseManager_ = nullptr;
}

/**
 * @tc.name: CheckAndReport001
 * @tc.desc: Verify the IsHdc function
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, CheckAndReport001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndReport001 Begin";
    try {
        string path = "/mnt/data/100/cloud_fuse/1";
        string childName = "/mnt/data/100/cloud_fuse/2";
        bool create = false;
        CheckAndReport(path, childName, create);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndReport001 Error";
    }
    GTEST_LOG_(INFO) << "CheckAndReport001 End";
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

/**
 * @tc.name: HasCache001
 * @tc.desc: cInode is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache001 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache001 Error";
    }
    GTEST_LOG_(INFO) << "HasCache001 End";
}

/**
 * @tc.name: HasCache002
 * @tc.desc: cInode->readSession is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache002 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        data.inodeCache.insert({100, cloudInode});

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache002 Error";
    }
    GTEST_LOG_(INFO) << "HasCache002 End";
}

/**
 * @tc.name: HasCache003
 * @tc.desc: ioctlData is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache003 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache003 Error";
    }
    GTEST_LOG_(INFO) << "HasCache003 End";
}

/**
 * @tc.name: HasCache004
 * @tc.desc: ioctlData->offset < 0
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache004 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        HmdfsHasCache ioctlData;
        ioctlData.offset = -1;
        inBuf = &ioctlData;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache004 Error";
    }
    GTEST_LOG_(INFO) << "HasCache004 End";
}

/**
 * @tc.name: HasCache005
 * @tc.desc: ioctlData->offset > cInode->mBase->size
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache005 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->mBase = make_shared<MetaBase>("test");
        cloudInode->mBase->size = 10;
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        HmdfsHasCache ioctlData;
        ioctlData.offset = 11;
        inBuf = &ioctlData;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache005 Error";
    }
    GTEST_LOG_(INFO) << "HasCache005 End";
}

/**
 * @tc.name: HasCache006
 * @tc.desc: ioctlData->readSize < 0
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache006 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->mBase = make_shared<MetaBase>("test");
        cloudInode->mBase->size = 10;
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        HmdfsHasCache ioctlData;
        ioctlData.offset = 0;
        ioctlData.readSize = -1;
        inBuf = &ioctlData;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache006 Error";
    }
    GTEST_LOG_(INFO) << "HasCache006 End";
}

/**
 * @tc.name: HasCache007
 * @tc.desc: 1. ioctlData->offset + MAX_READ_SIZE < cInode->mBase->size
 *           2. IsPageCached(cInode, headIndex)->IsVideoType(cInode->mBase->name) false
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache007 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->mBase = make_shared<MetaBase>("test");
        cloudInode->mBase->size = 3 * MAX_READ_SIZE;
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        HmdfsHasCache ioctlData;
        ioctlData.offset = 0;
        ioctlData.readSize = 1;
        inBuf = &ioctlData;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache007 Error";
    }
    GTEST_LOG_(INFO) << "HasCache007 End";
}

/**
 * @tc.name: HasCache008
 * @tc.desc: 1. ioctlData->offset + MAX_READ_SIZE >= cInode->mBase->size
 *           2. IsPageCached(cInode, headIndex)->IsVideoType(cInode->mBase->name) false
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache008 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->mBase = make_shared<MetaBase>("test");
        cloudInode->mBase->size = 3 * MAX_READ_SIZE;
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        HmdfsHasCache ioctlData;
        ioctlData.offset = 2 * MAX_READ_SIZE;
        ioctlData.readSize = 1;
        inBuf = &ioctlData;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache008 Error";
    }
    GTEST_LOG_(INFO) << "HasCache008 End";
}

/**
 * @tc.name: HasCache009
 * @tc.desc: 1. ioctlData->offset + MAX_READ_SIZE < cInode->mBase->size
 *           2. IsPageCached(cInode, headIndex)->NOT_CACHE
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache009 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->mBase = make_shared<MetaBase>("VID_1761897125_003.mp4");
        cloudInode->mBase->size = 3 * MAX_READ_SIZE;
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        int filePageSize = static_cast<int32_t>(cloudInode->mBase->size / MAX_READ_SIZE + 1);
        CLOUD_CACHE_STATUS *tmp = new CLOUD_CACHE_STATUS[filePageSize]();
        std::unique_ptr<CLOUD_CACHE_STATUS[]> mp(tmp);
        cloudInode->cacheFileIndex = std::move(mp);

        HmdfsHasCache ioctlData;
        ioctlData.offset =  0;
        ioctlData.readSize = 1;
        inBuf = &ioctlData;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache009 Error";
    }
    GTEST_LOG_(INFO) << "HasCache009 End";
}

/**
 * @tc.name: HasCache0010
 * @tc.desc: 1. ioctlData->offset + MAX_READ_SIZE < cInode->mBase->size
 *           2. IsPageCached(cInode, headIndex)->HAS_CACHED IsPageCached(cInode, tailIndex)->NOT_CACHE
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache0010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache0010 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->mBase = make_shared<MetaBase>("VID_1761897125_003.mp4");
        cloudInode->mBase->size = 3 * MAX_READ_SIZE;
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        int filePageSize = static_cast<int32_t>(cloudInode->mBase->size / MAX_READ_SIZE + 1);
        CLOUD_CACHE_STATUS *tmp = new CLOUD_CACHE_STATUS[filePageSize]();
        std::unique_ptr<CLOUD_CACHE_STATUS[]> mp(tmp);
        cloudInode->cacheFileIndex = std::move(mp);
        cloudInode->cacheFileIndex.get()[0] = HAS_CACHED;

        HmdfsHasCache ioctlData;
        ioctlData.offset =  0;
        ioctlData.readSize = 1;
        inBuf = &ioctlData;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache0010 Error";
    }
    GTEST_LOG_(INFO) << "HasCache0010 End";
}

/**
 * @tc.name: HasCache0011
 * @tc.desc: 1. ioctlData->offset + MAX_READ_SIZE < cInode->mBase->size
 *           2. IsPageCached(cInode, headIndex)->HAS_CACHED IsPageCached(cInode, tailIndex)->HAS_CACHED
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, HasCache0011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasCache0011 Begin";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 100;
        const void* inBuf = nullptr;

        FuseData data;
        shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
        cloudInode->mBase = make_shared<MetaBase>("VID_1761897125_003.mp4");
        cloudInode->mBase->size = 3 * MAX_READ_SIZE;
        cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
        data.inodeCache.insert({100, cloudInode});

        int filePageSize = static_cast<int32_t>(cloudInode->mBase->size / MAX_READ_SIZE + 1);
        CLOUD_CACHE_STATUS *tmp = new CLOUD_CACHE_STATUS[filePageSize]();
        std::unique_ptr<CLOUD_CACHE_STATUS[]> mp(tmp);
        cloudInode->cacheFileIndex = std::move(mp);
        cloudInode->cacheFileIndex.get()[0] = HAS_CACHED;
        cloudInode->cacheFileIndex.get()[1] = HAS_CACHED;

        HmdfsHasCache ioctlData;
        ioctlData.offset =  0;
        ioctlData.readSize = 1;
        inBuf = &ioctlData;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_ioctl(_, _, _, _)).WillOnce(Return(E_OK));

        HasCache(req, ino, inBuf);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HasCache0011 Error";
    }
    GTEST_LOG_(INFO) << "HasCache0011 End";
}

/**
 * @tc.name:CloudReadHelperTest001
 * @tc.desc: Verify CloudReadHelperTest func
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, CloudReadHelperTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudReadHelperTest001 Begin";
    try {
        fuse_req_t req = new struct fuse_req;
        req->ctx.pid = 100;
        pid_t pid = GetPidFromTid(req->ctx.pid);
        shared_ptr<CloudInode> cInode = make_shared<CloudInode>();
        cInode->readCtlMap.insert({pid, true});
        size_t size = 0;
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        bool ret = CloudReadHelper(req, size, cInode);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudReadHelperTest001 Error";
    }
    GTEST_LOG_(INFO) << "CloudReadHelperTest001 End";
}

/**
 * @tc.name:CloudReadHelperTest002
 * @tc.desc: Verify CloudReadHelperTest func
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, CloudReadHelperTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudReadHelperTest002 Begin";
    try {
        fuse_req_t req = new struct fuse_req;
        req->ctx.pid = 100;
        pid_t pid = GetPidFromTid(req->ctx.pid);
        shared_ptr<CloudInode> cInode = make_shared<CloudInode>();
        cInode->readCtlMap.insert({pid, false});
        size_t size = MAX_READ_SIZE + 1;
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        bool ret = CloudReadHelper(req, size, cInode);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudReadHelperTest002 Error";
    }
    GTEST_LOG_(INFO) << "CloudReadHelperTest002 End";
}

/**
 * @tc.name:CloudReadHelperTest003
 * @tc.desc: Verify CloudReadHelperTest func
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, CloudReadHelperTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudReadHelperTest003 Begin";
    try {
        fuse_req_t req = new struct fuse_req;
        req->ctx.pid = 100;
        pid_t pid = GetPidFromTid(req->ctx.pid);
        shared_ptr<CloudInode> cInode = make_shared<CloudInode>();
        cInode->readCtlMap.insert({pid, false});
        size_t size = 0;
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        bool ret = CloudReadHelper(req, size, cInode);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudReadHelperTest003 Error";
    }
    GTEST_LOG_(INFO) << "CloudReadHelperTest003 End";
}

/**
 * @tc.name:GetSessionTest001
 * @tc.desc: Verify GetSessionTest func
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, GetSessionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSessionTest001 Begin";
    try {
        string path = "test/key/1";
        struct fuse_session *fuseSession = new struct fuse_session;
        fuseManager_->sessions_.insert({path, fuseSession});

        struct fuse_session *outFuseSession = fuseManager_->GetSession(path);
        EXPECT_EQ(outFuseSession, fuseSession);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSessionTest001 Error";
    }
    GTEST_LOG_(INFO) << "GetSessionTest001 End";
}

/**
 * @tc.name:GetSessionTest002
 * @tc.desc: Verify GetSessionTest func
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, GetSessionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSessionTest002 Begin";
    try {
        string path = "test/key/2";

        struct fuse_session *outFuseSession = fuseManager_->GetSession(path);
        EXPECT_EQ(outFuseSession, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSessionTest002 Error";
    }
    GTEST_LOG_(INFO) << "GetSessionTest002 End";
}

/**
 * @tc.name: DoReadSliceTest001
 * @tc.desc: 测试当视频文件且缓存可用时,视频文件,DoReadSlice 应返回 true
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, DoReadSliceTest001, TestSize.Level1) {
    fuse_req_t req = new struct fuse_req;
    req->ctx.pid = 100;
    pid_t pid = GetPidFromTid(req->ctx.pid);

    FuseData* data = new FuseData();
    data->photoBundleName = "example_bundle_name";
    data->userId = 1;

    shared_ptr<CloudInode> cInode = make_shared<CloudInode>();
    cInode->mBase = make_shared<MetaBase>("VID_1761897125_003.mp4");
    cInode->mBase->size = 3 * MAX_READ_SIZE;
    cInode->path = "/./DoReadSliceTest001";
    size_t newSize = 10;
    cInode->cacheFileIndex = std::make_unique<CLOUD_CACHE_STATUS[]>(newSize);
    for (size_t i = 0; i < newSize; ++i) {
        cInode->cacheFileIndex[i] = NOT_CACHE;
    }
    cInode->readCacheMap.insert(std::make_pair(1, std::make_shared<ReadCacheInfo>()));

    auto readSessionMock = make_shared<CloudAssetReadSessionMock>(100, "test", "test", "test", "test");
    shared_ptr<ReadArguments> readArgs = make_shared<ReadArguments>(0, "", 0, 0);
    readArgs->offset = 0;
    size_t bufferSize = 128;
    readArgs->buf = std::shared_ptr<char>(new char[bufferSize]);

    readArgs->readResult = std::make_shared<int64_t>(0);
    readArgs->readStatus = std::make_shared<CLOUD_READ_STATUS>(READING);
    bool needCheck = false;

    EXPECT_CALL(*readSessionMock, PRead(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(data)));
    EXPECT_CALL(*insMock, MyOpen).WillOnce(Return(0));
    bool result = DoReadSlice(req, cInode, readSessionMock, readArgs, needCheck);
    ffrt::wait();
    EXPECT_TRUE(result);

    cInode->readCacheMap.clear();
    EXPECT_TRUE(cInode->readCacheMap.empty());
}

/**
 * @tc.name: DoReadSliceTest002
 * @tc.desc: 测试当视频文件且缓存可用时,非视频文件，DoReadSlice 应返回 true
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, DoReadSliceTest002, TestSize.Level1) {
    fuse_req_t req = new struct fuse_req;
    req->ctx.pid = 100;
    pid_t pid = GetPidFromTid(req->ctx.pid);

    FuseData* data = new FuseData();
    data->photoBundleName = "example_bundle_name";
    data->userId = 1;

    shared_ptr<CloudInode> cInode = make_shared<CloudInode>();
    cInode->mBase = make_shared<MetaBase>("1761897125_003.mp4");
    cInode->mBase->size = 3 * MAX_READ_SIZE;
    cInode->path = "/./DoReadSliceTest002";
    size_t newSize = 10;
    cInode->cacheFileIndex = std::make_unique<CLOUD_CACHE_STATUS[]>(newSize);
    for (size_t i = 0; i < newSize; ++i) {
        cInode->cacheFileIndex[i] = NOT_CACHE;
    }
    cInode->readCacheMap.insert(std::make_pair(2, std::make_shared<ReadCacheInfo>()));
    cInode->readCtlMap.insert({pid, true});
    cInode->readCtlMap.clear();
    EXPECT_TRUE(cInode->readCtlMap.empty());

    auto readSessionMock = make_shared<CloudAssetReadSessionMock>(100, "test", "test", "test", "test");
    shared_ptr<ReadArguments> readArgs = make_shared<ReadArguments>(0, "", 0, 0);
    readArgs->offset = 0;
    size_t bufferSize = 128;
    readArgs->buf = std::shared_ptr<char>(new char[bufferSize]);

    readArgs->readResult = std::make_shared<int64_t>(0);
    readArgs->readStatus = std::make_shared<CLOUD_READ_STATUS>(READING);
    bool needCheck = false;

    EXPECT_CALL(*readSessionMock, PRead(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(data)));
    EXPECT_CALL(*insMock, MyOpen).WillOnce(Return(0));
    bool result = DoReadSlice(req, cInode, readSessionMock, readArgs, needCheck);
    ffrt::wait();
    EXPECT_TRUE(result);

    cInode->readCacheMap.clear();
    EXPECT_TRUE(cInode->readCacheMap.empty());
}

/**
 * @tc.name: CloudReadOnCacheFileTest001
 * @tc.desc: 当缓存索引不存在且读取成功时,函数应进入后续操作
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, CloudReadOnCacheFileTest001, TestSize.Level1) {
    shared_ptr<ReadArguments> readArgs = make_shared<ReadArguments>(0, "", 0, 0);
    shared_ptr<CloudInode> cInode = make_shared<CloudInode>();
    shared_ptr<FFRTParamData> data = make_shared<FFRTParamData>("", "");

    readArgs->offset = 0;
    cInode->mBase = make_shared<MetaBase>("VID_1761897125_003.mp4");
    cInode->mBase->size = 100;

    // 设置缓存索引不存在
    uint64_t cacheIndex = readArgs->offset / MAX_READ_SIZE;
    cInode->readCacheMap.clear();
    EXPECT_TRUE(cInode->readCacheMap.empty());
    size_t newSize = 10;
    cInode->cacheFileIndex = std::make_unique<CLOUD_CACHE_STATUS[]>(newSize);
    for (size_t i = 0; i < newSize; ++i) {
        cInode->cacheFileIndex[i] = NOT_CACHE;
    }

    auto readSessionMock = make_shared<CloudAssetReadSessionMock>(100, "test", "test", "test", "test");
    EXPECT_CALL(*readSessionMock, PRead(_, _, _, _)).WillOnce(Return(0));
    CloudReadOnCacheFile(readArgs, cInode, readSessionMock, data);

    // 验证函数进入后续操作
    EXPECT_EQ(cInode->cacheFileIndex.get()[cacheIndex], HAS_CACHED);
    cInode->readCacheMap.clear();
    EXPECT_TRUE(cInode->readCacheMap.empty());
}

/**
 * @tc.name: CloudReadOnCloudFileTest001
 * @tc.desc: 落盘成功，设置标志位
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, CloudReadOnCloudFileTest001, TestSize.Level1) {
    shared_ptr<FFRTParamData> data = make_shared<FFRTParamData>("", "");
    shared_ptr<ReadArguments> readArgs = make_shared<ReadArguments>(0, "", 0, 0);
    readArgs->cond = make_shared<ffrt::condition_variable>();;

    shared_ptr<CloudInode> cInode = make_shared<CloudInode>();
    pid_t pid = 1;
    readArgs->offset = 0;
    cInode->mBase = make_shared<MetaBase>("VID_1761897125_003.mp4");
    cInode->mBase->size = 100;
    cInode->readCacheMap.clear();
    EXPECT_TRUE(cInode->readCacheMap.empty());

    size_t newSize = 10;
    cInode->cacheFileIndex = std::make_unique<CLOUD_CACHE_STATUS[]>(newSize);
    for (size_t i = 0; i < newSize; ++i) {
        cInode->cacheFileIndex[i] = NOT_CACHE;
    }

    auto readSessionMock = make_shared<CloudAssetReadSessionMock>(100, "test", "test", "test", "test");
    EXPECT_CALL(*readSessionMock, PRead(_, _, _, _)).WillOnce(Return(1024));
    CloudReadOnCloudFile(pid, data, readArgs, cInode, readSessionMock);

    // 验证函数进入后续操作
    EXPECT_EQ(*readArgs->readResult, 1024);

    cInode->readCacheMap.clear();
    EXPECT_TRUE(cInode->readCacheMap.empty());
}

/**
 * @tc.name: CloudReleaseTest001
 * @tc.desc: close接口传递是否是视频标志成功
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, CloudReleaseTest001, TestSize.Level1) {
    fuse_req_t req = new struct fuse_req;
    req->ctx.pid = 1;
    fuse_ino_t ino = 10;
    struct fuse_file_info fi;
    fi.fh = 100;

    shared_ptr<CloudInode> cloudInode = make_shared<CloudInode>();
    cloudInode->readSession = make_shared<CloudFile::CloudAssetReadSession>(100, "", "", "", "");
    cloudInode->sessionRefCount = 1;
    cloudInode->mBase = make_shared<MetaBase>("VID_1761897125_003.mp4");
    cloudInode->mBase->size = 100;
    cloudInode->mBase->fileType = FILE_TYPE_CONTENT;
    cloudInode->path = "/./CloudReleaseTest001";
    size_t newSize = 10;
    cloudInode->cacheFileIndex = std::make_unique<CLOUD_CACHE_STATUS[]>(newSize);
    for (size_t i = 0; i < newSize; ++i) {
        cloudInode->cacheFileIndex[i] = NOT_CACHE;
    }
    auto readSessionMock = make_shared<CloudAssetReadSessionMock>(100, "test", "test", "test", "test");
    cloudInode->readSession = readSessionMock;

    FuseData* data = new FuseData();
    data->photoBundleName = "example_bundle_name";
    data->userId = 1;
    data->database = make_shared<CloudFile::CloudDatabase>(data->userId, data->photoBundleName);
    data->inodeCache.insert({10, cloudInode});
    uint64_t key = 100;
    auto newCloudFdInfo = std::make_shared<struct CloudFdInfo>();
    data->cloudFdCache[key] = newCloudFdInfo;

    EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(data)));
    EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*readSessionMock, Close(_)).WillOnce(Return(false));
    CloudRelease(req, ino, &fi);
    EXPECT_EQ(cloudInode->sessionRefCount, 0);
    ffrt::wait();
}

/**
 * @tc.name: CreateReadSessionTest001
 * @tc.desc: 创建选择视频缓存ReadSession
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, CreateReadSessionTest001, TestSize.Level1) {

    shared_ptr<CloudInode> cInode = make_shared<CloudInode>();
    cInode->readSession = nullptr;
    cInode->mBase = make_shared<MetaBase>("VID_1761897125_003.mp4");
    cInode->mBase->size = 100;
    cInode->mBase->fileType = FILE_TYPE_CONTENT;
    cInode->path = "/./CreateReadSessionTest001";

    FuseData* data = new FuseData();
    data->photoBundleName = "example_bundle_name";
    data->userId = 1;
    data->database = make_shared<CloudFile::CloudDatabase>(data->userId, data->photoBundleName);

    std::string recordId = "test_recordId";

    EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(data)));
    cInode->readSession = CreateReadSession(cInode, data->database, recordId, data);

    EXPECT_NE(cInode->readSession, nullptr);
}

/**
 * @tc.name: ReadFileToBufTest001
 * @tc.desc: 读取缓存成功,读取到1024字节
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, ReadFileToBufTest001, TestSize.Level1) {
    std::string path = "/path/to/file";
    char buf[1024];
    size_t size = 1024;
    off_t off = 0;

    EXPECT_CALL(*insMock, MyOpen).WillOnce(Return(0));
    EXPECT_CALL(*insMock, MyPread(_, _, _, _)).WillOnce(Return(1024));
    int64_t result = ReadFileToBuf(path, buf, size, off);
    EXPECT_EQ(result, 1024);
}

/**
 * @tc.name: ReadFileToBufTest002
 * @tc.desc: 读取缓存失败,读取到0字节
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, ReadFileToBufTest002, TestSize.Level1) {
    std::string path = "/path/to/file";
    char buf[1024];
    size_t size = 1024;
    off_t off = 0;

    EXPECT_CALL(*insMock, MyOpen).WillOnce(Return(0));
    EXPECT_CALL(*insMock, MyPread(_, _, _, _)).WillOnce(Return(0));
    int64_t result = ReadFileToBuf(path, buf, size, off);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: ReadFileToBufTest003
 * @tc.desc: 读取缓存成功,读取到末尾0字节
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, ReadFileToBufTest003, TestSize.Level1) {
    std::string path = "/path/to/file";
    char buf[1024];
    size_t size = 1024;
    off_t off = 0;

    EXPECT_CALL(*insMock, MyOpen).WillOnce(Return(0));
    EXPECT_CALL(*insMock, MyPread(_, _, _, _)).WillOnce(Return(-1));
    int64_t result = ReadFileToBuf(path, buf, size, off);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: ReadFileToBufTest004
 * @tc.desc: 读取缓存成功,中间有中断信号
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, ReadFileToBufTest004, TestSize.Level1) {
    std::string path = "/path/to/file";
    char buf[1024];
    size_t size = 1024;
    off_t off = 0;

    EXPECT_CALL(*insMock, MyOpen).WillOnce(Return(0));
    EXPECT_CALL(*insMock, MyPread(_, _, _, _)).WillOnce(SetErrnoAndReturn(EINTR, 1024));
    int64_t result = ReadFileToBuf(path, buf, size, off);
    EXPECT_EQ(result, 1024);
}

/**
 * @tc.name: ReadFileToBufTest005
 * @tc.desc: open文件失败,读取到0字节
 * @tc.type: FUNC
 */
HWTEST_F(FuseManagerStaticTest, ReadFileToBufTest005, TestSize.Level1) {
    std::string path = "/path/to/file";
    char buf[1024];
    size_t size = 1024;
    off_t off = 0;

    EXPECT_CALL(*insMock, MyOpen).WillOnce(Return(-1));
    int64_t result = ReadFileToBuf(path, buf, size, off);
    EXPECT_EQ(result, -1);
}

} // namespace OHOS::FileManagement::CloudSync::Test