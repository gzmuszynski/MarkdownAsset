// Copyright (C) 2024 Gwaredd Mountain - All Rights Reserved.

#include "SMarkdownAssetEditor.h"

#include "Fonts/SlateFontInfo.h"
#include "Internationalization/Text.h"
#include "MarkdownAsset.h"
#include "UObject/Class.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Interfaces/IPluginManager.h"
#include "MarkdownAssetEditorSettings.h"
#include "MarkdownBinding.h"

#define LOCTEXT_NAMESPACE "SMarkdownAssetEditor"


//---------------------------------------------------------------------------------------------------------------------

SMarkdownAssetEditor::~SMarkdownAssetEditor()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll( this );

	if( WebBrowser.IsValid() )
	{
		WebBrowser->CloseBrowser();
	}
}

//---------------------------------------------------------------------------------------------------------------------

void SMarkdownAssetEditor::Construct( const FArguments& InArgs, UMarkdownAsset* InMarkdownAsset, const TSharedRef<ISlateStyle>& InStyle )
{
	MarkdownAsset = InMarkdownAsset;

	if( !FModuleManager::Get().IsModuleLoaded( "WebBrowser" ) )
	{
        FMessageDialog::Open( EAppMsgType::Ok, LOCTEXT( "WebBrowserModuleMissing", "You need to enable the WebBrowser plugin to run the Markdown editor." ) );
        return;
	}

	auto Settings = GetDefault<UMarkdownAssetEditorSettings>();

	FString ContentDir = IPluginManager::Get().FindPlugin( TEXT( "MarkdownAsset" ) )->GetContentDir();
	FString FullPath   = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead( *ContentDir );
	FString URL        = FullPath / ( Settings->bDarkSkin ? TEXT( "dark.html" ) : TEXT( "light.html" ) );

	WebBrowser = SAssignNew( WebBrowser, SWebBrowserView )
		.InitialURL( URL )
		.BackgroundColor( Settings->bDarkSkin ? FLinearColor( 0.1f, 0.1f, 0.1f, 1.0f ).ToFColor(true) : FLinearColor( 1.0f, 1.0f, 1.0f, 1.0f ).ToFColor(true) )
		.OnConsoleMessage( this, &SMarkdownAssetEditor::HandleConsoleMessage )
	;

	// setup binding
	UMarkdownBinding* Binding = NewObject<UMarkdownBinding>();
	Binding->Text = MarkdownAsset->Text;
	Binding->OnSetText.AddLambda( [this, Binding]() { MarkdownAsset->MarkPackageDirty(); MarkdownAsset->Text = Binding->GetText(); });
	WebBrowser->BindUObject( TEXT( "MarkdownBinding" ), Binding, true );

	ChildSlot
	[
		SNew( SVerticalBox )
		+ SVerticalBox::Slot()
		.FillHeight( 1.0f )
		[
			WebBrowser.ToSharedRef()
		]
	];

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP( this, &SMarkdownAssetEditor::HandleMarkdownAssetPropertyChanged );
}

//---------------------------------------------------------------------------------------------------------------------

FReply SMarkdownAssetEditor::OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent )
{
	// consume tilde key to prevent it from being passed to unreal and opening the console

	if( InKeyEvent.GetKey() == EKeys::Tilde )
	{
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

//---------------------------------------------------------------------------------------------------------------------

void SMarkdownAssetEditor::HandleMarkdownAssetPropertyChanged( UObject* Object, FPropertyChangedEvent& PropertyChangedEvent )
{
	//if( Object == MarkdownAsset )
	//{
	//	EditableTextBox->SetText( MarkdownAsset->Text );
	//}
}

void SMarkdownAssetEditor::HandleConsoleMessage( const FString& Message, const FString& Source, int32 Line, EWebBrowserConsoleLogSeverity Serverity )
{
	//UE_LOG( LogTemp, Warning, TEXT( "Browser: %s" ), *Message );	
}

#undef LOCTEXT_NAMESPACE
