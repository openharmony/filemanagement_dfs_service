/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cloud_file_cache_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager.h"
#include "cloud_sync_manager_mock.h"
#include "dfs_error.h"
#include "download_callback_impl_ani.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;

class CloudFileCacheCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudFileCacheCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudFileCacheCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudFileCacheCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudFileCacheCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Constructor
 * @tc.desc: Verify the CloudFileCacheCore::Constructor function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, ConstructorTest1, TestSize.Level1)
{
    auto data = CloudFileCacheCore::Constructor();
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: DoOn
 * @tc.desc: Verify the CloudFileCacheCore::DoOn function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, DoOnTest1, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    auto callback = std::make_shared<CloudFileCacheCallbackImplAni>();
    std::string event = "progress";
    auto ret = cloudFileCache->DoOn(event, callback);
    EXPECT_TRUE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, 0);
}

/**
 * @tc.name: DoOff
 * @tc.desc: Verify the CloudFileCacheCore::DoOff function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, DoOffTest1, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    std::string event = "progress";
    auto ret = cloudFileCache->DoOff(event);
    EXPECT_TRUE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, 0);
}

/**
 * @tc.name: DoStart
 * @tc.desc: Verify the CloudFileCacheCore::DoStart function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, DoStartTest1, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    std::string uri = "testuri";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StartFileCache(_, _, _, _, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudFileCache->DoStart(uri);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStart
 * @tc.desc: Verify the CloudFileCacheCore::DoStart function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, DoStartTest2, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    std::string uri = "testuri";
    int32_t fieldKey = 0;
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StartFileCache(_, _, _, _, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudFileCache->DoStart({uri}, fieldKey);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStop
 * @tc.desc: Verify the CloudFileCacheCore::DoStop function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, DoStopTest1, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    std::string uri = "testuri";
    bool needClean = false;
    auto ret = cloudFileCache->DoStop(uri, needClean);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::FILEIO_SYS_CAP_TAG + E_INVAL_ARG);
}

/**
 * @tc.name: DoStop
 * @tc.desc: Verify the CloudFileCacheCore::DoStop function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, DoStopTest2, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    int64_t downloadId = 0;
    bool needClean = false;
    auto ret = cloudFileCache->DoStop(downloadId, needClean);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::FILEIO_SYS_CAP_TAG + ModuleFileIO::E_INVAL);
}

/**
 * @tc.name: DoStop
 * @tc.desc: Verify the CloudFileCacheCore::DoStop function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, DoStopTest3, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    cloudFileCache->registerMap_.insert(
        std::make_pair("batchDownload", std::make_shared<CloudFileCacheCallbackImplAni>()));
    int64_t downloadId = 0;
    bool needClean = false;
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, StopFileCache(_, _, _)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudFileCache->DoStop(downloadId, needClean);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: DoStop
 * @tc.desc: Verify the CloudFileCacheCore::DoStop function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, DoStopTest4, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    cloudFileCache->registerMap_.insert(
        std::make_pair("batchDownload", std::make_shared<CloudFileCacheCallbackImplAni>()));
    std::string uri = "testuri";
    bool needClean = false;
    auto ret = cloudFileCache->DoStop(uri, needClean);
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    if (ret.IsSuccess()) {
        EXPECT_EQ(errorCode, 0);
    } else {
        EXPECT_EQ(errorCode, OHOS::FileManagement::FILEIO_SYS_CAP_TAG + E_INVAL_ARG);
    }
}

/**
 * @tc.name: CleanCache
 * @tc.desc: Verify the CloudFileCacheCore::CleanCache function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, CleanCacheTest1, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    std::string uri = "testuri";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, CleanCache(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudFileCache->CleanCache(uri);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: CleanFileCache
 * @tc.desc: Verify the CloudFileCacheCore::CleanFileCache function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, CleanFileCacheTest1, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    std::string uri = "testuri";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, CleanFileCache(_)).WillOnce(Return(OHOS::FileManagement::E_PERMISSION));
    auto ret = cloudFileCache->CleanFileCache(uri);
    EXPECT_FALSE(ret.IsSuccess());
    const auto &err = ret.GetError();
    int errorCode = err.GetErrNo();
    EXPECT_EQ(errorCode, OHOS::FileManagement::E_PERMISSION);
}

/**
 * @tc.name: CleanFileCache
 * @tc.desc: Verify the CloudFileCacheCore::CleanFileCache function
 * @tc.type: FUNC
 */
HWTEST_F(CloudFileCacheCoreTest, CleanFileCacheTest2, TestSize.Level1)
{
    CloudFileCacheCore *cloudFileCache = CloudFileCacheCore::Constructor().GetData().value();
    std::string uri = "testuri";
    auto &cloudMock = CloudSyncManagerImplMock::GetInstance();
    EXPECT_CALL(cloudMock, CleanFileCache(_)).WillOnce(Return(E_OK));
    auto ret = cloudFileCache->CleanFileCache(uri);
    EXPECT_TRUE(ret.IsSuccess());
}
} // namespace OHOS::FileManagement::CloudDisk::Test