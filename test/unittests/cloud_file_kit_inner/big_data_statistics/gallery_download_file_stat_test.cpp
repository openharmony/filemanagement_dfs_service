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
#include "gallery_download_file_stat.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

#define TYPE_DOWNLOAD_FILE_IMAGE 1
#define TYPE_DOWNLOAD_FILE_VIDEO 2
#define DOWNLOAD_FILE_BYTE_SIZE 1e6
#define DURAITON_MODE 970

class GalleryDownloadFileStatTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void GalleryDownloadFileStatTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void GalleryDownloadFileStatTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void GalleryDownloadFileStatTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void GalleryDownloadFileStatTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_001 Start";
    try {
        GalleryDownloadFileStat::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_001 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_002 Start";
    try {
        GalleryDownloadFileStat::GetInstance().OutputToFile();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_002 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_003 Start";
    try {
        GalleryDownloadFileStat::GetInstance().Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_003 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_004 Start";
    try {
        GalleryDownloadFileStat galleryDownloadFileStat;
        uint32_t mediaType = TYPE_DOWNLOAD_FILE_IMAGE;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_004 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_005 Start";
    try {
        GalleryDownloadFileStat galleryDownloadFileStat;
        uint32_t mediaType = TYPE_DOWNLOAD_FILE_VIDEO;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_005 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_006 Start";
    try {
        GalleryDownloadFileStat galleryDownloadFileStat;
        uint32_t mediaType = TYPE_DOWNLOAD_FILE_IMAGE;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE * 2;
        uint64_t duration = DURAITON_MODE;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 4;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 6;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 8;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 15;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_006 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_006 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_007 Start";
    try {
        GalleryDownloadFileStat galleryDownloadFileStat;
        uint32_t mediaType = TYPE_DOWNLOAD_FILE_VIDEO;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE * 20;
        uint64_t duration = DURAITON_MODE;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 40;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 80;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 200;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 400;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        size = DOWNLOAD_FILE_BYTE_SIZE * 800;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_007 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_007 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_008 Start";
    try {
        GalleryDownloadFileStat galleryDownloadFileStat;
        uint32_t mediaType = TYPE_DOWNLOAD_FILE_IMAGE;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE * 2 + TYPE_DOWNLOAD_FILE_IMAGE;
        uint64_t duration = DURAITON_MODE;

        galleryDownloadFileStat.OutputToFile();
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 4 + TYPE_DOWNLOAD_FILE_IMAGE;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 6 + TYPE_DOWNLOAD_FILE_IMAGE;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 8 + TYPE_DOWNLOAD_FILE_IMAGE;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 15 + TYPE_DOWNLOAD_FILE_IMAGE;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        galleryDownloadFileStat.Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_008 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_008 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_009 Start";
    try {
        GalleryDownloadFileStat galleryDownloadFileStat;
        uint32_t mediaType = TYPE_DOWNLOAD_FILE_VIDEO;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE * 20 + TYPE_DOWNLOAD_FILE_VIDEO;
        uint64_t duration = DURAITON_MODE;

        galleryDownloadFileStat.OutputToFile();
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 40 + TYPE_DOWNLOAD_FILE_VIDEO;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 80 + TYPE_DOWNLOAD_FILE_VIDEO;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 200 + TYPE_DOWNLOAD_FILE_VIDEO;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 400 + TYPE_DOWNLOAD_FILE_VIDEO;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);

        size = DOWNLOAD_FILE_BYTE_SIZE * 800 + TYPE_DOWNLOAD_FILE_VIDEO;
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        galleryDownloadFileStat.Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_009 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_009 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_010 Start";
    try {
        GalleryDownloadFileStat galleryDownloadFileStat;
        uint32_t mediaType = TYPE_DOWNLOAD_FILE_IMAGE;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;
        
        galleryDownloadFileStat.OutputToFile();
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        galleryDownloadFileStat.Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_010 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_010 End";
}

HWTEST_F(GalleryDownloadFileStatTest, GalleryDownloadFileStatTest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_011 Start";
    try {
        GalleryDownloadFileStat galleryDownloadFileStat;
        uint32_t mediaType = TYPE_DOWNLOAD_FILE_VIDEO;
        uint64_t size = DOWNLOAD_FILE_BYTE_SIZE;
        uint64_t duration = DURAITON_MODE;

        galleryDownloadFileStat.OutputToFile();
        galleryDownloadFileStat.UpdateDownloadStat(mediaType, size, duration);
        galleryDownloadFileStat.Report();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_011 ERROR";
    }
    GTEST_LOG_(INFO) << "GalleryDownloadFileStatTest_011 End";
}
}