// Copyright Soccertitan


#include "UI/ViewModel/CrimItemViewModel.h"

#include "CrimItemDefinition.h"
#include "CrimItemFastTypes.h"
#include "Engine/AssetManager.h"
#include "ItemContainer/CrimItemContainerBase.h"
#include "ItemDefinitionFragment/CrimItemDefFrag_UI.h"

UCrimItemViewModel::UCrimItemViewModel()
{
	Bundles.Add("UI");
}

void UCrimItemViewModel::OnItemSet()
{
	const FCrimItem* ItemPtr = GetItem().GetPtr<FCrimItem>();

	// Updating the delegate for listening to item changes.
	if (ItemContainerBase.Get() != ItemPtr->GetItemContainer())
	{
		if (ItemChangedDelegateHandle.IsValid() && IsValid(ItemContainerBase.Get()))
		{
			ItemContainerBase.Get()->OnItemChangedDelegate.Remove(ItemChangedDelegateHandle);
		}

		ItemContainerBase = ItemPtr->GetItemContainer();
		if (IsValid(ItemPtr->GetItemContainer()))
		{
			ItemChangedDelegateHandle = ItemPtr->GetItemContainer()->OnItemChangedDelegate.AddUObject(this, &UCrimItemViewModel::Internal_OnItemChanged);
		}
	}

	FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this,
		&UCrimItemViewModel::Internal_OnItemDefinitionLoaded, ItemPtr->GetItemDefinition());
	FPrimaryAssetId AssetId = UAssetManager::Get().GetPrimaryAssetIdForPath(
		ItemPtr->GetItemDefinition().ToSoftObjectPath());
	ItemDefStreamableHandle = UAssetManager::Get().PreloadPrimaryAssets(
		{AssetId}, Bundles, true, Delegate);

	SetQuantity(ItemPtr->Quantity);
}

void UCrimItemViewModel::OnItemDefinitionLoaded(const UCrimItemDefinition* ItemDefinition)
{
	const FCrimItemDefFrag_UI* UIFrag = ItemDefinition->GetFragmentByType<FCrimItemDefFrag_UI>();
	SetItemName(UIFrag->ItemName);
	SetItemDescription(UIFrag->ItemDescription);
	SetIcon(UIFrag->ItemIcon.Get());
}

void UCrimItemViewModel::SetItemName(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemName, InValue);
}

void UCrimItemViewModel::SetItemDescription(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemDescription, InValue);
}

void UCrimItemViewModel::SetIcon(UTexture2D* InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Icon, InValue);
}

void UCrimItemViewModel::SetQuantity(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Quantity, InValue);
}

void UCrimItemViewModel::Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UCrimItemDefinition> ItemDefinition)
{
	OnItemDefinitionLoaded(ItemDefinition.Get());
}

void UCrimItemViewModel::Internal_OnIconLoaded(TSoftObjectPtr<UTexture2D> InIcon)
{
	SetIcon(InIcon.Get());
}

void UCrimItemViewModel::Internal_OnItemChanged(UCrimItemContainerBase* ItemContainer, const FFastCrimItem& FastItem)
{
	const FCrimItem* ChangedItem = FastItem.Item.GetPtr<FCrimItem>();
	if (ChangedItem->GetItemGuid() == GetItemGuid())
	{
		SetItem(FastItem.Item);
	}
}
