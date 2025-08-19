// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemFastTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif
#include "CrimItemContainerBase.generated.h"

class UCrimItemContainerRule;
class UCrimItemContainerViewModelBase;
DECLARE_MULTICAST_DELEGATE_TwoParams(FCrimItemContainerFastItemSignature, UCrimItemContainerBase*, const FFastCrimItem&);

/**
 * An object that holds one or more item instances. Like an inventory, treasure chest, item pickup, etc...
 * Managed by and replicated through the CrimItemManagerComponent. Designed to be subclassed to allow defining how
 * items are added and managed in the Container.
 */
UCLASS(ClassGroup = "Crim Item System", Blueprintable, BlueprintType)
class CRIMITEMSYSTEM_API UCrimItemContainerBase : public UObject
{
	GENERATED_BODY()

	friend UCrimItemManagerComponent;
	
	/** The unique tag identifying this container. */
	UPROPERTY(Replicated)
	FGameplayTag ContainerGuid;

protected:
	/** The user-facing display name of this container. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer|UI")
	FText DisplayName;

	/** The ViewModel to create for this item container. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer|UI")
	TSoftClassPtr<UCrimItemContainerViewModelBase> ViewModelClass;

	/** Tags that this container has. */
	UPROPERTY(EditAnywhere, Category = "CrimItemContainer")
	FGameplayTagContainer OwnedTags;

public:
	UCrimItemContainerBase();
	virtual void PostInitProperties() override;
	virtual bool IsSupportedForNetworking() const override {return true;}
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

	/** Called when an item is added to the container. */
	FCrimItemContainerFastItemSignature OnItemAddedDelegate;
	/** Called when an item is removed from the container. */
	FCrimItemContainerFastItemSignature OnItemRemovedDelegate;
	/** Called when an item's property has changed in the container. */
	FCrimItemContainerFastItemSignature OnItemChangedDelegate;
	
	/** Returns the Container's Guid. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	const FGameplayTag& GetContainerGuid() const;

	/** Returns the user facing display name. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	FText GetDisplayName() const {return DisplayName;}

	/** Returns the view model class.*/
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	TSoftClassPtr<UCrimItemContainerViewModelBase> GetViewModelClass() const {return ViewModelClass;}

	/** Returns the Container's owned gameplay tags. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	const FGameplayTagContainer& GetOwnedTags() const;

	/** Gets the ItemManager component that owns this container. */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	UCrimItemManagerComponent* GetItemManagerComponent() const {return ItemManagerComponent;}

	/** Returns a const reference to the ItemList */
	const FFastCrimItemList& GetItemList() const;
	
	/** Returns a const reference to all items in the ItemContainer */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer")
	const TArray<FFastCrimItem>& GetItems() const;
	
	/**
	 * @param ItemGuid The unique Guid to search for.
	 * @return A copy of the item matching the ItemId.
	 */
	FFastCrimItem* GetItemByGuid(FGuid ItemGuid) const;
	
	/**
	 * @param ItemGuid The unique Guid to search for.
	 * @return A copy of the item matching the ItemId.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer", DisplayName = "GetItemByGuid")
	TInstancedStruct<FCrimItem> K2_GetItemByGuid(FGuid ItemGuid) const;

	/**
	 * @return The first item found with the matching ItemDefinition.
	 */
	FFastCrimItem* GetItemByDefinition(const UCrimItemDefinition* ItemDefinition) const;
	
	/**
	 * @return The first item found with the matching ItemDefinition.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemContainer", DisplayName = "GetItemByDefinition")
	TInstancedStruct<FCrimItem> K2_GetItemByDefinition(const UCrimItemDefinition* ItemDefinition) const;

	/**
	 * @return All items in the container by ItemDefinition.
	 */
	TArray<FFastCrimItem*> GetItemsByDefinition(const UCrimItemDefinition* ItemDefinition) const;
	
	/**
	 * @return All items in the container by ItemDefinition.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer", DisplayName = "GetItemsByDefinition")
	TArray<TInstancedStruct<FCrimItem>> K2_GetItemsByDefinition(const UCrimItemDefinition* ItemDefinition) const;

	/**
	 * Returns the first item that matches the TestItem.
	 * See UCrimItem::IsMatching
	 */
	FFastCrimItem* FindMatchingItem(const TInstancedStruct<FCrimItem>& TestItem) const;
	
	/**
	 * Returns the first item that matches the TestItem.
	 * See UCrimItem::IsMatching
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer", DisplayName = "FindMatchingItem")
	TInstancedStruct<FCrimItem> K2_FindMatchingItem(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * @note See FCrimItem::IsMatching
	 * @param TestItem The item to check against.
	 * @return Pointers to all matching items.
	 */
	TArray<FFastCrimItem*> FindMatchingItems(const TInstancedStruct<FCrimItem>& TestItem) const;
	
	/**
	 * @note See FCrimItem::IsMatching
	 * @param TestItem The item to check against.
	 * @return A copy of all items that match the TestItem.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemContainer", DisplayName = "FindMatchingItems")
	TArray<TInstancedStruct<FCrimItem>> K2_FindMatchingItems(const TInstancedStruct<FCrimItem>& TestItem) const;

	/**
	 * Creates a new item instance and initializes it with the ItemDefinition.
	 * @param ItemDefinition The ItemDefinition to associate with the item.
	 * @param Quantity The number of items in the stack with a minimum set to 1.
	 * @return The newly created item.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemContainer")
	static TInstancedStruct<FCrimItem> CreateItem(const UCrimItemDefinition* ItemDefinition, const int32 Quantity = 1);

	/**
	 * Checks to make sure the item is valid before duplicating.
	 * @param Item The item to duplicate.
	 * @return An exact copy of the item with a new ItemGuid.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemContainer")
	static TInstancedStruct<FCrimItem> DuplicateItem(UPARAM(ref) const TInstancedStruct<FCrimItem>& Item);
	
	/**
	 * Tries to add an item to this container.
	 * @param Item The item to add.
	 * @return The actual amount of the item that was added and any errors if the item could not be added in full.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	FCrimAddItemResult TryAddItem(UPARAM(ref) const TInstancedStruct<FCrimItem>& Item);

	/**
	 * Checks to see if the item can be added to the ItemContainer.
	 * @param Item The item to check.
	 * @param OutError The error if applicable.
	 * @return True, if the item can be added to the ItemContainer.
	 */
	virtual bool CanAddItem(const TInstancedStruct<FCrimItem>& Item, FGameplayTag& OutError) const;

	/**
	 * Consumes the specified quantity of the item. The item's quantity can't go below 0. If it is 0, the item is 
	 * removed from the ItemContainer.
	 * @param ItemGuid The item to consume quantity from.
	 * @param Quantity The amount to subtract from the item.
	 * @param bRemoveItem If false, the item will not be removed from the ItemContainer when the quantity reaches 0.
	 * @return The amount actually consumed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	int32 ConsumeItem(const FGuid ItemGuid, const int32 Quantity, bool bRemoveItem = true);

	/**
	 * Gets all items by ItemDefinition. Then subtracts quantity from them until the amount subtracted has reached
	 * 0. Then, if an item's quantity is 0, removes the item from the container.
	 * @param ItemDefinition The ItemDef to look for amongst items.
	 * @param Quantity The total quantity to subtract from the items.
	 * @param bRemoveItem If false, the items will not be removed from the ItemContainer when the quantity reaches 0.
	 * @return The amount actually consumed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	int32 ConsumeItemsByDefinition(const UCrimItemDefinition* ItemDefinition, const int32 Quantity, bool bRemoveItem = true);

	/**
	 * Finds an item by Guid to remove from the ItemContainer.
	 * @param ItemGuid The item to remove.
	 * @return A copy of the Item that was removed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	TInstancedStruct<FCrimItem> RemoveItem(const FGuid ItemGuid);

	/**
	 * Finds all items by ItemDefinition to remove from the ItemContainer.
	 * @param ItemDefinition Items with this definition will be removed.
	 * @return A copy of all Items that were removed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemContainer")
	TArray<TInstancedStruct<FCrimItem>> RemoveItemsByDefinition(const UCrimItemDefinition* ItemDefinition);

	/**
	 * @param Item The item to check.
	 * @return True, if the Item can be directly removed from the ItemContainer.
	 */
	virtual bool CanRemoveItem(const TInstancedStruct<FCrimItem>& Item) const;

	/**
	 * You must manually call this when an Item stored in this ItemContainer has been modified.
	 */
	void MarkItemDirty(FFastCrimItem& FastItem);

	UFUNCTION(BlueprintPure, Category = "CrimItemContainer")
	bool HasAuthority() const {return bOwnerIsNetAuthority;}
	
protected:

	/**
	 * Is called from the ItemManagerComponent upon creating this ItemContainer.
	 * Use this for BeginPlay functionality.
	 */
	virtual void Initialize(UCrimItemManagerComponent* ItemManager, FGameplayTag NewContainerGuid);

	/**
	 * Should be overriden by subclasses to determine custom logic. By default, it will add the item without any changes.
	 * @param Item The item to evaluate.
	 * @return The plan on how to add the Item to the ItemContainer.
	 */
	virtual FCrimAddItemPlan GetAddItemPlan(const TInstancedStruct<FCrimItem>& Item) const;

	/**
	 * Called in ExecuteAddItemPlan just before the item is added to the ItemContainer ItemList. Gives you a chance to
	 * modify the item before it's added to the container. It is a duplicate of the original item.
	 * @param Item The item you can modify.
	 */
	virtual void OnPreAddItem(TInstancedStruct<FCrimItem>& Item){}

	/**
	 * Called in ExecuteAddItemPlan just before an existing item is modified in the ItemContainer ItemList. Gives you a
	 * chance to modify the item before it is marked dirty.
	 * @param Item The item you can modify.
	 * @param TemplateItem The reference item being added to the ItemContainer.
	 */
	virtual void OnPreModifyItem(TInstancedStruct<FCrimItem>& Item, const TInstancedStruct<FCrimItem> TemplateItem){}
	
	/** Called when an item is added to the container. */
	virtual void OnItemAdded(const FFastCrimItem& FastItem){}
	/** Called when an item is added to the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnItemAdded")
	void K2_OnItemAdded(const FFastCrimItem& FastItem);

	/** Called when an item is removed from the container. */
	virtual void OnItemRemoved(const FFastCrimItem& FastItem){}
	/** Called when an item is removed from the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnItemRemoved")
	void K2_OnItemRemoved(const FFastCrimItem& FastItem);

	/** Called when an item has been changed in the container. */
	virtual void OnItemChanged(const FFastCrimItem& FastItem) {}
	/** Called when an item has been changed in the container. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemContainer", DisplayName = "OnItemChanged")
	void K2_OnItemChanged(const FFastCrimItem& FastItem);

	/**
	 * Adds the Item to this container's FastArray of items.
	 * @note Assumes all data is valid before adding to the List.
	 */
	void Internal_AddItem(TInstancedStruct<FCrimItem>& Item);

	/** Removes the item from the container by Guid */
	void Internal_RemoveItem(const FGuid& ItemGuid);

	/**
	 * Executes the AddItemPlan and returns a copy of the Items added or modified in the Container.
	 * The plan will ensure no duplicate ItemGuids are added to the Container.
	 */
	TArray<TInstancedStruct<FCrimItem>> ExecuteAddItemPlan(const TInstancedStruct<FCrimItem>& Item, const FCrimAddItemPlan& AddItemPlan);
	
private:
	UPROPERTY(Replicated)
	FFastCrimItemList ItemList;

	/** The owner of this item container. */
	UPROPERTY(Replicated)
	TObjectPtr<UCrimItemManagerComponent> ItemManagerComponent;
	UPROPERTY()
	bool bOwnerIsNetAuthority = false;
	
	void BindToItemListDelegates();

	void Internal_OnItemAdded(const FFastCrimItem& FastItem);
	void Internal_OnItemRemoved(const FFastCrimItem& FastItem);
	void Internal_OnItemChanged(const FFastCrimItem& FastItem);
};
