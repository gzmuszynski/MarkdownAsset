// Copyright (C) 2024 Gwaredd Mountain - All Rights Reserved.

#pragma once

#include "Internationalization/Text.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectSaveContext.h"

#include "MarkdownAsset.generated.h"

DECLARE_DYNAMIC_DELEGATE(FMarkdownAssetChangedDelegate);

UCLASS( BlueprintType, hidecategories = ( Object ) )
class MARKDOWNASSET_API UMarkdownAsset : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "MarkdownAsset" )
	FText Text;

	UPROPERTY()
	FMarkdownAssetChangedDelegate OnChanged;
	
#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		OnChanged.Execute();
	}

	virtual void PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext) override
	{
		Super::PostSaveRoot(ObjectSaveContext);
		OnChanged.Execute();
	}
#endif
};
