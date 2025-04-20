// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ContentBrowserItemData.h"
#include "AssetRegistry/AssetData.h"

class FMarkdownContentBrowserFolderItemDataPayload final : public IContentBrowserItemDataPayload
{
public:
	explicit FMarkdownContentBrowserFolderItemDataPayload(const FName& InInternalPath):
		InternalPath(InInternalPath)
	{
	}

	const FName& GetInternalPath() const
	{
		return InternalPath;
	}

private:
	FName InternalPath;
};

class FMarkdownContentBrowserFileItemDataPayload final : public IContentBrowserItemDataPayload
{
public:
	explicit FMarkdownContentBrowserFileItemDataPayload(const FName& InInternalPath, UClass* InClass);

	static bool GetItemAttribute(const FName& InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue);

	const FName& GetInternalPath() const;

	UClass* GetClass() const;

	const FAssetData& GetAssetData() const;

	void UpdateThumbnail(FAssetThumbnail& OutThumbnail) const;

private:
	FName InternalPath;

	TWeakObjectPtr<UClass> Class;

	FAssetData AssetData;
};
