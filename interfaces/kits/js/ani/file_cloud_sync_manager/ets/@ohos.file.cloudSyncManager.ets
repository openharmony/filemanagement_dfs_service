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

import { BusinessError, AsyncCallback } from "@ohos.base";

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