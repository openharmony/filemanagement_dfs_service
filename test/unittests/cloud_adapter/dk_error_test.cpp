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

#include "dk_error.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace DriveKit;

class DKErrorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DKErrorTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DKErrorTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DKErrorTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DKErrorTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: HasError
 * @tc.desc: Verify the DKError::HasError function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKErrorTest, HasError, TestSize.Level1)
{
    DKError dkError;
    bool ret = dkError.HasError();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SetLocalError
 * @tc.desc: Verify the DKError::SetLocalError function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKErrorTest, SetLocalError, TestSize.Level1)
{
    DKLocalErrorCode code = DKLocalErrorCode::NO_ERROR;
    DKError dkError;
    dkError.SetLocalError(code);
    EXPECT_EQ(dkError.dkErrorCode, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: SetServerError
 * @tc.desc: Verify the DKError::SetServerError function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKErrorTest, SetServerError, TestSize.Level1)
{
    int code = 0;
    DKError dkError;
    dkError.SetServerError(code);
    EXPECT_EQ(dkError.serverErrorCode, code);
}
} // namespace OHOS::FileManagement::CloudSync::Test
