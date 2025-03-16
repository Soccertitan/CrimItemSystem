// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "StructUtils/InstancedStruct.h"

#include "CrimItemTypes.generated.h"

class UCrimItemContainer;
class ACrimItemDrop;
class UCrimItem;
class UCrimItemDef;
struct FCrimItemTagStackContainer;

/**
 * Defines limitations for the quantity of an item.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemQuantityLimit
{
	GENERATED_BODY()

	FCrimItemQuantityLimit()
	{
	}

	/** Return the max quantity or MAX_Int32 if unlimited. */
	int32 GetMaxQuantity() const;

private:
	/** Limit the quantity of this item. */
	UPROPERTY(EditAnywhere, Meta = (InlineEditConditionToggle), Category = "CrimItem")
	bool bLimitQuantity = false;

	/** The maximum quantity allowed for the item. */
	UPROPERTY(EditAnywhere, Meta = (EditCondition="bLimitQuantity", ClampMin = 1), Category = "CrimItem")
	int32 MaxQuantity = 1;
};


/**
 * Represents a single gameplay tag and a count.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FCrimItemTagStack()
	{
	}

	FCrimItemTagStack(FGameplayTag InTag, int32 InCount)
		: Tag(InTag),
		  Count(InCount)
	{
	}

	FString ToDebugString() const;

	bool operator==(const FCrimItemTagStack& Other) const
	{
		return Tag == Other.Tag && Count == Other.Count;
	}

	bool operator!=(const FCrimItemTagStack& Other) const
	{
		return Tag != Other.Tag || Count != Other.Count;
	}

	const FGameplayTag& GetTag() const;
	int32 GetCount() const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FGameplayTag Tag;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 Count = 0;

	// Used to track delta's.
	UPROPERTY(NotReplicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 LastObservedCount = INDEX_NONE;

	friend FCrimItemTagStackContainer;
};


/**
 * Container of game item tag stacks, designed for fast replication.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FCrimItemTagStackContainerUpdatedSignature, const FGameplayTag&, int32 /*New Count*/, int32 /*Old Count*/);

	FCrimItemTagStackContainer()
	{
	}

	FCrimItemTagStackContainerUpdatedSignature OnTagCountUpdatedDelegate;

	/** Add stacks to a tag. */
	void AddStack(FGameplayTag Tag, int32 DeltaCount);

	/** Remove stacks from a tag. If DeltaCount is -1 removes all. */
	void RemoveStack(FGameplayTag Tag, int32 DeltaCount);

	/** Return the stack count for a tag, or 0 if the tag is not present. */
	int32 GetStackCount(FGameplayTag Tag) const
	{
		return StackCountMap.FindRef(Tag);
	}

	/** Returns a const reference to the current TagStats. */
	const TArray<FCrimItemTagStack>& GetTagStats() const;

	/** Return true if there is at least one count of a tag. */
	bool ContainsTag(FGameplayTag Tag, bool bExactMatch = false) const;

	/** Empties all stats in this container. */
	void Empty();

	// FFastArraySerializer
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FCrimItemTagStack, FCrimItemTagStackContainer>(Items, DeltaParms, *this);
	}

	void PostSerialize(const FArchive& Ar);

	FString ToDebugString() const;

	bool operator ==(const FCrimItemTagStackContainer& Other) const
	{
		return Items == Other.Items;
	}

	bool operator !=(const FCrimItemTagStackContainer& Other) const
	{
		return !(*this == Other);
	}

private:
	/** Replicated array of gameplay tag stacks. */
	UPROPERTY()
	TArray<FCrimItemTagStack> Items;

	/** Cached map of stack counts by tag, for faster lookup. */
	UPROPERTY(NotReplicated, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TMap<FGameplayTag, int32> StackCountMap;
};

template <>
struct TStructOpsTypeTraits<FCrimItemTagStackContainer> : public TStructOpsTypeTraitsBase2<FCrimItemTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithPostSerialize = true,
	};
};

UENUM(BlueprintType)
enum class ECrimItemAddResult : uint8
{
	NoItemsAdded UMETA(DIsplayName = "No items added"),
	SomeItemsAdded UMETA(DisplayName = "Some items added"),
	AllItemsAdded UMETA(DisplayName = "All items added")
};

/**
 * Describes how to add an item to a slot.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemAddPlanSlot
{
	GENERATED_BODY()

	FCrimItemAddPlanSlot(){}
	FCrimItemAddPlanSlot(int32 Slot, int32 Quantity, bool bIsSlotEmpty) :
		TargetSlot(Slot),
		QuantityToAdd(Quantity),
		bSlotIsEmpty(bIsSlotEmpty){}

	// The slot to add the item to.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TargetSlot = INDEX_NONE;

	// The amount of item to add.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 QuantityToAdd = 0;

	// If the slot is empty and can have an item.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSlotIsEmpty = true;

	// Returns true if the TargetSlot is >= 0 and the QuantityToAdd is > 0.
	bool IsValid() const;
};

/**
 * Represents a plan for adding an item to a container. Which slots to place the item and if a new item
 * needs to be created for the slot.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemAddPlan
{
	GENERATED_BODY()

	FCrimItemAddPlan(){}
	FCrimItemAddPlan(int32 InItemQuantity) : AmountToGive(InItemQuantity), AmountGiven(0) {}

	//The amount of the item that we tried to add
	UPROPERTY(BlueprintReadOnly)
	int32 AmountToGive = 0;

	//The amount of the item that was actually added in the end. Maybe we tried adding 10 items, but only 8 could be added because of capacity/weight
	UPROPERTY(BlueprintReadOnly)
	int32 AmountGiven = 0;

	//The result
	UPROPERTY(BlueprintReadOnly)
	ECrimItemAddResult Result = ECrimItemAddResult::NoItemsAdded;

	// Describes the reason for failure to add all items.
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Error;

	/** Returns true if all SlotPlans have valid slots and quantity to add are greater than 0. */
	bool IsValid() const;
	
	/** Adds a new plan to the SlotsPlans array.*/
	void AddSlotPlan(const FCrimItemAddPlanSlot& InSlotPlan);

	const TArray<FCrimItemAddPlanSlot>& GetSlotPlans() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<FCrimItemAddPlanSlot> SlotPlans;

	// Adds to the AmountGiven and updates the ECrimItemAddResult.
	void UpdateAmountGiven(int32 NewValue);
};

/**
 * The spec used when creating new items (not copies of items).
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemSpec
{
	GENERATED_BODY()
	
	/** The item definition to associate with the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UCrimItemDef> ItemDef;

	/** The quantity amount to grant for this item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	/** Will apply the default stats from the ItemDef if true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bApplyDefaultStats = true;

	/** Stats to grant the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, int32> ItemStats;
};

/**
 * Spec on linking a parent container to child containers.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemContainerLinkSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ParentContainerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTag> ChildContainerIds;
};

/**
 * Params for the ItemDropManager on how to create a new ItemDrop actor.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemDropParams
{
	GENERATED_BODY()

	/** The ItemDrop actor class to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACrimItemDrop> ItemDropClass;

	/** The spawn location for the ItemDrop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SpawnLocation = FVector();

	/** The context for dropping the item. This will be passed along to the ItemDrop. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UObject> Context;
};
