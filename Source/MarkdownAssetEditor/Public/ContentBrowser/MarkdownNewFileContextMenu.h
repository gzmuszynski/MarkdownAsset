// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Delegates/Delegate.h"

class UToolMenu;

class FMarkdownNewFileContextMenu
{
public:
	DECLARE_DELEGATE_OneParam(FOnNewFileRequested, const FName &/*SelectedPath*/);

	/** Makes the context menu widget */
	static void MakeContextMenu(
		UToolMenu* Menu, 
		const TArray<FName>& InSelectedClassPaths,
		const FOnNewFileRequested& InOnNewClassRequested
		);

private:
	/** Create a new class at the specified path */
	static void ExecuteNewClass(FName InPath, FOnNewFileRequested InOnNewClassRequested);
};
