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

#include "dk_record_field.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace DriveKit;

class DKRecordFieldTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DKRecordFieldTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DKRecordFieldTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DKRecordFieldTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DKRecordFieldTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DKRecordField001
 * @tc.desc: Verify the DKRecordField::DKRecordField function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, DKRecordField001, TestSize.Level1)
{
    DKRecordField recordField;
    DKRecordField dkRecordField(recordField);
    EXPECT_EQ(recordField.type_, DKRecordFieldType::FIELD_TYPE_NULL);
}

/**
 * @tc.name: DKRecordField002
 * @tc.desc: Verify the DKRecordField::GetString function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, DKRecordField002, TestSize.Level1)
{
    const char *value = "DKRecordFieldTest";
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_STRING);
}

/**
 * @tc.name: GetType
 * @tc.desc: Verify the DKRecordField::GetType function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetType, TestSize.Level1)
{
    int value = 1;
    DKRecordField dkRecordField(value);
    DKRecordFieldType ret = dkRecordField.GetType();
    EXPECT_EQ(ret, DKRecordFieldType::FIELD_TYPE_INT);
}

/**
 * @tc.name: StealDKFiledValue
 * @tc.desc: Verify the DKRecordField::StealDKFiledValue function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, StealDKFiledValue, TestSize.Level1)
{
    DKFieldValue value;
    int64_t val = 111;
    DKRecordField dkRecordField(val);
    dkRecordField.StealDKFiledValue(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_NULL);
}

/**
 * @tc.name: GetInt001
 * @tc.desc: Verify the DKRecordField::GetInt function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetInt001, TestSize.Level1)
{
    int value = 1;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_INT);
    DKLocalErrorCode ret = dkRecordField.GetInt(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetInt002
 * @tc.desc: Verify the DKRecordField::GetInt function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetInt002, TestSize.Level1)
{
    int value = 1;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetInt(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetLong001
 * @tc.desc: Verify the DKRecordField::GetLong function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetLong001, TestSize.Level1)
{
    int64_t value = 111;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_INT);
    DKLocalErrorCode ret = dkRecordField.GetLong(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetLong002
 * @tc.desc: Verify the DKRecordField::GetLong function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetLong002, TestSize.Level1)
{
    int64_t value = 111;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetLong(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetDouble001
 * @tc.desc: Verify the DKRecordField::GetDouble function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetDouble001, TestSize.Level1)
{
    double value = 111.11;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_DOUBLE);
    DKLocalErrorCode ret = dkRecordField.GetDouble(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetDouble002
 * @tc.desc: Verify the DKRecordField::GetDouble function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetDouble002, TestSize.Level1)
{
    double value = 111.11;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetDouble(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetBool001
 * @tc.desc: Verify the DKRecordField::GetBool function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetBool001, TestSize.Level1)
{
    bool value = false;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_BOOL);
    DKLocalErrorCode ret = dkRecordField.GetBool(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetBool002
 * @tc.desc: Verify the DKRecordField::GetBool function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetBool002, TestSize.Level1)
{
    bool value = false;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetBool(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetString001
 * @tc.desc: Verify the DKRecordField::GetString function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetString001, TestSize.Level1)
{
    string value = "DKRecordFieldTest";
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_STRING);
    DKLocalErrorCode ret = dkRecordField.GetString(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetString002
 * @tc.desc: Verify the DKRecordField::GetString function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetString002, TestSize.Level1)
{
    string value = "DKRecordFieldTest";
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetString(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetBlob001
 * @tc.desc: Verify the DKRecordField::GetBlob function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetBlob001, TestSize.Level1)
{
    std::vector<uint8_t> value;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_BLOB);
    DKLocalErrorCode ret = dkRecordField.GetBlob(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetBlob002
 * @tc.desc: Verify the DKRecordField::GetBlob function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetBlob002, TestSize.Level1)
{
    std::vector<uint8_t> value;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetBlob(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetRecordList001
 * @tc.desc: Verify the DKRecordField::GetRecordList function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetRecordList001, TestSize.Level1)
{
    DKRecordFieldList value;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_LIST);
    DKLocalErrorCode ret = dkRecordField.GetRecordList(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetRecordList002
 * @tc.desc: Verify the DKRecordField::GetRecordList function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetRecordList002, TestSize.Level1)
{
    DKRecordFieldList value;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetRecordList(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetRecordMap001
 * @tc.desc: Verify the DKRecordField::GetRecordMap function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetRecordMap001, TestSize.Level1)
{
    DKRecordFieldMap value;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_MAP);
    DKLocalErrorCode ret = dkRecordField.GetRecordMap(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetRecordMap002
 * @tc.desc: Verify the DKRecordField::GetRecordMap function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetRecordMap002, TestSize.Level1)
{
    DKRecordFieldMap value;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetRecordMap(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetAsset001
 * @tc.desc: Verify the DKRecordField::GetAsset function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetAsset001, TestSize.Level1)
{
    DKAsset value;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_ASSET);
    DKLocalErrorCode ret = dkRecordField.GetAsset(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetAsset002
 * @tc.desc: Verify the DKRecordField::GetAsset function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetAsset002, TestSize.Level1)
{
    DKAsset value;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetAsset(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetReference001
 * @tc.desc: Verify the DKRecordField::GetReference function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetReference001, TestSize.Level1)
{
    DKReference value;
    DKRecordField dkRecordField(value);
    EXPECT_EQ(dkRecordField.type_, DKRecordFieldType::FIELD_TYPE_REFERENCE);
    DKLocalErrorCode ret = dkRecordField.GetReference(value);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetReference002
 * @tc.desc: Verify the DKRecordField::GetReference function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKRecordFieldTest, GetReference002, TestSize.Level1)
{
    DKReference value;
    DKRecordField dkRecordField;
    DKLocalErrorCode ret = dkRecordField.GetReference(value);
    EXPECT_NE(ret, DKLocalErrorCode::NO_ERROR);
}
} // namespace OHOS::FileManagement::CloudSync::Test
