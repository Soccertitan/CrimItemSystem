// Copyright Soccertitan


#include "UI/ViewModel/CrimItemViewModelBase.h"

#include "CrimItemContainer.h"
#include "CrimItemDefinition.h"
#include "Engine/AssetManager.h"

UCrimItemViewModelBase::UCrimItemViewModelBase()
{
	Bundles.Add("UI");
}

void UCrimItemViewModelBase::SetItem(const TInstancedStruct<FCrimItem>& InItem)
{
	checkf(InItem.IsValid(), TEXT("The passed in Item is invalid in %s"), *GetName());

	Item = InItem;
	const FCrimItem* ItemPtr = Item.GetPtr<FCrimItem>();
	ItemGuid = ItemPtr->GetItemGuid();
	OnItemSet();
	
	ItemPtr->GetItemContainer()->OnItemChangedDelegate.AddUObject(this, &UCrimItemViewModelBase::Internal_OnItemChanged);

	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this,
		&UCrimItemViewModelBase::Internal_OnItemDefinitionLoaded, ItemPtr->GetItemDefinition());
	FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(ItemPtr->GetItemDefinition().ToSoftObjectPath());
	ItemDefStreamableHandle = UAssetManager::Get().PreloadPrimaryAssets({AssetId}, Bundles, true, Delegate);
}

const TInstancedStruct<FCrimItem>& UCrimItemViewModelBase::GetItem() const
{
	return Item;
}

void UCrimItemViewModelBase::Internal_OnItemChanged(UCrimItemContainer* ItemContainer, const FFastCrimItem& FastItem)
{
	const FCrimItem* ChangedItem = FastItem.Item.GetPtr<FCrimItem>();
	if (ChangedItem->GetItemGuid() != ItemGuid)
	{
		return;
	}

	Item = FastItem.Item;
	OnItemChanged();
}

void UCrimItemViewModelBase::Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UCrimItemDefinition> ItemDefinition)
{
	OnItemDefinitionLoaded(ItemDefinition.Get());
}
