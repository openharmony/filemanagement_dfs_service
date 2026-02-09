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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "assistant.h"
#include "cloud_disk_common.h"
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
 * @tc.name: GetDentrySlotsTest001
 * @tc.desc: Verify the GetDentrySlots function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetDentrySlotsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDentrySlotsTest001 Start";
    try {
        uint32_t ret = GetDentrySlots(16);
        EXPECT_EQ(ret, static_cast<uint32_t>((16 + BYTE_PER_SLOT - 1) >> HMDFS_SLOT_LEN_BITS));

        ret = GetDentrySlots(0);
        EXPECT_EQ(ret, static_cast<uint32_t>((BYTE_PER_SLOT - 1) >> HMDFS_SLOT_LEN_BITS));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDentrySlotsTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDentrySlotsTest001 End";
}

/**
 * @tc.name: GetDentryGroupPosTest001
 * @tc.desc: Verify the GetDentryGroupPos function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetDentryGroupPosTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDentryGroupPosTest001 Start";
    try {
        off_t ret = GetDentryGroupPos(1);
        EXPECT_EQ(ret, DENTRYGROUP_SIZE + DENTRYGROUP_HEADER);

        ret = GetDentryGroupPos(0);
        EXPECT_EQ(ret, DENTRYGROUP_HEADER);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDentryGroupPosTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDentryGroupPosTest001 End";
}

/**
 * @tc.name: GetDentryGroupCntTest001
 * @tc.desc: Verify the GetDentryGroupCnt function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetDentryGroupCntTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDentryGroupCntTest001 Start";
    try {
        uint64_t ret = GetDentryGroupCnt(DENTRYGROUP_HEADER + DENTRYGROUP_SIZE);
        EXPECT_EQ(ret, 1);

        ret = GetDentryGroupCnt(DENTRYGROUP_HEADER - 1);
        EXPECT_EQ(ret, 0);

        ret = GetDentryGroupCnt(DENTRYGROUP_HEADER);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDentryGroupCntTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDentryGroupCntTest001 End";
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
        uint32_t ret = GetOverallBucket(MAX_BUCKET_LEVEL + 1);
        EXPECT_EQ(ret, E_OK);

        ret = GetOverallBucket(1);
        EXPECT_EQ(ret, static_cast<uint32_t>((1ULL << (1 + 1))) - 1);

        ret = GetOverallBucket(0);
        EXPECT_EQ(ret, static_cast<uint32_t>((1ULL << 1)) - 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetOverallBucketTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetOverallBucketTest001 End";
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

        level = 0;
        buckets = GetOverallBucket(level);
        ret = GetDcacheFileSize(level);
        EXPECT_EQ(ret, buckets * DENTRYGROUP_SIZE * BUCKET_BLOCKS + DENTRYGROUP_HEADER);

        level = MAX_BUCKET_LEVEL - 1;
        buckets = GetOverallBucket(level);
        ret = GetDcacheFileSize(level);
        EXPECT_EQ(ret, buckets * DENTRYGROUP_SIZE * BUCKET_BLOCKS + DENTRYGROUP_HEADER);

        level = MAX_BUCKET_LEVEL;
        buckets = GetOverallBucket(level);
        ret = GetDcacheFileSize(level);
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
        uint32_t ret = GetBucketByLevel(MAX_BUCKET_LEVEL + 1);
        EXPECT_EQ(ret, E_OK);

        ret = GetBucketByLevel(1);
        EXPECT_EQ(ret, static_cast<uint32_t>((1ULL << 1)));

        ret = GetBucketByLevel(0);
        EXPECT_EQ(ret, static_cast<uint32_t>((1ULL)));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketByLevelTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketByLevelTest001 End";
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

        level = 1;
        uint64_t curLevelMaxBucks = (1ULL << level);
        buckoffset = curLevelMaxBucks + 1;
        ret = GetBucketaddr(level, buckoffset);
        EXPECT_EQ(ret, E_OK);

        buckoffset = curLevelMaxBucks - 1;
        ret = GetBucketaddr(level, buckoffset);
        EXPECT_EQ(ret, static_cast<uint32_t>(curLevelMaxBucks) + buckoffset - 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBucketaddrTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketaddrTest001 End";
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
        unsigned long ret = GetBidxFromLevel(MAX_BUCKET_LEVEL + 1, 1);
        EXPECT_EQ(ret, E_OK);

        level = 1;
        uint32_t bucket = GetBucketByLevel(level);
        ret = GetBidxFromLevel(level, namehash);
        EXPECT_EQ(ret, BUCKET_BLOCKS * GetBucketaddr(level, namehash % bucket));

        level = 0;
        namehash = 1;
        bucket = GetBucketByLevel(level);
        ret = GetBidxFromLevel(level, namehash);
        EXPECT_EQ(ret, BUCKET_BLOCKS * GetBucketaddr(level, namehash % bucket));

        level = MAX_BUCKET_LEVEL;
        namehash = 1;
        ret = GetBidxFromLevel(level, namehash);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBidxFromLevelTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBidxFromLevelTest001 End";
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
        uint64_t index = 1;
        auto ctx = std::make_shared<DcacheLookupCtx>();
        auto dentryBlk = std::make_unique<CloudDiskServiceDentryGroup>();
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(DENTRYGROUP_SIZE));
        auto ret = FindDentryPage(index, ctx.get());
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
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(1));
        auto ret = FindDentryPage(index, ctx.get());
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryPageTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryPageTest002 End";
}

/**
 * @tc.name: FindDentryPageTest003
 * @tc.desc: Verify the FindDentryPage function with index 0
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindDentryPageTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryPageTest003 Start";
    try {
        uint64_t index = 0;
        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->fd = 1;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(DENTRYGROUP_SIZE));
        auto ret = FindDentryPage(index, ctx.get());
        EXPECT_NE(ret, nullptr);
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
 * @tc.name: FindInBlockByIdTest002
 * @tc.desc: Verify the FindInBlockById function with matching recordId
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest002 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {0};
        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = "testRecordId123";
        uint8_t revalidate = VALIDATE;
        dentryBlk.nsl[0].namelen = 13;
        dentryBlk.nsl[0].revalidate = VALIDATE;
        memcpy_s(dentryBlk.nsl[0].recordId, RECORD_ID_LEN, ctx->recordId.c_str(), ctx->recordId.length());
        BitOps::SetBit(0, dentryBlk.bitmap);
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        EXPECT_NE(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest002 End";
}

/**
 * @tc.name: FindInBlockByIdTest003
 * @tc.desc: Verify the FindInBlockById function with INVALIDATE revalidate
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest003 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {0};
        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = "testRecordId123";
        uint8_t revalidate = INVALIDATE;
        dentryBlk.nsl[0].namelen = 13;
        dentryBlk.nsl[0].revalidate = INVALIDATE;
        memcpy_s(dentryBlk.nsl[0].recordId, RECORD_ID_LEN, ctx->recordId.c_str(), ctx->recordId.length());
        BitOps::SetBit(0, dentryBlk.bitmap);
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        EXPECT_NE(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest003 End";
}

/**
 * @tc.name: FindInBlockByIdTest004
 * @tc.desc: Verify the FindInBlockById function with no matching revalidate
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest004 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {0};
        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = "testRecordId123";
        uint8_t revalidate = VALIDATE;
        dentryBlk.nsl[0].namelen = 13;
        dentryBlk.nsl[0].revalidate = INVALIDATE;
        memcpy_s(dentryBlk.nsl[0].recordId, RECORD_ID_LEN, ctx->recordId.c_str(), ctx->recordId.length());
        BitOps::SetBit(0, dentryBlk.bitmap);
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest004 End";
}

/**
 * @tc.name: FindInBlockTest001
 * @tc.desc: Verify the FindInBlock function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest001 Start";
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
 * @tc.name: FindInBlockTest002
 * @tc.desc: Verify the FindInBlock function with matching name and hash
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest002 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {0};
        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = "testname";
        ctx->hash = 12345;
        uint8_t revalidate = VALIDATE;
        dentryBlk.nsl[0].namelen = ctx->name.length();
        dentryBlk.nsl[0].hash = ctx->hash;
        dentryBlk.nsl[0].revalidate = VALIDATE;
        memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, ctx->name.c_str(), ctx->name.length());
        BitOps::SetBit(0, dentryBlk.bitmap);
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_NE(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest002 End";
}

/**
 * @tc.name: FindInBlockTest003
 * @tc.desc: Verify the FindInBlock function with INVALIDATE revalidate
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest003 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {0};
        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = "testname";
        ctx->hash = 12345;
        uint8_t revalidate = INVALIDATE;
        dentryBlk.nsl[0].namelen = ctx->name.length();
        dentryBlk.nsl[0].hash = ctx->hash;
        dentryBlk.nsl[0].revalidate = INVALIDATE;
        memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, ctx->name.c_str(), ctx->name.length());
        BitOps::SetBit(0, dentryBlk.bitmap);
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_NE(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest003 End";
}

/**
 * @tc.name: FindInBlockTest004
 * @tc.desc: Verify the FindInBlock function with no matching revalidate
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest004 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {0};
        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = "testname";
        ctx->hash = 12345;
        uint8_t revalidate = VALIDATE;
        dentryBlk.nsl[0].namelen = ctx->name.length();
        dentryBlk.nsl[0].hash = ctx->hash;
        dentryBlk.nsl[0].revalidate = INVALIDATE;
        memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, ctx->name.c_str(), ctx->name.length());
        BitOps::SetBit(0, dentryBlk.bitmap);
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest004 End";
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
 * @tc.name: InLevelTest003
 * @tc.desc: Verify the InLevel function with byId false
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, InLevelTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InLevelTest003 Start";
    try {
        uint32_t level = 1;
        shared_ptr<DcacheLookupCtx> ctx = make_shared<DcacheLookupCtx>();
        ctx->fd = 1;
        ctx->hash = 1;
        bool byId = false;
        uint8_t revalidate = VALIDATE;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(1));
        auto ret = InLevel(level, ctx.get(), byId, revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InLevelTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "InLevelTest003 End";
}
 
/**
 * @tc.name: InLevelTest004
 * @tc.desc: Verify the InLevel function with successful find
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, InLevelTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InLevelTest004 Start";
    try {
        uint32_t level = 1;
        shared_ptr<DcacheLookupCtx> ctx = make_shared<DcacheLookupCtx>();
        ctx->fd = 1;
        ctx->hash = 1;
        ctx->name = "testname";
        bool byId = false;
        uint8_t revalidate = VALIDATE;
        CloudDiskServiceDentryGroup dentryBlk = {0};
        dentryBlk.nsl[0].namelen = ctx->name.length();
        dentryBlk.nsl[0].hash = ctx->hash;
        dentryBlk.nsl[0].revalidate = VALIDATE;
        memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, ctx->name.c_str(), ctx->name.length());
        BitOps::SetBit(0, dentryBlk.bitmap);
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(DoAll(
            testing::Invoke([&dentryBlk](int, off_t, size_t, void *data) -> int64_t {
                memcpy_s(data, DENTRYGROUP_SIZE, &dentryBlk, DENTRYGROUP_SIZE);
                return DENTRYGROUP_SIZE;
            }), Return(DENTRYGROUP_SIZE)));
        InLevel(level, ctx.get(), byId, revalidate);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InLevelTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "InLevelTest004 End";
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
        GTEST_LOG_(INFO) << "FindDentryTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryTest001 End";
}

/**
 * @tc.name: FindDentryTest002
 * @tc.desc: Verify the FindDentry function with byId true
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindDentryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryTest002 Start";
    try {
        shared_ptr<DcacheLookupCtx> ctx = make_shared<DcacheLookupCtx>();
        ctx->fd = 1;
        ctx->recordId = "testRecordId123";
        bool byId = true;
        uint8_t revalidate = VALIDATE;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(1));
        auto ret = FindDentry(ctx.get(), byId, revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryTest002 End";
}

/**
 * @tc.name: FindDentryTest003
 * @tc.desc: Verify the FindDentry function with INVALIDATE revalidate
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindDentryTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryTest003 Start";
    try {
        shared_ptr<DcacheLookupCtx> ctx = make_shared<DcacheLookupCtx>();
        ctx->fd = 1;
        uint8_t revalidate = INVALIDATE;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(1));
        auto ret = FindDentry(ctx.get(), false, revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryTest003 End";
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
 * @tc.name: InitDcacheLookupCtxTest002
 * @tc.desc: Verify the InitDcacheLookupCtx function with different parameters
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, InitDcacheLookupCtxTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitDcacheLookupCtxTest002 Start";
    try {
        shared_ptr<DcacheLookupCtx> ctx = make_shared<DcacheLookupCtx>();
        MetaBase base;
        base.name = "TestName";
        uint32_t hash = 12345;
        int fd = 1;
        InitDcacheLookupCtx(ctx.get(), base, hash, fd);
        EXPECT_EQ(ctx->fd, fd);
        EXPECT_EQ(ctx->name, base.name);
        EXPECT_EQ(ctx->bidx, 0);
        EXPECT_EQ(ctx->page, nullptr);
        EXPECT_EQ(ctx->hash, hash);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitDcacheLookupCtxTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "InitDcacheLookupCtxTest002 End";
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
 * @tc.name: RoomForFilenameTest004
 * @tc.desc: Verify the RoomForFilename function with slots equals maxSlots
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, RoomForFilenameTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RoomForFilenameTest004 Start";
    try {
        size_t slots = 5;
        uint32_t maxSlots = 5;
        uint8_t bitmap[] = {0, 0, 0, 0, 0};
        uint32_t ret = RoomForFilename(bitmap, slots, maxSlots);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RoomForFilenameTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "RoomForFilenameTest004 End";
}

/**
 * @tc.name: RoomForFilenameTest005
 * @tc.desc: Verify the RoomForFilename function with all bits set
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, RoomForFilenameTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RoomForFilenameTest005 Start";
    try {
        size_t slots = 2;
        uint32_t maxSlots = 5;
        uint8_t bitmap[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        uint32_t ret = RoomForFilename(bitmap, slots, maxSlots);
        EXPECT_EQ(ret, maxSlots);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RoomForFilenameTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "RoomForFilenameTest005 End";
}

/**
 * @tc.name: RoomForFilenameTest006
 * @tc.desc: Verify the RoomForFilename function with partial free space
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, RoomForFilenameTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RoomForFilenameTest006 Start";
    try {
        size_t slots = 3;
        uint32_t maxSlots = 5;
        uint8_t bitmap[] = {0, 0, 0, 1, 1};
        uint32_t ret = RoomForFilename(bitmap, slots, maxSlots);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RoomForFilenameTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "RoomForFilenameTest006 End";
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
        std::string index = "testindex";
        std::string inode = "testinode";
        GetDentryFileByPath(userId, index, inode);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDentryFileByPathTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDentryFileByPathTest001 End";
}
} // namespace OHOS::FileManagement::CloudDiskService
