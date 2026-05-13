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
 * @tc.desc: Verify the FindInBlockById function with matching revalidate and recordId
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest002 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].namelen = 5;
        std::string recordId = "test_record_id";
        (void)memcpy_s(dentryBlk.nsl[0].recordId, RECORD_ID_LEN, recordId.c_str(), recordId.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = recordId;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(ret, &dentryBlk.nsl[0]);
        EXPECT_EQ(ctx->bitPos, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest002 End";
}

/**
 * @tc.name: FindInBlockByIdTest003
 * @tc.desc: Verify the FindInBlockById function with non-matching revalidate
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest003 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = INVALIDATE;
        dentryBlk.nsl[0].namelen = 5;
        std::string recordId = "test_record_id";
        (void)memcpy_s(dentryBlk.nsl[0].recordId, RECORD_ID_LEN, recordId.c_str(), recordId.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = recordId;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest003 End";
}

/**
 * @tc.name: FindInBlockByIdTest004
 * @tc.desc: Verify the FindInBlockById function with non-matching recordId
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest004 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].namelen = 5;
        std::string recordId1 = "record_one_test";
        (void)memcpy_s(dentryBlk.nsl[0].recordId, RECORD_ID_LEN, recordId1.c_str(), recordId1.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = "record_two_test";
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest004 End";
}

/**
 * @tc.name: FindInBlockByIdTest005
 * @tc.desc: Verify the FindInBlockById function skips non-matching dentries by slot count
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest005 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        std::string recordId1 = "record_one_test";
        uint32_t slots1 = GetDentrySlots(recordId1.length());
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].namelen = recordId1.length();
        (void)memcpy_s(dentryBlk.nsl[0].recordId, RECORD_ID_LEN, recordId1.c_str(), recordId1.length());

        std::string recordId2 = "record_two_test";
        uint32_t bitPos2 = slots1;
        BitOps::SetBit(bitPos2, dentryBlk.bitmap);
        dentryBlk.nsl[bitPos2].revalidate = VALIDATE;
        dentryBlk.nsl[bitPos2].namelen = recordId2.length();
        (void)memcpy_s(dentryBlk.nsl[bitPos2].recordId, RECORD_ID_LEN, recordId2.c_str(), recordId2.length());
        if (slots1 > 1) {
            dentryBlk.nsl[1].namelen = 0;
        }

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = recordId2;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(ret, &dentryBlk.nsl[bitPos2]);
        EXPECT_EQ(ctx->bitPos, bitPos2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest005 End";
}

/**
 * @tc.name: FindInBlockByIdTest006
 * @tc.desc: Verify the FindInBlockById function skips namelen==0 (continuation) slots
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest006 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].namelen = 0;

        std::string recordId = "test_record_id";
        BitOps::SetBit(1, dentryBlk.bitmap);
        dentryBlk.nsl[1].revalidate = VALIDATE;
        dentryBlk.nsl[1].namelen = 5;
        (void)memcpy_s(dentryBlk.nsl[1].recordId, RECORD_ID_LEN, recordId.c_str(), recordId.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = recordId;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(ret, &dentryBlk.nsl[1]);
        EXPECT_EQ(ctx->bitPos, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest006 End";
}

/**
 * @tc.name: FindInBlockByIdTest007
 * @tc.desc: Verify the FindInBlockById function with zero revalidate mask never matches
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockByIdTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockByIdTest007 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].namelen = 5;
        std::string recordId = "test_record_id";
        (void)memcpy_s(dentryBlk.nsl[0].recordId, RECORD_ID_LEN, recordId.c_str(), recordId.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->recordId = recordId;
        uint8_t revalidate = 0;
        auto ret = FindInBlockById(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockByIdTest007 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockByIdTest007 End";
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
 * @tc.desc: Verify the FindInBlock function with all conditions matching
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest002 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        std::string name = "test_name";
        uint32_t hash = 42;
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].hash = hash;
        dentryBlk.nsl[0].namelen = name.length();
        (void)memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, name.c_str(), name.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = name;
        ctx->hash = hash;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(ret, &dentryBlk.nsl[0]);
        EXPECT_EQ(ctx->bitPos, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest002 End";
}

/**
 * @tc.name: FindInBlockTest003
 * @tc.desc: Verify the FindInBlock function with non-matching hash
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest003 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        std::string name = "test_name";
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].hash = 42;
        dentryBlk.nsl[0].namelen = name.length();
        (void)memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, name.c_str(), name.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = name;
        ctx->hash = 100;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest003 End";
}

/**
 * @tc.name: FindInBlockTest004
 * @tc.desc: Verify the FindInBlock function with non-matching namelen
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest004 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        uint32_t hash = 42;
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].hash = hash;
        dentryBlk.nsl[0].namelen = 5;
        (void)memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, "short", 5);

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = "longer_name";
        ctx->hash = hash;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest004 End";
}

/**
 * @tc.name: FindInBlockTest005
 * @tc.desc: Verify the FindInBlock function with non-matching fileName content
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest005 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        uint32_t hash = 42;
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].hash = hash;
        dentryBlk.nsl[0].namelen = 5;
        (void)memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, "aaaaa", 5);

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = "bbbbb";
        ctx->hash = hash;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest005 End";
}

/**
 * @tc.name: FindInBlockTest006
 * @tc.desc: Verify the FindInBlock function with non-matching revalidate
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest006 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        std::string name = "test_name";
        uint32_t hash = 42;
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = INVALIDATE;
        dentryBlk.nsl[0].hash = hash;
        dentryBlk.nsl[0].namelen = name.length();
        (void)memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, name.c_str(), name.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = name;
        ctx->hash = hash;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest006 End";
}

/**
 * @tc.name: FindInBlockTest007
 * @tc.desc: Verify the FindInBlock function skips non-matching dentries by slot count
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest007 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        std::string name1 = "12345";
        std::string name2 = "67890";
        uint32_t hash = 42;
        uint32_t slots1 = GetDentrySlots(name1.length());

        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].hash = hash;
        dentryBlk.nsl[0].namelen = name1.length();
        (void)memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, name1.c_str(), name1.length());

        uint32_t bitPos2 = slots1;
        BitOps::SetBit(bitPos2, dentryBlk.bitmap);
        dentryBlk.nsl[bitPos2].revalidate = VALIDATE;
        dentryBlk.nsl[bitPos2].hash = hash;
        dentryBlk.nsl[bitPos2].namelen = name2.length();
        (void)memcpy_s(dentryBlk.fileName[bitPos2], DENTRY_NAME_LEN, name2.c_str(), name2.length());
        if (slots1 > 1) {
            dentryBlk.nsl[1].namelen = 0;
        }

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = name2;
        ctx->hash = hash;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(ret, &dentryBlk.nsl[bitPos2]);
        EXPECT_EQ(ctx->bitPos, bitPos2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest007 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest007 End";
}

/**
 * @tc.name: FindInBlockTest008
 * @tc.desc: Verify the FindInBlock function skips namelen==0 continuation slots
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest008 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        std::string name = "test_name";
        uint32_t hash = 42;
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].namelen = 0;

        BitOps::SetBit(1, dentryBlk.bitmap);
        dentryBlk.nsl[1].revalidate = VALIDATE;
        dentryBlk.nsl[1].hash = hash;
        dentryBlk.nsl[1].namelen = name.length();
        (void)memcpy_s(dentryBlk.fileName[1], DENTRY_NAME_LEN, name.c_str(), name.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = name;
        ctx->hash = hash;
        uint8_t revalidate = VALIDATE;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(ret, &dentryBlk.nsl[1]);
        EXPECT_EQ(ctx->bitPos, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest008 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest008 End";
}

/**
 * @tc.name: FindInBlockTest009
 * @tc.desc: Verify the FindInBlock function with zero revalidate mask never matches
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, FindInBlockTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindInBlockTest009 Start";
    try {
        CloudDiskServiceDentryGroup dentryBlk = {};
        std::string name = "test_name";
        uint32_t hash = 42;
        BitOps::SetBit(0, dentryBlk.bitmap);
        dentryBlk.nsl[0].revalidate = VALIDATE;
        dentryBlk.nsl[0].hash = hash;
        dentryBlk.nsl[0].namelen = name.length();
        (void)memcpy_s(dentryBlk.fileName[0], DENTRY_NAME_LEN, name.c_str(), name.length());

        auto ctx = std::make_shared<DcacheLookupCtx>();
        ctx->name = name;
        ctx->hash = hash;
        uint8_t revalidate = 0;
        auto ret = FindInBlock(dentryBlk, ctx.get(), revalidate);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindInBlockTest009 ERROR";
    }
    GTEST_LOG_(INFO) << "FindInBlockTest009 End";
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
