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
#include "cloud_asset_read_session_mock.h"
#include "dfs_error.h"
#include "file_operations_cloud.cpp"
#include "ffrt_inner.h"

enum class GenerateType : int32_t {
    FAILED_GEN,
    SUCC_GEN_ID_EMPTY,
    SUCC_GEN_ID_VALID,
};
static GenerateType g_generateType = GenerateType::FAILED_GEN;
namespace OHOS::FileManagement::CloudFile {
class CloudDatabaseMock : public CloudDatabase {
    using CloudDatabase::CloudDatabase;
    int32_t Convert2CloudId(const std::string &localId, std::string &cloudId) override
    {
        if (localId == "validLocalId") {
            cloudId = "validCloudId";
            return E_OK;
        }
        return E_INVAL_ARG;
    }
    int32_t GenerateIds(int count, std::vector<std::string> &ids) override
    {
        if (g_generateType == GenerateType::FAILED_GEN) {
            return E_INVAL_ARG;
        }
        if (g_generateType == GenerateType::SUCC_GEN_ID_EMPTY) {
            return E_OK;
        }
        if (g_generateType == GenerateType::SUCC_GEN_ID_VALID) {
            ids.push_back("valid_cloudId");
            return E_OK;
        }
        return E_OK;
    }
};

class CloudFileKitMock : public CloudFile::CloudFileKit {
    std::shared_ptr<CloudDatabase> GetCloudDatabase(const int32_t userId, const std::string &bundleName) override
    {
        return std::make_shared<CloudDatabaseMock>(userId, bundleName);
    }
    int32_t GenerateLocalIds(const std::string &bundleName, int count, std::vector<std::string> &ids) override
    {
        if (g_generateType == GenerateType::FAILED_GEN) {
            return E_INVAL_ARG;
        }
        if (g_generateType == GenerateType::SUCC_GEN_ID_EMPTY) {
            return E_OK;
        }
        if (g_generateType == GenerateType::SUCC_GEN_ID_VALID) {
            ids.push_back("valid_cloudId");
            return E_OK;
        }
        return E_OK;
    }
};
} // namespace OHOS::FileManagement::CloudFile

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
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void FileOperationsCloudStaticTest::SetUpTestCase(void)
{
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileOperationsCloudStaticTest::TearDownTestCase(void)
{
    Assistant::ins = nullptr;
    insMock = nullptr;
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
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        data->readSessionCache[path] = readSession;
        data->readSessionCache[path]->sessionCount = 1;

        HandleNewSession(data.get(), sessionParam, filePtr, database);
        EXPECT_EQ(filePtr->readSession->sessionCount, data->readSessionCache[path]->sessionCount);
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
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        SessionCountParams sessionParam = {path, cloudId, assets};
        if (data->readSessionCache.find(path) != data->readSessionCache.end()) {
            data->readSessionCache.erase(path);
        }

        HandleNewSession(data.get(), sessionParam, filePtr, database);
        EXPECT_EQ(filePtr->readSession->sessionCount, 0);
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
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        data->readSessionCache[path] = readSession;
        data->readSessionCache[path]->sessionCount = 2;

        HandleNewSession(data.get(), sessionParam, filePtr, database);
        EXPECT_EQ(filePtr->readSession->sessionCount, data->readSessionCache[path]->sessionCount);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNewSessionTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNewSessionTest003 End";
}

/**
 * @tc.name: HandleNewSessionTest004
 * @tc.desc: Verify the HandleNewSession function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleNewSessionTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNewSessionTest004 Start";
    try {
        string path = "/data";
        string cloudId = "invalidLocalId";
        string assets = "content";
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabaseMock>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        SessionCountParams sessionParam = {path, cloudId, assets};
        if (data->readSessionCache.find(path) != data->readSessionCache.end()) {
            data->readSessionCache.erase(path);
        }
        data->userId = 100;

        HandleNewSession(data.get(), sessionParam, filePtr, database);
        EXPECT_EQ(filePtr->readSession, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNewSessionTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNewSessionTest004 End";
}

/**
 * @tc.name: HandleNewSessionTest005
 * @tc.desc: Verify the HandleNewSession function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, HandleNewSessionTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNewSessionTest005 Start";
    try {
        string path = "/data";
        string cloudId = "validLocalId";
        string assets = "content";
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabaseMock>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        SessionCountParams sessionParam = {path, cloudId, assets};
        if (data->readSessionCache.find(path) != data->readSessionCache.end()) {
            data->readSessionCache.erase(path);
        }
        data->userId = 100;

        HandleNewSession(data.get(), sessionParam, filePtr, database);
        EXPECT_EQ(filePtr->readSession->sessionCount, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNewSessionTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNewSessionTest005 End";
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
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        if (data->readSessionCache.find(path) != data->readSessionCache.end()) {
            data->readSessionCache.erase(path);
        }

        ErasePathCache(path, data.get());
        EXPECT_EQ(data->readSessionCache.count(path), 0);
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
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        string cloudId = "100";
        string assets = "content";
        data->userId = 100;
        std::shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");
        std::shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        struct SessionCountParams sessionParam = {path, cloudId, assets};
        auto readSession = database->NewAssetReadSession(data->userId, "file", cloudId, assets, path);
        data->readSessionCache[path] = readSession;

        ErasePathCache(path, data.get());
        EXPECT_EQ(data->readSessionCache.count(path), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ErasePathCacheTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "ErasePathCacheTest002 End";
}

/**
 * @tc.name: GenerateCloudId001
 * @tc.desc: Verify the GenerateCloudId function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GenerateCloudId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudId001 Start";
    try {
        string bundleName = system::GetParameter(FILEMANAGER_KEY, "");
        CloudFile::CloudFileKit::GetInstance()->instance_ = nullptr;
        // call func
        string cloudId;
        int32_t ret = GenerateCloudId(100, cloudId, bundleName);
        EXPECT_EQ(ret, ENOSYS);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudId001 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudId001 End";
}

/**
 * @tc.name: GenerateCloudId002
 * @tc.desc: Verify the GenerateCloudId function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GenerateCloudId002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudId002 Start";
    try {
        string bundleName = system::GetParameter(FILEMANAGER_KEY, "");
        CloudFileKitMock *cloudFileKit = new (std::nothrow) CloudFileKitMock();
        CloudFile::CloudFileKit::GetInstance()->instance_ = cloudFileKit;
        g_generateType = GenerateType::FAILED_GEN;

        // call func
        string cloudId;
        int32_t ret = GenerateCloudId(100, cloudId, bundleName);
        EXPECT_EQ(ret, ENOSYS);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudId002 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudId002 End";
}

/**
 * @tc.name: GenerateCloudId003
 * @tc.desc: Verify the GenerateCloudId function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GenerateCloudId003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudId003 Start";
    try {
        string bundleName = system::GetParameter(FILEMANAGER_KEY, "");
        CloudFileKitMock *cloudFileKit = new (std::nothrow) CloudFileKitMock();
        CloudFile::CloudFileKit::GetInstance()->instance_ = cloudFileKit;
        g_generateType = GenerateType::SUCC_GEN_ID_EMPTY;

        // call func
        string cloudId;
        int32_t ret = GenerateCloudId(100, cloudId, bundleName);
        EXPECT_EQ(ret, ENOSYS);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudId003 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudId003 End";
}

/**
 * @tc.name: GenerateCloudId004
 * @tc.desc: Verify the GenerateCloudId function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GenerateCloudId004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudId004 Start";
    try {
        string bundleName = system::GetParameter(FILEMANAGER_KEY, "");
        CloudFileKitMock *cloudFileKit = new (std::nothrow) CloudFileKitMock();
        CloudFile::CloudFileKit::GetInstance()->instance_ = cloudFileKit;
        g_generateType = GenerateType::SUCC_GEN_ID_VALID;

        // call func
        string cloudId;
        int32_t ret = GenerateCloudId(100, cloudId, bundleName);
        EXPECT_EQ(ret, 0);
        EXPECT_EQ(cloudId, "valid_cloudId");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudId004 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudId004 End";
}

/**
 * @tc.name: GenerateCloudId005
 * @tc.desc: Verify the GenerateCloudId function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GenerateCloudId005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudId005 Start";
    try {
        string bundleName = "com.demo.test";
        CloudFile::CloudFileKit::GetInstance()->instance_ = nullptr;

        // call func
        string cloudId;
        int32_t ret = GenerateCloudId(100, cloudId, bundleName);
        EXPECT_EQ(ret, ENOSYS);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudId005 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudId005 End";
}

/**
 * @tc.name: GenerateCloudId006
 * @tc.desc: Verify the GenerateCloudId function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GenerateCloudId006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudId006 Start";
    try {
        string bundleName = "com.demo.test";
        CloudFileKitMock *cloudFileKit = new (std::nothrow) CloudFileKitMock();
        CloudFile::CloudFileKit::GetInstance()->instance_ = cloudFileKit;
        g_generateType = GenerateType::FAILED_GEN;

        // call func
        string cloudId;
        int32_t ret = GenerateCloudId(100, cloudId, bundleName);
        EXPECT_EQ(ret, ENOSYS);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudId006 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudId006 End";
}

/**
 * @tc.name: GenerateCloudId007
 * @tc.desc: Verify the GenerateCloudId function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GenerateCloudId007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudId007 Start";
    try {
        string bundleName = "com.demo.test";
        CloudFileKitMock *cloudFileKit = new (std::nothrow) CloudFileKitMock();
        CloudFile::CloudFileKit::GetInstance()->instance_ = cloudFileKit;
        g_generateType = GenerateType::SUCC_GEN_ID_EMPTY;

        // call func
        string cloudId;
        int32_t ret = GenerateCloudId(100, cloudId, bundleName);
        EXPECT_EQ(ret, ENOSYS);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudId007 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudId007 End";
}

/**
 * @tc.name: GenerateCloudId008
 * @tc.desc: Verify the GenerateCloudId function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudStaticTest, GenerateCloudId008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudId008 Start";
    try {
        string bundleName = "com.demo.test";
        CloudFileKitMock *cloudFileKit = new (std::nothrow) CloudFileKitMock();
        CloudFile::CloudFileKit::GetInstance()->instance_ = cloudFileKit;
        g_generateType = GenerateType::SUCC_GEN_ID_VALID;

        // call func
        string cloudId;
        int32_t ret = GenerateCloudId(100, cloudId, bundleName);
        EXPECT_EQ(ret, 0);
        EXPECT_EQ(cloudId, "valid_cloudId");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudId008 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudId008 End";
}

/**
 * @tc.name: DoSessionInitTest001
 * @tc.desc: Verify the DoSessionInit function
 * @tc.type: FUNC
 */
HWTEST_F(FileOperationsCloudStaticTest, DoSessionInitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoSessionInitTest001 Start";
    try {
        MetaBase metabase;
        shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        CloudOpenParams cloudOpenParams = {metabase, metaFile, filePtr};

        auto error = make_shared<CloudFile::CloudError>(CloudFile::CloudError::CK_NO_ERROR);
        auto openFinish = make_shared<bool>(false);
        auto cond = make_shared<ffrt::condition_variable>();
        SessionInitParams sessionInitParams = {filePtr, error, openFinish};
        string bundlePath = "";

        EXPECT_CALL(*insMock, fuse_lowlevel_notify_inval_entry(_, _, _, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock, fuse_lowlevel_notify_inval_inode(_, _, _, _)).WillRepeatedly(Return(0));
        DoSessionInit(sessionInitParams, cond, cloudOpenParams, bundlePath);

        ffrt::wait();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoSessionInitTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoSessionInitTest001 End";
}

/**
 * @tc.name: DoSessionInitTest002
 * @tc.desc: Verify the DoSessionInit function
 * @tc.type: FUNC
 */
HWTEST_F(FileOperationsCloudStaticTest, DoSessionInitTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoSessionInitTest002 Start";
    try {
        string path = "/data";
        string cloudId = "100";
        string assets = "content";
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        data->userId = 100;
        shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");

        MetaBase metabase;
        shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        auto sessionMock = make_shared<CloudAssetReadSessionMock>(100, "test", "test", "test", "test");
        filePtr->readSession = sessionMock;
        CloudOpenParams cloudOpenParams = {metabase, metaFile, filePtr};

        auto error = make_shared<CloudFile::CloudError>(CloudFile::CloudError::CK_NO_ERROR);
        auto openFinish = make_shared<bool>(false);
        auto cond = make_shared<ffrt::condition_variable>();
        SessionInitParams sessionInitParams = {filePtr, error, openFinish};
        string bundlePath = "";

        EXPECT_CALL(*sessionMock, InitSession()).WillOnce(Return(CloudFile::CloudError::CK_NO_ERROR));
        EXPECT_CALL(*sessionMock, ChownUidForSyncDisk(_, _)).WillOnce(Return(true));

        EXPECT_CALL(*insMock, fuse_lowlevel_notify_inval_entry(_, _, _, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock, fuse_lowlevel_notify_inval_inode(_, _, _, _)).WillRepeatedly(Return(0));
        DoSessionInit(sessionInitParams, cond, cloudOpenParams, bundlePath);

        ffrt::wait();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoSessionInitTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoSessionInitTest002 End";
}

/**
 * @tc.name: DoSessionInitTest003
 * @tc.desc: Verify the DoSessionInit function
 * @tc.type: FUNC
 */
HWTEST_F(FileOperationsCloudStaticTest, DoSessionInitTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoSessionInitTest003 Start";
    try {
        string path = "/data";
        string cloudId = "100";
        string assets = "content";
        shared_ptr<CloudDiskFuseData> data = make_shared<CloudDiskFuseData>();
        data->userId = 100;
        shared_ptr<CloudDatabase> database = make_shared<CloudDatabase>(100, "test");

        MetaBase metabase;
        metabase.fileType = FILE_TYPE_THUMBNAIL;
        shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        auto sessionMock = make_shared<CloudAssetReadSessionMock>(100, "test", "test", "test", "test");
        filePtr->readSession = sessionMock;
        CloudOpenParams cloudOpenParams = {metabase, metaFile, filePtr};

        auto error = make_shared<CloudFile::CloudError>(CloudFile::CloudError::CK_NO_ERROR);
        auto openFinish = make_shared<bool>(false);
        auto cond = make_shared<ffrt::condition_variable>();
        SessionInitParams sessionInitParams = {filePtr, error, openFinish};
        string bundlePath = "";

        EXPECT_CALL(*sessionMock, InitSession()).WillOnce(Return(CloudFile::CloudError::CK_NETWORK_ERROR));
        EXPECT_CALL(*sessionMock, ChownUidForSyncDisk(_, _)).WillOnce(Return(false));

        EXPECT_CALL(*insMock, fuse_lowlevel_notify_inval_entry(_, _, _, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock, fuse_lowlevel_notify_inval_inode(_, _, _, _)).WillRepeatedly(Return(0));
        DoSessionInit(sessionInitParams, cond, cloudOpenParams, bundlePath);

        ffrt::wait();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoSessionInitTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "DoSessionInitTest003 End";
}

/**
 * @tc.name: DoCloudOpenTest001
 * @tc.desc: Verify the DoCloudOpen function
 * @tc.type: FUNC
 */
HWTEST_F(FileOperationsCloudStaticTest, DoCloudOpenTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCloudOpenTest001 Start";
    try {
        fuse_req_t req = nullptr;
        fuse_file_info *fi = new fuse_file_info;
        CloudDiskFuseData *data = new CloudDiskFuseData;
        shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();

        MetaBase metabase;
        shared_ptr<CloudDiskMetaFile> metaFile = make_shared<CloudDiskMetaFile>(0, "com.example.test", "123");
        shared_ptr<CloudDiskFile> filePtr = make_shared<CloudDiskFile>();
        CloudOpenParams cloudOpenParams = {metabase, metaFile, filePtr};

        EXPECT_CALL(*insMock, fuse_lowlevel_notify_inval_entry(_, _, _, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock, fuse_lowlevel_notify_inval_inode(_, _, _, _)).WillRepeatedly(Return(0));
        DoCloudOpen(req, fi, inoPtr, data, cloudOpenParams);

        ffrt::wait();

        delete fi;
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCloudOpenTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCloudOpenTest001 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test