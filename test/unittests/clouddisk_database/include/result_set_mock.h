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

#ifndef OHOS_CLOUD_DISK_DATABASE_RESULT_SET_MOCK_H
#define OHOS_CLOUD_DISK_DATABASE_RESULT_SET_MOCK_H

#include "result_set.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
class ResultSetMock : public ResultSet {
public:
    ~ResultSetMock() {}
    MOCK_METHOD1(GetAllColumnNames, int(std::vector<std::string> &));
    MOCK_METHOD1(GetColumnCount, int(int &));
    MOCK_METHOD2(GetColumnType, int(int, ColumnType &));
    MOCK_METHOD2(GetColumnIndex, int(const std::string &, int &));
    MOCK_METHOD2(GetColumnName, int(int, std::string &));
    MOCK_METHOD1(GetRowCount, int(int &));
    MOCK_METHOD1(GoTo, int(int));
    MOCK_METHOD1(GoToRow, int(int));
    MOCK_METHOD0(GoToFirstRow, int());
    MOCK_METHOD0(GoToLastRow, int());
    MOCK_METHOD0(GoToNextRow, int());
    MOCK_METHOD0(GoToPreviousRow, int());
    MOCK_METHOD1(IsEnded, int(bool &));
    MOCK_METHOD1(IsAtLastRow, int(bool &));
    MOCK_METHOD2(GetBlob, int(int, std::vector<uint8_t> &));
    MOCK_METHOD2(GetString, int(int, std::string &));
    MOCK_METHOD2(GetInt, int(int, int &));
    MOCK_METHOD2(GetLong, int(int, int64_t &));
    MOCK_METHOD2(GetDouble, int(int, double &));
    MOCK_METHOD2(IsColumnNull, int(int, bool &));
    MOCK_METHOD0(Close, int());
    MOCK_METHOD1(GetRow, int(RowEntity &));

    MOCK_CONST_METHOD0(IsClosed, bool());
    MOCK_CONST_METHOD1(GetRowIndex, int(int &));
    MOCK_CONST_METHOD1(IsStarted, int(bool &));
    MOCK_CONST_METHOD1(IsAtFirstRow, int(bool &));

    MOCK_METHOD2(GetAsset, int(int32_t, ValueObject::Asset &));
    MOCK_METHOD2(GetAssets, int(int32_t, ValueObject::Assets &));
    MOCK_METHOD2(Get, int(int32_t, ValueObject &));
    MOCK_METHOD1(GetModifyTime, int(std::string &));
    MOCK_METHOD2(GetSize, int(int32_t, size_t &));
};
} // namespace OHOS::FileManagement::CloudDisk::Test
#endif