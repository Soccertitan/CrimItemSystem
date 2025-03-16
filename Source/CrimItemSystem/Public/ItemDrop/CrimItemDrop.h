// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrimItemDrop.generated.h"

struct FGameplayTag;
struct FCrimItemAddPlan;
class ACrimItemDropManager;
class UCrimItemManagerComponent;
class UCrimItem;
class ACrimItemDrop;

DECLARE_MULTICAST_DELEGATE_OneParam(FCrimItemDropGenericSignature, ACrimItemDrop*);

/**
 * An actor that represents a single instance of an Item. This can be taken by another actor with an ItemManagerComponent.
 * 
 */
UCLASS(ClassGroup = "Crim Item System", Blueprintable, BlueprintType, Abstract)
class CRIMITEMSYSTEM_API ACrimItemDrop : public AActor
{
	GENERATED_BODY()

public:
	ACrimItemDrop();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called on server when the item quantity of this ItemDrop reaches 0. */
	FCrimItemDropGenericSignature OnAllItemsTaken;

	/**
	 * The ItemManagerComponent can try and take this item drop and all the quantity.
	 * @param ItemManagerComponent The ItemManager taking this ItemDrop.
	 * @param ContainerId The container to put the item in.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDrop")
	void TakeItem(UCrimItemManagerComponent* ItemManagerComponent, FGameplayTag ContainerId);

	/**
	 * @return The item instance held by this item drop.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemDrop")
	UCrimItem* GetItem() const;

	/**
	 * @return True if the CrimItem is a valid instance.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemDrop")
	bool HasValidItemInstance() const;

	/**
	 * Decides if the passed in ItemManager can attempt to take this ItemDrop.
	 * @param ItemManager The ItemManagerComponent to check if it can take the item.
	 * @return True if the ItemManager can take the item.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CrimItemDrop")
	bool CanTakeItem(UCrimItemManagerComponent* ItemManager) const;

	/**
	 * Initializes this actor with an Item and optional context data.
	 * @param InItem The item to assign this ItemDrop.
	 * @param Context Custom user data that can be passed in and processed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDrop")
	virtual void InitializeItemDrop(UCrimItem* InItem, UObject* Context);
	
protected:

	/**
	 * Called on the server when this actor has been initialized with an item.
	 * @param InItem The item this actor was initialized with. The Item is guaranteed to be valid.
	 * @param Context Custom user data that can be passed in and processed.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "InitializeItemDrop")
	void K2_InitializeItemDrop(UCrimItem* InItem, UObject* Context);

private:
	UPROPERTY(Replicated)
	TObjectPtr<UCrimItem> Item;

	/** Cached here for quick reference. */
	UPROPERTY()
	TObjectPtr<ACrimItemDropManager> ItemDropManager;

	friend ACrimItemDropManager;
};
