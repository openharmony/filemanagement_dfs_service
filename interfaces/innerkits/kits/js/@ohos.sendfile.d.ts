/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
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

import {AsyncCallback, Callback} from "./basic";
/**
 * Provides Environment APIs.
 *
 * @since 8
 * @syscap SystemCapability.FileManagement.File.DistributeFile
 */
declare namespace SendFile {
  /**
   * send file.
   *
   * @since 8
   */
  function sendFile(deviceId: string, sourPath: Array<string>, destPath: Array<string>, fileCount: number, callback: AsyncCallback<number>);
  function sendFile(deviceId: string, sourPath: Array<string>, destPath: Array<string>, fileCount: number): Promise<number>;

  export interface TransResult {
    /**
     * send file error code.
     */
    errCode: number;
    /**
     * receive file full path.
     */
    fileName: Array<string>;
    /**
     * receive file count.
     */
    fileCount: number;
  }
  /**
   * event listen on.
   *
   * @since 8
   */
  function on(type: 'sendFinished' | 'receiveFinished', callback: AsyncCallback<TransResult>): void;
  /**
   * event listen off.
   *
   * @since 8
   */
  function off(type: 'sendFinished' | 'receiveFinished'): void;
}

export default SendFile;