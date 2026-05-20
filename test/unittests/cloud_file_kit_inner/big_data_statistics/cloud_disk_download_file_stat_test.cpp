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
 
#include <gmock/gmock.h>
#include <gtest/gtest.h>
 
#include "assistant.h"
#include "cloud_disk_download_file_stat.h"
 
namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
 
#define TYPE_DOWNLOAD_FILE_IMAGE 1
#define TYPE_DOWNLOAD_FILE_VIDEO 2
#define DOWNLOAD_FILE_BYTE_SIZE 1e6
#define DURAITON_MODE 970
#define DOWNLOAD_IMAGE_SIZE_RANGE_TWO 2
#define DOWNLOAD_IMAGE_SIZE_RANGE_FOUR 4
#define DOWNLOAD_IMAGE_SIZE_RANGE_SIX 6
#define DOWNLOAD_IMAGE_SIZE_RANGE_EIGHT 8
#define DOWNLOAD_IMAGE_SIZE_RANGE_FIFTEEN 15
#define DOWNLOAD_VIDEO_SIZE_RANGE_TWENTY 20
#define DOWNLOAD_VIDEO_SIZE_RANGE_FORTY 40
#define DOWNLOAD_VIDEO_SIZE_RANGE_EIGHTY 80
#define DOWNLOAD_VIDEO_SIZE_RANGE_TWOHUNDRED 200
#define DOWNLOAD_VIDEO_SIZE_RANGE_FOURHUNDRED 400
#define DOWNLOAD_VIDEO_SIZE_RANGE_EIGHTHUNDRED 800
 
class MockCloudDiskDownloadFileStat : public CloudDiskDownloadFileStat {
public:
    MOCK_METHOD0(ReadVecFromLocal, CloudDiskDownloadFileStatInfo());
};
 
class CloudDiskDownloadFileStatTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<AssistantMock> insMock_ = nullptr;
};
 
void CloudDiskDownloadFileStatTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}
 
void CloudDiskDownloadFileStatTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}
 
void CloudDiskDownloadFileStatTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    AssistantMock::EnableMock();
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
}
 
void CloudDiskDownloadFileStatTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    AssistantMock::DisableMock();
    Assistant::ins = nullptr;
    insMock_ = nullptr;
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_001 Start";
    try {
        CloudDiskDownloadFileStat::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_001 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_002 Start";
    try {
        CloudDiskDownloadFileStat::GetInstance().OutputToFile();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_002 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_003 Start";
    try {
        CloudDiskDownloadFileStat::GetInstance().Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_003 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_004 Start";
    try {
        CloudDiskDownloadFileStat clouddiskDownloadFileStat;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_004 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_005 Start";
    try {
        CloudDiskDownloadFileStat clouddiskDownloadFileStat;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_005 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_006 Start";
    try {
        CloudDiskDownloadFileStat clouddiskDownloadFileStat;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_TWO;
        uint64_t duration = DURAITON_MODE;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_FOUR;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_SIX;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_EIGHT;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_FIFTEEN;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_006 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_006 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_007 Start";
    try {
        CloudDiskDownloadFileStat clouddiskDownloadFileStat;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_FORTY;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_EIGHTY;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_TWOHUNDRED;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_FOURHUNDRED;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_EIGHTHUNDRED;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_007 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_007 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_008 Start";
    try {
        CloudDiskDownloadFileStat clouddiskDownloadFileStat;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_TWO + TYPE_DOWNLOAD_FILE_IMAGE;
        uint64_t duration = DURAITON_MODE;
        clouddiskDownloadFileStat.OutputToFile();
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_FOUR + TYPE_DOWNLOAD_FILE_IMAGE;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_SIX + TYPE_DOWNLOAD_FILE_IMAGE;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_EIGHT + TYPE_DOWNLOAD_FILE_IMAGE;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_IMAGE_SIZE_RANGE_FIFTEEN + TYPE_DOWNLOAD_FILE_IMAGE;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        clouddiskDownloadFileStat.Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_008 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_008 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_009 Start";
    try {
        CloudDiskDownloadFileStat clouddiskDownloadFileStat;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE + TYPE_DOWNLOAD_FILE_VIDEO;
        uint64_t duration = DURAITON_MODE;
        clouddiskDownloadFileStat.OutputToFile();
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_FORTY + TYPE_DOWNLOAD_FILE_VIDEO;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_EIGHTY + TYPE_DOWNLOAD_FILE_VIDEO;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_TWOHUNDRED + TYPE_DOWNLOAD_FILE_VIDEO;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_FOURHUNDRED + TYPE_DOWNLOAD_FILE_VIDEO;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
 
        size = DOWNLOAD_FILE_BYTE_SIZE * DOWNLOAD_VIDEO_SIZE_RANGE_EIGHTHUNDRED + TYPE_DOWNLOAD_FILE_VIDEO;
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        clouddiskDownloadFileStat.Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_009 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_009 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_010 Start";
    try {
        CloudDiskDownloadFileStat clouddiskDownloadFileStat;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;
        clouddiskDownloadFileStat.OutputToFile();
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        clouddiskDownloadFileStat.Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_010 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_010 End";
}
 
HWTEST_F(CloudDiskDownloadFileStatTest, CloudDiskDownloadFileStatTest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_011 Start";
    try {
        CloudDiskDownloadFileStat clouddiskDownloadFileStat;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;
        clouddiskDownloadFileStat.OutputToFile();
        clouddiskDownloadFileStat.UpdateDownloadStat(size, duration);
        clouddiskDownloadFileStat.Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_011 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskDownloadFileStatTest_011 End";
}
 
 
/**
 * @tc.name: ReportDownloadFileStatTest_001
 * @tc.desc: Verify the ReportDownloadFileStat function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDiskDownloadFileStatTest, ReportDownloadFileStatTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportDownloadFileStatTest_001 Start";
    CloudDiskDownloadFileStat clouddiskDownloadFileStat;
    CloudDiskDownloadFileStatInfo info;
    info.bundleName = "bundlename.test.com";
    try {
        auto ret = clouddiskDownloadFileStat.ReportDownloadFileStat(info);
        EXPECT_EQ(ret, 0);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReportDownloadFileStatTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "ReportDownloadFileStatTest_001 End";
}
 
/**
 * @tc.name: UpdateDownloadBundleNameTest_001
 * @tc.desc: Verify the UpdateDownloadBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDiskDownloadFileStatTest, UpdateDownloadBundleNameTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateDownloadBundleNameTest_001 Start";
    CloudDiskDownloadFileStat clouddiskDownloadFileStat;
    string bundleName = "test.bundle.name";
    try {
        clouddiskDownloadFileStat.UpdateDownloadBundleName(bundleName);
        EXPECT_EQ(clouddiskDownloadFileStat.stat_.bundleName, bundleName);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateDownloadBundleNameTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateDownloadBundleNameTest_001 End";
}
 
/**
 * @tc.name: HandleBundleNameTest_001
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDiskDownloadFileStatTest, HandleBundleNameTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_001 Start";
    CloudDiskDownloadFileStat downloadStat;
    downloadStat.stat_.bundleName = "test.bundle.name";
    CloudDiskDownloadFileStatInfo info;
    info.bundleName = "test.bundle.name";
    try {
        downloadStat.HandleBundleName(info);
        EXPECT_EQ(downloadStat.stat_.bundleName, "test.bundle.name");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_001 End";
}
 
/**
 * @tc.name: HandleBundleNameTest_002
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDiskDownloadFileStatTest, HandleBundleNameTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_002 Start";
    CloudDiskDownloadFileStat downloadStat;
    downloadStat.stat_.bundleName = "test.bundle.name";
    CloudDiskDownloadFileStatInfo info;
    info.bundleName = "different.bundle.name";
    try {
        downloadStat.HandleBundleName(info);
        EXPECT_EQ(downloadStat.stat_.bundleName, "test.bundle.name");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_002 End";
}
 
/**
 * @tc.name: HandleBundleNameTest_003
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDiskDownloadFileStatTest, HandleBundleNameTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_003 Start";
    CloudDiskDownloadFileStat downloadStat;
    downloadStat.stat_.bundleName = "";
    CloudDiskDownloadFileStatInfo info;
    info.bundleName = "test.bundle.name";
    try {
        downloadStat.HandleBundleName(info);
        EXPECT_EQ(downloadStat.stat_.bundleName, "test.bundle.name");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_003 End";
}
 
/**
 * @tc.name: HandleBundleNameTest_004
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDiskDownloadFileStatTest, HandleBundleNameTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_004 Start";
    CloudDiskDownloadFileStat downloadStat;
    downloadStat.stat_.bundleName = "";
    CloudDiskDownloadFileStatInfo info;
    info.bundleName = "";
    try {
        downloadStat.HandleBundleName(info);
        EXPECT_EQ(downloadStat.stat_.bundleName, "");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_004 End";
}
 
/**
 * @tc.name: HandleBundleNameTest_005
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDiskDownloadFileStatTest, HandleBundleNameTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_005 Start";
    CloudDiskDownloadFileStat downloadStat;
    downloadStat.stat_.bundleName = "test.bundle.name";
    CloudDiskDownloadFileStatInfo info;
    info.bundleName = "";
    try {
        downloadStat.HandleBundleName(info);
        EXPECT_EQ(downloadStat.stat_.bundleName, "test.bundle.name");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_005 End";
}
 
}