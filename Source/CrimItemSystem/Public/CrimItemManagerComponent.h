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
class UCrimItemContainer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCrimItemManagerComponentItemSignature, UCrimItemManagerComponent*, ItemManagerComponent, UCrimItemContainer*, ItemContainer, const FFastCrimItem&, Item);

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
	UPROPERTY(BlueprintAssignable)
	FCrimItemManagerComponentItemSignature OnItemAddedDelegate;
	
	/** Called when an item has been removed from a container. */
	UPROPERTY(BlueprintAssignable)
	FCrimItemManagerComponentItemSignature OnItemRemovedDelegate;

	/** Called when an Item's properties has changed in a container. */
	UPROPERTY(BlueprintAssignable)
	FCrimItemManagerComponentItemSignature OnItemChangedDelegate;

	/**
	 * @param ContainerId The ContainerId to search for.
	 * @return The ItemContainer with the matching ContainerId.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	UCrimItemContainer* GetItemContainer(FGameplayTag ContainerId) const;

	/** Returns true if this ItemManager manages the passed in ItemContainer. */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	bool HasItemContainer(const UCrimItemContainer* ItemContainer) const;

	/** Gets all item containers this ItemManager has. */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	const TArray<FFastCrimItemContainer>& GetItemContainers() const;

	/**
	 * @param ItemId The item to search for.
	 * @return A pointer to the found item.
	 */
	FFastCrimItem* GetItemById(FGuid ItemId) const;
	
	/**
	 * @param ItemId The item to search for.
	 * @return A copy of the found item.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent", DisplayName = "GetItemById")
	TInstancedStruct<FCrimItem> K2_GetItemById(FGuid ItemId) const;

	/**
	 * @param ItemDef The ItemDef to check.
	 * @return A pointer of all items with matching item definitions.
	 */
	TArray<FFastCrimItem*> GetItemsFromDefinition(const UCrimItemDefinition* ItemDef) const;
	
	/**
	 * @param ItemDef The ItemDef to check.
	 * @return A copy of all items with matching item definitions.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "CrimItemManagerComponent", DisplayName = "GetItemsByDefinition")
	TArray<TInstancedStruct<FCrimItem>> K2_GetItemsFromDefinition(const UCrimItemDefinition* ItemDef) const;

	/**
	 * Iterates through all ItemContainers for items with a matching ItemDefinition.
	 * CollectionLimit - Total Items stacks.
	 * @param ItemDef The ItemDef to check.
	 * @return The number of remaining item stacks that are allowed for the item to be managed by this ItemManagerComponent.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemManagerComponent")
	int32 GetRemainingCollectionCapacityForItem(const UCrimItemDefinition* ItemDef) const;

	/**
	 * Gets all items with the matching ItemDef. Then subtracts quantity from them until the amount subtracted has reached
	 * 0. Then, if an item's quantity is 0, removes the item from the ItemContainer.
	 * @param ItemDef The ItemDef to look for amongst items.
	 * @param Quantity The amount to subtract from the items.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void ConsumeItemsByDefinition(const UCrimItemDefinition* ItemDef, int32 Quantity);

	/**
	 * Creates a new item container and initializes it.
	 * @param ContainerId The Id to set the new container with.
	 * @param ItemContainerClass The item container class to create.
	 * @return The newly created ItemContainer or a nullptr if creating a new one failed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	UCrimItemContainer* CreateItemContainer(FGameplayTag ContainerId, TSubclassOf<UCrimItemContainer> ItemContainerClass);

	/**
	 * Destroys all items from the ItemContainer and the ItemContainer. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemManagerComponent")
	void DestroyItemContainer(UCrimItemContainer* ItemContainer);
	
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
	void LoadSavedData(UPARAM(ref) const FCrimItemManagerSaveData& SaveData);

	/* Returns true if this Component's Owner Actor has authority. */
	bool HasAuthority() const;

private:
	/** Cached value of whether our owner is a simulated Actor. */
	UPROPERTY()
	bool bCachedIsNetSimulated = false;
	void CacheIsNetSimulated();

	/** All the ItemContainers managed by the ItemManager component. */
	UPROPERTY(Replicated)
	FFastCrimItemContainerList ItemContainerList;

	/** The startup item containers. Mapped to a GameplayTag for the ContainerId. */
	UPROPERTY(EditAnywhere, Category = "CrimItemManagerComponent", meta=(ForceInlineRow))
	TMap<FGameplayTag, TSubclassOf<UCrimItemContainer>> StartupItemContainers;

	void InitializeStartupItemContainers();
	void BindToItemContainerListDelegates();
	void BindToItemContainerDelegates(UCrimItemContainer* ItemContainer);
};
