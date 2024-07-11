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

#ifndef SECURITY_GUARD_RDB_PREDICATES_MOCK_H
#define SECURITY_GUARD_RDB_PREDICATES_MOCK_H

#include <cstdint>
#include <string>

#include <gtest/gtest.h>

namespace OHOS::NativeRdb {
class AbsPredicates {
public:
    AbsPredicates() = default;
    virtual ~AbsPredicates() = default;
    virtual AbsPredicates *EqualTo(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *EqualTo(std::string field, int value) {return nullptr;};
    virtual AbsPredicates *NotEqualTo(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *BeginWrap() {return nullptr;};
    virtual AbsPredicates *EndWrap() {return nullptr;};
    virtual AbsPredicates *Or() {return nullptr;};
    virtual AbsPredicates *And() {return nullptr;};
    virtual AbsPredicates *Contains(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *BeginsWith(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *EndsWith(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *IsNull(std::string field) {return nullptr;};
    virtual AbsPredicates *IsNotNull(std::string field) {return nullptr;};
    virtual AbsPredicates *Like(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *Glob(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *Between(std::string field, std::string low, std::string high) {return nullptr;};
    virtual AbsPredicates *NotBetween(std::string field, std::string low, std::string high) {return nullptr;};
    virtual AbsPredicates *GreaterThan(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *LessThan(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *GreaterThanOrEqualTo(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *LessThanOrEqualTo(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *OrderByAsc(std::string field) {return nullptr;};
    virtual AbsPredicates *OrderByDesc(std::string field) {return nullptr;};
    virtual AbsPredicates *Distinct() {return nullptr;};
    virtual AbsPredicates *Limit(int value) {return nullptr;};
    virtual AbsPredicates *Offset(int rowOffset) {return nullptr;};
    virtual AbsPredicates *GroupBy(std::vector<std::string> fields) {return nullptr;};
    virtual AbsPredicates *IndexedBy(std::string indexName) {return nullptr;};
    virtual AbsPredicates *In(std::string field, std::vector<std::string> values) {return nullptr;};
    virtual AbsPredicates *NotIn(std::string field, std::vector<std::string> values) {return nullptr;};
};

class AbsRdbPredicates : public AbsPredicates {
public:
    AbsRdbPredicates(std::string string) : AbsPredicates() {};
    ~AbsRdbPredicates() override {};
};

class RdbPredicates : public AbsRdbPredicates {
public:
    explicit RdbPredicates(std::string string) : AbsRdbPredicates(string){};
    ~RdbPredicates() override {};
    virtual AbsPredicates *NotEqualTo(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *BeginWrap() {return nullptr;};
    virtual AbsPredicates *EndWrap() {return nullptr;};
    virtual AbsPredicates *Or() {return nullptr;};
    virtual AbsPredicates *And() {return nullptr;};
    virtual AbsPredicates *Contains(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *BeginsWith(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *EndsWith(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *IsNull(std::string field) {return nullptr;};
    virtual AbsPredicates *IsNotNull(std::string field) {return nullptr;};
    virtual AbsPredicates *Like(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *Glob(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *Between(std::string field, std::string low, std::string high) {return nullptr;};
    virtual AbsPredicates *NotBetween(std::string field, std::string low, std::string high) {return nullptr;};
    virtual AbsPredicates *GreaterThan(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *LessThan(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *GreaterThanOrEqualTo(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *LessThanOrEqualTo(std::string field, std::string value) {return nullptr;};
    virtual AbsPredicates *OrderByAsc(std::string field) {return nullptr;};
    virtual AbsPredicates *OrderByDesc(std::string field) {return nullptr;};
    virtual AbsPredicates *Distinct() {return nullptr;};
    virtual AbsPredicates *Limit(int value) {return nullptr;};
    virtual AbsPredicates *Offset(int rowOffset) {return nullptr;};
    virtual AbsPredicates *GroupBy(std::vector<std::string> fields) {return nullptr;};
    virtual AbsPredicates *IndexedBy(std::string indexName) {return nullptr;};
    virtual AbsPredicates *In(std::string field, std::vector<std::string> values) {return nullptr;};
    virtual AbsPredicates *NotIn(std::string field, std::vector<std::string> values) {return nullptr;};
};
} // namespace OHOS::NativeRdb
#endif // SECURITY_GUARD_RDB_PREDICATES_MOCK_H