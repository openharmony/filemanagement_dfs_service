/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "base_interface_lib_mock.h"
#include "directory_ex.h"
#include "meta_file.h"

#define open(s, flag, mode) MyOpen(s, flag, mode)
#include "meta_file.cpp"
#define MyOpen(s, flag, mode) open(s, flag, mode)

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

enum ErrStatus {
    E_FAIL = -1,
    E_SUCCESS = 0,
    E_EINVAL = 22,
    DENTRY_PER_GROUP = 60,
    MAX_BUCKET_LEVEL = 63
};

class DentryMetaFileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    static inline shared_ptr<InterfaceLibMock> interfaceLibMock_ = nullptr;
};

void DentryMetaFileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    interfaceLibMock_ = make_shared<InterfaceLibMock>();
    InterfaceLibMock::baseInterfaceLib_ = interfaceLibMock_;
}

void DentryMetaFileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    InterfaceLibMock::baseInterfaceLib_ = nullptr;
    interfaceLibMock_ = nullptr;
}

void DentryMetaFileTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DentryMetaFileTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetMetaFile001
 * @tc.desc: Verify the GetMetaFile Func
 * @tc.type: FUNC
 */
HWTEST_F(DentryMetaFileTest, GetMetaFile001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMetaFile001 Start";
    try {
        uint32_t userId = 100;

        EXPECT_CALL(*interfaceLibMock_, MyOpen(_, _, _)).WillOnce(SetErrnoAndReturn(EDQUOT, -1));

        auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId, "/o/p/q/r/s/t");
        
        MetaFileMgr::GetInstance().ClearAll();
        EXPECT_EQ(mFile, nullptr);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " GetMetaFile001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetMetaFile001 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test