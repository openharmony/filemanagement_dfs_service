/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "asset/asset_adapter_sa_client.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class TestAssetAdapterSaClient : public IRemoteStub<IAssetRecvCallback> {
    int32_t OnStart(const std::string &srcNetworkId,
                    const std::string &dstNetworkId,
                    const std::string &sessionId,
                    const std::string &dstBundleName) override
    {
        return 0;
    }

    int32_t OnRecvProgress(const std::string &srcNetworkId,
                           const sptr<AssetObj> &assetObj,
                           uint64_t totalBytes,
                           uint64_t processBytes) override
    {
        return 0;
    }

    int32_t OnFinished(const std::string &srcNetworkId,
                               const sptr<AssetObj> &assetObj,
                               int32_t result) override
    {
        return 0;
    }
};

class AssetAdapterSaClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AssetAdapterSaClientTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void AssetAdapterSaClientTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AssetAdapterSaClientTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AssetAdapterSaClientTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AssetAdapterSaClient_RemoveListener_0100
 * @tc.desc: The execution of the RemoveListener failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(AssetAdapterSaClientTest, AssetAdapterSaClient_RemoveListener_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AssetAdapterSaClient_RemoveListener_0100 Start";

    auto &client = AssetAdapterSaClient::GetInstance();
    client.listeners_.emplace_back(nullptr);
    const sptr<TestAssetAdapterSaClient> listener = sptr(new TestAssetAdapterSaClient());
    client.listeners_.emplace_back(listener);
    auto ret = client.RemoveListener(listener);
    EXPECT_EQ(ret, E_SA_LOAD_FAILED);
    GTEST_LOG_(INFO) << "AssetAdapterSaClient_RemoveListener_0100 End";
}
}