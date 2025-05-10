#include "ContentBrowser/MarkdownContentBrowserHierarchy.h"
#include "MarkdownAsset.h"
#include "FileHelpers.h"

#define DYNAMIC_ROOT_INTERNAL_PATH FString(TEXT("/Documentation"))

#define DYNAMIC_ROOT_VIRTUAL_PATH FString(TEXT("/All")) / DYNAMIC_ROOT_INTERNAL_PATH

UMarkdownFile::~UMarkdownFile()
{
	if (Asset->IsValidLowLevel())
	{
		Asset->MarkAsGarbage();
	}
}

UMarkdownAsset* UMarkdownFile::GetMarkdownAsset()
{
	if (Asset)
	{
		return Asset;
	}
	FString FullPath = FPaths::ProjectDir() + DYNAMIC_ROOT_INTERNAL_PATH + FilePath.ToString();
	if (FPaths::FileExists(FullPath))
	{
		Asset = NewObject<UMarkdownAsset>(this, FName(GetName()+"_C"), RF_Standalone);
		FString String;
		
		FFileHelper::LoadFileToString(String, *FullPath);
		
		Asset->Text = FText::FromString(String);
		Asset->OnChanged.BindDynamic(this, &UMarkdownFile::OnAssetChanged);
		return Asset;
	}
	return nullptr;
}

void UMarkdownFile::OnAssetChanged()
{
	if (Asset)
	{
		FString FullPath = FPaths::ProjectDir() + DYNAMIC_ROOT_INTERNAL_PATH + FilePath.ToString();
		FFileHelper::SaveStringToFile(Asset->Text.ToString(), *FullPath);
	}
}

void FMarkdownContentBrowserHierarchyNode::DestroyUObjects()
{
	for (UMarkdownFile* File : MDFiles)
	{
		File->MarkAsGarbage();
	}
	for (auto [Key,Value] : Children)
	{
		Value->DestroyUObjects();
	}
}

FMarkdownContentBrowserHierarchy::FMarkdownContentBrowserHierarchy()
{
	PopulateHierarchy();
}

TSharedPtr<FMarkdownContentBrowserHierarchyNode> FMarkdownContentBrowserHierarchy::FindNode(const FName& InPath) const
{
	if (!Root.IsValid())
	{
		return MakeShared<FMarkdownContentBrowserHierarchyNode>();
	}

	auto Path = InPath.ToString();

	if (Path.StartsWith(DYNAMIC_ROOT_INTERNAL_PATH))
	{
		Path = Path.RightChop(DYNAMIC_ROOT_INTERNAL_PATH.Len());
	}

	if (Path.IsEmpty())
	{
		Path = TEXT("/");
	}

	auto Node = Root;

	EnumeratePath(Path,
	              [&Node](const FName& InInternalPath)
	              {
		              const auto Child = Node->GetChildren().Find(InInternalPath);

		              if (!Child)
		              {
			              return false;
		              }

		              Node = *Child;

		              return true;
	              });

	return Node;
}

void FMarkdownContentBrowserHierarchy::GetMatchingFolders(const TSharedPtr<FMarkdownContentBrowserHierarchyNode>& InNode, TArray<FName>& OutFolders)
{
	for (const auto& [Key, Value] : InNode->GetChildren())
	{
		OutFolders.Add(*(DYNAMIC_ROOT_INTERNAL_PATH / Key.ToString()));

		GetMatchingFolders(Value, OutFolders);
	}
}

void FMarkdownContentBrowserHierarchy::GetMatchingMDFiles(const TSharedPtr<FMarkdownContentBrowserHierarchyNode>& InNode, TArray<UMarkdownFile*>& OutFiles)
{
	for (const auto& Class : InNode->GetMDFiles())
	{
		OutFiles.Add(Class);
	}

	for (const auto& [PLACEHOLDER, Value] : InNode->GetChildren())
	{
		for (const auto& Class : Value->GetMDFiles())
		{
			OutFiles.Add(Class);
		}

		GetMatchingMDFiles(Value, OutFiles);
	}
}

TArray<FName> FMarkdownContentBrowserHierarchy::GetMatchingFolders(const FName& InPath, const bool bRecurse) const
{
	const auto& Node = FindNode(InPath);

	if (!Node.IsValid())
	{
		return TArray<FName>();
	}

	TArray<FName> MatchingFolders;

	for (const auto& [Key, Value] : Node->GetChildren())
	{
		MatchingFolders.Add(*(DYNAMIC_ROOT_INTERNAL_PATH / Key.ToString()));

		if (bRecurse)
		{
			GetMatchingFolders(Value, MatchingFolders);
		}
	}

	return MatchingFolders;
}

TArray<UMarkdownFile*> FMarkdownContentBrowserHierarchy::GetMatchingMDFiles(
	const FName& InPath, const bool bRecurse) const
{
	TArray<UMarkdownFile*> MatchingClasses;

	const auto Node = FindNode(InPath);

	if (!Node.IsValid())
	{
		return MatchingClasses;
	}

	for (const auto& Class : Node->GetMDFiles())
	{
		MatchingClasses.Add(Class);
	}

	if (bRecurse)
	{
		for (const auto& [PLACEHOLDER, Value] : Node->GetChildren())
		{
			GetMatchingMDFiles(Value, MatchingClasses);
		}
	}

	return MatchingClasses;
}

FString FMarkdownContentBrowserHierarchy::ConvertInternalPathToFileSystemPath(const FString& InInternalPath)
{
	auto FileSystemPath = InInternalPath;

	if (FileSystemPath.IsEmpty() ||
		!FileSystemPath.StartsWith(TEXT("/")))
	{
		return FString();
	}

	const auto SecondSlashIndex = FileSystemPath.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 1);

	FileSystemPath = SecondSlashIndex != INDEX_NONE
		                 ? FileSystemPath.RightChop(SecondSlashIndex)
		                 : TEXT("");

	if (FileSystemPath.IsEmpty())
	{
		return FString();
	}

	return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + DYNAMIC_ROOT_INTERNAL_PATH) / FileSystemPath;
}

bool FMarkdownContentBrowserHierarchy::EnumeratePath(const FString& InPath, const TFunctionRef<bool(const FName&)>& InCallback)
{
	const auto Path = *InPath;

	auto PathStart = Path + 1;

	const auto PathEnd = Path + InPath.Len();

	for (;;)
	{
		if (PathStart >= PathEnd || *PathStart == TEXT('/'))
		{
			if (const FName SubPath(UE_PTRDIFF_TO_INT32(PathStart - Path), Path);
				!SubPath.IsNone())
			{
				if (!InCallback(SubPath))
				{
					return false;
				}
			}
		}

		if (PathStart >= PathEnd)
		{
			break;
		}

		++PathStart;
	}

	return true;
}

void FMarkdownContentBrowserHierarchy::AddMDFile(UMarkdownFile* InFile) const
{
	if (InFile == nullptr)
	{
		return;
	}

	FString Path = InFile->FilePath.ToString();
	const auto& RelativePath = Path.Left(Path.Find("/", ESearchCase::IgnoreCase,
		ESearchDir::FromEnd));

	auto Node = Root;

	if (!RelativePath.IsEmpty())
	{
		EnumeratePath(RelativePath,
            		[&Node](const FName& InInternalPath)
            		{
            			auto& Child = Node->GetChildren().FindOrAdd(InInternalPath);
        
            			if (!Child.IsValid())
            			{
            			    Child = MakeShared<FMarkdownContentBrowserHierarchyNode>();
            			}
        
            			Node = Child;
        
            			return true;
            		});
	}
	
	Node->GetMDFiles().Add(InFile);
}

void FMarkdownContentBrowserHierarchy::PopulateHierarchy()
{
	if (Root.IsValid())
	{
		Root->DestroyUObjects();
	}
	Root = MakeShared<FMarkdownContentBrowserHierarchyNode>();

	FString Path = FPaths::ProjectDir() + DYNAMIC_ROOT_INTERNAL_PATH;
	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *Path , TEXT("*.md"), true, false);
	for (FString File : Files)
	{
		File.RemoveFromStart(Path);
		FString ObjectName = File;
		if (ObjectName.Contains("/"))
		{
			int Index;
			ObjectName.FindLastChar('/', Index);
			ObjectName.RightChopInline(Index+1);
		}
		int Index = ObjectName.Find(".md");
		ObjectName.LeftInline(Index);
		
		UMarkdownFile* MDFile = NewObject<UMarkdownFile>(GetTransientPackage(), FName(ObjectName), RF_Standalone);
		MDFile->FilePath = FName(File);
		AddMDFile(MDFile);
	}
}
