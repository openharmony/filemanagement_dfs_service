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
 * @brief This file defines the error codes for the cloud disk management module.
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
 * @brief Enumerates the error codes of the cloud disk management module.
 *
 * @since 21
 */
typedef enum CloudDisk_ErrorCode {
    /**
     * @brief The API is called successfully.
     *
     * @since 21
     */
    CLOUD_DISK_OK = 0,
    /**
     * @brief The permission verification fails.
     *
     * @since 21
     */
    CLOUD_DISK_PERMISSION_DENIED = 201,
    /**
     * @brief This feature is not supported on the device.
     *
     * @since 21
     */
    CLOUD_DISK_NOT_SUPPORTED = 801,
    /**
     * @brief The input parameter is invalid.
     *
     * @since 21
     */
    CLOUD_DISK_INVALID_ARG = 34400001,
    /**
     * @brief The sync root path is not authorized.
     *
     * @since 21
     */
    CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED = 34400002,
    /**
     * @brief IPC connection fails.
     *
     * @since 21
     */
    CLOUD_DISK_IPC_FAILED = 34400003,
    /**
     * @brief The number of sync root paths exceeds the upper limit.
     *
     * @since 21
     */
    CLOUD_DISK_SYNC_FOLDER_LIMIT_EXCEEDED = 34400004,
    /**
     * @brief The sync root path conflicts with the existing sync root path of the application.
     *
     * @since 21
     */
    CLOUD_DISK_CONFLICT_THIS_APP = 34400005,
    /**
     * @brief The sync root path conflicts with the existing sync root path of another application.
     *
     * @since 21
     */
    CLOUD_DISK_CONFLICT_OTHER_APP = 34400006,
    /**
     * @brief The sync root path fails to be registered.
     *
     * @since 21
     */
    CLOUD_DISK_REGISTER_SYNC_FOLDER_FAILED = 34400007,
    /**
     * @brief The sync root path is not registered.
     *
     * @since 21
     */
    CLOUD_DISK_SYNC_FOLDER_NOT_REGISTERED = 34400008,
    /**
     * @brief The sync root path fails to be unregistered.
     *
     * @since 21
     */
    CLOUD_DISK_UNREGISTER_SYNC_FOLDER_FAILED = 34400009,
    /**
     * @brief The sync root path does not exist.
     *
     * @since 21
     */
    CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST = 34400010,
    /**
     * @brief The change listener is not registered.
     *
     * @since 21
     */
    CLOUD_DISK_LISTENER_NOT_REGISTERED = 34400011,
    /**
     * @brief The change listener has been registered.
     *
     * @since 21
     */
    CLOUD_DISK_LISTENER_ALREADY_REGISTERED = 34400012,
    /**
     * @brief The change sequence is invalid. You are advised to query all change sequences.
     *
     * @since 21
     */
    CLOUD_DISK_INVALID_CHANGE_SEQUENCE = 34400013,
    /**
     * @brief Temporary failure (for example, due to high underlying I/O load or insufficient memory). Please try again.
     *
     * @since 21
     */
    CLOUD_DISK_TRY_AGAIN = 34400014,
    /**
     * @brief This feature is not allowed on the current device.
     *
     * @since 21
     */
    CLOUD_DISK_NOT_ALLOWED = 34400015,
    /**
     * @brief A file with the same name already exists in the target path.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_FILE_ALREADY_EXISTS = 34400016,
    /**
     * @brief The target path is not a placeholder file.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_NOT_A_PLACEHOLDER = 34400017,
    /**
     * @brief The target path is already a placeholder file.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_IS_A_PLACEHOLDER = 34400018,
    /**
     * @brief Hydration in progress.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_HYDRATE_IN_PROGRESS = 34400019,
    /**
     * @brief The available disk space is insufficient.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_NO_SPACE_LEFT = 34400020,
    /**
     * @brief The callback table is not registered.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED = 34400021,
    /**
     * @brief The callback table has been registered.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED = 34400022,
    /**
     * @brief The parent directory of the target path is not a directory.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_NOT_A_DIRECTORY = 34400023,
    /**
     * @brief The target path does not exist.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_FILE_NOT_EXIST = 34400024,
    /**
     * @brief The file name or path is too long.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_NAME_TOO_LONG = 34400025,
    /**
     * @brief The file is too large.
     *
     * @since 26.1.0
     */
    OH_CLOUD_DISK_FILE_TOO_LARGE = 34400026
} CloudDisk_ErrorCode;

#ifdef __cplusplus
}
#endif
/** @} */
#endif // FILEMANAGEMENT_KIT_OH_CLOUD_DISK_MANAGER_ERROR_CODE_H
