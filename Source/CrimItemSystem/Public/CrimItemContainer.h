// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "CrimItemList.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif
#include "CrimItemContainer.generated.h"

class UCrimItemDef;
class UCrimItemContainerRule;
class UCrimItemManagerComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FCrimItemContainerListSignature, UCrimItemContainer*, const FCrimItemListEntry&);

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
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer")
	FText DisplayName;

	/** Tags that this container has. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer")
	FGameplayTagContainer OwnedTags;

	/** Limits the number of individual item instances that can be in this list. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer", meta = (InlineEditConditionToggle))
	bool bLimitCapacity = false;

	/** The limited capacity available in this container when bLimitCapacity is true. */
	UPROPERTY(SaveGame, Replicated, EditAnywhere, meta = (EditCondition = "bLimitCapacity", ClampMin=0), Category = "CrimItemContainer")
	int32 MaxCapacity = 0;

	/** Automatically combine and stack items that are added to the container (unless explicitly added to an empty spot). */
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

	/** Returns the Container's owned gameplay tags. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	const FGameplayTagContainer& GetOwnedTags() const;

	/** Gets the ItemManager component that owns this container. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	UCrimItemManagerComponent* GetItemManagerComponent() const {return ItemManagerComponent;}

	/** Called when an item is added to the ItemList. */
	FCrimItemContainerListSignature OnEntryAddedDelegate;
	/** Called when an item is removed from the ItemList. */
	FCrimItemContainerListSignature OnEntryRemovedDelegate;
	/** Called when an item has been replaced in the ItemList. */
	FCrimItemContainerListSignature OnEntryUpdatedDelegate;

	/**
	 * Returns an item at the specified slot.
	 * @param Slot A number >= 0.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	UCrimItem* GetItemInSlot(int32 Slot) const;

	/**
	 * @param ItemId The unique Guid to search for.
	 * @return The item matching the ItemId.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	UCrimItem* GetItemFromId(FGuid ItemId) const;
	
	/** Returns a const reference to the items in the ItemList */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	const TArray<FCrimItemListEntry>& GetItems() const;

	/**
	 * @return The first item found with the matching ItemDefinition.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	UCrimItem* GetItemFromDef(const UCrimItemDef* ItemDef) const;
	
	/**
	 * @param bIgnoreDuplicates If false, will only add unique item instances to the array.
	 * @return All items in the container by ItemDefinition.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	TArray<UCrimItem*> GetItemsFromDef(const UCrimItemDef* ItemDef, bool bIgnoreDuplicates = true) const;

	/**
	 * Returns the first item that matches the TestItem.
	 * See UCrimItem::IsMatching
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	UCrimItem* GetMatchingItem(const UCrimItem* TestItem) const;
	
	/**
	 * Returns all items that matches the TestItem.
	 * See UCrimItem::IsMatching
	 * @param TestItem The item to check against.
	 * @param bIgnoreDuplicates If false, will only add unique item instances to the array.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	TArray<UCrimItem*> GetMatchingItems(const UCrimItem* TestItem, bool bIgnoreDuplicates = true) const;

	/**
	 * Returns all slots that match the TestItem.
	 * See UCrimItem::IsMatching
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	TArray<int32> GetMatchingItemSlots(const UCrimItem* TestItem) const;
	
	/**
	 * Gets all slots that the specific item is in.
	 * @param TestItem The Item to search for.
	 * @return The slots the item is in.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	TArray<int32> GetSlotsFromItem(const UCrimItem* TestItem) const;

	/**
	 * Searches the ItemList where item's have been slotted (An item assigned to slot 0 or greater) and finds the
	 * next lowest slot available.
	 * @param Slot The slot to start searching at.
	 * @return The next empty slot. INDEX_NONE is no available empty slots.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	int32 GetNextEmptySlot(int32 Slot = 0) const;
	
	/**
	 * @return True if the item instance exists in the container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool HasItem(const UCrimItem* TestItem) const;
	
	/**
	 * @return True if the item exists in the container. 
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool HasItemById(FGuid ItemId) const;

	/**
	 * Checks to see if the slot is >= 0 and less than MaxCapacity. 
	 * @param Slot The slot to check.
	 * @return Returns true if the slot is valid.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsValidSlot(int32 Slot) const;

	/**
	 * Calls GetValidSlots and checks if the passed in Slot is contained in the array.
	 * @param TestItem The item to test.
	 * @param Slot The slot to check.
	 * @return True if the specified slot is valid for the item.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsValidSlotForItem(const UCrimItem* TestItem, int32 Slot) const;

	/**
	 * Finds the most restrictive rule and applies it.
	 * @param TestItem The item to check.
	 * @return True, if this container can have this item.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual bool CanContainItem(const UCrimItem* TestItem) const;

	/**
	 * Finds the most restrictive rule and applies it.
	 * @param TestItem The item to check.
	 * @return True, if this container can have it's item removed.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual bool CanRemoveItem(const UCrimItem* TestItem) const;

	/**
	 * Finds all the valid slots and adds them to the array. Checks the container rules and adds them together.
	 * @param TestItem The item to check.
	 * @return Returns an array of valid slots the item can be in or empty if all slots are valid.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual TArray<int32> GetValidSlots(const UCrimItem* TestItem) const;

	/**
	 *	Parent Container Functionality
	 *
	 *	Parent Containers are a 'primary' container for storing items. Items can be stored here and should respect
	 *	stack, container, and collection limits. This container should not ever have multiple items of the same instance
	 *	stored in a slot on these containers.
	 *
	 *	Primary functions is the GetAddItemPlan and GetAddItemPlanForSlot. It will inform the ItemManager component
	 *	on how to add items to this container.
	 */
public:
	/**
	 * Informs the caller on how to add items to this container. Specifically, if a new item needs to be created,
	 * which slots, and how many to add to each slot.
	 * @param Item The item you want to add to this container.
	 * @param ItemQuantity The amount from that item to add.
	 * @param MaxNewItemStacks The amount of new item stacks that are allowed to be created. Derived from the item's collection limit.
	 * @return The plan to add items to the container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	FCrimItemAddPlan GetAddItemPlan(UCrimItem* Item, int32 ItemQuantity, int32 MaxNewItemStacks) const;

	/**
	 * Informs the caller on how to add an item to the specified slot in this container. If the slot has an item in it that
	 * does not match, an invalid SlotPlan will be returned.
	 * @param Item The item you want to add to this container.
	 * @param ItemQuantity The amount from that item to add.
	 * @param MaxNewItemStacks The amount of new item stacks that are allowed to be created. Derived from the item's collection limit.
	 * @param Slot The slot to place the item in.
	 * @return The plan to add an item to this container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	FCrimItemAddPlan GetAddItemPlanForSlot(UCrimItem* Item, int32 ItemQuantity, int32 MaxNewItemStacks, int32 Slot) const;

	/**
	 * @param ItemPlan The ItemPlan to update.
	 * @param Item The item to check
	 * @param ItemQuantity The quantity of item to add.
	 * @param Slot The slot to add the item. -1 for undefined.
	 * @return Returns true if the item could be added. False if item can't be added.
	 */
	bool CanAddItem(FCrimItemAddPlan& ItemPlan, UCrimItem* Item, int32 ItemQuantity, int32 Slot = -1) const;

	/**
	 * @return True if this container is a parent.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsParentContainer() const;

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
	 * @return The maximum number of quantity allowed to be added to the container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	int32 GetRemainingCapacityForItem(UCrimItem* TestItem) const;

	/**
	 * Will check the container rules and the item definition. Whichever value is most restrictive.
	 * @param TestItem The Item to check.
	 * @return Returns the maximum number of unique item instances this item can consume in this container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual int32 GetItemContainerLimit(const UCrimItem* TestItem) const;

	/**
	 * Will check the container rules and the item definition. Will use the most restrictive option compared between
	 * the item Def and the Rules.
	 * @param TestItem The item to check.
	 * @return Return the maximum allowed quantity for a single stack of an item.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	virtual int32 GetItemQuantityLimit(const UCrimItem* TestItem) const;
	
	/**
	 * Compares TestItem->GetQuantity() >= GetItemQuantityLimit()
	 * @param TestItem The item to check.
	 * @return False, if there is available space.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsItemAtQuantityLimit(const UCrimItem* TestItem) const;

	/**
	 * Counts the number of Matching Items and compares it to GetItemContainerLimit.
	 * @param TestItem The item to check.
	 * @return True if no more item stacks can be created in this container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsItemAtContainerLimit(const UCrimItem* TestItem) const;

	/**
	 * Child Container Functionality
	 *
	 * A child container cannot hold the primary instance of an Item Instance. The Item Manager must only add references
	 * to this ItemContainer from one of its parent containers. If an item in its parent container is removed, it will
	 * also remove it from this container.
	 *
	 * This type of container is ideal if you want to 'equip' items but not remove it from the primary container.
	 */
public:
	
	/**
	 * @return True if this container is a child.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsChildContainer() const;

private:
	
	/**
	 * Called on a ParentContainer to update all children that it had an item removed.
	 * @param OldItem The item that was removed from the parent.
	 */
	void RefreshChildContainers(UCrimItem* OldItem);

	//---------------------------------------------------------------------------------------------
	// Container Link Functionality
	//---------------------------------------------------------------------------------------------
public:
	
	/**
	 * @return All linked containers.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemContainer")
	TArray<UCrimItemContainer*> GetLinkedContainers() const;

	/**
	 * 
	 * @param TestItemContainer The container to test.
	 * @return True if the TestItemContainer is linked to this container.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool IsContainerLinked(UCrimItemContainer* TestItemContainer) const;

private:

	/**
	 * Links the TargetContainer to this container.
	 * @param TargetContainer The container to link with.
	 * @return True if successful in linking the container.
	 */
	bool LinkContainers(UCrimItemContainer* TargetContainer);

	/**
	 * Unlinks this container with the TargetContainer.
	 * @param TargetContainer The container to unlink with.
	 * @return True if successful in unlinking the containers.
	 */
	bool UnlinkContainers(UCrimItemContainer* TargetContainer);

	//-------------------------------------------------------------------------
	// End of Container Link Functionality
	//-------------------------------------------------------------------------
	
protected:
	/** Called when an item is added to the list. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnEntryAdded")
	void K2_OnEntryAdded(const FCrimItemListEntry& Item);
	/** Called when an item is added to the list. */
	virtual void OnEntryAdded(const FCrimItemListEntry& Item){}
	/** Called when an item is removed from the list. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnEntryRemoved")
	void K2_OnEntryRemoved(const FCrimItemListEntry& Item);
	/** Called when an item is removed from the list. */
	virtual void OnEntryRemoved(const FCrimItemListEntry& Item){}
	/** Called when an item has replaced an existing item in the list. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnEntryUpdated")
	void K2_OnEntryUpdated(const FCrimItemListEntry& Item);
	/** Called when an item has replaced an existing item in the list. */
	virtual void OnEntryUpdated(const FCrimItemListEntry& Item){}

	/** Is called in the ItemManagerComponent. Use this for BeginPlay functionality. */
	virtual void Initialize(UCrimItemManagerComponent* ItemManager, FGameplayTag NewContainerId, bool bInIsParent = true);
	
private:
	UPROPERTY(Replicated)
	FCrimItemList ItemList;

	/**
	 * If true, this container can store items. False, it's considered a child and can only reference items from the
	 * LinkedContainers.
	 */
	UPROPERTY(Replicated)
	bool bIsParent = true;
	
	/** Contains an array of ItemContainers. */
	UPROPERTY(Replicated)
	TArray<TObjectPtr<UCrimItemContainer>> LinkedContainers;

	/** The owner of this item container. */
	UPROPERTY(Replicated)
	TObjectPtr<UCrimItemManagerComponent> ItemManagerComponent;
	UPROPERTY()
	bool bOwnerIsNetAuthority = false;

	void BindToItemListDelegates();
};
