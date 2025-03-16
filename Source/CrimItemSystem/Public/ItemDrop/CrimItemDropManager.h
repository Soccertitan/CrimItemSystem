// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Info.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemDropManager.generated.h"

struct FCrimItemSpec;
struct FCrimItemDropParams;
class UCrimItem;
class ACrimItemDrop;
class UCrimItemManagerComponent;

/**
 * Manages the replication for ItemDrops and their items.
 */
UCLASS(BlueprintType, Blueprintable)
class CRIMITEMSYSTEM_API ACrimItemDropManager : public AInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UCrimItemManagerComponent> ItemManagerComponent;
	
public:
	ACrimItemDropManager();

	/**
	 * Drops an existing item from an ItemManagerComponent.
	 * @param ItemToDrop 
	 * @param Quantity 
	 * @param Params 
	 * @return The newly created ItemDrop.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDropManager")
	ACrimItemDrop* DropItem(UCrimItem* ItemToDrop, int32 Quantity, FCrimItemDropParams Params);

	/**
	 * Creates a new item drop from an ItemSpec.
	 * @param ItemSpec 
	 * @param Params 
	 * @return The newly created ItemDrop.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDropManager")
	ACrimItemDrop* DropItemBySpec(TInstancedStruct<FCrimItemSpec> ItemSpec, FCrimItemDropParams Params);

protected:
	virtual void BeginPlay() override;

	/** The maximum number of ItemDropActors allowed to be spawned in the world. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxItemDrops = 100;

	/** Removes the item from the managed ItemDrops.*/
	virtual void OnItemDropTaken(ACrimItemDrop* ItemDrop);

	void ClearItemDrop(ACrimItemDrop* ItemDrop);

private:

	UPROPERTY()
	TArray<ACrimItemDrop*> ItemDrops;
	// Cached off value of the ItemContainerId
	UPROPERTY()
	FGameplayTag ItemContainerId;

	/** If at MaxItemDrops, makes enough space for one new item drop. */
	void ClearItemDrops();

	/** Creates the ItemDrop and adds it to the array. */
	ACrimItemDrop* CreateItemDrop(UCrimItem* Item, FCrimItemDropParams& Params);
};
