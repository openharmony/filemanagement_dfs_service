#include "sdk_helper.h"

#include "dfs_error.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

int32_t SdkHelper::Init(const int32_t userId, const std::string &bundleName)
{
    return E_OK;
}

int32_t SdkHelper::GetLock(DriveKit::DKLock &lock)
{
    return E_OK;
}

void SdkHelper::DeleteLock(DriveKit::DKLock &lock)
{
    return;
}

int32_t SdkHelper::FetchRecords(shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor cursor,
    function<void(std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        const DriveKit::DKError &)> callback)
{
    return E_OK;
}

int32_t SdkHelper::FetchRecordWithId(std::shared_ptr<DriveKit::DKContext> context, DriveKit::DKRecordId recordId,
    FetchRecordCallback callback)
{
    return E_OK;
}

int32_t SdkHelper::FetchDatabaseChanges(std::shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor cursor,
    std::function<void(const std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<std::vector<DriveKit::DKRecord>>, DriveKit::DKQueryCursor,
        bool, const DriveKit::DKError &)> callback)
{
    return E_OK;
}

int32_t SdkHelper::CreateRecords(shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKRecord> &records,
    std::function<void(std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &)> callback)
{
    return E_OK;
}
int32_t SdkHelper::DeleteRecords(shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKRecord> &records,
    std::function<void(std::shared_ptr<DriveKit::DKContext>, std::shared_ptr<const DriveKit::DKDatabase>,
        std::shared_ptr<const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>,
        const DriveKit::DKError &)> callback)
{
    return E_OK;
}

int32_t SdkHelper::ModifyRecords(shared_ptr<DriveKit::DKContext> context,
    vector<DriveKit::DKRecord> &records, DriveKit::DKDatabase::ModifyRecordsCallback callback)
{
    return E_OK;
}

int32_t SdkHelper::DownloadAssets(shared_ptr<DriveKit::DKContext> context,
    std::vector<DriveKit::DKDownloadAsset> &assetsToDownload, DriveKit::DKAssetPath downLoadPath,
    DriveKit::DKDownloadId &id,
    std::function<void(std::shared_ptr<DriveKit::DKContext>,
                       std::shared_ptr<const DriveKit::DKDatabase>,
                       const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                       const DriveKit::DKError &)> resultCallback,
    std::function<void(std::shared_ptr<DriveKit::DKContext>, DriveKit::DKDownloadAsset,
                       DriveKit::TotalSize, DriveKit::DownloadSize)> progressCallback)
{
    return E_OK;
}

int32_t SdkHelper::CancelDownloadAssets(int32_t id)
{
    return E_OK;
}

int32_t SdkHelper::GetStartCursor(shared_ptr<DriveKit::DKContext> context, DriveKit::DKQueryCursor &cursor)
{
    return E_OK;
}

std::shared_ptr<DriveKit::DKAssetReadSession> SdkHelper::GetAssetReadSession(DriveKit::DKRecordType recordType,
                                                                             DriveKit::DKRecordId recordId,
                                                                             DriveKit::DKFieldKey assetKey,
                                                                             DriveKit::DKAssetPath assetPath)
{
    database_ = new DriveKit::DKDatabase();
    return database_->NewAssetReadSession(recordType, recordId, assetKey, assetPath);
}
}
}
}