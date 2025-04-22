#pragma once
#include "MarkdownContentBrowserHierarchy.generated.h"


UCLASS()
class MARKDOWNASSETEDITOR_API UMarkdownFile : public UObject
{
	GENERATED_BODY()
public:
	FName FilePath;
};

struct FMarkdownContentBrowserHierarchyNode
{
	TSet<UMarkdownFile*>& GetMDFiles()
	{
		return MDFiles;
	}

	TMap<FName, TSharedPtr<FMarkdownContentBrowserHierarchyNode>>& GetChildren()
	{
		return Children;
	}

	TSet<UMarkdownFile*> MDFiles;

	TMap<FName, TSharedPtr<FMarkdownContentBrowserHierarchyNode>> Children;
};

class FMarkdownContentBrowserHierarchy
{
public:
	FMarkdownContentBrowserHierarchy();

	TSharedPtr<FMarkdownContentBrowserHierarchyNode> FindNode(const FName& InPath) const;

	static void GetMatchingFolders(const TSharedPtr<FMarkdownContentBrowserHierarchyNode>& InNode, TArray<FName>& OutFolders);

	static void GetMatchingMDFiles(const TSharedPtr<FMarkdownContentBrowserHierarchyNode>& InNode, TArray<UMarkdownFile*>& OutFiles);

	TArray<FName> GetMatchingFolders(const FName& InPath, const bool bRecurse = false) const;

	TArray<UMarkdownFile*> GetMatchingMDFiles(const FName& InPath, const bool bRecurse = false) const;

	static FString ConvertInternalPathToFileSystemPath(const FString& InInternalPath);

private:
	static bool EnumeratePath(const FString& InPath, const TFunctionRef<bool(const FName&)>& InCallback);

	void AddMDFile(UMarkdownFile* InFile) const;

	void PopulateHierarchy();

private:
	TSharedPtr<FMarkdownContentBrowserHierarchyNode> Root;
};
