/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "network/softbus/softbus_session.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int TEST_SESSION_ID = 10;
std::shared_ptr<SoftbusSession> g_session;

class SoftbusSessionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
};

void SoftbusSessionTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    g_session = make_shared<SoftbusSession>(TEST_SESSION_ID);
}

void SoftbusSessionTest::TearDownTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    g_session = nullptr;
}

/**
 * @tc.name: SoftbusSessionTest_IsFromServer_010
 * @tc.desc: Verify the IsFromServer function.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(SoftbusSessionTest, SoftbusSessionTest_IsFromServer_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionTest_IsFromServer_010 start";
    try {
        bool res = g_session->IsFromServer();
        EXPECT_TRUE(res == false);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    GTEST_LOG_(INFO) << "SoftbusSessionTest_IsFromServer_010 end";
}

/**
 * @tc.name: SoftbusSessionTest_GetCid_010
 * @tc.desc: Verify the GetCid function.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(SoftbusSessionTest, SoftbusSessionTest_GetCid_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionTest_GetCid_010 start";
    try {
        string cid = g_session->GetCid();
        EXPECT_TRUE(cid == "");
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    GTEST_LOG_(INFO) << "SoftbusSessionTest_GetCid_010 end";
}

/**
 * @tc.name: SoftbusSessionTest_GetHandle_010
 * @tc.desc: Verify the GetHandle function.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(SoftbusSessionTest, SoftbusSessionTest_GetHandle_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionTest_GetHandle_010 start";
    try {
        int32_t socket = g_session->GetHandle();
        EXPECT_TRUE(socket == INVALID_SOCKET_FD);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    GTEST_LOG_(INFO) << "SoftbusSessionTest_GetHandle_010 end";
}

/**
 * @tc.name: SoftbusSessionTest_Release
 * @tc.desc: Verify the Release function.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(SoftbusSessionTest, SoftbusSessionTest_Release, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionTest_Release start";
    try {
        g_session->Release();
        EXPECT_TRUE(true);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    GTEST_LOG_(INFO) << "SoftbusSessionTest_Release end";
}

/**
 * @tc.name: SoftbusSessionTest_DisableSessionListener
 * @tc.desc: Verify the DisableSessionListener function.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(SoftbusSessionTest, SoftbusSessionTest_DisableSessionListener, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusSessionTest_DisableSessionListener start";
    try {
        g_session->DisableSessionListener();
        EXPECT_TRUE(true);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    GTEST_LOG_(INFO) << "SoftbusSessionTest_DisableSessionListener end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
