// Copyright Epic Games, Inc. All Rights Reserved.

#include "ContentBrowser/MarkdownNewFileContextMenu.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"

#define LOCTEXT_NAMESPACE "ContentBrowserClassDataSource"

void FMarkdownNewFileContextMenu::MakeContextMenu(
	UToolMenu* Menu, 
	const TArray<FName>& InSelectedClassPaths,
	const FOnNewFileRequested& InOnNewClassRequested
	)
{
	if (InSelectedClassPaths.Num() == 0)
	{
		return;
	}

	const FName FirstSelectedPath = InSelectedClassPaths[0];
	const bool bHasSinglePathSelected = InSelectedClassPaths.Num() == 1;

	auto CanExecuteClassActions = [bHasSinglePathSelected]() -> bool
	{
		// We can execute class actions when we only have a single path selected
		return bHasSinglePathSelected;
	};
	const FCanExecuteAction CanExecuteClassActionsDelegate = FCanExecuteAction::CreateLambda(CanExecuteClassActions);

	// Add Class
	if(InOnNewClassRequested.IsBound())
	{
		FName ClassCreationPath = FirstSelectedPath;
		FText NewClassToolTip;
		if(bHasSinglePathSelected)
		{
			NewClassToolTip = FText::Format(LOCTEXT("NewFileTooltip_CreateIn", "Create a new documentation file in {0}."), FText::FromName(ClassCreationPath));
		}
		else
		{
			NewClassToolTip = LOCTEXT("NewFileTooltip_InvalidNumberOfPaths", "Can only create classes when there is a single path selected.");
		}

		{
			FToolMenuSection& Section = Menu->AddSection("ContentBrowserNewDocumentation", LOCTEXT("MarkdownMenuHeading", "Documentation File"));
			Section.AddMenuEntry(
				"NewFile",
				LOCTEXT("NewFileLabel", "New Documentation File"),
				NewClassToolTip,
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.AddCodeToProject"),
				FUIAction(
					FExecuteAction::CreateStatic(&FMarkdownNewFileContextMenu::ExecuteNewClass, ClassCreationPath, InOnNewClassRequested),
					CanExecuteClassActionsDelegate
					)
				);
		}
	}
}

void FMarkdownNewFileContextMenu::ExecuteNewClass(FName InPath, FOnNewFileRequested InOnNewClassRequested)
{
	// An empty path override will cause the class wizard to use the default project path
	InOnNewClassRequested.ExecuteIfBound(InPath);
}

#undef LOCTEXT_NAMESPACE
