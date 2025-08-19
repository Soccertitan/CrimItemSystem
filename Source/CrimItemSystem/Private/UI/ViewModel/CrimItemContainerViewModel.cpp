// Copyright Soccertitan


#include "UI/ViewModel/CrimItemContainerViewModel.h"

#include "ItemContainer/CrimItemContainerBase.h"
#include "CrimItemDefinition.h"
#include "ItemContainer/CrimItemContainer.h"
#include "UI/ViewModel/CrimItemViewModelBase.h"

FText UCrimItemContainerViewModel::GetItemContainerName() const
{
	return GetItemContainer()->GetDisplayName();
}

TArray<UCrimItemViewModelBase*> UCrimItemContainerViewModel::GetItems() const
{
	return ItemViewModels;
}

int32 UCrimItemContainerViewModel::GetConsumedCapacity() const
{
	return GetCrimItemContainer()->GetConsumedCapacity();
}

int32 UCrimItemContainerViewModel::GetMaxCapacity() const
{
	return GetCrimItemContainer()->GetMaxCapacity();
}

UCrimItemContainer* UCrimItemContainerViewModel::GetCrimItemContainer() const
{
	return Cast<UCrimItemContainer>(GetItemContainer());
}

void UCrimItemContainerViewModel::OnItemContainerSet()
{
	Super::OnItemContainerSet();

	checkf(GetCrimItemContainer(), TEXT("The ItemContainer is not of type CrimItemContainer. Update the Item Container "
		"ViewModel in %s"), *GetItemContainer()->GetName());

	ItemViewModels.Empty();
	for (const FFastCrimItem& FastItem : GetItemContainer()->GetItems())
	{
		UCrimItemViewModelBase* NewVM = CreateItemViewModel(FastItem.Item);
		ItemViewModels.Add(NewVM);
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMaxCapacity);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItemContainerName);
	BroadcastUpdates();
}

void UCrimItemContainerViewModel::OnItemAdded(const TInstancedStruct<FCrimItem>& Item)
{
	ItemViewModels.Add(CreateItemViewModel(Item));
	BroadcastUpdates();
}

void UCrimItemContainerViewModel::OnItemRemoved(const TInstancedStruct<FCrimItem>& Item)
{
	const FCrimItem* ItemPtr = Item.GetPtr<FCrimItem>();

	UCrimItemViewModelBase* VMToRemove = nullptr;
	for (UCrimItemViewModelBase* VM : ItemViewModels)
	{
		if (VM->GetItemGuid() == ItemPtr->GetItemGuid())
		{
			VMToRemove = VM;
			break;
		}
	}

	if (IsValid(VMToRemove))
	{
		ItemViewModels.Remove(VMToRemove);
		BroadcastUpdates();
	}
}

void UCrimItemContainerViewModel::BroadcastUpdates()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetConsumedCapacity);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItems);
}
