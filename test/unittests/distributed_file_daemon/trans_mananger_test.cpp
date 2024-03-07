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

#include "trans_mananger.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class TransManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<TransManager> transManager_;
};

void TransManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void TransManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TransManagerTest::SetUp(void)
{
    transManager_ = std::make_shared<TransManager>();
    GTEST_LOG_(INFO) << "SetUp";
}

void TransManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(TransManagerTest, DfsService_AddTransTask_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_AddTransTask_001_Start";
    string sessionName;
    sptr<IFileTransListener> listener;
    transManager_->AddTransTask(sessionName, listener);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_AddTransTask_001_End";
}

HWTEST_F(TransManagerTest, DfsService_AddTransTask_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_AddTransTask_002_Start";
    string sessionName;
    sptr<IFileTransListener> listener;
    TransManager::GetInstance().AddTransTask(sessionName, listener);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_AddTransTask_002_End";
}

HWTEST_F(TransManagerTest, DfsService_DeleteTransTask_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_DeleteTransTask_001_Start";
    string sessionName;
    transManager_->DeleteTransTask(sessionName);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_DeleteTransTask_001_End";
}

HWTEST_F(TransManagerTest, DfsService_DeleteTransTask_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_DeleteTransTask_002_Start";
    string sessionName;
    TransManager::GetInstance().DeleteTransTask(sessionName);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_DeleteTransTask_002_End";
}

HWTEST_F(TransManagerTest, DfsService_GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetInstance_001_Start";
    TransManager::GetInstance();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_GetInstance_001_End";
}

HWTEST_F(TransManagerTest, DfsService_NotifyFileFailed_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_NotifyFileFailed_001_Start";
    string sessionName;
    transManager_->NotifyFileFailed(sessionName);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_NotifyFileFailed_001_End";
}

HWTEST_F(TransManagerTest, DfsService_NotifyFileFailed_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_NotifyFileFailed_002_Start";
    string sessionName;
    TransManager::GetInstance().NotifyFileFailed(sessionName);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_NotifyFileFailed_002_End";
}

HWTEST_F(TransManagerTest, DfsService_NotifyFileFinished_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_NotifyFileFinished_001_Start";
    string sessionName;
    transManager_->NotifyFileFinished(sessionName);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_NotifyFileFinished_001_End";
}

HWTEST_F(TransManagerTest, DfsService_NotifyFileFinished_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_NotifyFileFinished_002_Start";
    string sessionName;
    TransManager::GetInstance().NotifyFileFinished(sessionName);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_NotifyFileFinished_002_End";
}

HWTEST_F(TransManagerTest, DfsService_NotifyFileProgress_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_NotifyFileProgress_001_Start";
    string sessionName;
    uint64_t total = 0;
    uint64_t processed = 0;;
    transManager_->NotifyFileProgress(sessionName, total, processed);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_NotifyFileProgress_001_End";
}

HWTEST_F(TransManagerTest, DfsService_NotifyFileProgress_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_NotifyFileProgress_002_Start";
    string sessionName;
    uint64_t total = 0;
    uint64_t processed = 0;;
    TransManager::GetInstance().NotifyFileProgress(sessionName, total, processed);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "DfsService_NotifyFileProgress_002_End";
}
}