// Copyright Epic Games, Inc. All Rights Reserved.


#include "ContentBrowser/MarkdownContentBrowserFolderItemDataPayload.h"

#include "Misc/Paths.h"
#include "AssetThumbnail.h"
#include "ContentBrowserDataSource.h"
#include "MarkdownAsset.h"
#include "ContentBrowser/MarkdownContentBrowserHierarchy.h"


FMarkdownContentBrowserFileItemDataPayload::FMarkdownContentBrowserFileItemDataPayload(const FName& InInternalPath, UMarkdownFile* InClass):
	InternalPath(InInternalPath),
	MDFile(InClass),
	AssetData(NewObject<UMarkdownAsset>(GetTransientPackage(), FName(InClass->GetName() + "_Asset")))
{
}

bool FMarkdownContentBrowserFileItemDataPayload::GetItemAttribute(const FName& InAttributeKey,
												   FContentBrowserItemDataAttributeValue& OutAttributeValue)
{
	if (InAttributeKey == ContentBrowserItemAttributes::ItemTypeDisplayName)
	{
		OutAttributeValue.SetValue(TEXT("Markdown Document"));

		return true;
	}

	return false;
}

const FName& FMarkdownContentBrowserFileItemDataPayload::GetInternalPath() const
{
	return InternalPath;
}

UMarkdownFile* FMarkdownContentBrowserFileItemDataPayload::GetMDFile() const
{
	return MDFile;
}

const FAssetData& FMarkdownContentBrowserFileItemDataPayload::GetAssetData() const
{
	return AssetData;
}

void FMarkdownContentBrowserFileItemDataPayload::UpdateThumbnail(FAssetThumbnail& OutThumbnail) const
{
	OutThumbnail.SetAsset(AssetData);
}
