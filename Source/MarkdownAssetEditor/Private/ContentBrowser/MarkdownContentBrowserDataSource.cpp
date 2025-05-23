#include "ContentBrowser/MarkdownContentBrowserDataSource.h"
#include "CollectionManagerModule.h"
#include "ContentBrowserDataUtils.h"
#include "ContentBrowserItemPath.h"
#include "SourceCodeNavigation.h"
#define UE_F_NAME_PERMISSION_LIST 0
#if UE_F_NAME_PERMISSION_LIST
#include "Misc/NamePermissionList.h"
#endif
#include "AssetToolsModule.h"
#include "ContentBrowserDataMenuContexts.h"
#include "ToolMenuDelegates.h"
#include "ToolMenus.h"
#include "ContentBrowser/MarkdownNewFileContextMenu.h"
#include "MarkdownAsset.h"
#include "ContentBrowser/MarkdownContentBrowserFolderItemDataPayload.h"
#define UE_ASSET_DATA_GET_SOFT_OBJECT_PATH 0
#define UE_U_CONTENT_BROWSER_DATA_SOURCE_NOTIFY_ITEM_DATA_REFRESHED 0
#define DYNAMIC_ROOT_INTERNAL_PATH FString(TEXT("/Documentation"))

#define DYNAMIC_ROOT_VIRTUAL_PATH FString(TEXT("/All")) / DYNAMIC_ROOT_INTERNAL_PATH

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(MarkdownContentBrowserDataSource)
#endif

#define LOCTEXT_NAMESPACE "UMarkdownDataSource"

void UMarkdownContentBrowserDataSource::Initialize(const bool InAutoRegister)
{
	Super::Initialize(InAutoRegister);

	/*OnDynamicClassUpdatedHandle = FUnrealCSharpCoreModuleDelegates::OnDynamicClassUpdated.AddUObject(
		this, &UMarkdownDataSource::OnDynamicClassUpdated);

	OnEndGeneratorHandle = FUnrealCSharpCoreModuleDelegates::OnEndGenerator.AddUObject(
		this, &UMarkdownDataSource::OnEndGenerator);*/

	CollectionManager = &FCollectionManagerModule::GetModule().Get();

	if (const auto Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AddNewContextMenu"))
	{
		Menu->AddDynamicSection(*FString::Printf(TEXT(
			                        "MarkdownSection_DataSource_%s"),
		                                         *GetName()
		                        ),
		                        FNewToolMenuDelegate::CreateLambda(
			                        [WeakThis = TWeakObjectPtr<UMarkdownContentBrowserDataSource>(this)](UToolMenu* InMenu)
			                        {
				                        if (WeakThis.IsValid())
				                        {
					                        WeakThis->PopulateAddNewContextMenu(InMenu);
				                        }
			                        }));
	}

	BuildRootPathVirtualTree();
}

void UMarkdownContentBrowserDataSource::Shutdown()
{
	CollectionManager = nullptr;

	MarkdownHierarchy.Reset();

	/*if (OnEndGeneratorHandle.IsValid())
	{
		FUnrealCSharpCoreModuleDelegates::OnEndGenerator.Remove(OnEndGeneratorHandle);
	}

	if (OnDynamicClassUpdatedHandle.IsValid())
	{
		FUnrealCSharpCoreModuleDelegates::OnDynamicClassUpdated.Remove(OnDynamicClassUpdatedHandle);
	}*/

	Super::Shutdown();
}

void UMarkdownContentBrowserDataSource::CompileFilter(const FName InPath, const FContentBrowserDataFilter& InFilter,
                                       FContentBrowserDataCompiledFilter& OutCompiledFilter)
{
#if UE_F_NAME_PERMISSION_LIST
	const FNamePermissionList* ClassPermissionList = nullptr;
#else
	const FPathPermissionList* ClassPermissionList = nullptr;
#endif

	if (const auto DataClassFilter = InFilter.ExtraFilters.FindFilter<FContentBrowserDataClassFilter>())
	{
		if (DataClassFilter->ClassPermissionList)
		{
			if (DataClassFilter->ClassPermissionList->HasFiltering())
			{
				ClassPermissionList = DataClassFilter->ClassPermissionList.Get();
			}
		}
	}

	const auto bIncludeFolders =
		EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFolders);

	const auto bIncludeFiles = EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFiles);

	const auto bIncludeClasses = EnumHasAnyFlags(InFilter.ItemCategoryFilter,
	                                             EContentBrowserItemCategoryFilter::IncludeClasses);

	auto& FilterList = OutCompiledFilter.CompiledFilters.FindOrAdd(this);

	auto& [Classes, Folders] = FilterList.FindOrAddFilter<FMarkdownContentBrowserDataFilter>();

	if (!bIncludeClasses || (!bIncludeFolders && !bIncludeFiles))
	{
		return;
	}

	RefreshVirtualPathTreeIfNeeded();

	TSet<FName> InternalPaths;

	FName ConvertInternalPath;

	const auto ContentBrowserPathType = TryConvertVirtualPath(InPath, ConvertInternalPath);

	if (ContentBrowserPathType == EContentBrowserPathType::Internal)
	{
		InternalPaths.Add(ConvertInternalPath);
	}
	else if (ContentBrowserPathType != EContentBrowserPathType::Virtual)
	{
		return;
	}

	if (bIncludeFolders)
	{
		if (InFilter.bRecursivePaths)
		{
			if (ContentBrowserPathType == EContentBrowserPathType::Virtual)
			{
				RootPathVirtualTree.EnumerateSubPaths(InPath,
				                                      [this, &InternalPaths](
				                                      FName VirtualSubPath, FName InternalSubPath)
				                                      {
					                                      if (!InternalSubPath.IsNone())
					                                      {
						                                      if (IsRootInternalPath(InternalSubPath))
						                                      {
							                                      InternalPaths.Add(InternalSubPath);
						                                      }
					                                      }
					                                      return true;
				                                      }, true);
			}
		}
		else
		{
			if (ContentBrowserPathType == EContentBrowserPathType::Virtual)
			{
				FContentBrowserCompiledVirtualFolderFilter* CompiledVirtualFolderFilter = nullptr;

				RootPathVirtualTree.EnumerateSubPaths(InPath,
				                                      [this, &InternalPaths, &CompiledVirtualFolderFilter, &FilterList](
				                                      FName VirtualSubPath, FName InternalSubPath)
				                                      {
					                                      if (!InternalSubPath.IsNone())
					                                      {
						                                      if (IsRootInternalPath(InternalSubPath))
						                                      {
							                                      InternalPaths.Add(InternalSubPath);
						                                      }
					                                      }
					                                      else
					                                      {
						                                      auto bPassesFilter = false;

						                                      RootPathVirtualTree.EnumerateSubPaths(VirtualSubPath,
							                                      [&CompiledVirtualFolderFilter, &FilterList,
								                                      &bPassesFilter](
							                                      FName RecursiveVirtualSubPath,
							                                      FName RecursiveInternalSubPath)
							                                      {
								                                      bPassesFilter = bPassesFilter ||
								                                      (!RecursiveInternalSubPath.IsNone() &&
									                                      IsRootInternalPath(
										                                      RecursiveInternalSubPath));

								                                      return !bPassesFilter;
							                                      }, true);

						                                      if (bPassesFilter)
						                                      {
							                                      if (!CompiledVirtualFolderFilter)
							                                      {
								                                      CompiledVirtualFolderFilter = &FilterList.
									                                      FindOrAddFilter<
										                                      FContentBrowserCompiledVirtualFolderFilter>();
							                                      }

							                                      if (!CompiledVirtualFolderFilter->CachedSubPaths.
								                                      Contains(VirtualSubPath))
							                                      {
								                                      CompiledVirtualFolderFilter->CachedSubPaths.Add(
									                                      VirtualSubPath,
									                                      CreateVirtualFolderItem(VirtualSubPath));
							                                      }
						                                      }
					                                      }
					                                      return true;
				                                      }, false);

				Folders.Append(InternalPaths);

				return;
			}
		}
	}
	else if (bIncludeFiles)
	{
		if (InFilter.bRecursivePaths)
		{
			if (ContentBrowserPathType == EContentBrowserPathType::Virtual)
			{
				RootPathVirtualTree.EnumerateSubPaths(InPath,
				                                      [this, &InternalPaths](
				                                      FName VirtualSubPath, FName InternalSubPath)
				                                      {
					                                      if (!InternalSubPath.IsNone())
					                                      {
						                                      if (IsRootInternalPath(InternalSubPath))
						                                      {
							                                      InternalPaths.Add(InternalSubPath);
						                                      }
					                                      }

					                                      return true;
				                                      }, true);

				if (InternalPaths.IsEmpty())
				{
					return;
				}
			}
		}
		else
		{
			if (ContentBrowserPathType == EContentBrowserPathType::Virtual)
			{
				return;
			}
		}
	}

	if (InternalPaths.IsEmpty() || !MarkdownHierarchy.IsValid())
	{
		return;
	}

	if (bIncludeFolders)
	{
		const auto& MatchingFolders = MarkdownHierarchy->GetMatchingFolders(
			ConvertInternalPath, InFilter.bRecursivePaths);

		if (ContentBrowserPathType == EContentBrowserPathType::Virtual)
		{
			for (const auto& InternalPath : InternalPaths)
			{
				Folders.Add(InternalPath);
			}
		}

		for (const auto& MatchingFolder : MatchingFolders)
		{
			Folders.Add(MatchingFolder);
		}
	}

	if (bIncludeFiles)
	{
		if (const auto& MatchingClasses = MarkdownHierarchy->GetMatchingMDFiles(
			ConvertInternalPath, InFilter.bRecursivePaths); !MatchingClasses.IsEmpty())
		{
#if UE_F_TOP_LEVEL_ASSET_PATH
			TSet<FTopLevelAssetPath> ClassPathsToInclude;
#else
			TSet<FName> ClassPathsToInclude;
#endif

			if (const auto DataCollectionFilter = InFilter.ExtraFilters.FindFilter<
				FContentBrowserDataCollectionFilter>())
			{
#if UE_F_TOP_LEVEL_ASSET_PATH
				TArray<FTopLevelAssetPath> ClassPathsForCollections;
#else
				TArray<FName> ClassPathsForCollections;
#endif

				if (GetClassPaths(DataCollectionFilter->SelectedCollections,
				                  DataCollectionFilter->bIncludeChildCollections,
				                  ClassPathsForCollections) &&
					ClassPathsForCollections.IsEmpty())
				{
					return;
				}

				ClassPathsToInclude.Append(ClassPathsForCollections);
			}

			for (auto MatchingClass : MatchingClasses)
			{
				const auto bPassesInclusiveFilter = ClassPathsToInclude.IsEmpty() ||
					ClassPathsToInclude.Contains(
#if UE_F_TOP_LEVEL_ASSET_PATH
						FTopLevelAssetPath(MatchingClass));
#else
						*MatchingClass->GetFName().ToString());
#endif

				const auto bPassesPermissionCheck = !ClassPermissionList ||
					ClassPermissionList->PassesFilter(
#if UE_F_TOP_LEVEL_ASSET_PATH
						MatchingClass->GetClassPathName().ToString());
#else
						MatchingClass->GetFName().ToString());
#endif

				if (bPassesInclusiveFilter && bPassesPermissionCheck)
				{
					Classes.Add(MatchingClass);
				}
			}
		}
	}
}

void UMarkdownContentBrowserDataSource::EnumerateItemsMatchingFilter(const FContentBrowserDataCompiledFilter& InFilter,
                                                      TFunctionRef<bool(FContentBrowserItemData&&)> InCallback)
{
	const auto DataFilterList = InFilter.CompiledFilters.Find(this);

	if (!MarkdownHierarchy.IsValid())
		UpdateHierarchy();

	if (!DataFilterList)
	{
		return;
	}

	const auto ClassDataFilter = DataFilterList->FindFilter<FMarkdownContentBrowserDataFilter>();

	if (!ClassDataFilter)
	{
		return;
	}

	if (EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFolders))
	{
		for (const auto& Folder : ClassDataFilter->Folders)
		{
			if (!InCallback(CreateFolderItem(Folder)))
			{
				return;
			}
		}
	}

	if (EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFiles))
	{
		for (const auto& Class : ClassDataFilter->Classes)
		{
			if (!InCallback(CreateFileItem(Class)))
			{
				return;
			}
		}
	}
}

void UMarkdownContentBrowserDataSource::EnumerateItemsAtPath(const FName InPath,
                                              const EContentBrowserItemTypeFilter InItemTypeFilter,
                                              TFunctionRef<bool(FContentBrowserItemData&&)> InCallback)
{
	FName InternalPath;

	if (!TryConvertVirtualPathToInternal(InPath, InternalPath))
	{
		return;
	}

	if (EnumHasAnyFlags(InItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFolders))
	{
		if (MarkdownHierarchy->FindNode(InternalPath))
		{
			InCallback(CreateFolderItem(InternalPath));
		}
	}

	if (EnumHasAnyFlags(InItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFiles))
	{
		if (const auto Node = MarkdownHierarchy->FindNode(InternalPath))
		{
			for (const auto Class : Node->GetMDFiles())
			{
				InCallback(CreateFileItem(Class));
			}
		}
	}
}

bool UMarkdownContentBrowserDataSource::EnumerateItemsForObjects(const TArrayView<UObject*> InObjects,
                                                  TFunctionRef<bool(FContentBrowserItemData&&)> InCallback)
{
	for (const auto Object : InObjects)
	{
		if (const auto Class = Cast<UClass>(Object))
		{
			if (!InCallback(CreateFileItem({})))
			{
				return false;
			}
		}
	}

	return true;
}

bool UMarkdownContentBrowserDataSource::DoesItemPassFilter(const FContentBrowserItemData& InItem,
                                            const FContentBrowserDataCompiledFilter& InFilter)
{
	const auto DataFilterList = InFilter.CompiledFilters.Find(this);

	if (!DataFilterList)
	{
		return false;
	}

	const auto ClassDataFilter = DataFilterList->FindFilter<FMarkdownContentBrowserDataFilter>();

	if (!ClassDataFilter)
	{
		return false;
	}

	switch (InItem.GetItemType())
	{
	case EContentBrowserItemFlags::Type_Folder:
		{
			if (EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFolders) &&
				!ClassDataFilter->Folders.IsEmpty())
			{
				if (const auto FolderItemDataPayload = GetFolderItemDataPayload(InItem))
				{
					return ClassDataFilter->Folders.Contains(FolderItemDataPayload->GetInternalPath());
				}
			}
		}
		break;

	case EContentBrowserItemFlags::Type_File:
		{
			if (EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFiles) &&
				!ClassDataFilter->Classes.IsEmpty())
			{
				if (const auto FileItemDataPayload = GetFileItemDataPayload(InItem))
				{
					return ClassDataFilter->Classes.Contains(FileItemDataPayload->GetMDFile());
				}
			}
		}
		break;

	default:
		break;
	}

	return false;
}

bool UMarkdownContentBrowserDataSource::GetItemAttribute(const FContentBrowserItemData& InItem, const bool InIncludeMetaData,
                                          const FName InAttributeKey,
                                          FContentBrowserItemDataAttributeValue& OutAttributeValue)
{
	const auto FileItemDataPayload = GetFileItemDataPayload(InItem);

	return FileItemDataPayload ? FileItemDataPayload->GetItemAttribute(InAttributeKey, OutAttributeValue) : false;
}

bool UMarkdownContentBrowserDataSource::CanEditItem(const FContentBrowserItemData& InItem, FText* OutErrorMsg)
{
	return true;
}

bool UMarkdownContentBrowserDataSource::EditItem(const FContentBrowserItemData& InItem)
{
	const auto ItemDataPayload = GetFileItemDataPayload(InItem);
	{
		static const FName NAME_AssetTools = "AssetTools";
		auto AssetTools = &FModuleManager::GetModuleChecked<FAssetToolsModule>(NAME_AssetTools).Get();
		AssetTools->OpenEditorForAssets({ItemDataPayload->GetMDFile()->GetMarkdownAsset()});
		return true;
	}
	return ItemDataPayload
		       ? FSourceCodeNavigation::OpenSourceFile(ItemDataPayload->GetInternalPath().ToString())
		       : false;
}

bool UMarkdownContentBrowserDataSource::BulkEditItems(TArrayView<const FContentBrowserItemData> InItems)
{
	for (const auto& Item : InItems)
	{
		EditItem(Item);
	}

	return true;
}


bool UMarkdownContentBrowserDataSource::AppendItemReference(const FContentBrowserItemData& InItem, FString& InOutStr)
{
	if (const auto FileItemDataPayload = GetFileItemDataPayload(InItem))
	{
		if (!InOutStr.IsEmpty())
		{
			InOutStr += LINE_TERMINATOR;
		}

		InOutStr += FileItemDataPayload->GetAssetData().GetExportTextName();

		return true;
	}

	return false;
}

bool UMarkdownContentBrowserDataSource::UpdateThumbnail(const FContentBrowserItemData& InItem, FAssetThumbnail& InThumbnail)
{
	if (const auto FileItemDataPayload = GetFileItemDataPayload(InItem))
	{
		FileItemDataPayload->UpdateThumbnail(InThumbnail);

		return true;
	}

	return false;
}

bool UMarkdownContentBrowserDataSource::TryGetCollectionId(const FContentBrowserItemData& InItem,
#if UE_TRY_GET_COLLECTION_ID_F_SOFT_OBJECT_PATH
                                            FName& OutCollectionId)
#else
                                            FSoftObjectPath& OutCollectionId)
#endif
{
	if (const auto FileItemPayload = GetFileItemDataPayload(InItem))
	{
#if UE_ASSET_DATA_GET_SOFT_OBJECT_PATH
		OutCollectionId = FSoftObjectPath(FileItemPayload->GetAssetData().GetSoftObjectPath());
#else
		OutCollectionId = FileItemPayload->GetAssetData().ObjectPath;
#endif

		return true;
	}

	return false;
}

bool UMarkdownContentBrowserDataSource::Legacy_TryGetPackagePath(const FContentBrowserItemData& InItem, FName& OutPackagePath)
{
	if (const auto FolderItemDataPayload = GetFolderItemDataPayload(InItem))
	{
		OutPackagePath = FolderItemDataPayload->GetInternalPath();

		return true;
	}

	return false;
}

bool UMarkdownContentBrowserDataSource::Legacy_TryGetAssetData(const FContentBrowserItemData& InItem, FAssetData& OutAssetData)
{
	if (const auto FileItemPayload = GetFileItemDataPayload(InItem))
	{
		OutAssetData = FileItemPayload->GetAssetData();

		return true;
	}

	return false;
}

bool UMarkdownContentBrowserDataSource::Legacy_TryConvertPackagePathToVirtualPath(const FName InPackagePath, FName& OutPath)
{
	return TryConvertInternalPathToVirtual(InPackagePath, OutPath);
}

bool UMarkdownContentBrowserDataSource::Legacy_TryConvertAssetDataToVirtualPath(const FAssetData& InAssetData,
                                                                 const bool InUseFolderPaths, FName& OutPath)
{
	return TryConvertInternalPathToVirtual(
#if UE_ASSET_DATA_GET_SOFT_OBJECT_PATH
		InUseFolderPaths ? InAssetData.PackagePath : *InAssetData.GetSoftObjectPath().ToString(),
#else
		InUseFolderPaths ? InAssetData.PackagePath : InAssetData.ObjectPath,
#endif
		OutPath);
}

void UMarkdownContentBrowserDataSource::BuildRootPathVirtualTree()
{
	Super::BuildRootPathVirtualTree();

	RootPathAdded(FNameBuilder(*DYNAMIC_ROOT_INTERNAL_PATH));
}

void UMarkdownContentBrowserDataSource::OnNewClassRequested(const FName& InSelectedPath)
{
	if (auto SelectedFileSystemPath = FMarkdownContentBrowserHierarchy::ConvertInternalPathToFileSystemPath(InSelectedPath.ToString());
		!SelectedFileSystemPath.IsEmpty())
	{
		if (SelectedFileSystemPath.EndsWith("Documentation"))
		{
			SelectedFileSystemPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / "Documentation");
		}

		//FDynamicNewClassUtils::OpenAddDynamicClassToProjectDialog(SelectedFileSystemPath);
	}
}

void UMarkdownContentBrowserDataSource::PopulateAddNewContextMenu(UToolMenu* InMenu)
{
	const auto ContextObject = InMenu->FindContext<UContentBrowserDataMenuContext_AddNewMenu>();

	TArray<FName> SelectedClassPaths;

	for (const auto& SelectedPath : ContextObject->SelectedPaths)
	{
		if (FName InternalPath; TryConvertVirtualPathToInternal(SelectedPath, InternalPath))
		{
			SelectedClassPaths.Add(InternalPath);
		}
	}

	FMarkdownNewFileContextMenu::FOnNewFileRequested OnOpenNewDynamicClassRequested;

	if (!SelectedClassPaths.IsEmpty())
	{
		OnOpenNewDynamicClassRequested = FMarkdownNewFileContextMenu::FOnNewFileRequested::CreateStatic(
			&UMarkdownContentBrowserDataSource::OnNewClassRequested);
	}

	FMarkdownNewFileContextMenu::MakeContextMenu(InMenu, SelectedClassPaths, OnOpenNewDynamicClassRequested);
}

void UMarkdownContentBrowserDataSource::OnDynamicClassUpdated()
{
	UpdateHierarchy();
}

void UMarkdownContentBrowserDataSource::OnEndGenerator()
{
	UpdateHierarchy();
}

void UMarkdownContentBrowserDataSource::UpdateHierarchy()
{
	MarkdownHierarchy.Reset();

	MarkdownHierarchy = MakeShareable(new FMarkdownContentBrowserHierarchy());

	SetVirtualPathTreeNeedsRebuild();

#if UE_U_CONTENT_BROWSER_DATA_SOURCE_NOTIFY_ITEM_DATA_REFRESHED
	NotifyItemDataRefreshed();
#else
	for (const auto& MatchingClass : MarkdownHierarchy->GetMatchingMDFiles(*DYNAMIC_ROOT_INTERNAL_PATH, true))
	{
		QueueItemDataUpdate(FContentBrowserItemDataUpdate::MakeItemAddedUpdate(CreateFileItem(MatchingClass)));
	}

	for (const auto& MatchingFolder : MarkdownHierarchy->GetMatchingFolders(*DYNAMIC_ROOT_INTERNAL_PATH, true))
	{
		QueueItemDataUpdate(FContentBrowserItemDataUpdate::MakeItemAddedUpdate(CreateFolderItem(MatchingFolder)));
	}
#endif
}

bool UMarkdownContentBrowserDataSource::IsRootInternalPath(const FName& InPath)
{
	return InPath.ToString().StartsWith(DYNAMIC_ROOT_INTERNAL_PATH);
}

FString UMarkdownContentBrowserDataSource::GetVirtualPath(const FName InClass)
{
	return !InClass.IsNone() ? DYNAMIC_ROOT_VIRTUAL_PATH / InClass.ToString() : FString();
}

TSharedPtr<const FMarkdownContentBrowserFileItemDataPayload> UMarkdownContentBrowserDataSource::GetFileItemDataPayload(
	const FContentBrowserItemData& InItemData) const
{
	return InItemData.GetOwnerDataSource() == this && InItemData.IsFile()
		       ? StaticCastSharedPtr<const FMarkdownContentBrowserFileItemDataPayload>(InItemData.GetPayload())
		       : nullptr;
}

TSharedPtr<const FMarkdownContentBrowserFolderItemDataPayload>
UMarkdownContentBrowserDataSource::GetFolderItemDataPayload(
	const FContentBrowserItemData& InItemData) const
{
	return InItemData.GetOwnerDataSource() == this && InItemData.IsFolder()
		       ? StaticCastSharedPtr<const FMarkdownContentBrowserFolderItemDataPayload>(InItemData.GetPayload())
		       : nullptr;
}

FContentBrowserItemData UMarkdownContentBrowserDataSource::CreateFolderItem(const FName& InFolderPath)
{
	FName VirtualPath;

	TryConvertInternalPathToVirtual(InFolderPath, VirtualPath);

	const auto FolderItemName = FPackageName::GetShortName(InFolderPath);

	FText DisplayNameOverride;

	if (InFolderPath == *DYNAMIC_ROOT_INTERNAL_PATH)
	{
		DisplayNameOverride = LOCTEXT("MarkdownFolderDisplayName", "Documentation");
	}
	else
	{
		DisplayNameOverride = ContentBrowserDataUtils::GetFolderItemDisplayNameOverride(
			InFolderPath, FolderItemName, true);
	}

	return FContentBrowserItemData(this,
	                               EContentBrowserItemFlags::Type_Folder | EContentBrowserItemFlags::Category_Misc
	                               ,
	                               VirtualPath,
	                               *FolderItemName,
	                               MoveTemp(DisplayNameOverride),
	                               MakeShared<FMarkdownContentBrowserFolderItemDataPayload>(InFolderPath)
	                               , InFolderPath
	);
}

FContentBrowserItemData UMarkdownContentBrowserDataSource::CreateFileItem(UMarkdownFile* InFile)
{
	if (InFile)
	return FContentBrowserItemData(this,
	                               EContentBrowserItemFlags::Type_File | EContentBrowserItemFlags::Category_Misc,
	                               *GetVirtualPath(*InFile->GetName()),
	                               *InFile->GetName(),
	                               FText(),
	                               MakeShared<FMarkdownContentBrowserFileItemDataPayload>(*InFile->GetName(), InFile)
	                               , *InFile->GetName()
	);
	return {};
}

bool UMarkdownContentBrowserDataSource::GetClassPaths(const TArrayView<const FCollectionNameType>& InCollections,
                                       const bool bIncludeChildCollections,
#if UE_F_TOP_LEVEL_ASSET_PATH
                                       TArray<FTopLevelAssetPath>& OutClassPaths) const
#else
                                       TArray<FName>& OutClassPaths) const
#endif
{
	if (!InCollections.IsEmpty())
	{
		const auto CollectionRecursionFlags = bIncludeChildCollections
			                                      ? ECollectionRecursionFlags::SelfAndChildren
			                                      : ECollectionRecursionFlags::Self;

		for (const auto& [Name, Type] : InCollections)
		{
			CollectionManager->GetClassesInCollection(Name, Type, OutClassPaths, CollectionRecursionFlags);
		}

		return true;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
