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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_CACHE_ANI_H
#define OHOS_FILEMGMT_CLOUD_FILE_CACHE_ANI_H

#include "cloud_file_cache_core.h"

namespace OHOS::FileManagement::CloudSync {

class CloudFileCacheAni final {
public:
    static void CloudFileCacheConstructor(ani_env *env, ani_object object);
    static void CloudFileCacheOn(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void CloudFileCacheOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void CloudFileCacheOff1(ani_env *env, ani_object object, ani_string evt);
    static void CloudFileCacheStart(ani_env *env, ani_object object, ani_string uri);
    static ani_double CloudFileCacheStartBatch(ani_env *env,
                                               ani_object object,
                                               ani_array_ref uriList,
                                               ani_enum_item fileType);
    static void CloudFileCacheStop(ani_env *env, ani_object object, ani_string uri, ani_boolean needClean);
    static void CloudFileCacheStopBatch(ani_env *env, ani_object object, ani_double taskId, ani_boolean needClean);
    static void CloudFileCacheCleanCache(ani_env *env, ani_object object, ani_string uri);
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_CACHE_ANI_H