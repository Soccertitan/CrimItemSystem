// Copyright Soccertitan


#include "UI/ViewModel/CrimItemContainerViewModel.h"

#include "CrimItemContainer.h"
#include "CrimItemDefinition.h"
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
	return GetItemContainer()->GetConsumedCapacity();
}

int32 UCrimItemContainerViewModel::GetMaxCapacity() const
{
	return GetItemContainer()->GetMaxCapacity();
}

void UCrimItemContainerViewModel::OnItemContainerSet()
{
	Super::OnItemContainerSet();

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
