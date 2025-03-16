// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "CrimItemContainerList.generated.h"

class UCrimItemManagerComponent;
struct FCrimItemContainerList;
class UCrimItemContainer;

/**
 * Holds an ItemContainer.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemContainerListEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FString ToDebugString() const;

	UCrimItemContainer* GetItemContainer() const { return ItemContainer; }

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCrimItemContainer> ItemContainer = nullptr;

	friend FCrimItemContainerList;

	friend bool operator==(const FCrimItemContainerListEntry& X, const FCrimItemContainerListEntry& Y)
	{
		return X.ItemContainer == Y.ItemContainer;
	}
	friend bool operator!=(const FCrimItemContainerListEntry& X, const FCrimItemContainerListEntry& Y)
	{
		return X.ItemContainer != Y.ItemContainer;
	}
};

/**
 * Holds a list of item containers.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemContainerList : public FFastArraySerializer
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_OneParam(FCrimItemContainerListUpdatedSignature, const FCrimItemContainerListEntry& );
	
	FCrimItemContainerList() {}

	/** Called when an ItemContainer has been added to the list. */
	FCrimItemContainerListUpdatedSignature OnEntryAddedDelegate;
	/** Called when an ItemContainer is removed from the list. */
	FCrimItemContainerListUpdatedSignature OnEntryRemovedDelegate;
	
	/** Adds a new item container the array. */
	void AddEntry(UCrimItemContainer* ItemContainer);

	/**
	 * Removes the matching item container from the array.
	 */
	void RemoveEntry(UCrimItemContainer* ItemContainer);
	
	/** Gets a const reference to the item containers. */
	const TArray<FCrimItemContainerListEntry>& GetItemContainers() const;

	// FFastArraySerializer
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FCrimItemContainerListEntry, FCrimItemContainerList>(Items, DeltaParms, *this);
	}

private:
	/** Replicated list of items and their quantity. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<FCrimItemContainerListEntry> Items;
};

template <>
struct TStructOpsTypeTraits<FCrimItemContainerList> : public TStructOpsTypeTraitsBase2<FCrimItemContainerList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
