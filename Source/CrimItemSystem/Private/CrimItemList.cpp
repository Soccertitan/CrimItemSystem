// Copyright Soccertitan


#include "CrimItemList.h"

#include "CrimItem.h"


FString FCrimItemListEntry::ToDebugString() const
{
	return Item ? Item->ToDebugString() : TEXT("(none)");
}

const TArray<FCrimItemListEntry>& FCrimItemList::GetItems() const
{
	return Items;
}

const TArray<UCrimItem*>& FCrimItemList::GetItemsInOrder() const
{
	return OrderedItems;
}

UCrimItem* FCrimItemList::GetItemAtSlot(int32 Slot) const
{
	if (Slot < 0)
	{
		return nullptr;
	}

	if (OrderedItems.IsValidIndex(Slot))
	{
		return OrderedItems[Slot];
	}
	
	return nullptr;
}

int32 FCrimItemList::NextAvailableSlot(int32 StartingSlot) const
{
	if (StartingSlot < 0)
	{
		StartingSlot = 0;
	}
	
	if (StartingSlot >= OrderedItems.Num())
	{
		return StartingSlot;
	}

	int32 NextSlot = StartingSlot;
	for (NextSlot; NextSlot < OrderedItems.Num(); NextSlot++)
	{
		// The value will be -1 if it's available.
		if (OrderedItems[NextSlot] == nullptr)
		{
			return NextSlot;
		}
	}
	return NextSlot;
}

void FCrimItemList::AddEntry(UCrimItem* Item)
{
	check(IsValid(Item))

	int32 NewSlot = NextAvailableSlot();
	CacheSlot(Item, NewSlot);

	FCrimItemListEntry& NewEntry = Items.AddDefaulted_GetRef();
	NewEntry.Item = Item;
	NewEntry.Slot = NewSlot;
	NewEntry.LastObserved = Item;
	OnEntryAddedDelegate.Broadcast(NewEntry);
	MarkItemDirty(NewEntry);
}

void FCrimItemList::AddEntryAt(UCrimItem* Item, int32 Slot)
{
	check(IsValid(Item))
	
	if (Slot < 0)
	{
		return;
	}
	
	// Search for an existing slot and update.
	for (FCrimItemListEntry& Entry : Items)
	{
		if (Entry.Slot == Slot)
		{
			CacheSlot(Item, Slot);
			Entry.LastObserved = Entry.Item;
			Entry.Item = Item;
			OnEntryUpdatedDelegate.Broadcast(Entry);
			MarkItemDirty(Entry);
			return;
		}
	}

	CacheSlot(Item, Slot);

	// Did not find an existing slot earlier. Create a new entry in the array.
	FCrimItemListEntry& NewEntry = Items.AddDefaulted_GetRef();
	NewEntry.Item = Item;
	NewEntry.LastObserved = Item;
	NewEntry.Slot = Slot;
	OnEntryAddedDelegate.Broadcast(NewEntry);
	MarkItemDirty(NewEntry);
}

void FCrimItemList::RemoveEntry(UCrimItem* Item, bool bRemoveAll)
{
	if (!IsValid(Item))
	{
		return;
	}
	
	bool bRemovedEntry = false;
	for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
	{
		FCrimItemListEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			CacheSlot(nullptr, Entry.GetSlot());
			
			Entry.LastObserved = Entry.Item;
			FCrimItemListEntry TempEntry(Entry);
			EntryIt.RemoveCurrentSwap();
			OnEntryRemovedDelegate.Broadcast(TempEntry);
			bRemovedEntry = true;
			
			if (!bRemoveAll)
			{
				break;
			}
		}
	}

	if (bRemovedEntry)
	{
		MarkArrayDirty();
	}
}

void FCrimItemList::RemoveEntryAt(int32 Slot)
{
	if (Slot < 0)
	{
		return;
	}
	
	for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
	{
		FCrimItemListEntry& Entry = *EntryIt;
		if (Entry.Slot == Slot)
		{
			CacheSlot(nullptr, Slot);
			
			Entry.LastObserved = Entry.Item;
			FCrimItemListEntry TempEntry(Entry);
			EntryIt.RemoveCurrentSwap();
			OnEntryRemovedDelegate.Broadcast(TempEntry);
			MarkArrayDirty();
			return;
		}
	}
}

void FCrimItemList::Reset()
{
	TArray<FCrimItemListEntry> TempEntries = Items;
	Items.Empty();
	OrderedItems.Empty();
	for (FCrimItemListEntry& Entry : TempEntries)
	{
		OnEntryRemovedDelegate.Broadcast(Entry);
	}
	MarkArrayDirty();
}

void FCrimItemList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Idx : RemovedIndices)
	{
		CacheSlot(nullptr, Items[Idx].GetSlot());
		OnEntryRemovedDelegate.Broadcast(Items[Idx]);
		Items[Idx].LastObserved = Items[Idx].Item;
	}
}

void FCrimItemList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		CacheSlot(Items[Idx].GetItem(), Items[Idx].GetSlot());
		OnEntryAddedDelegate.Broadcast(Items[Idx]);
		Items[Idx].LastObserved = Items[Idx].Item;
	}
}

void FCrimItemList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Idx : ChangedIndices)
	{
		CacheSlot(Items[Idx].GetItem(), Items[Idx].GetSlot());
		OnEntryUpdatedDelegate.Broadcast(Items[Idx]);
		Items[Idx].LastObserved = Items[Idx].Item;
	}
}

void FCrimItemList::CacheSlot(UCrimItem* Item, int32 Slot)
{
	if (Slot >= OrderedItems.Num())
	{
		OrderedItems.SetNum(Slot + 1);
	}

	OrderedItems[Slot] = Item;
}

// void FCrimItemList::CacheSlot(int32 Slot, bool bClearSlot)
// {
// 	if (Slot >= OrderedSlots.Num())
// 	{
// 		int32 Delta = Slot + 1 - OrderedSlots.Num();
// 		TArray<int32> NewSlots;
// 		NewSlots.Init(INDEX_NONE, Delta);
// 		OrderedSlots.Append(NewSlots);
// 	}
//
// 	if (bClearSlot)
// 	{
// 		OrderedSlots[Slot] = INDEX_NONE;
// 	}
// 	else
// 	{
// 		OrderedSlots[Slot] = Slot;
// 	}
// }
