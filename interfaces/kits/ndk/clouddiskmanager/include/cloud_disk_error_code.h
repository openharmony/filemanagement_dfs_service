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

/**
 * @addtogroup CloudDisk
 * @{
 *
 * @brief Provides APIs and error code for managing cloud disks.
 * @since 21
 * @version 1.0
 */

/**
 * @file cloud_disk_error_code.h
 *
 * @brief Provides the definitions of cloud disk manager error codes.
 *
 * @library libohclouddiskmanager.so
 * @kit CoreFileKit
 * @syscap SystemCapability.FileManagement.CloudDiskManager
 * @since 21
 * @version 1.0
 */

#ifndef FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_ERROR_CODE_H
#define FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines error codes for the cloud disk synchronization manager feature.
 * @since 21
 */
typedef enum CloudDisk_ErrorCode {
    /**
     * @error Operation completed successfully.
     */
    CLOUD_DISK_OK = 0,
    /**
     * @error Permission verification failed.
     */
    CLOUD_DISK_PERMISSION_DENIED = 201,
    /**
     * @error Device not supported.
     */
    CLOUD_DISK_NOT_SUPPORTED = 801,
    /**
     * @error Invalid input parameter.
     */
    CLOUD_DISK_INVALID_ARG = 34400001,
    /**
     * @error Sync folder path unauthorized.
     */
    CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED = 34400002,
    /**
     * @error Inter-process communication (IPC) failure.
     */
    CLOUD_DISK_IPC_FAILED = 34400003,
    /**
     * @error Sync folder exceeds the allowed limit.
     */
    CLOUD_DISK_SYNC_FOLDER_LIMIT_EXCEEDED = 34400004,
    /**
     * @error Sync folder conflicts with an existing sync folder of this application.
     */
    CLOUD_DISK_CONFLICT_THIS_APP = 34400005,
    /**
     * @error Sync folder conflicts with an existing sync folder of another application.
     */
    CLOUD_DISK_CONFLICT_OTHER_APP = 34400006,
    /**
     * @error Failed to register sync folder.
     */
    CLOUD_DISK_REGISTER_SYNC_FOLDER_FAILED = 34400007,
    /**
     * @error Sync folder is not registered.
     */
    CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED = 34400008,
    /**
     * @error Failed to unregister sync folder.
     */
    CLOUD_DISK_UNREGISTER_SYNC_FOLDER_FAILED = 34400009,
    /**
     * @error Sync folder path does not exist.
     */
    CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST = 34400010,
    /**
     * @error Change listener is not registered.
     */
    CLOUD_DISK_LISTENER_NOT_REGISTERED = 34400011,
    /**
     * @error Change listener is already registered.
     */
    CLOUD_DISK_LISTENER_ALREADY_REGISTERED = 34400012,
    /**
     * @error Invalid change sequence. Full query is recommended.
     */
    CLOUD_DISK_INVALID_CHANGE_SEQUENCE = 34400013,
    /**
     * @error Temporary failure. Retry is recommended (e.g., network issues).
     */
    CLOUD_DISK_TRY_AGAIN = 34400014,
    /**
     * @error Cloud disk is not allowed on this device.
     */
    CLOUD_DISK_NOT_ALLOWED = 34400015,
} CloudDisk_ErrorCode;

#ifdef __cplusplus
}
#endif
/** @} */
#endif // FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_ERROR_CODE_H
