// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "CrimItemFastTypes.h"
#include "CrimItemSaveDataTypes.h"
#include "CrimItemTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemManagerComponent.generated.h"


class UCrimItemDefinition;
class UCrimItemContainerBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCrimItemManagerComponentItemSignature, UCrimItemManagerComponent*, ItemManagerComponent, UCrimItemContainerBase*, ItemContainer, const FFastCrimItem&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCrimItemManagerComponentItemContainerSignature, UCrimItemManagerComponent*, ItemManagerComponent, UCrimItemContainerBase*, ItemContainer);

/**
 * Manages a collection of ItemContainers and their items.
 */
UCLASS(ClassGroup = "Crim Item System", meta=(BlueprintSpawnableComponent))
class CRIMITEMSYSTEM_API UCrimItemManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCrimItemManagerComponent();
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PreNetReceive() override;
	virtual void OnRegister() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called when a new item has been added to a container. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnItemAdded")
	FCrimItemManagerComponentItemSignature OnItemAddedDelegate;
	
	/** Called when an item has been removed from a container. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnItemRemoved")
	FCrimItemManagerComponentItemSignature OnItemRemovedDelegate;

	/** Called when an Item's properties have changed in a container. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnItemChanged")
	FCrimItemManagerComponentItemSignature OnItemChangedDelegate;

	/** Called when an ItemContainer has been added to the list. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnItemContainerAdded")
	FCrimItemManagerComponentItemContainerSignature OnItemContainerAddedDelegate;

	/** Called when an ItemContainer has been added to the list. */
	UPROPERTY(BlueprintAssignable, DisplayName = "OnItemContainerRemoved")
	FCrimItemManagerComponentItemContainerSignature OnItemContainerRemovedDelegate;
	
	/**
	 * @param ContainerGuid The ContainerId to search for.
	 * @return The ItemContainer with the matching ContainerId.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	UCrimItemContainerBase* GetItemContainerByGuid(UPARAM(meta = (Categories = "ItemContainer")) FGameplayTag ContainerGuid) const;

	/** Returns true if this ItemManager manages the passed in ItemContainer. */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	bool HasItemContainer(const UCrimItemContainerBase* ItemContainer) const;

	/** Gets all item containers this ItemManager has. */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	const TArray<FFastCrimItemContainerItem>& GetItemContainers() const;

	/**
	 * @param ItemGuid The item to search for.
	 * @return A pointer to the found item.
	 */
	FFastCrimItem* GetItemByGuid(FGuid ItemGuid) const;
	
	/**
	 * @param ItemGuid The item to search for.
	 * @return A copy of the found item.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent", DisplayName = "GetItemByGuid")
	TInstancedStruct<FCrimItem> K2_GetItemByGuid(FGuid ItemGuid) const;

	/**
	 * @param ItemDefinition The ItemDef to check.
	 * @return A pointer of all items with matching item definitions.
	 */
	TArray<FFastCrimItem*> GetItemsByDefinition(const UCrimItemDefinition* ItemDefinition) const;
	
	/**
	 * @param ItemDefinition The ItemDef to check.
	 * @return A copy of all items with matching item definitions.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemManagerComponent", DisplayName = "GetItemsByDefinition")
	TArray<TInstancedStruct<FCrimItem>> K2_GetItemsByDefinition(const UCrimItemDefinition* ItemDefinition) const;

	/**
	 * Gets all items with the matching ItemDef. Then subtracts quantity from them until the amount subtracted has reached
	 * 0. Then, if an item's quantity is 0, removes the item from the ItemContainer.
	 * @param ItemDefinition The ItemDef to look for amongst items.
	 * @param Quantity The amount to subtract from the items.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void ConsumeItemsByDefinition(const UCrimItemDefinition* ItemDefinition, int32 Quantity);

	/**
	 * Creates a new item container and initializes it.
	 * @param ContainerGuid The Guid to set the new container with. If invalid, will create one anyway.
	 * @param ItemContainerClass The item container class to create.
	 * @return The newly created ItemContainer or a nullptr if creating a new one failed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	UCrimItemContainerBase* CreateItemContainer(FGameplayTag ContainerGuid, TSubclassOf<UCrimItemContainerBase> ItemContainerClass);

	/**
	 * Remove all items from the ItemContainer and the ItemContainer. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void RemoveItemContainer(UCrimItemContainerBase* ItemContainer);
	
	/**
	 * Collects all the unique item instances and ItemContainers. Saves the data in a struct.
	 * @return The current SaveData for the ItemManager.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	FCrimItemManagerSaveData GetSaveData() const;

	/**
	 * Sets the ItemManager to the SavedData's state.
	 * @param SaveData The save data.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void LoadSavedData(UPARAM(ref) const FCrimItemManagerSaveData& SaveData);

	/* Returns true if this Component's Owner Actor has authority. */
	bool HasAuthority() const;

protected:

	virtual void OnItemContainerAdded(const FFastCrimItemContainerItem& Entry);
	virtual void OnItemContainerRemoved(const FFastCrimItemContainerItem& Entry);

	virtual void OnItemAdded(UCrimItemContainerBase* ItemContainer, const FFastCrimItem& Item);
	virtual void OnItemRemoved(UCrimItemContainerBase* ItemContainer, const FFastCrimItem& Item);
	virtual void OnItemChanged(UCrimItemContainerBase* ItemContainer, const FFastCrimItem& Item);

private:
	/** Cached value of whether our owner is a simulated Actor. */
	UPROPERTY()
	bool bCachedIsNetSimulated = false;
	void CacheIsNetSimulated();

	/** All the ItemContainers managed by the ItemManager component. */
	UPROPERTY(Replicated)
	FFastCrimItemContainerList ItemContainerList;

public:
	/** The startup item containers. Mapped to a GameplayTag for the ContainerId. */
	UPROPERTY(EditAnywhere, Category = "CrimItemManagerComponent", meta=(ForceInlineRow, Categories = "ItemContainer"))
	TMap<FGameplayTag, FCrimStartupItems> StartupItems;

private:
	void InitializeStartupItems();
	void BindToItemContainerListDelegates();
	void BindToItemContainerDelegates(UCrimItemContainerBase* ItemContainer);
};
