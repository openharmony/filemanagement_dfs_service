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
    paramDisk.opsType = NotifyOpsType::DAEMON_MKDIR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
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
    paramDisk.opsType = NotifyOpsType::DAEMON_RMDIR;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
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
    paramDisk.opsType = NotifyOpsType::DAEMON_UNLINK;
    ParamDiskOthers paramOthers;
    CloudDiskNotify.TryNotify(paramDisk, paramOthers);
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
    paramDisk.opsType = NotifyOpsType::OPS_NONE;
    ParamDiskOthers paramOthers;
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
    paramService.opsType = NotifyOpsType::SERVICE_UPDATE;
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
    paramService.opsType = NotifyOpsType::SERVICE_DELETE;
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
    paramService.opsType = NotifyOpsType::SERVICE_DELETE_BATCH;
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
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS