// Copyright Soccertitan


#include "CrimItemContainerList.h"

#include "CrimItemContainer.h"


FString FCrimItemContainerListEntry::ToDebugString() const
{
	return ItemContainer ? ItemContainer->GetFName().ToString() : TEXT("(none)");
}

void FCrimItemContainerList::AddEntry(UCrimItemContainer* ItemContainer)
{
	if (IsValid(ItemContainer))
	{
		// Duplicate check.
		for (const FCrimItemContainerListEntry& Entry : Items)
		{
			if (Entry.ItemContainer == ItemContainer)
			{
				return;
			}
		}

		FCrimItemContainerListEntry& NewEntry = Items.AddDefaulted_GetRef();
		NewEntry.ItemContainer = ItemContainer;
		OnEntryAddedDelegate.Broadcast(NewEntry);
		MarkItemDirty(NewEntry);
	}
}

void FCrimItemContainerList::RemoveEntry(UCrimItemContainer* ItemContainer)
{
	if (IsValid(ItemContainer))
	{
		for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
		{
			FCrimItemContainerListEntry& Entry = *EntryIt;
			if (Entry.ItemContainer == ItemContainer)
			{
				FCrimItemContainerListEntry TempEntry(Entry);
				EntryIt.RemoveCurrentSwap();
				OnEntryRemovedDelegate.Broadcast(TempEntry);
				return;
			}
		}
	}
}

const TArray<FCrimItemContainerListEntry>& FCrimItemContainerList::GetItemContainers() const
{
	return Items;
}

void FCrimItemContainerList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		OnEntryAddedDelegate.Broadcast(Items[Idx]);
	}
}

void FCrimItemContainerList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Idx : RemovedIndices)
	{
		OnEntryRemovedDelegate.Broadcast(Items[Idx]);
	}
}
