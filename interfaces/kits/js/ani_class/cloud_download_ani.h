/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_FILEMGMT_CLOUD_DOWNLOAD_ANI_H
#define OHOS_FILEMGMT_CLOUD_DOWNLOAD_ANI_H

#include "cloud_file_core.h"

class CloudDownloadAni final {
public:
    static void DownloadConstructor(ani_env *env, ani_object object);
    static void DownloadOn(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void DownloadOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void DownloadOff1(ani_env *env, ani_object object, ani_string evt);
    static void DownloadStart(ani_env *env, ani_object object, ani_string uri);
    static void DownloadStop(ani_env *env, ani_object object, ani_string uri);
};
#endif // OHOS_FILEMGMT_CLOUD_DOWNLOAD_ANI_H