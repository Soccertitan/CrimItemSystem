// Copyright Soccertitan


#include "ItemDrop/CrimItemDropList.h"

#include "CrimItem.h"
#include "ItemDrop/CrimItemDrop.h"


void FCrimItemDropList::AddEntry(FCrimItemDropListEntry& Entry)
{
	if (!IsValid(Entry.ItemDropActor) || !IsValid(Entry.Item))
	{
		return;
	}

	for (const FCrimItemDropListEntry& Item : Items)
	{
		if (Item == Entry)
		{
			return;
		}
	}

	FCrimItemDropListEntry& NewItem = Items.AddDefaulted_GetRef();
	NewItem.Item = Entry.Item;
	NewItem.ItemDropActor = Entry.ItemDropActor;
	MarkItemDirty(NewItem);
}

void FCrimItemDropList::RemoveEntry(ACrimItemDrop* ItemDropActor)
{
	if (!IsValid(ItemDropActor))
	{
		return;
	}

	bool bRemovedEntry = false;
	for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
	{
		FCrimItemDropListEntry& Entry = *EntryIt;
		if (Entry.ItemDropActor == ItemDropActor)
		{
			EntryIt.RemoveCurrentSwap();
			bRemovedEntry = true;
		}
	}

	if (bRemovedEntry)
	{
		MarkArrayDirty();
	}
}

const TArray<FCrimItemDropListEntry>& FCrimItemDropList::GetItems() const
{
	return Items;
}
