/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <sys/stat.h>
#include <unistd.h>

#include "async_work.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class AsyncWorkTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AsyncWorkTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void AsyncWorkTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AsyncWorkTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AsyncWorkTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetPromiseOrCallBackWorkTest001
 * @tc.desc: Verify the ForceCreateDirectory function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(AsyncWorkTest, GetPromiseOrCallBackWorkTest001, TestSize.Level1)
{
    napi_env env = nullptr;
    napi_callback_info info = nullptr;
    LibN::NFuncArg funcArg(env, info);

    std::unique_ptr<LibN::NAsyncWork> asyncWork = LibN::GetPromiseOrCallBackWork(env, funcArg, 1);
    EXPECT_NE(asyncWork, nullptr);
}

HWTEST_F(AsyncWorkTest, GetPromiseOrCallBackWorkTest002, TestSize.Level1)
{
    napi_env env = nullptr;
    napi_callback_info info = nullptr;
    LibN::NFuncArg funcArg(env, info);

    std::unique_ptr<LibN::NAsyncWork> asyncWork = LibN::GetPromiseOrCallBackWork(env, funcArg, 0);
    EXPECT_EQ(asyncWork, nullptr);
}

}