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

#include <memory>
#include <unistd.h>
#include <utility>

#include "gtest/gtest.h"
#include "network/softbus/softbus_handler.h"
#include "dfs_error.h"
#include "network/softbus/softbus_file_receive_listener.h"
#include "network/softbus/softbus_file_send_listener.h"
#include "network/softbus/softbus_session_listener.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;
class SoftbusHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: SoftbusHandlerTest_CreateSessionServer_0100
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: AR2024022300780
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_CreateSessionServer_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0100 start";
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    DFS_CHANNEL_ROLE role = DFS_CHANNLE_ROLE_SOURCE;
    std::string physicalPath = "/data/test";
    handler.serverIdMap_.insert(std::make_pair(sessionName, 2))
    int32_t result = handler.CreateSessionServer(packageName, sessionName, role, physicalPath);

    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_CreateSessionServer_0200
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: AR2024022300780
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_CreateSessionServer_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0100 start";
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    DFS_CHANNEL_ROLE role = DFS_CHANNLE_ROLE_SOURCE;
    std::string physicalPath = "/data/test";

    int32_t result = handler.CreateSessionServer(packageName, sessionName, role, physicalPath);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_CreateSessionServer_0100 end";
}


/**
 * @tc.name: SoftbusHandlerTest_OpenSession_0100
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: AR2024022300780
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_OpenSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0100 start";
    SoftBusHandler handler;
    std::string packageName = "com.example.test";
    std::string sessionName = "testSession";
    DFS_CHANNEL_ROLE role = DFS_CHANNLE_ROLE_SOURCE;
    std::string physicalPath = "/data/test";

    int32_t result = handler.OpenSession(packageName, sessionName, role, physicalPath);
    EXPECT_EQ(result, E_OPEN_SESSION);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_OpenSession_0100 end";
}

/**
 * @tc.name: SoftbusHandlerTest_ChangeOwnerIfNeeded_0100
 * @tc.desc: Verify the ChangeOwnerIfNeeded by Cid function.
 * @tc.type: FUNC
 * @tc.require: AR2024022300780
 */
HWTEST_F(SoftbusHandlerTest, SoftbusHandlerTest_ChangeOwnerIfNeeded_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_ChangeOwnerIfNeeded_0100 start";
    SoftBusHandler handler;
    handler.ChangeOwnerIfNeeded(1, "sessionName");
    int32_t result = handler.OpenSession("packname", "sessionName", DFS_CHANNLE_ROLE_SOURCE, "/data/test");
    EXPECT_EQ(result, E_OPEN_SESSION);
    GTEST_LOG_(INFO) << "SoftbusHandlerTest_ChangeOwnerIfNeeded_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
