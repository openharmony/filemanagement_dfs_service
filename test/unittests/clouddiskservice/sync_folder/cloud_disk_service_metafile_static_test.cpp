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

#include <gtest/gtest.h>

#include "assistant.h"
#include "cloud_disk_service_error.h"

#include "cloud_disk_service_metafile.cpp"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
using namespace testing;
using namespace testing::ext;

class CloudDiskServiceMetafileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AssistantMock> insMock;
};

void CloudDiskServiceMetafileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskServiceMetafileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskServiceMetafileTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void CloudDiskServiceMetafileTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    Assistant::ins = nullptr;
    insMock = nullptr;
}

/**
 * @tc.name: GetBucketByLevelTest001
 * @tc.desc: Verify the GetBucketByLevel function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetOverallBucketTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOverallBucketTest001 Start";
    try {
        uint32_t level = MAX_BUCKET_LEVEL + 1;
        uint32_t ret = GetOverallBucket(level);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetOverallBucketTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetOverallBucketTest001 End";
}

/**
 * @tc.name: GetOverallBucketTest002
 * @tc.desc: Verify the GetOverallBucket function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetOverallBucketTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOverallBucketTest002 Start";
    try {
        uint32_t level = 1;
        uint32_t ret = GetOverallBucket(level);
        EXPECT_EQ(ret, static_cast<uint32_t>((1ULL << (level + 1))) - 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetOverallBucketTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GGetOverallBucketTest002 End";
}

/**
 * @tc.name: GetDcacheFileSizeTest001
 * @tc.desc: Verify the GetDcacheFileSize function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetDcacheFileSizeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDcacheFileSizeTest001 Start";
    try {
        uint32_t level = 1;
        size_t buckets = GetOverallBucket(level);
        size_t ret = GetDcacheFileSize(level);
        EXPECT_EQ(ret, buckets * DENTRYGROUP_SIZE * BUCKET_BLOCKS + DENTRYGROUP_HEADER);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDcacheFileSizeTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDcacheFileSizeTest001 End";
}

/**
 * @tc.name: GetBucketByLevelTest001
 * @tc.desc: Verify the GetBucketByLevel function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetBucketByLevelTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketByLevelTest001 Start";
    try {
        uint32_t level = MAX_BUCKET_LEVEL + 1;
        uint32_t ret = GetBucketByLevel(level);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketByLevelTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketByLevelTest001 End";
}

/**
 * @tc.name: GetBucketByLevelTest002
 * @tc.desc: Verify the GetBucketByLevel function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetBucketByLevelTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketByLevelTest002 Start";
    try {
        uint32_t level = 1;
        uint32_t ret = GetBucketByLevel(level);
        EXPECT_EQ(ret, static_cast<uint32_t>((1ULL << level)));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketByLevelTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketByLevelTest002 End";
}

/**
 * @tc.name: GetBucketaddrTest001
 * @tc.desc: Verify the GetBucketaddr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetBucketaddrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketaddrTest001 Start";
    try {
        uint32_t level = MAX_BUCKET_LEVEL + 1;
        uint32_t buckoffset = 0;
        uint32_t ret = GetBucketaddr(level, buckoffset);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketaddrTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketaddrTest001 End";
}

/**
 * @tc.name: GetBucketaddrTest002
 * @tc.desc: Verify the GetBucketaddr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetBucketaddrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketaddrTest002 Start";
    try {
        uint32_t level = 1;
        uint64_t curLevelMaxBucks = (1ULL << level);
        uint32_t buckoffset = curLevelMaxBucks + 1;
        uint32_t ret = GetBucketaddr(level, buckoffset);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketaddrTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketaddrTest002 End";
}

/**
 * @tc.name: GetBucketaddrTest003
 * @tc.desc: Verify the GetBucketaddr function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetBucketaddrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketaddrTest003 Start";
    try {
        uint32_t level = 1;
        uint64_t curLevelMaxBucks = (1ULL << level);
        uint32_t buckoffset = curLevelMaxBucks - 1;
        uint32_t ret = GetBucketaddr(level, buckoffset);
        EXPECT_EQ(ret, static_cast<uint32_t>(curLevelMaxBucks) + buckoffset - 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketaddrTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketaddrTest003 End";
}

/**
 * @tc.name: GetBidxFromLevelTest001
 * @tc.desc: Verify the GetBidxFromLevel function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetBidxFromLevelTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBidxFromLevelTest001 Start";
    try {
        uint32_t level = MAX_BUCKET_LEVEL + 1;
        uint32_t namehash = 1;
        unsigned long ret = GetBidxFromLevel(level, namehash);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBidxFromLevelTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBidxFromLevelTest001 End";
}

/**
 * @tc.name: GetBidxFromLevelTest002
 * @tc.desc: Verify the GetBidxFromLevel function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetBidxFromLevelTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBidxFromLevelTest002 Start";
    try {
        uint32_t level = 1;
        uint32_t namehash = 1;
        uint32_t bucket = GetBucketByLevel(level);
        unsigned long ret = GetBidxFromLevel(level, namehash);
        EXPECT_EQ(ret, BUCKET_BLOCKS * GetBucketaddr(level, namehash % bucket));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBidxFromLevelTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBidxFromLevelTest002 End";
}

/**
 * @tc.name: FindDentryPageTest001
 * @tc.desc: Verify the FindDentryPage function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindDentryPageTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryPageTest001 Start";
    try {
        uint64_t index = 0;
        auto ctx = std::make_shared<DcacheLookupCtx>();
        auto dentryBlk = std::make_unique<CloudDiskServiceDentryGroup>();
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(1));
        auto ret = FindDentryPage(index, ctx.get());
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryPageTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryPageTest001 End";
}

/**
 * @tc.name: FindDentryPageTest002
 * @tc.desc: Verify the FindDentryPage function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindDentryPageTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryPageTest002 Start";
    try {
        uint64_t index = 1;
        auto ctx = std::make_shared<DcacheLookupCtx>();
        auto dentryBlk = std::make_unique<CloudDiskServiceDentryGroup>();
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(DENTRYGROUP_SIZE));
        auto ret = FindDentryPage(index, ctx.get());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryPageTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryPageTest002 End";
}

/**
 * @tc.name: FindDentryPageTest003
 * @tc.desc: Verify the FindDentryPage function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindDentryPageTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryPageTest003 Start";
    try {
        uint64_t index = 1;
        auto ctx = std::make_shared<DcacheLookupCtx>();
        auto dentryBlk = std::make_unique<CloudDiskServiceDentryGroup>();
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).Times(2).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(1));
        auto ret = FindDentryPage(index, ctx.get());
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryPageTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryPageTest003 End";
}

/**
 * @tc.name: FindInBlockByIdTest001
 * @tc.desc: Verify the FindInBlockById function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest001 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk;
        auto ctx = std::make_shared<DcacheLookupCtx>();
        uint8_t revalidate = 0;
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest001 End";
}

/**
 * @tc.name: FindInBlockTest001
 * @tc.desc: Verify the FindInBlock function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockIdTest001 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk;
        auto ctx = std::make_shared<DcacheLookupCtx>();
        uint8_t revalidate = 0;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest001 End";
}

/**
 * @tc.name: InLevelTest001
 * @tc.desc: Verify the InLevel function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, InLevelTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InLevelTest001 Start";
    try {
        uint32_t level = MAX_BUCKET_LEVEL + 1;
        DcacheLookupCtx *ctx = nullptr;
        bool byId = true;
        uint8_t revalidate = 0;
        auto ret = InLevel(level, ctx, byId, revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InLevelTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "InLevelTest001 End";
}

/**
 * @tc.name: InLevelTest002
 * @tc.desc: Verify the InLevel function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, InLevelTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InLevelTest002 Start";
    try {
        uint32_t level = 1;
        shared_ptr<DcacheLookupCtx> ctx = make_shared<DcacheLookupCtx>();
        bool byId = true;
        uint8_t revalidate = 0;
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).Times(2).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(1));
        auto ret = InLevel(level, ctx.get(), byId, revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InLevelTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "InLevelTest002 End";
}

/**
 * @tc.name: FindDentryTest001
 * @tc.desc: Verify the FindDentry function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindDentryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryTest001 Start";
    try {
        shared_ptr<DcacheLookupCtx> ctx = make_shared<DcacheLookupCtx>();
        FindDentry(ctx.get());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryTest002 End";
}

/**
 * @tc.name: InitDcacheLookupCtxTest001
 * @tc.desc: Verify the InitDcacheLookupCtx function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, InitDcacheLookupCtxTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitDcacheLookupCtxTest001 Start";
    try {
        shared_ptr<DcacheLookupCtx> ctx = make_shared<DcacheLookupCtx>();
        MetaBase base;
        base.name = "MetaBase_name";
        InitDcacheLookupCtx(ctx.get(), base, 0, 0);
        EXPECT_EQ(ctx->fd, 0);
        EXPECT_EQ(ctx->name, base.name);
        EXPECT_EQ(ctx->bidx, 0);
        EXPECT_EQ(ctx->page, nullptr);
        EXPECT_EQ(ctx->hash, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitDcacheLookupCtxTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "InitDcacheLookupCtxTest001 End";
}

/**
 * @tc.name: RoomForFilenameTest001
 * @tc.desc: Verify the RoomForFilename function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, RoomForFilenameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RoomForFilenameTest001 Start";
    try {
        size_t slots = 2;
        uint32_t maxSlots = 5;
        uint8_t bitmap[] = {1, 1, 1, 1, 1};
        uint32_t ret = RoomForFilename(bitmap, slots, maxSlots);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RoomForFilenameTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "RoomForFilenameTest001 End";
}

/**
 * @tc.name: RoomForFilenameTest002
 * @tc.desc: Verify the RoomForFilename function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, RoomForFilenameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RoomForFilenameTest002 Start";
    try {
        size_t slots = 2;
        uint32_t maxSlots = 5;
        uint8_t bitmap[] = {0, 0, 0, 0, 0};
        uint32_t ret = RoomForFilename(bitmap, slots, maxSlots);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RoomForFilenameTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "RoomForFilenameTest002 End";
}

/**
 * @tc.name: RoomForFilenameTest003
 * @tc.desc: Verify the RoomForFilename function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, RoomForFilenameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RoomForFilenameTest003 Start";
    try {
        size_t slots = 2;
        uint32_t maxSlots = 5;
        uint8_t bitmap[] = {1, 1, 1, 1, 0};
        uint32_t ret = RoomForFilename(bitmap, slots, maxSlots);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RoomForFilenameTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "RoomForFilenameTest003 End";
}

/**
 * @tc.name: GetDentryFileByPathTest001
 * @tc.desc: Verify the GetDentryFileByPath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetDentryFileByPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDentryFileByPathTest001 Start";
    try {
        uint32_t userId = 100;
        std::string index = "index";
        std::string inode = "inode";
        GetDentryFileByPath(userId, index, inode);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDentryFileByPathTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDentryFileByPathTest001 End";
}
} // namespace OHOS::FileManagement::CloudDiskService
