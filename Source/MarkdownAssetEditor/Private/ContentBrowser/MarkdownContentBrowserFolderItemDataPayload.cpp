// Copyright Epic Games, Inc. All Rights Reserved.


#include "ContentBrowser/MarkdownContentBrowserFolderItemDataPayload.h"

#include "Misc/Paths.h"
#include "AssetThumbnail.h"
#include "ContentBrowserDataSource.h"


FMarkdownContentBrowserFileItemDataPayload::FMarkdownContentBrowserFileItemDataPayload(const FName& InInternalPath, UClass* InClass):
	InternalPath(InInternalPath),
	Class(InClass),
	AssetData(InClass)
{
}

bool FMarkdownContentBrowserFileItemDataPayload::GetItemAttribute(const FName& InAttributeKey,
												   FContentBrowserItemDataAttributeValue& OutAttributeValue)
{
	if (InAttributeKey == ContentBrowserItemAttributes::ItemTypeDisplayName)
	{
		OutAttributeValue.SetValue(TEXT("Documentation"));

		return true;
	}

	return false;
}

const FName& FMarkdownContentBrowserFileItemDataPayload::GetInternalPath() const
{
	return InternalPath;
}

UClass* FMarkdownContentBrowserFileItemDataPayload::GetClass() const
{
	return Class.Get();
}

const FAssetData& FMarkdownContentBrowserFileItemDataPayload::GetAssetData() const
{
	return AssetData;
}

void FMarkdownContentBrowserFileItemDataPayload::UpdateThumbnail(FAssetThumbnail& OutThumbnail) const
{
	OutThumbnail.SetAsset(AssetData);
}
