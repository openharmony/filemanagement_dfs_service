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

import { BusinessError, AsyncCallback, Callback } from "@ohos.base";

export default namespace cloudSyncManager  {

    export enum Action {
        RETAIN_DATA = 0,
        CLEAR_DATA = 1
    }

    export interface ExtraData {
        eventId: string;
        extraData: string;
    }

    export class ExtraDataInner implements ExtraData {
        constructor(
            eventId: string,
            extraData: string
        ){
            this.eventId = eventId;
            this.extraData = extraData;
        }
        eventId: string;
        extraData: string;
    }

    export enum DownloadStopReason {
        NO_STOP = 0,
        NETWORK_UNAVAILABLE = 1,
        LOCAL_STORAGE_FULL = 2,
        TEMPERATURE_LIMIT = 3,
        USER_STOPPED = 4,
        APP_UNLOAD = 5,
        OTHER_REASON = 6
    }

    export enum DownloadState {
        RUNNING = 0,
        COMPLETED = 1,
        STOPPED = 2
    }

    export interface CloudFileInfo {
        cloudFileCount: int;
        cloudFileTotalSize: long;
        localFileCount: int;
        localFileTotalSize: long;
        bothFileCount: int;
        bothFileTotalSize: long;
    }

    class CloudFileInfoInner implements CloudFileInfo {
        constructor(
            cloudFileCount: int,
            cloudFileTotalSize: long,
            localFileCount: int,
            localFileTotalSize: long,
            bothFileCount: int,
            bothFileTotalSize: long
        ) {
            this.cloudFileCount = cloudFileCount;
            this.cloudFileTotalSize = cloudFileTotalSize;
            this.localFileCount = localFileCount;
            this.localFileTotalSize = localFileTotalSize;
            this.bothFileCount = bothFileCount;
            this.bothFileTotalSize = bothFileTotalSize;
        }
        cloudFileCount: int;
        cloudFileTotalSize: long;
        localFileCount: int;
        localFileTotalSize: long;
        bothFileCount: int;
        bothFileTotalSize: long;
    }

    export class DownloadProgress {
        constructor(
            state: DownloadState,
            successfulCount: int,
            failedCount: int,
            totalCount: int,
            downloadedSize: long,
            totalSize: long,
            stopReason: DownloadStopReason
        ) {
            this.state = state;
            this.successfulCount = successfulCount;
            this.failedCount = failedCount;
            this.totalCount = totalCount;
            this.downloadedSize = downloadedSize;
            this.totalSize = totalSize;
            this.stopReason = stopReason;
        }
        state: DownloadState;
        successfulCount: int;
        failedCount: int;
        totalCount: int;
        downloadedSize: long;
        totalSize: long;
        stopReason: DownloadStopReason;
    }

    export class DowngradeDownload {
        static {
            loadLibraryWithPermissionCheck("cloud_sync_manager_ani", "@ohos.file.cloudSyncManager");
        }
        private nativePtr: long = 0;
        native constructor(bundleName: string);

        bindNativePtr(ptr: long): void {
            if (this.nativePtr == 0) {
                this.nativePtr = ptr;
            }
        }

        getCloudFileInfo(): Promise<CloudFileInfo> {
            return new Promise<CloudFileInfo>((resolve: (result: CloudFileInfo) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((): CloudFileInfo => this.getCloudFileInfoInner());
                promise.then<void, void>((ret: Any): void => {
                    resolve(ret as CloudFileInfo);
                }, (e: Error): void => {
                    reject(e as BusinessError<void>);
                });
            });
        }

        startDownload(callback: Callback<DownloadProgress>): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((
                    callbackIn: Callback<DownloadProgress>
                ): int => {
                    this.startDownloadInner(callbackIn);
                    return 0;
                }, callback);
                promise.then<void, void>((ret: Any): void => {
                    resolve(undefined);
                }, (e: Error): void => {
                    reject(e as BusinessError<void>);
                });
            });
        }

        stopDownload(): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((): int => this.stopDownloadInner());
                promise.then<void, void>((ret: Any): void => {
                    resolve(undefined);
                }, (e: Error): void => {
                    reject(e as BusinessError<void>);
                });
            });
        }

        native DowngradeGetCloudFileInfo(): CloudFileInfo;
        native DowngradeStartDownload(callback: Callback<DownloadProgress>): void;
        native DowngradeStopDownload(): void;

        getCloudFileInfoInner(): CloudFileInfo {
            return this.DowngradeGetCloudFileInfo();
        }

        startDownloadInner(callback: Callback<DownloadProgress>): int {
            this.DowngradeStartDownload(callback);
            return 0;
        }

        stopDownloadInner(): int {
            this.DowngradeStopDownload();
            return 0;
        }
    }

    class StaticFunction {
        static {
            loadLibraryWithPermissionCheck("cloud_sync_manager_ani", "@ohos.file.cloudSyncManager");
        }
        static native changeAppCloudSwitchInner(accountId: string, bundleName: string, status: boolean): void;
        static native notifyDataChangeInner(accountId: string, bundleName: string): void;
        static native enableCloudInner(accountId: string, switches: Record<string, boolean>): void;
        static native disableCloudInner(accountId: string): void;
        static native cleanInner(accountId: string, appActions: Record<string, Action>): void;
        static native notifyEventChangeInner(userId: int, extraData: ExtraData): void;
        static native getBundlesLocalFilePresentStatusInner(bundleNames: Array<string>): Array<LocalFilePresentStatus>;
    };

    export function changeAppCloudSwitch(accountId: string, bundleName: string, status: boolean): Promise<void> {
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError) => void): void => {
            let promise = taskpool.execute((
                accountId: string,
                bundleName: string,
                status: boolean
            ): int => {
                StaticFunction.changeAppCloudSwitchInner(accountId, bundleName, status);
                return 0;
            }, accountId, bundleName, status);
            promise.then<void, void>((): void => {
                resolve(undefined);
            }, (e: Error): void => {
                reject(e as BusinessError<void>);
            });
        });
    }

    export function changeAppCloudSwitch(
        accountId: string,
        bundleName: string,
        status: boolean,
        callback: AsyncCallback<void>
    ): void {
        let promise = taskpool.execute((
            accountId: string,
            bundleName: string,
            status: boolean
        ): int => {
            StaticFunction.changeAppCloudSwitchInner(accountId, bundleName, status);
            return 0;
        }, accountId, bundleName, status);
        promise.then<void, void>((): void => {
            callback(null, undefined);
        }, (e: Error): void => {
            callback(e as BusinessError<void>, undefined);
        });
    }

    export function notifyDataChange(accountId: string, bundleName: string): Promise<void> {
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError) => void): void => {
            let promise = taskpool.execute((
                accountId: string,
                bundleName: string
            ): int => {
                StaticFunction.notifyDataChangeInner(accountId, bundleName);
                return 0;
            }, accountId, bundleName);
            promise.then<void, void>((): void => {
                resolve(undefined);
            }, (e: Error): void => {
                reject(e as BusinessError<void>);
            });
        });
    }

    export function notifyDataChange(accountId: string, bundleName: string, callback: AsyncCallback<void>): void {
        let promise = taskpool.execute((
            accountId: string,
            bundleName: string
        ): int => {
            StaticFunction.notifyDataChangeInner(accountId, bundleName);
            return 0;
        }, accountId, bundleName);
        promise.then<void, void>((): void => {
            callback(null, undefined);
        }, (e: Error): void => {
            callback(e as BusinessError<void>, undefined);
        });
    }

    export function enableCloud(accountId: string, switches: Record<string, boolean>): Promise<void> {
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError) => void): void => {
            let promise = taskpool.execute((
                accountId: string,
                switches: Record<string, boolean>
            ): int => {
                StaticFunction.enableCloudInner(accountId, switches);
                return 0;
            }, accountId, switches);
            promise.then<void, void>((): void => {
                resolve(undefined);
            }, (e: Error): void => {
                reject(e as BusinessError<void>);
            });
        });
    }

    export function enableCloud(accountId: string, switches: Record<string, boolean>, callback: AsyncCallback<void>): void {
        let promise = taskpool.execute((
            accountId: string,
            switches: Record<string, boolean>
        ): int => {
            StaticFunction.enableCloudInner(accountId, switches);
            return 0;
        }, accountId, switches);
        promise.then<void, void>((): void => {
            callback(null, undefined);
        }, (e: Error): void => {
            callback(e as BusinessError<void>, undefined);
        });
    }

    export function disableCloud(accountId: string): Promise<void> {
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError) => void): void => {
            let promise = taskpool.execute((
                accountId: string
            ): int => {
                StaticFunction.disableCloudInner(accountId);
                return 0;
            }, accountId);
            promise.then<void, void>((): void => {
                resolve(undefined);
            }, (e: Error): void => {
                reject(e as BusinessError<void>);
            });
        });
    }

    export function disableCloud(accountId: string, callback: AsyncCallback<void>): void {
        let promise = taskpool.execute((
            accountId: string
        ): int => {
            StaticFunction.disableCloudInner(accountId);
            return 0;
        }, accountId);
        promise.then<void, void>((): void => {
            callback(null, undefined);
        }, (e: Error): void => {
            callback(e as BusinessError<void>, undefined);
        });
    }

    export function clean(accountId: string, appActions: Record<string, Action>): Promise<void> {
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError) => void): void => {
            let promise = taskpool.execute((
                accountId: string,
                appActions: Record<string, Action>
            ): int => {
                StaticFunction.cleanInner(accountId, appActions);
                return 0;
            }, accountId, appActions);
            promise.then<void, void>((): void => {
                resolve(undefined);
            }, (e: Error): void => {
                reject(e as BusinessError<void>);
            });
        });
    }

    export function clean(accountId: string, appActions: Record<string, Action>, callback: AsyncCallback<void>): void {
        let promise = taskpool.execute((
            accountId: string,
            appActions: Record<string, Action>
        ): int => {
            StaticFunction.cleanInner(accountId, appActions);
            return 0;
        }, accountId, appActions);
        promise.then<void, void>((): void => {
            callback(null, undefined);
        }, (e: Error): void => {
            callback(e as BusinessError<void>, undefined);
        });
    }

    export function notifyDataChange(userId: int, extraData: ExtraData): Promise<void> {
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError) => void): void => {
            let promise = taskpool.execute((
                userId: int,
                extraData: ExtraData
            ): int => {
                StaticFunction.notifyEventChangeInner(userId, extraData);
                return 0;
            }, userId, extraData);
            promise.then<void, void>((): void => {
                resolve(undefined);
            }, (e: Error): void => {
                reject(e as BusinessError<void>);
            });
        });
    }

    export function notifyDataChange(userId: int, extraData: ExtraData, callback: AsyncCallback<void>): void {
        let promise = taskpool.execute((
            userId: int,
            extraData: ExtraData
        ): int => {
            StaticFunction.notifyEventChangeInner(userId, extraData);
            return 0;
        }, userId, extraData);
        promise.then<void, void>((): void => {
            callback(null, undefined);
        }, (e: Error): void => {
            callback(e as BusinessError<void>, undefined);
        });
    }

    export interface LocalFilePresentStatus {
        bundleName: string;
        isLocalFilePresent: boolean;
    }

    class LocalFilePresentStatusInner implements LocalFilePresentStatus {
        constructor(
            bundleName: string,
            isLocalFilePresent: boolean
        ) {
            this.bundleName = bundleName;
            this.isLocalFilePresent = isLocalFilePresent;
        }
        public bundleName: string;
        public isLocalFilePresent: boolean;
    }

    export function GetBundlesLocalFilePresentStatus(bundleNames: Array<string>): 
        Promise<Array<LocalFilePresentStatus>> {
        return new Promise<Array<LocalFilePresentStatus>>((resolve: (result: Array<LocalFilePresentStatus>) => void,
            reject: (e: BusinessError<void>) => void): void => {
            let promise = taskpool.execute((
                bundleNames: Array<string>
            ): Array<LocalFilePresentStatus> => {
                return StaticFunction.getBundlesLocalFilePresentStatusInner(bundleNames);
            }, bundleNames);
            promise.then<void, void>((ret: Any): void => {
                resolve(ret as Array<LocalFilePresentStatus>);
            }, (e: Error): void => {
                reject(e as BusinessError<void>);
            });
        });
    }

}
