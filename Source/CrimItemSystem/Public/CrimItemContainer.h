// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
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

DECLARE_MULTICAST_DELEGATE_TwoParams(FCrimItemContainerFastItemSignature, UCrimItemContainer*, const FFastCrimItem&);

/**
 * An object that holds one or more item instances. Like an inventory, treasure chest, item pickup, or equipment.
 * Managed by the CrimItemManagerComponent.
 */
UCLASS(ClassGroup = "Crim Item System", BlueprintType, Blueprintable)
class CRIMITEMSYSTEM_API UCrimItemContainer : public UObject
{
	GENERATED_BODY()
	
	friend UCrimItemManagerComponent;

	/** The unique tag identifying this container. */
	UPROPERTY(Replicated)
	FGameplayTag ContainerId;

	/** The user-facing display name of this container. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer|UI")
	FText DisplayName;

	/** The ViewModel to create for this item container. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer|UI", NoClear)
	TSoftClassPtr<UCrimItemContainerViewModelBase> ViewModelClass;

	/** Tags that this container has. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer")
	FGameplayTagContainer OwnedTags;

	/** Limits the number of individual item instances that can be in this list. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer", meta = (InlineEditConditionToggle))
	bool bLimitCapacity = false;

	/** The limited capacity available in this container when bLimitCapacity is true. */
	UPROPERTY(SaveGame, Replicated, EditAnywhere, meta = (EditCondition = "bLimitCapacity", ClampMin=0), Category = "CrimItemContainer")
	int32 MaxCapacity = 0;

	/** Automatically combine and stack items that are added to the container via GetAddItemPlan. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer")
	bool bAutoStack = true;

	/** Additional rules that govern if an item is allowed in this container. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer")
	TArray<TObjectPtr<UCrimItemContainerRule>> ItemContainerRules;
	
public:
	UCrimItemContainer();
	virtual void PostInitProperties() override;
	virtual bool IsSupportedForNetworking() const override {return true;}
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

	/**
	 *	General Item Container Functionality
	 *
	 *	This section contains functions that are used by both Parent/Child containers. These functions are general
	 *	accessors for finding items.
	 */
public:
	
	/** Returns the Container's Id. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	const FGameplayTag& GetContainerId() const;

	/** Returns the user facing display name. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	FText GetDisplayName() const {return DisplayName;}

	/** Returns the view model class.*/
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	TSubclassOf<UCrimItemContainerViewModel> GetViewModelClass() const;

	/** Returns the Container's owned gameplay tags. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	const FGameplayTagContainer& GetOwnedTags() const;

	/** Gets the ItemManager component that owns this container. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	UCrimItemManagerComponent* GetItemManagerComponent() const {return ItemManagerComponent;}

	/** Called when an item is added to the container. */
	FCrimItemContainerFastItemSignature OnItemAddedDelegate;
	/** Called when an item is removed from the container. */
	FCrimItemContainerFastItemSignature OnItemRemovedDelegate;
	/** Called when an item's property has changed in the container. */
	FCrimItemContainerFastItemSignature OnItemChangedDelegate;
	
	/**
	 * @param ItemGuid The unique Guid to search for.
	 * @return A copy of the item matching the ItemId.
	 */
	FFastCrimItem* GetItemFromGuid(FGuid ItemGuid) const;
	
	/**
	 * @param ItemGuid The unique Guid to search for.
	 * @return A copy of the item matching the ItemId.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer", DisplayName = "GetItemFromId")
	TInstancedStruct<FCrimItem> K2_GetItemFromGuid(FGuid ItemGuid) const;
	
	/** Returns a const reference to all items in the ItemContainer */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	const TArray<FFastCrimItem>& GetItems() const;

	/**
	 * @return The first item found with the matching ItemDefinition.
	 */
	FFastCrimItem* GetItemFromDefinition(const UCrimItemDefinition* ItemDef) const;
	
	/**
	 * @return The first item found with the matching ItemDefinition.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer", DisplayName = "GetItemFromDefinition")
	TInstancedStruct<FCrimItem> K2_GetItemFromDefinition(const UCrimItemDefinition* ItemDef) const;

	/**
	 * @return All items in the container by ItemDefinition.
	 */
	TArray<FFastCrimItem*> GetItemsFromDefinition(const UCrimItemDefinition* ItemDef) const;
	
	/**
	 * @return All items in the container by ItemDefinition.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer", DisplayName = "GetItemsByDefinition")
	TArray<TInstancedStruct<FCrimItem>> K2_GetItemsFromDefinition(const UCrimItemDefinition* ItemDef) const;

	/**
	 * Returns the first item that matches the TestItem.
	 * See UCrimItem::IsMatching
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	TInstancedStruct<FCrimItem> GetMatchingItem(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * @note See FCrimItem::IsMatching
	 * @param TestItem The item to check against.
	 * @return Pointers to all matching items.
	 */
	TArray<FFastCrimItem*> GetMatchingItems(const TInstancedStruct<FCrimItem>& TestItem) const;
	
	/**
	 * @note See FCrimItem::IsMatching
	 * @param TestItem The item to check against.
	 * @return A copy of all items that match the TestItem.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer", DisplayName = "GetMatchingItems")
	TArray<TInstancedStruct<FCrimItem>> K2_GetMatchingItems(const TInstancedStruct<FCrimItem>& TestItem) const;
	
	/**
	 * @return True if the item exists in the container. 
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool HasItemById(FGuid ItemId) const;

	/**
	 * Finds the most restrictive rule and applies it.
	 * @param TestItem The item to check.
	 * @return True, if this container can have this item.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual bool CanContainItem(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * Finds the most restrictive rule and applies it.
	 * @param TestItem The item to check.
	 * @return True, if this container can have it's item removed.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual bool CanRemoveItem(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * Informs the caller on how to add items to this container. Specifically, if a new item needs to be created,
	 * which slots, and how many to add to each slot.
	 * @param Item The item you want to add to this container.
	 * @param ItemQuantity The amount from that item to add.
	 * @param MaxNewItemStacks The amount of new item stacks that are allowed to be created. Derived from the item's collection limit.
	 * @return The plan to add items to the container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual FCrimAddItemPlanResult GetAddItemPlan(const TInstancedStruct<FCrimItem>& Item, int32 ItemQuantity, int32 MaxNewItemStacks) const;

	/**
	 * @param Item The item to check
	 * @param ItemPlan The ItemPlan to update.
	 * @param ItemQuantity The quantity of item to add.
	 * @return Returns true if the item could be added.
	 */
	bool CanAddItem(const TInstancedStruct<FCrimItem>& Item, FCrimAddItemPlanResult& ItemPlan, int32 ItemQuantity) const;

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
	 * GetMaxCapacity() - GetConsumedCapacity()
	 * @return The amount of available space in this item list.
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
	 * Set's the MaxCapacity for this container. If bLimitCapacity is false this will do nothing.
	 * @param NewCount The amount to adjust the MaxCapacity number.
	 * @param bOverride If true, NewCount will override the current MaxCapacity. If false, the NewCount will
	 * be added to the MaxCapacity.
	 * @return Returns the new max capacity.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	int32 SetMaxCapacity(int32 NewCount, bool bOverride = false);

	/**
	 * Gets all matching items. Then adds up the quantity for each item and compares that number to the limit. The limit
	 * considers the remaining ItemContainer capacity and Item Container limits.
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

	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool HasAuthority() const;

	/**
	 * Adds an item to this container. It will make a copy of the item
	 * and add as much as possible to the container. This respects limits set on the container and ItemManager.
	 * @param Item The item to add.
	 * @param Quantity The amount of the item to add.
	 * @return The actual amount of the item that was added and any errors if the item could not be added in full.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	virtual FCrimAddItemPlanResult TryAddItem(const TInstancedStruct<FCrimItem>& Item, int32 Quantity);

	/**
	 * Creates a new item from the passed in item. Copying everything the original item had. Then this item is added
	 * directly to the ItemContainer. Use TryAddItem instead to respect container and ItemManager rules.
	 * @param Item The item to add.
	 * @param Quantity The quantity to set the new item with.
	 * @return A copy of the newly created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	virtual TInstancedStruct<FCrimItem> AddItem(const TInstancedStruct<FCrimItem>& Item, int32 Quantity);

	/**
	 * Tries to create an item from the ItemSpec then makes copies of the item to add to the container. This respects
	 * limits set on the container and ItemManager.
	 * @param ItemSpec How the new item will be created.
	 * @return How much of the item was actually added to the ItemManager.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	virtual FCrimAddItemPlanResult TryAddItemFromSpec(UPARAM(ref) const TInstancedStruct<FCrimItemSpec>& ItemSpec);

	/**
	 * Creates an item from an ItemSpec. The spec is passed to the item to apply the values. Then this new item is added
	 * directly to the ItemContainer. Use TryAddItemFromSpec instead to respect container and ItemManager rules.
	 * and ItemManager rules.
	 * @param ItemSpec How the new item will be created.
	 * @return A copy of the newly created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	virtual TInstancedStruct<FCrimItem> AddItemFromSpec(UPARAM(ref) const TInstancedStruct<FCrimItemSpec>& ItemSpec);

	/**
	 * Tries to create an item from the ItemDefinition and applies the default stats. Then makes copies of the item to
	 * add to the container. This respects limits set on the container and ItemManager.
	 * @param ItemDef The item definition.
	 * @param Quantity The amount of the item.
	 * @return How much of the item was actually added to the ItemManager
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	virtual FCrimAddItemPlanResult TryAddItemFromDefinition(const UCrimItemDefinition* ItemDef, int32 Quantity);

	/**
	 * Creates an item from an ItemDefinition and applies the default stats. Then this item is added directly to the
	 * ItemContainer. Use TryAddItemFromDefinition instead to respect container and ItemManager rules.
	 * @param ItemDef The item definition.
	 * @param Quantity The amount of the item.
	 * @return A copy of the newly created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	virtual TInstancedStruct<FCrimItem> AddItemFromDefinition(const UCrimItemDefinition* ItemDef, int32 Quantity);

	/**
	 * Consumes the specified quantity of the item. The item's quantity can't go below 0. If it is 0, removes the
	 * item from the ItemContainer.
	 * @param ItemId The item to consume quantity from.
	 * @param Quantity The amount to subtract from the item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	void ConsumeItem(UPARAM(ref) const FGuid& ItemId, int32 Quantity);

	/**
	 * Gets all items with the matching ItemDef. Then subtracts quantity from them until the amount subtracted has reached
	 * 0. Then, if an item's quantity is 0, removes the item from the container.
	 * @param ItemDef The ItemDef to look for amongst items.
	 * @param Quantity The total quantity to subtract from the items.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	void ConsumeItemsByDefinition(const UCrimItemDefinition* ItemDef, int32 Quantity);
	
	/**
	 * Tries to manually remove the item from being managed by the ItemContainer. If CanRemoveItem is true,
	 * the item will have all references removed and marked for garbage.
	 * @param ItemId The item to remove.
	 * @return True, if the item was removed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	bool RemoveItem(UPARAM(ref) const FGuid& ItemId);

	/**
	 * Checks to see if the Item can be moved to the target container. The TargetContainer must be
	 * different from the TestItem's container.
	 * @param TestItem The item to check.
	 * @param TargetContainer The target container you want to move the item to.
	 * @param Quantity The amount of the item to try and place in the TargetContainer.
	 * @param OutPlan The valid plan on how to add the item to the TargetContainer.
	 * @return True, if the item can be moved into the TargetContainer.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	bool CanMoveItemToContainer(const TInstancedStruct<FCrimItem>& TestItem, const UCrimItemContainer* TargetContainer, int32 Quantity,
		FCrimAddItemPlanResult& OutPlan) const;

	/**
	 * Tries to move some or all of an item from its current ItemContainer to the TargetContainer.
	 * @param ItemId The item you wish to move.
	 * @param TargetContainer The ItemContainer to move the item into.
	 * @param Quantity The amount of the item from the SourceContainer to the TargetContainer.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemManagerComponent")
	void MoveItemToContainer(UPARAM(ref) const FGuid& ItemId, UCrimItemContainer* TargetContainer, int32 Quantity);

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

	/** Call to mark that an Item has been modified */
	void MarkItemDirty(FFastCrimItem& FastItem);
	
protected:

	/** Is called in the ItemManagerComponent. Use this for BeginPlay functionality. */
	virtual void Initialize(UCrimItemManagerComponent* ItemManager, FGameplayTag NewContainerId);

	/**
	 * Creates a new item instance and initializes it.
	 * @param ItemDef The new item will be based off this ItemDefinition.
	 * @param ItemId The id to assign the new item with.
	 * @param OutItemInstance The newly created item.
	 * @return True if the item was created successfully.
	 */
	bool CreateItem(const UCrimItemDefinition* ItemDef, FGuid ItemId, TInstancedStruct<FCrimItem>& OutItemInstance);

	/** Executes the AddItemPlan. */
	void ExecuteAddItemPlan(FCrimAddItemPlanResult& Plan, const TInstancedStruct<FCrimItem>& TemplateItem);

	/** Adds the Item to this container's FastArray of items. */
	void AddItemToItemContainer(TInstancedStruct<FCrimItem>& Item);

	/** Removes the item from the container by Id */
	void RemoveFromItemList(const FGuid& ItemId);

	/** Called when an item is added to the container. */
	virtual void OnItemAdded(const FFastCrimItem& Item){}
	/** Called when an item is added to the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnItemAdded")
	void K2_OnItemAdded(const FFastCrimItem& Item);

	/** Called when an item is removed from the container. */
	virtual void OnItemRemoved(const FFastCrimItem& Item){}
	/** Called when an item is removed from the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnItemRemoved")
	void K2_OnItemRemoved(const FFastCrimItem& Item);

	/** Called when an item has been changed in the container. */
	virtual void OnItemChanged(const FFastCrimItem& Item) {}
	/** Called when an item has been changed in the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnItemChanged")
	void K2_OnItemChanged(const FFastCrimItem& Item);
	
private:
	UPROPERTY(Replicated)
	FFastCrimItemList ItemList;

	/** The owner of this item container. */
	UPROPERTY(Replicated)
	TObjectPtr<UCrimItemManagerComponent> ItemManagerComponent;
	UPROPERTY()
	bool bOwnerIsNetAuthority = false;

	void BindToItemListDelegates();
};
