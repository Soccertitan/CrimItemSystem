// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemDrop.generated.h"

struct FCrimItem;
class ACrimItemDropManager;
class ACrimItemDrop;
class UCrimItemManagerComponent;
class UCrimItemContainer;

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
	 * @param ItemContainer The container to put the item in.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDrop")
	void TakeItem(UCrimItemContainer* ItemContainer);

	/**
	 * @return A copy of the item held by this item drop.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemDrop")
	TInstancedStruct<FCrimItem> GetItem() const;

	UFUNCTION(BlueprintPure, Category = "CrimItemDrop")
	bool HasValidItem() const;

	/** Returns the ItemContainer from the ItemDrop */
	UFUNCTION(BlueprintPure, Category = "CrimItemDrop")
	UCrimItemContainer* GetItemContainer() const;

	/**
	 * Decides if the passed in ItemManager can attempt to take this ItemDrop.
	 * @param ItemManager The ItemManagerComponent to check if it can take the item.
	 * @return True if the ItemManager can take the item.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CrimItemDrop")
	bool CanTakeItem(UCrimItemManagerComponent* ItemManager) const;

	/**
	 * Initializes this actor with an Item and optional context data.
	 * @param InItemId The item to assign this ItemDrop.
	 * @param Context Custom user data that can be passed in and processed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDrop")
	virtual void InitializeItemDrop(FGuid InItemId, UObject* Context);
	
protected:

	/**
	 * Called on the server when this actor has been initialized with an item.
	 * @param InItemId The item this actor was initialized with. The Item is guaranteed to be valid.
	 * @param Context Custom user data that can be passed in and processed.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "InitializeItemDrop")
	void K2_InitializeItemDrop(FGuid InItemId, UObject* Context);

private:
	UPROPERTY(Replicated)
	FGuid ItemId;

	/** Cached reference to the ItemContainer from the ItemDropManager. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCrimItemContainer> ItemDropItemContainer;

	friend ACrimItemDropManager;
};
