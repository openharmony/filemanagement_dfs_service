/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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


#ifndef FILEMANAGEMENT_DFS_SERVICE_SERVICE_COLLABORATION_MANAGER_CAPI_H
#define FILEMANAGEMENT_DFS_SERVICE_SERVICE_COLLABORATION_MANAGER_CAPI_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ServiceCollaborationManagerHardwareType {
    SCM_UNKNOWN_TYPE = 0,
    SCM_DISPLAY = 1,
    SCM_MIC = 2,
    SCM_SPEAKER = 3,
    SCM_CAMERA = 4,
} ServiceCollaborationManagerHardwareType;

typedef enum ServiceCollaborationManagerBussinessStatus {
    SCM_IDLE = 1,
    SCM_PREPARE = 2,
    SCM_CONNECTING = 3,
    SCM_CONNECTED = 4
} ServiceCollaborationManagerBussinessStatus;

typedef enum ServiceCollaborationManagerResultCode {
    PASS = 1004720001,
    REJECT = 1004720002
} ServiceCollaborationManagerResultCode;

typedef struct HMS_ServiceCollaborationManager_HardwareRequestInfo {
    ServiceCollaborationManagerHardwareType hardWareType;
    bool canShare;
} HMS_ServiceCollaborationManager_HardwareRequestInfo;

typedef struct HMS_ServiceCollaborationManager_CommunicationRequestInfo {
    int32_t minBandwidth;
    int32_t maxLatency;
    int32_t minLatency;
    int32_t maxWaitTime;
    const char *dataType;
} HMS_ServiceCollaborationManager_CommunicationRequestInfo;

typedef struct HMS_ServiceCollaborationManager_ResourceRequestInfoSets {
    uint32_t remoteHardwareListSize;
    HMS_ServiceCollaborationManager_HardwareRequestInfo *remoteHardwareList;
    uint32_t localHardwareListSize;
    HMS_ServiceCollaborationManager_HardwareRequestInfo *localHardwareList;
    HMS_ServiceCollaborationManager_CommunicationRequestInfo *communicationRequest;
} HMS_ServiceCollaborationManager_ResourceRequestInfoSets;

typedef struct HMS_ServiceCollaborationManager_Callback {
    int32_t (*OnStop)(const char *peerNetworkId);
    int32_t (*ApplyResult)(int32_t errorcode, int32_t result, const char *reason);
} HMS_ServiceCollaborationManager_Callback;

typedef struct HMS_ServiceCollaborationManager_API {
    int32_t (*HMS_ServiceCollaborationManager_PublishServiceState)(const char *peerNetworkId, const char *serviceName,
        const char *extraInfo, ServiceCollaborationManagerBussinessStatus state);
    int32_t (*HMS_ServiceCollaborationManager_ApplyAdvancedResource)(const char *peerNetworkId, const char *serviceName,
        HMS_ServiceCollaborationManager_ResourceRequestInfoSets *resourceRequest,
        HMS_ServiceCollaborationManager_Callback *callback);
    int32_t (*HMS_ServiceCollaborationManager_RegisterLifecycleCallback)(const char *serviceName,
        HMS_ServiceCollaborationManager_Callback *callback);
    int32_t (*HMS_ServiceCollaborationManager_UnRegisterLifecycleCallback)(const char *serviceName);
} HMS_ServiceCollaborationManager_API;

int32_t HMS_ServiceCollaborationManager_Export(HMS_ServiceCollaborationManager_API *exportapi);

#ifdef __cplusplus
}
#endif

#endif // FILEMANAGEMENT_DFS_SERVICE_SERVICE_COLLABORATION_MANAGER_CAPI_H
