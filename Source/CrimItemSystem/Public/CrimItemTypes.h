// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"

#include "CrimItemTypes.generated.h"

class UCrimItemSet;
class UCrimItemInstance;
class UCrimItemContainerBase;
struct FCrimItem;
struct FCrimItemFragment;
struct FCrimItemTagStackContainer;
struct FFastCrimItem;
class ACrimItemDrop;
class UCrimItemManagerComponent;
class UCrimItemContainer;
class UCrimItemDefinition;

/**
 * Defines limitations for the quantity of an item.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemQuantityLimit
{
	GENERATED_BODY()

	FCrimItemQuantityLimit(){}

	/** Return the max quantity or MAX_Int32 if unlimited. */
	int32 GetMaxQuantity() const;

	/** Limit the quantity of this item. */
	UPROPERTY(EditAnywhere, Meta = (InlineEditConditionToggle), Category = "CrimItem")
	bool bLimitQuantity = false;

	/** The maximum quantity allowed for the item. */
	UPROPERTY(EditAnywhere, Meta = (EditCondition="bLimitQuantity", ClampMin = 1), Category = "CrimItem", DisplayName = "LimitQuantity")
	int32 MaxQuantity = 1;
};

UENUM(BlueprintType)
enum class ECrimAddItemResult : uint8
{
	NoItemsAdded UMETA(DIsplayName = "No items added"),
	SomeItemsAdded UMETA(DisplayName = "Some items added"),
	AllItemsAdded UMETA(DisplayName = "All items added")
};

/**
 * Describes how to add a specific item.
 */
USTRUCT()
struct CRIMITEMSYSTEM_API FCrimAddItemPlanEntry
{
	GENERATED_BODY()

	FCrimAddItemPlanEntry(){}
	FCrimAddItemPlanEntry(FFastCrimItem* InFastItemPtr, int32 Quantity) :
		FastItemPtr(InFastItemPtr),
		QuantityToAdd(Quantity)
		{}

	// The item to add quantity to. If nullptr, make a new item.
	FFastCrimItem* FastItemPtr = nullptr;
	
	// The amount of item to add.
	UPROPERTY()
	int32 QuantityToAdd = 0;

	// Returns true if the QuantityToAdd is > 0.
	bool IsValid() const;
};

/**
 * Represents a plan and expected results for adding an item to a container.
 */
USTRUCT()
struct CRIMITEMSYSTEM_API FCrimAddItemPlan
{
	GENERATED_BODY()

	FCrimAddItemPlan(){}
	FCrimAddItemPlan(int32 InItemQuantity) : AmountToGive(InItemQuantity), AmountGiven(0) {}

	// The amount of the item that we tried to add
	UPROPERTY()
	int32 AmountToGive = 0;

	// The amount of the item that was actually added in the end. Maybe we tried adding 10 items, but only 8 could be added because of capacity/weight
	UPROPERTY()
	int32 AmountGiven = 0;

	// The result
	UPROPERTY()
	ECrimAddItemResult Result = ECrimAddItemResult::NoItemsAdded;

	// Describes the reason for failure to add all items.
	UPROPERTY()
	FGameplayTag Error;

	/** Returns true if all SlotPlans have valid slots and quantity to add are greater than 0. */
	bool IsValid() const;
	
	/** Adds a new Entry to the Entries array. Will properly update the AmountGiven. */
	void AddEntry(const FCrimAddItemPlanEntry& InEntry);

	const TArray<FCrimAddItemPlanEntry>& GetEntries() const;

private:
	UPROPERTY()
	TArray<FCrimAddItemPlanEntry> Entries;

	// Adds to the AmountGiven and updates the ECrimItemAddResult.
	void UpdateAmountGiven(int32 NewValue);
};

/**
 * Represents the items added to an ItemContainer.
 */
USTRUCT(BlueprintType)
struct FCrimAddItemResult
{
	GENERATED_BODY()
	FCrimAddItemResult(){}
	FCrimAddItemResult(const FCrimAddItemPlan& InPlan, const TArray<TInstancedStruct<FCrimItem>>& InItems);

	/** A copy of the items added to the ItemContainer. */
	UPROPERTY(BlueprintReadOnly)
	TArray<TInstancedStruct<FCrimItem>> Items;

	// The amount of the item that we tried to add
	UPROPERTY(BlueprintReadOnly)
	int32 AmountToGive = 0;

	/**
	 * The amount of the item that was actually added in the end. Maybe we tried adding 10 items, but only 8 could be
	 * added because of capacity/weight.
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 AmountGiven = 0;

	// The result
	UPROPERTY(BlueprintReadOnly)
	ECrimAddItemResult Result = ECrimAddItemResult::NoItemsAdded;

	// Describes the reason for failure to add all items.
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Error;
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

//------------------------------------------------------------------------------
// CrimItemTagStack
//------------------------------------------------------------------------------

/**
 * Represents a single gameplay tag and a count.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemTagStack
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	int32 Count = 0;

	friend FCrimItemTagStackContainer;
};

/**
 * Container of game item tag stacks, designed for fast replication.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemTagStackContainer
{
	GENERATED_BODY()

	FCrimItemTagStackContainer() {}

	/** Add stacks to a tag. */
	void AddStack(FGameplayTag Tag, int32 DeltaCount);

	/** Subtracts stacks from a tag. */
	void SubtractStack(FGameplayTag Tag, int32 DeltaCount);

	/** Removes the Tag entirely */
	void RemoveStack(FGameplayTag Tag);

	/** Return the stack count for a tag, or 0 if the tag is not present. */
	int32 GetStackCount(FGameplayTag Tag) const;

	/** Returns a const reference to the current TagStats. */
	const TArray<FCrimItemTagStack>& GetTagStats() const;

	/** Return true if there is at least one count of a tag. */
	bool ContainsTag(FGameplayTag Tag, bool bExactMatch = false) const;

	/** Empties all stats in this container. */
	void Empty();

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
	UPROPERTY(EditAnywhere, SaveGame)
	TArray<FCrimItemTagStack> Items;
};

USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimStartupItems
{
	GENERATED_BODY()

	// The ItemContainer class to create.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCrimItemContainerBase> ItemContainerClass;

	// The items to try and add to the container.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UCrimItemSet>> ItemSets;
};
