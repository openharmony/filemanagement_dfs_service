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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBER_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBER_H

#include "async_common_event_result.h"
#include "common_event_data.h"
#include "common_event_subscribe_info.h"

namespace OHOS {
namespace EventFwk {
class CommonEventSubscriber {
public:
    CommonEventSubscriber() {}

    /**
     * A constructor used to create a CommonEventSubscriber instance with the
     * subscribeInfo parameter passed.
     *
     * @param subscribeInfo Indicates the subscribeInfo
     */
    explicit CommonEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo) {}

    virtual ~CommonEventSubscriber() {}

    /**
     * Calls back when the application receives a new common event.
     * Don't call other common event interface such as UnSubscribeCommonEvent() in the callback
     *
     * @param data Indicates the common event data.
     */
    virtual void OnReceiveEvent(const CommonEventData &data) = 0;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_SUBSCRIBER_H
