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
        cloudFileCount: number;
        cloudFileTotalSize: number;
        localFileCount: number;
        localFileTotalSize: number;
        bothFileCount: number;
        bothFileTotalSize: number;
    }

    class CloudFileInfoInner implements CloudFileInfo {
        constructor(
            cloudFileCount: number,
            cloudFileTotalSize: number,
            localFileCount: number,
            localFileTotalSize: number,
            bothFileCount: number,
            bothFileTotalSize: number
        ) {
            this.cloudFileCount = cloudFileCount;
            this.cloudFileTotalSize = cloudFileTotalSize;
            this.localFileCount = localFileCount;
            this.localFileTotalSize = localFileTotalSize;
            this.bothFileCount = bothFileCount;
            this.bothFileTotalSize = bothFileTotalSize;
        }
        cloudFileCount: number;
        cloudFileTotalSize: number;
        localFileCount: number;
        localFileTotalSize: number;
        bothFileCount: number;
        bothFileTotalSize: number;
    }

    export class DownloadProgress {
        constructor(
            state: DownloadState,
            successfulCount: number,
            failedCount: number,
            totalCount: number,
            downloadedSize: number,
            totalSize: number,
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
        successfulCount: number;
        failedCount: number;
        totalCount: number;
        downloadedSize: number;
        totalSize: number;
        stopReason: DownloadStopReason;
    }

    export class DowngradeDownload {
        static {
            loadLibrary("cloud_sync_manager_ani");
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
                reject: (e: BusinessError<void>) => void): CloudFileInfo => {
                let promise = taskpool.execute((): CloudFileInfo => this.getCloudFileInfoInner());
                promise.then<CloudFileInfo>((ret: CloudFileInfo): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
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
                promise.then<void>((ret: undefined): void => {
                resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
                });
            });
        }

        stopDownload(): Promise<void> {
            return new Promise<void>((resolve: (result: undefined) => void,
                reject: (e: BusinessError<void>) => void): void => {
                let promise = taskpool.execute((): int => this.stopDownloadInner());
                promise.then<void>((ret: undefined): void => {
                    resolve(ret);
                }, (e: BusinessError<void>): void => {
                    reject(e);
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
            loadLibrary("cloud_sync_manager_ani");
        }
        static native changeAppCloudSwitchInner(accountId: string, bundleName: string, status: boolean): void;
        static native notifyDataChangeInner(accountId: string, bundleName: string): void;
        static native enableCloudInner(accountId: string, switches: Record<string, boolean>): void;
        static native disableCloudInner(accountId: string): void;
        static native cleanInner(accountId: string, appActions: Record<string, Action>): void;
        static native notifyEventChangeInner(userId: number, extraData: ExtraData): void;
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
            promise.then<void>((): void => {
                resolve(undefined);
            }, (e: BusinessError<void>): void => {
                reject(e);
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
        promise.then<void>((): void => {
            let e = new BusinessError<void>();
            e.code = 0;
            callback(e, undefined);
        }, (e: BusinessError<void>): void => {
            callback(e, undefined);
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
            promise.then<void>((): void => {
                resolve(undefined);
            }, (e: BusinessError<void>): void => {
                reject(e);
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
        promise.then<void>((): void => {
            let e = new BusinessError<void>();
            e.code = 0;
            callback(e, undefined);
        }, (e: BusinessError<void>): void => {
            callback(e, undefined);
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
            promise.then<void>((): void => {
                resolve(undefined);
            }, (e: BusinessError<void>): void => {
                reject(e);
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
        promise.then<void>((): void => {
            let e = new BusinessError<void>();
            e.code = 0;
            callback(e, undefined);
        }, (e: BusinessError<void>): void => {
            callback(e, undefined);
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
            promise.then<void>((): void => {
                resolve(undefined);
            }, (e: BusinessError<void>): void => {
                reject(e);
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
        promise.then<void>((): void => {
            let e = new BusinessError<void>();
            e.code = 0;
            callback(e, undefined);
        }, (e: BusinessError<void>): void => {
            callback(e, undefined);
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
            promise.then<void>((): void => {
                resolve(undefined);
            }, (e: BusinessError<void>): void => {
                reject(e);
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
        promise.then<void>((): void => {
            let e = new BusinessError<void>();
            e.code = 0;
            callback(e, undefined);
        }, (e: BusinessError<void>): void => {
            callback(e, undefined);
        });
    }

    export function notifyDataChange(userId: number, extraData: ExtraData): Promise<void> {
        return new Promise<void>((resolve: (result: undefined) => void,
            reject: (e: BusinessError) => void): void => {
            let promise = taskpool.execute((
                userId: number,
                extraData: ExtraData
            ): int => {
                StaticFunction.notifyEventChangeInner(userId, extraData);
                return 0;
            }, userId, extraData);
            promise.then<void>((): void => {
                resolve(undefined);
            }, (e: BusinessError<void>): void => {
                reject(e);
            });
        });
    }

    export function notifyDataChange(userId: number, extraData: ExtraData, callback: AsyncCallback<void>): void {
        let promise = taskpool.execute((
            userId: number,
            extraData: ExtraData
        ): int => {
            StaticFunction.notifyEventChangeInner(userId, extraData);
            return 0;
        }, userId, extraData);
        promise.then<void>((): void => {
            let e = new BusinessError<void>();
            e.code = 0;
            callback(e, undefined);
        }, (e: BusinessError<void>): void => {
            callback(e, undefined);
        });
    }
}