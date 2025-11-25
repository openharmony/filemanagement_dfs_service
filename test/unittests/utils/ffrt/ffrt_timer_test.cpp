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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "ffrt_timer.h"

namespace OHOS::FileManagement::CloudFile {

using namespace std;
using namespace testing;
using namespace testing::ext;

class FfrtTimerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<FfrtTimer> ffrtTimer_{nullptr};
};

void FfrtTimerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FfrtTimerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FfrtTimerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    ffrtTimer_ = make_shared<FfrtTimer>("timerName");
}

void FfrtTimerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StartTest001
 * @tc.desc: Verify the Start function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(FfrtTimerTest, StartTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTest001 Start";
    try {
        TimerCallback callback = []() -> void {};
        uint32_t interval = 1;
        uint32_t repatTimes = 100;
        ffrtTimer_->running_ = false;
        ffrtTimer_->Start(callback, interval, repatTimes);
        EXPECT_TRUE(ffrtTimer_->isJoinable_);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "StartTest001 End";
}

/**
 * @tc.name: StartTest002
 * @tc.desc: Verify the Start function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(FfrtTimerTest, StartTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTest002 Start";
    try {
        uint32_t interval = 1;
        uint32_t repatTimes = 100;
        ffrtTimer_->running_ = false;
        ffrtTimer_->Start(nullptr, interval, repatTimes);
        EXPECT_TRUE(ffrtTimer_->isJoinable_);
        ffrt::wait();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "StartTest002 End";
}

/**
 * @tc.name: StartTest003
 * @tc.desc: Verify the Start function
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(FfrtTimerTest, StartTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTest003 Start";
    try {
        TimerCallback callback = []() -> void {};
        uint32_t interval = 1;
        uint32_t repatTimes = 100;
        ffrtTimer_->running_ = true;
        ffrtTimer_->Start(callback, interval, repatTimes);
        EXPECT_FALSE(ffrtTimer_->isJoinable_);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "StartTest003 End";
}
} // namespace OHOS::FileManagement::CloudFile