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

#include "dk_assets_downloader.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace DriveKit;

class DkAssetDownloaderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DkAssetDownloaderTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DkAssetDownloaderTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DkAssetDownloaderTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DkAssetDownloaderTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DownLoadAssets001
 * @tc.desc: Verify the DKAssetsDownloader::DownLoadAssets function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DkAssetDownloaderTest, DownLoadAssets001, TestSize.Level1)
{
    std::shared_ptr<DKDatabase> database = nullptr;
    std::shared_ptr<DKContext> context = nullptr;
    std::vector<DKDownloadAsset> assetsToDownload;
    DKAssetPath downLoadPath = "";
    DKDownloadId id = 0;

    std::function<void(std::shared_ptr<DKContext>,
                        std::shared_ptr<const DKDatabase>,
                        const std::map<DKDownloadAsset, DKDownloadResult> &,
                        const DKError &)> resultCallback = nullptr;
    std::function<void(std::shared_ptr<DKContext>,
                        DKDownloadAsset,
                        TotalSize,
                        DownloadSize)> progressCallback = nullptr;

    DKAssetsDownloader mDkDownloader(database);
    DKLocalErrorCode ret = mDkDownloader.DownLoadAssets(context, assetsToDownload,
                                                            downLoadPath, id, resultCallback, progressCallback);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: DownLoadAssets002
 * @tc.desc: Verify the DKAssetsDownloader::DownLoadAssets function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DkAssetDownloaderTest, DownLoadAssets002, TestSize.Level1)
{
    std::shared_ptr<DKDatabase> database = nullptr;
    DKDownloadAsset assetsToDownload;
    DKAssetsDownloader mDkDownloader(database);
    DKLocalErrorCode ret = mDkDownloader.DownLoadAssets(assetsToDownload);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: CancelDownloadAssets
 * @tc.desc: Verify the DKAssetsDownloader::CancelDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DkAssetDownloaderTest, CancelDownloadAssets, TestSize.Level1)
{
    std::shared_ptr<DKDatabase> database = nullptr;
    DKDownloadId id = 0;
    DKAssetsDownloader mDkDownloader(database);
    DKLocalErrorCode ret = mDkDownloader.CancelDownloadAssets(id);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}
} // namespace OHOS::FileManagement::CloudSync::Test
