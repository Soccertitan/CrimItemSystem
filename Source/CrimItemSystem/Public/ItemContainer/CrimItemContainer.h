// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemContainerBase.h"
#include "CrimItemFastTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif
#include "CrimItemContainer.generated.h"

class UCrimItemContainerViewModelBase;
class UCrimItemDefinition;
class UCrimItemContainerRule;
class UCrimItemManagerComponent;

/**
 * An object that holds one or more item instances. Like an inventory, treasure chest, item pickup, or equipment.
 * Managed by the CrimItemManagerComponent.
 */
UCLASS(ClassGroup = "Crim Item System", Blueprintable)
class CRIMITEMSYSTEM_API UCrimItemContainer : public UCrimItemContainerBase
{
	GENERATED_BODY()

protected:
	/** Limits the number of individual item instances that can be in this list. */
	UPROPERTY(EditAnywhere, Replicated, Category = "CrimItemContainer", SaveGame)
	FCrimItemQuantityLimit CapacityLimit;

	/** Automatically combine and stack items that are added to the container. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer")
	bool bAutoStack = true;

	/** Additional rules that govern if an item is allowed in this container. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer")
	TArray<TObjectPtr<UCrimItemContainerRule>> ItemContainerRules;

public:
	UCrimItemContainer();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool CanAddItem(const TInstancedStruct<FCrimItem>& Item, FGameplayTag& OutError) const override;
	virtual bool CanRemoveItem(const TInstancedStruct<FCrimItem>& Item) const override;

	/**
	 * Set's the MaxCapacity for this container. If bLimitCapacity is false this will do nothing.
	 * @param NewCount The amount to adjust the MaxCapacity number.
	 * @param bOverride If true, NewCount will override the current MaxCapacity. If false, the NewCount will
	 * be added to the MaxCapacity.
	 * @return Returns the new max capacity.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	int32 SetMaxCapacity(int32 NewCount, bool bOverride = false);
	
	/**
	 * @return The max number of item instances allowed in this container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	int32 GetMaxCapacity() const;

	/**
	 * @return Returns the number of items in the list.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	int32 GetConsumedCapacity() const;

	/**
	 * @return GetMaxCapacity() - GetConsumedCapacity()
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	int32 GetRemainingCapacity() const;

	/**
	 * GetConsumedCapacity() >= GetMaxCapacity()
	 * @return True if the inventory is at max capacity.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsAtMaxCapacity() const;

	/**
	 * Iterates through all ItemContainers from the cached ItemManager property for items with a matching ItemDefinition.
	 * CollectionLimit - Total Items stacks.
	 * @param TestItem The Item to check.
	 * @return The number of remaining item stacks that are allowed to be added for items across all ItemContainers.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	int32 GetRemainingCollectionCapacityForItem(const TInstancedStruct<FCrimItem>& TestItem) const;
	
	/**
	 * Gets all items with matching ItemDefinitions. Then adds up the quantity for each item and compares that number
	 * to the limit. The limit considers the remaining ItemContainer capacity and Item limits.
	 * @param TestItem The item to match against. 
	 * @return The maximum amount allowed to be added to the container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	int32 GetRemainingCapacityForItem(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * Will check the container rules and the item. Whichever value is most restrictive.
	 * @param TestItem The Item to check.
	 * @return Returns the maximum number of unique item instances this item can consume in this container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual int32 GetItemContainerLimit(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * Will check the container rules and the item. Whichever value is most restrictive.
	 * @param TestItem The item to check.
	 * @return Return the maximum allowed quantity for a single stack of an item.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual int32 GetItemQuantityLimit(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * Counts the number of Items with the same ItemDef and compares it to GetItemContainerLimit.
	 * @param TestItem The item to check.
	 * @return True if no more item stacks can be created in this container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsItemAtContainerLimit(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * @param TestItem The item to check if it can be split.
	 * @param Quantity The amount to try and split off from the TestItem.
	 * @return True, if the item can be split into two stacks in it's existing container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool CanSplitItemStack(const TInstancedStruct<FCrimItem>& TestItem, int32 Quantity) const;

	/**
	 * Tries to split the item stack in the existing container.
	 * @param ItemId The Item to try and split.
	 * @param Quantity The amount to split off from the original item into to the new item.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemContainer")
	void SplitItemStack(UPARAM(ref) const FGuid& ItemId, int32 Quantity);

	/**
	 * Checks to see if the items are matching and how much of the SourceItem can be added to the TargetItem.
	 * @param SourceItem The item you want to merge into the TargetItem.
	 * @param TargetItem The item you targeted to be merged with.
	 * @param OutMaxQuantity The maximum quantity of the SourceItem allowed to be added to the TargetItem.
	 * @return True, if SourceItem can be stacked into the TargetItem.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	bool CanStackItems(const TInstancedStruct<FCrimItem>& SourceItem, const TInstancedStruct<FCrimItem>& TargetItem, int32& OutMaxQuantity) const;

	/**
	 * Tries to stack the SourceItem into the TargetItem by the specified quantity.
	 * @param SourceItemId The item you want to merge.
	 * @param TargetItemId The SourceItem will attempt to merge into this item.
	 * @param Quantity The amount from the SourceItem to stack with the TargetItem.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemManagerComponent")
	void StackItems(UPARAM(ref) const FGuid& SourceItemId, UPARAM(ref) const FGuid& TargetItemId, int32 Quantity);

protected:

	virtual FCrimAddItemPlan GetAddItemPlan(const TInstancedStruct<FCrimItem>& Item) const override;
};
