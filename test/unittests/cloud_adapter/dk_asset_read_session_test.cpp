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

#include "dk_asset_read_session.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DkAssetReadSessionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DkAssetReadSessionTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DkAssetReadSessionTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DkAssetReadSessionTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DkAssetReadSessionTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Read
 * @tc.desc: Verify the DKAssetReadSession::Read function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DkAssetReadSessionTest, Read, TestSize.Level1)
{
    DriveKit::DKAssetReadSession mDkSession;
    int64_t size = 0;
    char *buffer = nullptr;
    DriveKit::DKError error;
    int64_t ret = mDkSession.Read(size, buffer, error);
    EXPECT_EQ(ret, int64_t());
}

/**
 * @tc.name: PRead
 * @tc.desc: Verify the DKAssetReadSession::PRead function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DkAssetReadSessionTest, PRead, TestSize.Level1)
{
    DriveKit::DKAssetReadSession mDkSession;
    int64_t size = 0, offset = 0;
    char *buffer = nullptr;
    DriveKit::DKError error;
    int64_t ret = mDkSession.PRead(offset, size, buffer, error);
    EXPECT_EQ(ret, int64_t());
}
} // namespace OHOS::FileManagement::CloudSync::Test
