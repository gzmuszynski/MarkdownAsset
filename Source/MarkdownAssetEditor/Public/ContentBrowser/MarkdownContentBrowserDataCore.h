// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ContentBrowserItemData.h"
#include "MarkdownContentBrowserHierarchy.h"

class FMarkdownContentBrowserFileItemDataPayload;
class FMarkdownContentBrowserFolderItemDataPayload;

class IAssetTypeActions;
class FAssetThumbnail;
class UContentBrowserDataSource;

namespace MarkdownContentBrowserData
{

	MARKDOWNASSETEDITOR_API bool IsEngineDocumentation(const FName InPath);

	MARKDOWNASSETEDITOR_API bool IsProjectDocumentation(const FName InPath);

	MARKDOWNASSETEDITOR_API bool IsPluginDocumentation(const FName InPath);

	MARKDOWNASSETEDITOR_API FContentBrowserItemData CreateMarkdownFolderItem(UContentBrowserDataSource* InOwnerDataSource, const FName InVirtualPath, const FName InFolderPath, const bool bIsFromPlugin);

	MARKDOWNASSETEDITOR_API FContentBrowserItemData CreateMarkdownFileItem(UContentBrowserDataSource* InOwnerDataSource, const FName InVirtualPath, const FName InClassPath, UMarkdownFile*
	                                                                    InClass, const bool bIsFromPlugin);

	MARKDOWNASSETEDITOR_API TSharedPtr<const FMarkdownContentBrowserFolderItemDataPayload> GetMarkdownFolderItemPayload(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem);

	MARKDOWNASSETEDITOR_API TSharedPtr<const FMarkdownContentBrowserFileItemDataPayload> GetMarkdownFileItemPayload(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem);
	
	MARKDOWNASSETEDITOR_API void EnumerateMarkdownFolderItemPayloads(const UContentBrowserDataSource* InOwnerDataSource, TArrayView<const FContentBrowserItemData> InItems, TFunctionRef<bool(const TSharedRef<const FMarkdownContentBrowserFolderItemDataPayload>&)> InFolderPayloadCallback);

	MARKDOWNASSETEDITOR_API void EnumerateMarkdownFileItemPayloads(const UContentBrowserDataSource* InOwnerDataSource, TArrayView<const FContentBrowserItemData> InItems, TFunctionRef<bool(const TSharedRef<const FMarkdownContentBrowserFileItemDataPayload>&)> InClassPayloadCallback);

	MARKDOWNASSETEDITOR_API void EnumerateMarkdownItemPayloads(const UContentBrowserDataSource* InOwnerDataSource, TArrayView<const FContentBrowserItemData> InItems, TFunctionRef<bool(const TSharedRef<const FMarkdownContentBrowserFolderItemDataPayload>&)> InFolderPayloadCallback, TFunctionRef<bool(const TSharedRef<const FMarkdownContentBrowserFileItemDataPayload>&)> InClassPayloadCallback);

	MARKDOWNASSETEDITOR_API void SetOptionalErrorMessage(FText* OutErrorMsg, FText InErrorMsg);

	MARKDOWNASSETEDITOR_API bool CanEditItem(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FText* OutErrorMsg);

	MARKDOWNASSETEDITOR_API bool CanEditMarkdownFileItem(const FMarkdownContentBrowserFileItemDataPayload& InClassPayload, FText* OutErrorMsg);

	MARKDOWNASSETEDITOR_API bool EditItems(IAssetTypeActions* InClassTypeActions, const UContentBrowserDataSource* InOwnerDataSource, TArrayView<const FContentBrowserItemData> InItems);

	MARKDOWNASSETEDITOR_API bool EditMarkdownFileItems(IAssetTypeActions* InMarkdownTypeActions, TArrayView<const TSharedRef<const FMarkdownContentBrowserFileItemDataPayload>> InMarkdownPayloads);

	MARKDOWNASSETEDITOR_API bool UpdateItemThumbnail(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FAssetThumbnail& InThumbnail);

	MARKDOWNASSETEDITOR_API bool UpdateMarkdownFileItemThumbnail(const FMarkdownContentBrowserFileItemDataPayload& InClassPayload, FAssetThumbnail& InThumbnail);

	MARKDOWNASSETEDITOR_API bool AppendItemReference(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FString& InOutStr);

	MARKDOWNASSETEDITOR_API bool AppendItemObjectPath(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FString& InOutStr);

	MARKDOWNASSETEDITOR_API bool AppendItemPackageName(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FString& InOutStr);

	MARKDOWNASSETEDITOR_API bool AppendMarkdownFileItemReference(const FMarkdownContentBrowserFileItemDataPayload& InClassPayload, FString& InOutStr);

	MARKDOWNASSETEDITOR_API bool AppendObjectPathFileItemReference(const FMarkdownContentBrowserFileItemDataPayload& InClassPayload, FString& InOutStr);

	MARKDOWNASSETEDITOR_API bool AppendPackageNameItemReference(const FMarkdownContentBrowserFileItemDataPayload& InClassPayload, FString& InOutStr);

	MARKDOWNASSETEDITOR_API bool GetItemPhysicalPath(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FString& OutDiskPath);

	MARKDOWNASSETEDITOR_API bool GetMarkdownFolderItemPhysicalPath(const FMarkdownContentBrowserFolderItemDataPayload& InFolderPayload, FString& OutDiskPath);

	MARKDOWNASSETEDITOR_API bool GetMarkdownFileItemPhysicalPath(const FMarkdownContentBrowserFileItemDataPayload& InClassPayload, FString& OutDiskPath);

	MARKDOWNASSETEDITOR_API bool GetItemAttribute(IAssetTypeActions* InClassTypeActions, const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue);

	MARKDOWNASSETEDITOR_API bool GetMarkdownFolderItemAttribute(const FMarkdownContentBrowserFolderItemDataPayload& InFolderPayload, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue);

	MARKDOWNASSETEDITOR_API bool GetMarkdownFileItemAttribute(IAssetTypeActions* InClassTypeActions, const FMarkdownContentBrowserFileItemDataPayload& InClassPayload, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue);

	MARKDOWNASSETEDITOR_API bool GetItemAttributes(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const bool InIncludeMetaData, FContentBrowserItemDataAttributeValues& OutAttributeValues);

	MARKDOWNASSETEDITOR_API bool GetMarkdownFileItemAttributes(const FMarkdownContentBrowserFileItemDataPayload& InClassPayload, const bool InIncludeMetaData, FContentBrowserItemDataAttributeValues& OutAttributeValues);

}
