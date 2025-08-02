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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <memory>

#include "assistant.h"
#include "dfs_error.h"
#include "file_operations_cloud.cpp"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;

constexpr int32_t USER_ID = 100;
class FileOperationsCloudStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FileOperationsCloud> fileOperationsCloud_ = nullptr;
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void FileOperationsCloudStaticTest::SetUpTestCase(void)
{
    fileOperationsCloud_ = make_shared<FileOperationsCloud>();
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileOperationsCloudStaticTest::TearDownTestCase(void)
{
    Assistant::ins = nullptr;
    insMock = nullptr;
    fileOperationsCloud_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileOperationsCloudStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileOperationsCloudStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: HandleNewSessionTest001
 * @tc.desc: Verify the HandleNewSession function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleNewSessionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNewSessionTest001 Start";
    try {
        string path = "/data";
        string cloudId = "100";
        string assets = "content";
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        data->readSessionCache[path] = readSession;
        data->readSessionCache[path]->sessionCount = 1;

        HandleNewSession(data, sessionParam, filePtr, database);
        EXPECT_EQ(filePtr->readSession->sessionCount, data->readSessionCache[path]->sessionCount);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNewSessionTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNewSessionTest001 End";
}

/**
 * @tc.name: HandleNewSessionTest002
 * @tc.desc: Verify the HandleNewSession function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleNewSessionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNewSessionTest002 Start";
    try {
        string path = "/data";
        string cloudId = "100";
        string assets = "content";
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        SessionCountParams sessionParam = {path, cloudId, assets};
        if (data->readSessionCache.find(path) != data->readSessionCache.end()) {
            data->readSessionCache.erase(path);
        }

        HandleNewSession(data, sessionParam, filePtr, database);
        EXPECT_EQ(filePtr->readSession->sessionCount, 0);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNewSessionTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNewSessionTest002 End";
}

/**
 * @tc.name: HandleNewSessionTest003
 * @tc.desc: Verify the HandleNewSession function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleNewSessionTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNewSessionTest003 Start";
    try {
        string path = "/data";
        string cloudId = "100";
        string assets = "content";
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        data->readSessionCache[path] = readSession;
        data->readSessionCache[path]->sessionCount = 2;

        HandleNewSession(data, sessionParam, filePtr, database);
        EXPECT_EQ(filePtr->readSession->sessionCount, data->readSessionCache[path]->sessionCount);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNewSessionTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNewSessionTest003 End";
}


/**
 * @tc.name: ErasePathCacheTest001
 * @tc.desc: Verify the ErasePathCache function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, ErasePathCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ErasePathCacheTest001 Start";
    try {
        string path = "/data";
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        if (data->readSessionCache.find(path) != data->readSessionCache.end()) {
            data->readSessionCache.erase(path);
        }

        ErasePathCache(path, data);
        EXPECT_EQ(data->readSessionCache.count(path), 0);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ErasePathCacheTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "ErasePathCacheTest001 End";
}

/**
 * @tc.name: ErasePathCacheTest002
 * @tc.desc: Verify the ErasePathCache function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, ErasePathCacheTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ErasePathCacheTest002 Start";
    try {
        string path = "/data";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        string cloudId = "100";
        string assets = "content";
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        struct SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        data->readSessionCache[path] = readSession;

        ErasePathCache(path, data);
        EXPECT_EQ(data->readSessionCache.count(path), 0);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ErasePathCacheTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "ErasePathCacheTest002 End";
}

/**
 * @tc.name: GetNewSessionTest001
 * @tc.desc: Verify the GetNewSession function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GetNewSessionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNewSessionTest001 Start";
    try {
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->cloudId = "123";
        inoPtr->bundleName = "com.example.test";
        string path = "/data";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(0, "com.example.test");
        MetaBase metaBase;
        metaBase.fileType = FILE_TYPE_THUMBNAIL;
        std::shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        CloudOpenParams cloudOpenParams = {metaBase, metaFile, filePtr};

        auto ret = GetNewSession(inoPtr, path, data, database, cloudOpenParams);
        EXPECT_EQ(ret, E_RDB);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNewSessionTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetNewSessionTest001 End";
}

/**
 * @tc.name: GetNewSessionTest002
 * @tc.desc: Verify the GetNewSession function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GetNewSessionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNewSessionTest002 Start";
    try {
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->cloudId = "123";
        inoPtr->bundleName = "com.example.test";
        string path = "/data";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(0, "com.example.test");
        MetaBase metaBase;
        metaBase.fileType = FILE_TYPE_CONTENT;
        std::shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        CloudOpenParams cloudOpenParams = {metaBase, metaFile, filePtr};

        auto ret = GetNewSession(inoPtr, path, data, database, cloudOpenParams);
        EXPECT_EQ(filePtr->readSession->sessionCount, data->readSessionCache[path]->sessionCount);
        EXPECT_EQ(ret, E_OK);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNewSessionTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetNewSessionTest002 End";
}

/**
 * @tc.name: HandleReleaseTest001
 * @tc.desc: Verify the ReleaseHandle function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleReleaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleReleaseTest001 Start";
    try {
        string path = "/data";
        string cloudId = "100";
        string assets = "content";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->bundleName = "Test";
        inoPtr->cloudId = "100";
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        struct SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        filePtr->readSession = readSession;
        filePtr->readSession->sessionCount = 1;

        HandleRelease(filePtr, inoPtr, data);
        EXPECT_EQ(filePtr->readSession, nullptr);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleReleaseTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleReleaseTest001 End";
}

/**
 * @tc.name: HandleReleaseTest002
 * @tc.desc: Verify the ReleaseHandle function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleReleaseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleReleaseTest002 Start";
    try {
        string path = "/data";
        string cloudId = "100";
        string assets = "content";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->bundleName = "Test";
        inoPtr->cloudId = "100";
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        struct SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        filePtr->readSession = readSession;
        filePtr->readSession->sessionCount = 2;

        HandleRelease(filePtr, inoPtr, data);
        EXPECT_EQ(filePtr->readSession->sessionCount, 1);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleReleaseTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleReleaseTest002 End";
}

/**
 * @tc.name: HandleReleaseTest003
 * @tc.desc: Verify the ReleaseHandle function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleReleaseTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleReleaseTest003 Start";
    try {
        string path = "/data";
        string cloudId = "100";
        string assets = "content";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->bundleName = "Test";
        inoPtr->cloudId = "100";
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        struct SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        filePtr->readSession = readSession;
        filePtr->readSession->sessionCount = 0;

        HandleRelease(filePtr, inoPtr, data);
        EXPECT_EQ(filePtr->readSession->sessionCount, -1);
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleReleaseTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleReleaseTest003 End";
}

/**
 * @tc.name: HandleOpenFailTest001
 * @tc.desc: Verify the ReleaseHandle function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleOpenFailTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOpenFailTest001 Start";
    try {
        string path = "/data";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        fuse_file_info *fi = new fuse_file_info;
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        filePtr->readSession = nullptr;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->fileName = "Test";
        fuse_ino_t ino = 0;
        HandleOpenErrorParams params = {filePtr, inoPtr, ino};
        string cloudId = "100";
        string assets = "content";
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        data->readSessionCache[path] = readSession;

        HandleOpenFail(params, path, data, fi);
        EXPECT_EQ(data->readSessionCache.count(path), 0);
        delete fi;
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOpenFailTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOpenFailTest001 End";
}

/**
 * @tc.name: HandleSessionNullTest001
 * @tc.desc: Verify the HandleSessionNull function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleSessionNullTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleSessionNullTest001 Start";
    try {
        string path = "/data";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        fuse_file_info *fi = new fuse_file_info;
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        filePtr->readSession = nullptr;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->fileName = "Test";
        fuse_ino_t ino = 0;
        HandleOpenErrorParams params = {filePtr, inoPtr, ino};
        string cloudId = "100";
        string assets = "content";
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        data->readSessionCache[path] = readSession;

        HandleSessionNull(params, data, fi);
        EXPECT_EQ(data->readSessionCache[path], readSession);
        delete fi;
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleSessionNullTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleSessionNullTest001 End";
}

/**
 * @tc.name: HandleCloudReopenTest001
 * @tc.desc: Verify the HandleCloudReopen function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleCloudReopenTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCloudReopenTest001 start";
    try {
        string cloudId = "100";
        string assets = "content";
        string path = "/data";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        fuse_file_info *fi = new fuse_file_info;
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        filePtr->readSession = nullptr;
        filePtr->type = CLOUD_DISK_FILE_TYPE_UNKNOWN;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->fileName = "Test";
        MetaBase metaBase(inoPtr->fileName);
        metaBase.fileType = FILE_TYPE_CONTENT;
        std::shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        CloudOpenParams params = {metaBase, metaFile, filePtr};
        fuse_req_t req = nullptr;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        filePtr->readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        EXPECT_CALL(*insMock, fuse_reply_open(_, _)).WillOnce(Return(E_OK));

        HandleCloudReopen(fi, data, inoPtr, params, req);
        EXPECT_EQ(filePtr->type, CLOUD_DISK_FILE_TYPE_CLOUD);
        delete fi;
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCloudReopenTest001 failed";
    }
    GTEST_LOG_(INFO) << "HandleCloudReopenTest001 end";
}

/**
 * @tc.name: HandleCloudReopenTest002
 * @tc.desc: Verify the HandleCloudReopen function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleCloudReopenTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCloudReopenTest002 start";
    try {
        string cloudId = "100";
        string assets = "content";
        string path = "/data";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 0;
        fuse_file_info *fi = new fuse_file_info;
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        filePtr->readSession = nullptr;
        filePtr->type = CLOUD_DISK_FILE_TYPE_UNKNOWN;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->fileName = "Test";
        MetaBase metaBase(inoPtr->fileName);
        metaBase.fileType = FILE_TYPE_LCD;
        std::shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        CloudOpenParams params = {metaBase, metaFile, filePtr};
        fuse_req_t req = nullptr;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        filePtr->readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        HandleCloudReopen(fi, data, inoPtr, params, req);
        EXPECT_EQ(filePtr->type, CLOUD_DISK_FILE_TYPE_UNKNOWN);
        delete fi;
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCloudReopenTest002 failed";
    }
    GTEST_LOG_(INFO) << "HandleCloudReopenTest002 end";
}

/**
 * @tc.name: HandleCloudReopenTest003
 * @tc.desc: Verify the HandleCloudReopen function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleCloudReopenTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCloudReopenTest003 start";
    try {
        string cloudId = "100";
        string assets = "content";
        string path = "/data";
        CloudDiskFuseData *data = new CloudDiskFuseData;
        data->userId = 100;
        fuse_file_info *fi = new fuse_file_info;
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        filePtr->readSession = nullptr;
        filePtr->type = CLOUD_DISK_FILE_TYPE_UNKNOWN;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->fileName = "Test";
        MetaBase metaBase(inoPtr->fileName);
        metaBase.fileType = FILE_TYPE_LCD;
        std::shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        CloudOpenParams params = {metaBase, metaFile, filePtr};
        fuse_req_t req = nullptr;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        filePtr->readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        EXPECT_CALL(*insMock, fuse_reply_open(_, _)).WillOnce(Return(E_OK));

        HandleCloudReopen(fi, data, inoPtr, params, req);
        EXPECT_EQ(filePtr->type, CLOUD_DISK_FILE_TYPE_LOCAL);
        delete fi;
        delete data;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCloudReopenTest003 failed";
    }
    GTEST_LOG_(INFO) << "HandleCloudReopenTest003 end";
}
}