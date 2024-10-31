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

#include "clouddisk_notify.h"
#include "clouddisk_notify_utils.h"
#include "clouddisk_rdbstore.h"
#include "dfs_error.h"
#include "ffrt_inner.h"
#include "file_column.h"
#include "securec.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDisk {
namespace Test {
using namespace testing::ext;
using namespace std;

class CloudDiskNotifyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskNotifyTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDiskNotifyTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDiskNotifyTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudDiskNotifyTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: TryNotifyTest001
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.inoPtr = nullptr;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_SETATTR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest002
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 10;
    paramDisk.opsType = NotifyOpsType::DAEMON_SETATTR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest003
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.inoPtr = nullptr;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_SETXATTR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest004
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 10;
    paramDisk.opsType = NotifyOpsType::DAEMON_SETXATTR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest005
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.inoPtr = nullptr;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 10;
    paramDisk.opsType = NotifyOpsType::DAEMON_RECYCLE;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest006
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    ino->bundleName = "";
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 0;
    paramDisk.opsType = NotifyOpsType::DAEMON_RECYCLE;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest007
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    ino->bundleName = "com.ohos.photos";
    ino->cloudId = "mock";
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_RECYCLE;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest008
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    ino->bundleName = "com.ohos.photos";
    ino->cloudId = "rootId";
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 10;
    paramDisk.opsType = NotifyOpsType::DAEMON_RECYCLE;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest009
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    ino->bundleName = "com.ohos.photos";
    ino->cloudId = "rootId";
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_RECYCLE;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest010
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    ino->bundleName = "com.ohos.photos";
    ino->cloudId = "rootId";
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_RESTORE;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest011
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.name = "";
    paramDisk.opsType = NotifyOpsType::DAEMON_MKDIR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest012
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.name = "test";
    paramDisk.inoPtr = nullptr;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 10;
    paramDisk.opsType = NotifyOpsType::DAEMON_MKDIR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest013
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.name = "test";
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_MKDIR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest014
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.name = "";
    paramDisk.opsType = NotifyOpsType::DAEMON_RMDIR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest015
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.name = "test";
    paramDisk.inoPtr = nullptr;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 10;
    paramDisk.opsType = NotifyOpsType::DAEMON_RMDIR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest016
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.name = "test";
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_RMDIR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest017
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.name = "test";
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_UNLINK;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest018
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 10;
    paramDisk.opsType = NotifyOpsType::DAEMON_RENAME;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest019
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.newName = "mock";
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_RENAME;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest020
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    paramDisk.newName = "test";
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_RENAME;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest021
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest021, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 10;
    paramDisk.opsType = NotifyOpsType::DAEMON_WRITE;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest022
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest022, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_WRITE;
    ParamDiskOthers paramOthers;
    paramOthers.dirtyType = static_cast<int32_t>(DirtyType::TYPE_NO_NEED_UPLOAD);
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest023
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest023, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    shared_ptr<CloudDiskInode> ino = make_shared<CloudDiskInode>();
    paramDisk.inoPtr = ino;
    paramDisk.data = new CloudDiskFuseData();
    paramDisk.data->userId = 100;
    paramDisk.opsType = NotifyOpsType::DAEMON_WRITE;
    ParamDiskOthers paramOthers;
    paramOthers.dirtyType = static_cast<int32_t>(DirtyType::TYPE_NEW);
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    delete paramDisk.data;
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyTest024
 * @tc.desc: Verify the TryNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyTest024, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamDisk paramDisk;
    ParamDiskOthers paramOthers;
    paramOthers.dirtyType = static_cast<int32_t>(DirtyType::TYPE_NEW);
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
    GTEST_LOG_(INFO) << "TryNotify End";
}

/**
 * @tc.name: TryNotifyServiceTest001
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.node.isRecycled = true;
    paramService.cloudId = "mock";
    paramService.opsType = NotifyOpsType::SERVICE_INSERT;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest002
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.node.isRecycled = true;
    paramService.cloudId = "rootId";
    paramService.opsType = NotifyOpsType::SERVICE_INSERT;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest003
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.node.isRecycled = false;
    paramService.node.cloudId = "mock";
    paramService.opsType = NotifyOpsType::SERVICE_INSERT;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest004
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "mock";
    paramService.opsType = NotifyOpsType::SERVICE_UPDATE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest005
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "rootId";
    paramService.node.fileName = "mock";
    paramService.opsType = NotifyOpsType::SERVICE_UPDATE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest006
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "rootId";
    paramService.opsType = NotifyOpsType::SERVICE_UPDATE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest007
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "mock";
    paramService.opsType = NotifyOpsType::SERVICE_UPDATE_RECYCLE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest008
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "rootId";
    paramService.node.cloudId = "mock";
    paramService.opsType = NotifyOpsType::SERVICE_UPDATE_RECYCLE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest009
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "rootId";
    paramService.node.cloudId = "rootId";
    paramService.node.isRecycled = false;
    paramService.opsType = NotifyOpsType::SERVICE_UPDATE_RECYCLE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest010
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "rootId";
    paramService.node.cloudId = "rootId";
    paramService.node.isRecycled = true;
    paramService.opsType = NotifyOpsType::SERVICE_UPDATE_RECYCLE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest011
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "";
    paramService.opsType = NotifyOpsType::SERVICE_DELETE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest012
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "rootId";
    paramService.opsType = NotifyOpsType::SERVICE_DELETE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest013
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.cloudId = "rootId";
    paramService.opsType = NotifyOpsType::SERVICE_DELETE_BATCH;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: TryNotifyServiceTest014
 * @tc.desc: Verify the TryNotifyService function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, TryNotifyServiceTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TryNotifyService Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyParamService paramService;
    paramService.opsType = NotifyOpsType::OPS_NONE;
    ParamServiceOther paramOthers;
    CloudDiskNotify.TryNotifyService(paramService, paramOthers);
    GTEST_LOG_(INFO) << "TryNotifyService End";
}

/**
 * @tc.name: GetDeleteNotifyDataTest001
 * @tc.desc: Verify the GetDeleteNotifyData function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, GetDeleteNotifyDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeleteNotifyData Start";
    CloudDiskNotify CloudDiskNotify;
    vector<NativeRdb::ValueObject> deleteIds;
    NativeRdb::ValueObject mock0;
    NativeRdb::ValueObject mock1;
    deleteIds.push_back(mock0);
    deleteIds.push_back(mock1);
    vector<NotifyData> notifyDataList;
    ParamServiceOther paramOthers;
    paramOthers.bundleName = "com.ohos.photos";
    paramOthers.userId = 100;
    int32_t ret = CloudDiskNotify.GetDeleteNotifyData(deleteIds, notifyDataList, paramOthers);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetDeleteNotifyData End";
}

/**
 * @tc.name: AddNotifyTest001
 * @tc.desc: Verify the AddNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, AddNotifyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyData notifyData;
    notifyData.type == NotifyType::NOTIFY_NONE;
    CloudDiskNotify.AddNotify(notifyData);
    GTEST_LOG_(INFO) << "AddNotify End";
}

/**
 * @tc.name: AddNotifyTest002
 * @tc.desc: Verify the AddNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, AddNotifyTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddNotify Start";
    CloudDiskNotify CloudDiskNotify;
    NotifyData notifyData;
    notifyData.type == NotifyType::NOTIFY_ADDED;
    CloudDiskNotify.AddNotify(notifyData);
    GTEST_LOG_(INFO) << "AddNotify End";
}

/**
 * @tc.name: NotifyChangeOuterTest001
 * @tc.desc: Verify the AddNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, NotifyChangeOuterTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyChangeOuter Start";
    CloudDiskNotify CloudDiskNotify;
    CloudDiskNotify.NotifyChangeOuter();
    GTEST_LOG_(INFO) << "NotifyChangeOuter End";
}

/**
 * @tc.name: NotifyChangeOuterTest002
 * @tc.desc: Verify the AddNotify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskNotifyTest, NotifyChangeOuterTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyChangeOuter Start";
    CacheNotifyInfo nf;
    CloudDiskNotify CloudDiskNotify;
    CloudDiskNotify.nfList_.push_back(nf);
    CloudDiskNotify.NotifyChangeOuter();
    GTEST_LOG_(INFO) << "NotifyChangeOuter End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS