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
#include "network/softbus/softbus_asset_send_listener.h"

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "utils_log.h"

#include "asset_obj.h"
#include "network/softbus/softbus_handler_asset.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;
const string testPath = "/data/test";

class SoftBusAssetSendListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SoftBusAssetSendListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SoftBusAssetSendListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SoftBusAssetSendListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SoftBusAssetSendListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SoftBusAssetSendListener_OnFile_0100
 * @tc.desc: test OnFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusAssetSendListenerTest, SoftBusAssetSendListener_OnFile_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusAssetSendListener_OnFile_0100 start";
    try {
        FileEvent event = {.type = FILE_EVENT_SEND_FINISH,
                           .fileCnt = 0,
                           .bytesProcessed = 0,
                           .bytesTotal = 0};
        int32_t socket = 1;
        SoftBusAssetSendListener::OnFile(socket, nullptr);
        SoftBusAssetSendListener::OnFile(socket, &event);

        event.fileCnt = 1;
        SoftBusAssetSendListener::OnFile(socket, &event);

        sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
        const char *filePath = "test";
        event.files = &filePath;
        SoftBusHandlerAsset::GetInstance().AddAssetObj(socket, assetObj);
        SoftBusAssetSendListener::OnFile(socket, &event);
        EXPECT_EQ(SoftBusHandlerAsset::GetInstance().GetAssetObj(socket), nullptr);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftBusAssetSendListener_OnFile_0100 end";
}

/**
 * @tc.name: SoftBusAssetSendListener_OnFile_0200
 * @tc.desc: test OnFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusAssetSendListenerTest, SoftBusAssetSendListener_OnFile_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusAssetSendListener_OnFile_0200 start";
    try {
        FileEvent event = {.type = FILE_EVENT_SEND_ERROR,
                           .fileCnt = 0,
                           .bytesProcessed = 0,
                           .bytesTotal = 0};
        int32_t socket = 1;
        SoftBusAssetSendListener::OnFile(socket, nullptr);
        SoftBusAssetSendListener::OnFile(socket, &event);

        event.fileCnt = 1;
        SoftBusAssetSendListener::OnFile(socket, &event);

        sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
        SoftBusHandlerAsset::GetInstance().AddAssetObj(socket, assetObj);
        const char *filePath = "test";
        event.files = &filePath;
        SoftBusAssetSendListener::OnFile(socket, &event);
        EXPECT_EQ(SoftBusHandlerAsset::GetInstance().GetAssetObj(socket), nullptr);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftBusAssetSendListener_OnFile_0200 end";
}

/**
 * @tc.name: SoftBusAssetSendListener_OnFile_0300
 * @tc.desc: test OnFile function.
 * @tc.type: FUNC
 * @tc.require: I9JXPR
 */
HWTEST_F(SoftBusAssetSendListenerTest, SoftBusAssetSendListener_OnFile_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftBusAssetSendListener_OnFile_0300 start";
    try {
        FileEvent event = {.type = FILE_EVENT_RECV_START,
                           .fileCnt = 0,
                           .bytesProcessed = 0,
                           .bytesTotal = 0};
        int32_t socket = 1;
        sptr<AssetObj> assetObj(new (std::nothrow) AssetObj());
        SoftBusHandlerAsset::GetInstance().AddAssetObj(socket, assetObj);
        SoftBusAssetSendListener::OnFile(socket, &event);
        EXPECT_NE(SoftBusHandlerAsset::GetInstance().GetAssetObj(socket), nullptr);
        SoftBusHandlerAsset::GetInstance().RemoveAssetObj(socket);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SoftBusAssetSendListener_OnFile_0300 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
