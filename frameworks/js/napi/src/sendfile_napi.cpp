/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "sendfile_napi.h"

#include "filemgmt_libn.h"
#include "sendfile.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement::LibN;
namespace {
    constexpr int32_t MAX_SEND_FILE_HAP_NUMBER = 50;
    constexpr int32_t SENDFILE_NAPI_BUF_LENGTH = 256;
    const std::string DfsAppUid{"SendFileTestUid"};
}

napi_value SendFile(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(DFS_ARG_CNT::FOUR, DFS_ARG_CNT::FIVE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    auto resultCode = std::make_shared<int32_t>();

    std::unique_ptr<char []> deviceId;
    std::vector<std::string> sourPath;
    std::vector<std::string> destPath;

    uint32_t fileCount = 0;

    std::tie(succ, deviceId, std::ignore) = NVal(env, funcArg[DFS_ARG_POS::FIRST]).ToUTF8String();
    napi_get_value_uint32(env, funcArg[DFS_ARG_POS::FOURTH], &fileCount);

    napi_value result;
    napi_status status;
    for (uint32_t i = 0; i < fileCount; ++i) {
        status = napi_get_element(env, funcArg[DFS_ARG_POS::SECOND], i, &result);
        if (napi_ok != status) {
            return nullptr;
        }
        std::unique_ptr<char []> path;
        std::tie(succ, path, std::ignore) = NVal(env, result).ToUTF8String();
        sourPath.push_back(path.get());

        status = napi_get_element(env, funcArg[DFS_ARG_POS::THIRD], i, &result);
        if (napi_ok != status) {
            return nullptr;
        }
        std::tie(succ, path, std::ignore) = NVal(env, result).ToUTF8String();
        destPath.push_back(path.get());
    }

    std::string deviceIdString(deviceId.get());
    auto cbExec = [deviceIdString, sourPath, destPath, fileCount, resultCode]() -> NError {
        *resultCode = ExecSendFile(deviceIdString.c_str(), sourPath, destPath, fileCount);
        LOGI("cbExec resultCode : %{public}d", *resultCode);
        return NError(*resultCode);
    };

    auto cbComplete = [resultCode](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        LOGI("cbComplete resultCode : %{public}d", *resultCode);
        return { NVal::CreateInt64(env, *resultCode) };
    };

    LOGI("JS SendFile created thread params count %{public}d", funcArg.GetArgc());
    std::string procedureName = "SendFile";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == DFS_ARG_CNT::FOUR) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else if (funcArg.GetArgc() == DFS_ARG_CNT::FIVE) {
        NVal cb(env, funcArg[DFS_ARG_POS::FIFTH]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }

    return NVal::CreateUndefined(env).val_;
}


napi_value JS_On(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    EventAgent* agent = nullptr;
    napi_unwrap(env, thisVar, (void**)&agent);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[1], &eventHandleType);
    NAPI_ASSERT(env, eventHandleType == napi_function, "type mismatch for parameter 2");

    char type[64] = { 0 };
    size_t typeLen = 0;

    napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);

    agent->On((const char*)type, argv[1]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value JS_Off(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    EventAgent* agent = nullptr;
    napi_unwrap(env, thisVar, (void**)&agent);

    NAPI_ASSERT(env, argc == requireArgc, "requires 1 parameter");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    char* type = nullptr;
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);

    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    type = new char[typeLen + 1];

    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    agent->Off(type);

    delete[] type;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value JS_Constructor(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr);

    char bundleName[SENDFILE_NAPI_BUF_LENGTH] = { 0 };
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], bundleName, sizeof(bundleName), &typeLen);
    LOGI("DEBUG_SENDFILE:JS_Constructor. %{public}s", bundleName);

    EventAgent* agent = new EventAgent(env, thisVar);
    {
        std::unique_lock<std::mutex> lock(g_uidMutex);
        if (g_mapUidToEventAgent.end() != g_mapUidToEventAgent.find(DfsAppUid)) {
            delete g_mapUidToEventAgent[DfsAppUid];
            g_mapUidToEventAgent.erase(DfsAppUid);
        }
        if (g_mapUidToEventAgent.size() <= MAX_SEND_FILE_HAP_NUMBER) {
            auto [ignored, inserted] = g_mapUidToEventAgent.insert(make_pair(DfsAppUid, agent));
            if (!inserted) {
                LOGE("map env to event agent error.");
                return nullptr;
            } else {
                LOGI("map size %{public}d", g_mapUidToEventAgent.size());
            }
        }
    }

    napi_wrap(env, thisVar, agent,
        [](napi_env env, void* data, void* hint) {
            auto iter = g_mapUidToEventAgent.find(DfsAppUid);
            if (g_mapUidToEventAgent.end() != iter) {
                auto agent = (EventAgent*)data;
                if (agent != nullptr) {
                    std::unique_lock<std::mutex> lock(g_uidMutex);
                    agent->ClearDevice();
                    g_mapUidToEventAgent.erase(DfsAppUid);
                    delete agent;
                }
            }
        },
        nullptr, nullptr);

    return thisVar;
}

/***********************************************
 * Module export and register
 ***********************************************/
napi_value SendFileExport(napi_env env, napi_value exports)
{
    const char className[] = "SendFile";
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("sendFile", SendFile),
        DECLARE_NAPI_FUNCTION("on", JS_On),
        DECLARE_NAPI_FUNCTION("off", JS_Off),
    };
    napi_value sendFileClass = nullptr;

    napi_define_class(env, className, sizeof(className), JS_Constructor, nullptr,
        sizeof(desc) / sizeof(desc[0]), desc, &sendFileClass);

    napi_set_named_property(env, exports, "SendFile", sendFileClass);

    RegisterSendFileNotifyCallback();
    return exports;
}

NAPI_MODULE(sendfile, SendFileExport)
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS