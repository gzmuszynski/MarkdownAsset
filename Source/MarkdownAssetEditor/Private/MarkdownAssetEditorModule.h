// Copyright (C) 2024 Gwaredd Mountain - All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class UMarkdownContentBrowserDataSource;
class UAssetEditorToolkitMenuContext;

class MARKDOWNASSETEDITOR_API FMarkdownAssetEditorModule : public IModuleInterface 
{
	
public:
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:

	/** Registers main menu and toolbar menu extensions. */
	void RegisterMenuExtensions();

	/** Register the EditorSettings screen. */
	void RegisterSettings();

	/** Unregister on mode shutdown */
	void UnregisterMenuExtensions();
	void UnregisterSettings();

	void EditorAction_OpenProjectDocumentation();
	void EditorAction_OpenAssetDocumentation(UAssetEditorToolkitMenuContext* ExecutionContext);

private:
	TStrongObjectPtr<UMarkdownContentBrowserDataSource> MarkdownDataSource;
};