/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef SECURITY_GUARD_RESULT_SET_MOCK_H
#define SECURITY_GUARD_RESULT_SET_MOCK_H

#include <cstdint>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace OHOS::NativeRdb {
class ResultSetInterface {
public:
    virtual ~ResultSetInterface() = default;
    virtual int GetBlob(int columnIndex, std::vector<uint8_t> &blob) = 0;
    virtual int GetString(int columnIndex, std::string &value) = 0;
    virtual int GetInt(int columnIndex, int &value) = 0;
    virtual int GetLong(int columnIndex, int64_t &value) = 0;
    virtual int GetDouble(int columnIndex, double &value) = 0;
    virtual int GetSize(int columnIndex, size_t &size) = 0;
    virtual int IsColumnNull(int columnIndex, bool &isNull) = 0;
    virtual int GoToRow(int position) = 0;
    virtual int GetAllColumnNames(std::vector<std::string> &columnNames) = 0;
    virtual int GetRowCount(int &count) = 0;
    virtual bool OnGo(int oldRowIndex, int newRowIndex) = 0;
    virtual int Close() = 0;
    virtual bool HasBlock() const = 0;
    virtual int GetColumnCount(int &count) = 0;
    virtual int GoToNextRow() = 0;
};

class ResultSet : public ResultSetInterface {
public:
    ResultSet() = default;
    ~ResultSet() override = default;
    MOCK_METHOD2(GetBlob, int(int columnIndex, std::vector<uint8_t> &blob));
    MOCK_METHOD2(GetString, int(int columnIndex, std::string &value));
    MOCK_METHOD2(GetInt, int(int columnIndex, int &value));
    MOCK_METHOD2(GetLong, int(int columnIndex, int64_t &value));
    MOCK_METHOD2(GetDouble, int(int columnIndex, double &value));
    MOCK_METHOD2(GetSize, int(int columnIndex, size_t &size));
    MOCK_METHOD2(IsColumnNull, int(int columnIndex, bool &isNull));
    MOCK_METHOD1(GoToRow, int(int position));
    MOCK_METHOD1(GetAllColumnNames, int(std::vector<std::string> &columnNames));
    MOCK_METHOD1(GetRowCount, int(int &count));
    MOCK_METHOD2(OnGo, bool(int oldRowIndex, int newRowIndex));
    MOCK_METHOD0(Close, int());
    MOCK_CONST_METHOD0(HasBlock, bool());
    MOCK_METHOD1(GetColumnCount, int(int &count));
    MOCK_METHOD0(GoToNextRow, int());
};
} // namespace OHOS::NativeRdb
#endif // SECURITY_GUARD_RESULT_SET_MOCK_H