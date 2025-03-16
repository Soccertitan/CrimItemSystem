// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "CrimItemContainerList.h"
#include "CrimItemSaveDataTypes.h"
#include "CrimItemTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemManagerComponent.generated.h"


class UCrimItemManagerRule_CanRemoveItem;
class UCrimItemGenerator;
class ACrimItemDrop;
struct FCrimItemListEntry;
class UCrimItemDef;
class UCrimItem;
class UCrimItemContainer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCrimItemManagerComponentItemSignature, UCrimItemManagerComponent*, ItemManagerComponent, UCrimItemContainer*, ItemContainer, const FCrimItemListEntry&, Item);

/**
 * Manages a collection of ItemContainers and their items.
 */
UCLASS(ClassGroup = "Crim Item System", meta=(BlueprintSpawnableComponent))
class CRIMITEMSYSTEM_API UCrimItemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCrimItemManagerComponent();
	virtual void InitializeComponent() override;
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreNetReceive() override;
	virtual void OnRegister() override;

	/** Called when a new item entry has been added to a container's list. */
	UPROPERTY(BlueprintAssignable)
	FCrimItemManagerComponentItemSignature OnItemAddedDelegate;
	/** Called when an item entry has been removed from the container's list. */
	UPROPERTY(BlueprintAssignable)
	FCrimItemManagerComponentItemSignature OnItemRemovedDelegate;
	/** Called when an old item has been replaced with a new one, in the same slot. */
	UPROPERTY(BlueprintAssignable)
	FCrimItemManagerComponentItemSignature OnItemUpdatedDelegate;

	/**
	 * 
	 * @param ContainerId The ContainerId to search for.
	 * @return The ItemContainer with the matching ContainerId.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	UCrimItemContainer* GetItemContainer(FGameplayTag ContainerId) const;

	/** Gets all item containers this ItemManager has. */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	const TArray<FCrimItemContainerListEntry>& GetItemContainers() const;

	/**
	 * @param Item The item to search for.
	 * @return All the unique ItemContainers the item is found in.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemManagerComponent")
	TArray<UCrimItemContainer*> GetItemContainersWithItem(UCrimItem* Item) const;

	/**
	 * @param ItemDef The ItemDef to check.
	 * @return All items with matching item definitions.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemManagerComponent")
	TArray<UCrimItem*> GetItemsByDefinition(const UCrimItemDef* ItemDef) const;

	/**
	 * @param ItemId The item to search for.
	 * @return The item reference, or nullptr if none found.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	UCrimItem* GetItemById(FGuid ItemId) const;

	/**
	 * Iterates through all ItemContainers for matching items.
	 * TestItem CollectionLimit - Total Items stacks.
	 * @param TestItem The item to check.
	 * @return The number of remaining item stacks that are allowed for the item to be managed by this ItemManagerComponent.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	int32 GetRemainingCollectionCapacityForItem(const UCrimItem* TestItem) const;

	/**
	 * @param ContainerId The container the item resides in.
	 * @param TestItem The item to check if it can be split.
	 * @param Slot The slot to check to place the new stack in. If it's less than 0, will find the first valid empty slot
	 * and update the Slot variable.
	 * @return True, if the item can be split into two stacks.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	bool CanSplitItemStack(FGameplayTag ContainerId, const UCrimItem* TestItem, UPARAM(ref) int32& Slot) const;

	/**
	 * Checks to see if the items are matching in both slots and the TargetSlots.
	 * @param ContainerId The ItemContainer to combine item stacks in.
	 * @param SourceSlot The slot you want to move items from.
	 * @param TargetSlot The slot you want to move items into.
	 * @return The maximum amount of the item from the SourceSlot that can be added to the TargetSlot. If 0, no stacking
	 * is allowed.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	bool CanStackItems(FGameplayTag ContainerId, int32 SourceSlot, int32 TargetSlot, int32& OutMaxQuantity) const;

	/**
	 * Checks to see if the item in the parent container can be assigned to the slot in the child container.
	 * @param ParentContainerId The parent container that will be assigned to the child container.
	 * @param ParentSlot The slot of the item.
	 * @param ChildContainerId The container that will be assigned the item.
	 * @param TargetSlot The slot to try and assign the item to. If the value is less than 0, it will update the TargetSlot
	 * with the first valid slot for the item.
	 * @return True if the item in the Parent Slot can be assigned to the TargetSlot.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	bool CanAssignItemToContainer(FGameplayTag ParentContainerId, int32 ParentSlot, FGameplayTag ChildContainerId, int32& TargetSlot) const;
	
	/**
	 * Adds an item not managed by this ItemManager to be managed by this ItemManager. It will make a copy of the item
	 * and add as much as possible to the container. This respects limits set on the container and ItemManager. 
	 * @param ContainerId The ItemContainer to add the item to.
	 * @param Item The item to add.
	 * @param Quantity The amount of the item to add.
	 * @return The actual amount of the item that was added and any errors if the item could not be added in full.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	FCrimItemAddPlan TryAddItem(FGameplayTag ContainerId, UCrimItem* Item, int32 Quantity);

	/**
	 * Creates a new item from the passed in item. Copying everything the original item had. Then this item is added
	 * directly to the ItemContainer. Use TryAddItem instead to respect container and ItemManager rules.
	 * @param ContainerId The container to add the item to.
	 * @param Item The item to copy.
	 * @param Quantity The quantity to set the new item with.
	 * @param Slot The slot to add the item in. If less than 0, first empty slot.
	 * @return The newly created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	UCrimItem* AddItem(FGameplayTag ContainerId, UCrimItem* Item, int32 Quantity, int32 Slot = -1);

	/**
	 * Tries to create an item from the ItemSpec then makes copies of the item to add to the container. This respects
	 * limits set on the container and ItemManager.
	 * @param ContainerId The ItemContainer to add the item to.
	 * @param ItemSpec How the new item will be created.
	 * @return How much of the item was actually added to the ItemManager.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	FCrimItemAddPlan TryAddItemFromSpec(FGameplayTag ContainerId, UPARAM(ref) const TInstancedStruct<FCrimItemSpec>& ItemSpec);

	/**
	 * Creates an item from an ItemSpec. The spec is passed to the item to apply the values.
	 * Adds the item to the first available slot in the TargetItemContainer. Use TryAddItemFromSpec to respect ItemContainer
	 * and ItemManager rules.
	 * @param ContainerId The container to add the item to.
	 * @param ItemSpec How the new item will be created.
	 * @param Slot The slot to add the item in. If less than 0, first empty slot.
	 * @return The new created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	UCrimItem* AddItemFromSpec(FGameplayTag ContainerId, UPARAM(ref) const TInstancedStruct<FCrimItemSpec>& ItemSpec, int32 Slot = -1);

	/**
	 * Tries to create an item from the ItemDefinition and applies the default stats. Then makes copies of the item to
	 * add to the container. This respects limits set on the container and ItemManager.
	 * @param ContainerId The container to add the item to.
	 * @param ItemDef The item definition.
	 * @param Quantity The amount of the item.
	 * @return How much of the item was actually added to the ItemManager
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	FCrimItemAddPlan TryAddItemFromDefinition(FGameplayTag ContainerId, const UCrimItemDef* ItemDef, int32 Quantity);

	/**
	 * Creates an item from an ItemDefinition and applies the default stats. Adds the item to the specified slot and will
	 * replace and destroy the existing item in slot. Use TryAddItemFromDefinition to respect ItemContainer and ItemManager rules.
	 * @param ContainerId The container to add the item to.
	 * @param ItemDef The item definition.
	 * @param Quantity The amount of the item.
	 * @param Slot The slot to add the item in. If less than 0, first empty slot.
	 * @return The newly created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	UCrimItem* AddItemFromDefinition(FGameplayTag ContainerId, const UCrimItemDef* ItemDef, int32 Quantity, int32 Slot = -1);

	/**
	 * Consumes the specified quantity of an item. The item's quantity can't go below 0. If it is 0, tries to release the
	 * item from the ItemManager.
	 * @param Item The item to consume quantity from.
	 * @param Quantity The amount to subtract from the item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void ConsumeItem(UCrimItem* Item, int32 Quantity);

	/**
	 * Gets all items with the matching ItemDef. Then subtracts quantity from them until the amount subtracted has reached
	 * 0. Then, if an item's quantity is 0, tries to release it from the ItemManger.
	 * @param ItemDef The ItemDef to look for amongst items.
	 * @param Quantity The amount to subtract from the items.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void ConsumeItemByDefinition(const UCrimItemDef* ItemDef, int32 Quantity);
	
	/**
	 * The Item, if managed by this ItemManagerComponent, will have all references removed and marked for garbage. 
	 * @param Item The item to release.
	 * @return Returns true if the Item was released.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	bool ReleaseItem(UCrimItem* Item);

	/**
	 * Creates a new item container and initializes it.
	 * @param ContainerId The Id to set the new container with.
	 * @param ItemContainerClass The item container class to create.
	 * @param bIsParent Will set the new container to be Parent (true) or Child (false) type.
	 * @return The newly created ItemContainer or a nullptr if creating a new one failed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	UCrimItemContainer* CreateItemContainer(FGameplayTag ContainerId, TSubclassOf<UCrimItemContainer> ItemContainerClass, bool bIsParent = true);

	/**
	 * Removes all items from the ItemContainer and if it's a parent, refreshes it's Children's Item's. Unlinks from each
	 * container.
	 * @param ContainerId The ItemContainer to destroy.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void DestroyItemContainerById(FGameplayTag ContainerId);

private:
	/**
	 * Removes all items from the ItemContainer and if it's a parent, refreshes it's Children's Item's. Unlinks from each
	 * container.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void DestroyItemContainer(UCrimItemContainer* ItemContainer);
	
public:
	/**
	 * Tries linking the parent and child containers together.
	 * @param ParentContainerId The parent container to link to the child.
	 * @param ChildContainerId The child container to link to the parent.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void LinkItemContainers(FGameplayTag ParentContainerId, FGameplayTag ChildContainerId);

	//---------------------------------------------------------
	// Server RPC Functions
	//---------------------------------------------------------
	
	/**
	 * Swaps the item in the FirstSlot with the item in the SecondSlot. This respects slot rules for the container.
	 * @param ContainerId The ItemContainer to swap items.
	 * @param FirstSlot The first item to swap. Must be >= 0.
	 * @param SecondSlot The second item to swap. Must be >= 0.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemManagerComponent")
	void SwapItemsBySlot(FGameplayTag ContainerId, int32 FirstSlot, int32 SecondSlot);

	/**
	 * Moves some or all of an item from the SourceContainer to the TargetContainer. The TargetSlot must be empty or
	 * have a matching item in the slot to move to.
	 * @param SourceContainerId The Container we want to the move the item out of.
	 * @param SourceSlot The slot the item is in.
	 * @param TargetContainerId The container we want to move the item into.
	 * @param TargetSlot The target slot to move the item into. If -1 the first available slot.
	 * @param Quantity The amount of the item from the SourceSlot you want to move to the TargetSlot.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemManagerComponent")
	void MoveItemToContainer(FGameplayTag SourceContainerId, int32 SourceSlot, FGameplayTag TargetContainerId, int32 TargetSlot, int32 Quantity);

	/**
	 * Tries to split the item in the ItemSlot and place it in the TargetSlot. Only works if the TargetSlot is empty.
	 * Calls CanSplitItemStack internally.
	 * @param ContainerId The ItemContainer the item resides in.
	 * @param SourceSlot The source slot of the item to split.
	 * @param TargetSlot The slot to place the new stack in.
	 * @param Quantity The amount to move from the original item to the new item.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemManagerComponent")
	void SplitItemStack(FGameplayTag ContainerId, int32 SourceSlot, int32 TargetSlot, int32 Quantity);

	/**
	 * Tries to stack the item in the SourceSlot with the item in the TargetSlot. Calls CanStackItems internally.
	 * @param ContainerId The ItemContainer the item resides in.
	 * @param SourceSlot The slot the Item we want to merge into the TargetSlot.
	 * @param TargetSlot The slot the Item will receive the SourceItem.
	 * @param Quantity The amount from the SourceSlot to stack with the TargetSlot.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemManagerComponent")
	void StackItems(FGameplayTag ContainerId, int32 SourceSlot, int32 TargetSlot, int32 Quantity);

	/**
	 * Takes a reference of an item from the ParentContainer and adds it to the ChildContainer to the specified slot.
	 * @param ParentContainerId The ParentContainer the item resides in.
	 * @param ParentSlot The item to find in the ParentContainer.
	 * @param ChildContainerId The child container to assign an item to.
	 * @param TargetSlot The slot to assign the item to in the target container. If -1, place in the first valid slot.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemManagerComponent")
	void AssignItemToContainer(FGameplayTag ParentContainerId, int32 ParentSlot, FGameplayTag ChildContainerId, int32 TargetSlot = -1);

	//---------------------------------------------------------
	// ~Server RPC Functions
	//---------------------------------------------------------
	
	/* Returns true if this Component's Owner Actor has authority. */
	bool HasAuthority() const;
	
	/**
	 * Collects all the unique item instances and ItemContainers. Saves the data in a struct.
	 * @return The current SaveData for the ItemManager.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	FCrimItemManagerSaveData GetSaveData() const;

	/**
	 * Returns the ItemManager back to the SavedData's state.
	 * @param SaveData The save data.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void RestoreSavedData(UPARAM(ref) const FCrimItemManagerSaveData& SaveData);
	
protected:
	virtual void BeginPlay() override;
	
	void CacheIsNetSimulated();

	/** Cached value of whether our owner is a simulated Actor. */
	UPROPERTY()
	bool bCachedIsNetSimulated = false;

private:
	/** All the item containers this component manages. */
	UPROPERTY(Replicated)
	FCrimItemContainerList ItemContainerList;

	/** The startup parent item containers. Mapped to a GameplayTag for the ContainerId. */
	UPROPERTY(EditAnywhere, Category = "CrimItemManagerComponent", meta=(ForceInlineRow))
	TMap<FGameplayTag, TSubclassOf<UCrimItemContainer>> StartupParentItemContainers;

	/** The startup child item containers. Mapped to a GameplayTag for the ContainerId. */
	UPROPERTY(EditAnywhere, Category = "CrimItemManagerComponent", meta=(ForceInlineRow))
	TMap<FGameplayTag, TSubclassOf<UCrimItemContainer>> StartupChildItemContainers;

	/** Defines how to link ParentContainers to ChildContainers. */
	UPROPERTY(EditAnywhere, Category = "CrimItemManagerComponent")
	TArray<FCrimItemContainerLinkSpec> StartupItemContainerLinkSpecs;

	void InitializeStartupItemContainers();
	void BindToItemContainerListDelegates();
	void BindToItemContainerDelegates(UCrimItemContainer* ItemContainer);
	void RemoveItemFromAllItemContainers(UCrimItem* Item);

	/**
	 * Creates a new item instance, initializes it, and add it's to the ReplicatedSubObject list.
	 * @param ItemDef The new item will be based off this ItemDefinition.
	 * @param ItemId The id to assign the new item with.
	 * @return The newly created item instance.
	 */
	UCrimItem* CreateItem(const UCrimItemDef* ItemDef, FGuid ItemId);

	/** Executes the AddItemPlan. */
	void ExecuteAddItemPlan(const FCrimItemAddPlan& Plan, UCrimItemContainer* ItemContainer, UCrimItem* Item);

	/** Adds an item directly to the ItemContainer. */
	void AddItemToContainer(UCrimItemContainer* ItemContainer, UCrimItem* NewItem, int32 Slot);
	
	/**
	 * Removes the item from the replicated SubObject list, all item containers, and marks it for garbage.
	 * @param Item The item to destroy
	 * @return Returns true if successful.
	 */
	bool DestroyItem(UCrimItem* Item);
	
	//-------------------------------------------------------------------
	// RPC Server Functions.
	//-------------------------------------------------------------------
private:
	UFUNCTION(Server, Reliable)
	void Server_SwapItemsBySlot(FGameplayTag ContainerId, int32 FirstSlot, int32 SecondSlot);
	UFUNCTION(Server, Reliable)
	void Server_MoveItemToContainer(FGameplayTag SourceContainerId, int32 SourceSlot, FGameplayTag TargetContainerId, int32 TargetSlot, int32 Quantity);
	UFUNCTION(Server, Reliable)
	void Server_AssignItemToContainer(FGameplayTag ParentContainerId, int32 ParentSlot, FGameplayTag ChildContainerId, int32 TargetSlot);
	UFUNCTION(Server, Reliable)
	void Server_SplitItemStack(FGameplayTag ContainerId, int32 SourceSlot, int32 TargetSlot, int32 Quantity);
	UFUNCTION(Server, Reliable)
	void Server_StackItems(FGameplayTag ContainerId, int32 SourceSlot, int32 TargetSlot, int32 Quantity);
};
