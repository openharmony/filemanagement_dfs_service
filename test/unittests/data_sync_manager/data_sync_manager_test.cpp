/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "data_sync_manager.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;

class DataSyncManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DataSyncManager> manager_ = nullptr;
};

void DataSyncManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
    manager_ = make_shared<DataSyncManager>();
}

void DataSyncManagerTest::TearDownTestCase(void)
{
    manager_ = nullptr;
    std::cout << "TearDownTestCase" << std::endl;
}

void DataSyncManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void DataSyncManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: StopDownloadAndUploadTaskTest001
 * @tc.desc: Verify the StopDownloadAndUploadTask function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, StopDownloadAndUploadTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadAndUploadTaskTest001 start";
    try {
        manager_->StopDownloadAndUploadTask();
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "StopDownloadAndUploadTaskTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopDownloadAndUploadTaskTest001 end";
}

/**
 * @tc.name: IsFinishPullTest001
 * @tc.desc: Verify the ChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, IsFinishPullTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFinishPullTest001 start";
    try {
        bool finishFlag;

        int32_t ret = manager_->IsFinishPull(100, "", finishFlag);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "IsFinishPullTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsFinishPullTest001 end";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS