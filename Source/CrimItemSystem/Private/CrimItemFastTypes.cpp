// Copyright Soccertitan


#include "CrimItemFastTypes.h"

#include "CrimItemContainer.h"
#include "CrimItemDefinition.h"


//----------------------------------------------------------------------------------------
// FastCrimItem
//----------------------------------------------------------------------------------------
void FFastCrimItem::Initialize(const TInstancedStruct<FCrimItem>& InItem)
{
	Item = InItem;

	// Make a copy of the Item for change comparison.
	PreReplicatedChangeItem = InItem;
}

void FFastCrimItem::PostReplicatedAdd(const FFastCrimItemList& InItemList)
{
	// Update our cached state.
	PreReplicatedChangeItem = Item;

	InItemList.OnItemAddedDelegate.Broadcast(*this);
}

void FFastCrimItem::PostReplicatedChange(const FFastCrimItemList& InItemList)
{
	InItemList.OnItemChangedDelegate.Broadcast(*this);
	PreReplicatedChangeItem = Item;
}

void FFastCrimItem::PreReplicatedRemove(const FFastCrimItemList& InItemList)
{
	InItemList.OnItemRemovedDelegate.Broadcast(*this);
}

//----------------------------------------------------------------------------------------
// FastCrimItemList
//----------------------------------------------------------------------------------------

void FFastCrimItemList::AddItem(const TInstancedStruct<FCrimItem>& Item)
{
	check(Item.IsValid());

	FFastCrimItem& NewItem = Items.AddDefaulted_GetRef();
	NewItem.Initialize(Item);

	OnItemAddedDelegate.Broadcast(NewItem);
	MarkItemDirty(NewItem);
}

bool FFastCrimItemList::RemoveItem(const FGuid& ItemId)
{
	for (int32 i = Items.Num() - 1; i >= 0; i--)
	{
		const FCrimItem* ItemPtr = Items[i].Item.GetPtr<FCrimItem>();
		if (ItemPtr && ItemPtr->GetItemGuid() == ItemId)
		{
			FFastCrimItem OldItem = Items[i];
			Items.RemoveAtSwap(i);

			OnItemRemovedDelegate.Broadcast(OldItem);
			MarkArrayDirty();
			return true;
		}
	}
	return false;
}

const TArray<FFastCrimItem>& FFastCrimItemList::GetItems() const
{
	return Items;
}

FFastCrimItem* FFastCrimItemList::GetItem(const FGuid& ItemGuid) const
{
	for (const FFastCrimItem& Item : Items)
	{
		const FCrimItem* ItemPtr = Item.Item.GetPtr<FCrimItem>();
		if (ItemPtr && ItemPtr->GetItemGuid() == ItemGuid)
		{
			return const_cast<FFastCrimItem*>(&Item);
		}
	}
	return nullptr;
}

int32 FFastCrimItemList::GetNum() const
{
	return Items.Num();
}

void FFastCrimItemList::Reset()
{
	TArray<FFastCrimItem> TempEntries = Items;
	Items.Empty();
	for (FFastCrimItem& Entry : TempEntries)
	{
		OnItemRemovedDelegate.Broadcast(Entry);
	}
	MarkArrayDirty();
}

//--------------------------------------------------------------------------
// FastCrimItemContainer
//--------------------------------------------------------------------------
FString FFastCrimItemContainer::ToDebugString() const
{
	return ItemContainer ? ItemContainer->GetFName().ToString() : TEXT("(none)");
}

void FFastCrimItemContainer::PostReplicatedAdd(const FFastCrimItemContainerList& InContainer)
{
	InContainer.OnItemContainerAddedDelegate.Broadcast(*this);
}

void FFastCrimItemContainer::PreReplicatedRemove(const FFastCrimItemContainerList& InContainer)
{
	InContainer.OnItemContainerRemovedDelegate.Broadcast(*this);
}

//----------------------------------------------------------------------------------------
// FastCrimItemContainerList
//----------------------------------------------------------------------------------------

void FFastCrimItemContainerList::AddItemContainer(UCrimItemContainer* ItemContainer)
{
	if (IsValid(ItemContainer))
	{
		// Duplicate check.
		for (const FFastCrimItemContainer& Entry : Items)
		{
			if (Entry.ItemContainer == ItemContainer)
			{
				return;
			}
		}

		FFastCrimItemContainer& NewEntry = Items.AddDefaulted_GetRef();
		NewEntry.ItemContainer = ItemContainer;
		OnItemContainerAddedDelegate.Broadcast(NewEntry);
		MarkItemDirty(NewEntry);
	}
}

void FFastCrimItemContainerList::RemoveItemContainer(UCrimItemContainer* ItemContainer)
{
	if (IsValid(ItemContainer))
	{
		for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
		{
			FFastCrimItemContainer& Entry = *EntryIt;
			if (Entry.ItemContainer == ItemContainer)
			{
				FFastCrimItemContainer TempEntry(Entry);
				EntryIt.RemoveCurrentSwap();
				OnItemContainerRemovedDelegate.Broadcast(TempEntry);
				return;
			}
		}
	}
}

const TArray<FFastCrimItemContainer>& FFastCrimItemContainerList::GetItemContainers() const
{
	return Items;
}

