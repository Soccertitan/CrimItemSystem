// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "CrimItemList.generated.h"

class UCrimItemContainer;
class UCrimItem;
struct FCrimItemList;

/**
 * Holds an item instance.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemListEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FString ToDebugString() const;

	UCrimItem* GetItem() const { return Item; }
	int32 GetSlot() const { return Slot; }

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCrimItem> Item = nullptr;

	// The conceptual slot index the item occupies. Slot 0 is considered the first slot.
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 Slot = INDEX_NONE;

	// Not replicated version of the last item that was assigned.
	UPROPERTY(NotReplicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCrimItem> LastObserved = nullptr;

	friend FCrimItemList;
};

/**
 * Holds a list of items
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemList : public FFastArraySerializer
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_OneParam(FCrimItemListUpdatedSignature, const FCrimItemListEntry& );
	
	FCrimItemList() {}

	/** Called when an entry has been added to the list. */
	FCrimItemListUpdatedSignature OnEntryAddedDelegate;
	/** Called when an Item replaces an existing item. */
	FCrimItemListUpdatedSignature OnEntryUpdatedDelegate;
	/** Called when an item is removed from the list. */
	FCrimItemListUpdatedSignature OnEntryRemovedDelegate;
	
	/**
	 * Adds the item to the next available slot in the list.
	 */
	void AddEntry(UCrimItem* Item);
	
	/**
	 * Adds an item in the list at the specified slot. If an item already exists at the slot, it will overwrite the
	 * existing item.
	 * @param Item The item to add.
	 * @param Slot The slot to place the item. Must be >= to 0.
	 */
	void AddEntryAt(UCrimItem* Item, int32 Slot);
	
	/**
	 * Removes the items that are the same item instance from the list.
	 * @param bRemoveAll If true, will remove all matching instances from the list. 
	 */
	void RemoveEntry(UCrimItem* Item, bool bRemoveAll = false);
	
	/**
	 * Remove the item at the specified slot.
	 * @param Slot The Slot to remove an entry.
	 */
	void RemoveEntryAt(int32 Slot);

	/** Removes all items in the list. */
	void Reset();
	
	/** Gets a const reference to the items. */
	const TArray<FCrimItemListEntry>& GetItems() const;

	/** Gets a const in slot order reference to the items. */
	const TArray<UCrimItem*>& GetItemsInOrder() const;

	/**
	 * Finds an item at the specified slot.
	 * @param Slot Must be a value >= 0.
	 */
	UCrimItem* GetItemAtSlot(int32 Slot) const;

	/**
	 * Find the next slot that does not have an item.
	 * @param StartingSlot The slot (index) to start searching at.
	 * @return The next empty slot.
	 */
	int32 NextAvailableSlot(int32 StartingSlot = 0) const;

	// FFastArraySerializer
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FCrimItemListEntry, FCrimItemList>(Items, DeltaParms, *this);
	}

private:
	/** Replicated list of items and their quantity. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<FCrimItemListEntry> Items;

	/** Cached array of items that are in slot order. For faster lookup if slot order is important. */
	UPROPERTY(NotReplicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<TObjectPtr<UCrimItem>> OrderedItems;

	/**
	 * Updates the OrderedItems cache with the new item.
	 * @param Item The item to cache or nullptr to clear.
	 * @param Slot The slot/Index to cache the item at.
	 */
	void CacheSlot(UCrimItem* Item, int32 Slot);
};

template <>
struct TStructOpsTypeTraits<FCrimItemList> : public TStructOpsTypeTraitsBase2<FCrimItemList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
