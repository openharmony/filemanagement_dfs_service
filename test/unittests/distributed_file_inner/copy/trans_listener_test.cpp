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
#include <copy/file_copy_manager.h>
#include <copy/trans_listener.h>
#include <gtest/gtest.h>
#include "dfs_error.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class TransListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    uint64_t process = 0;
    uint64_t fileSize = 0;
    using callBack = std::function<void(uint64_t processSize, uint64_t totalFileSize)>;
    callBack listener = [&](uint64_t processSize, uint64_t totalFileSize) {
        process = processSize;
        fileSize = totalFileSize;
    };
};

void TransListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void TransListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TransListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TransListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
* @tc.name: TransListener_0001
* @tc.desc: The execution of the CopyTosandbox success.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0001 Start";
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);
    int32_t ret = ptr->CopyToSandBox(srcuri);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "TransListener_0001 End";
}

/**
* @tc.name: TransListener_0002
* @tc.desc: The execution of the Cancel success.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0002 Start";
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);
    int32_t ret = ptr->Cancel();
    EXPECT_EQ(ret, E_SA_LOAD_FAILED);
    GTEST_LOG_(INFO) << "TransListener_0002 End";
}

/**
* @tc.name: TransListener_0003
* @tc.desc: The execution of the OnFileReceive success.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0003 Start";
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);
    int32_t ret = ptr->OnFileReceive(process, fileSize);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "TransListener_0003 End";
}

/**
* @tc.name: TransListener_0004
* @tc.desc: The execution of the OnFinished success.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0004 Start";
    string sessionName = "test";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);
    int32_t ret = ptr->OnFinished(sessionName);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "TransListener_0004 End";
}

/**
* @tc.name: TransListener_0005
* @tc.desc: The execution of the OnFailed success.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(TransListenerTest, TransListener_0005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListener_0005 Start";
    string sessionName = "test";
    int32_t errorCode = 0;
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    auto ptr = std::make_shared<TransListener>(desturi, listener);
    int32_t ret = ptr->OnFailed(sessionName, errorCode);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "TransListener_0005 End";
}

}