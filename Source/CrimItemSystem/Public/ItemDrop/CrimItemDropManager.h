// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemDropManager.generated.h"

struct FCrimItemSpec;
struct FCrimItemDropParams;
class ACrimItemDrop;
class UCrimItemManagerComponent;
class UCrimItemContainer;

/**
 * Manages the replication for ItemDrops and their items.
 */
UCLASS(BlueprintType, Blueprintable)
class CRIMITEMSYSTEM_API ACrimItemDropManager : public AInfo
{
	GENERATED_BODY()

	friend ACrimItemDrop;
	
	UPROPERTY()
	TObjectPtr<UCrimItemManagerComponent> ItemManagerComponent;
	
public:
	ACrimItemDropManager();
	virtual void BeginPlay() override;

	/**
	 * Drops an existing item from an ItemManagerComponent.
	 * @param SourceContainer The container that has the item to drop.
	 * @param SourceItemId The Item to drop.
	 * @param Quantity The amount to drop from the item.
	 * @param Params Defines the ItemDropActor.
	 * @return The newly created ItemDrop.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDropManager")
	ACrimItemDrop* DropItem(UCrimItemContainer* SourceContainer, UPARAM(ref) const FGuid& SourceItemId, int32 Quantity, FCrimItemDropParams Params);

	/**
	 * Creates a new item drop from an ItemSpec.
	 * @param ItemSpec 
	 * @param Params 
	 * @return The newly created ItemDrop.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDropManager")
	ACrimItemDrop* DropItemBySpec(TInstancedStruct<FCrimItemSpec> ItemSpec, FCrimItemDropParams Params);

protected:

	/** The maximum number of ItemDropActors allowed to be spawned in the world. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxItemDrops = 100;

	/** Removes the item from the managed ItemDrops.*/
	virtual void OnItemDropTaken(ACrimItemDrop* ItemDrop);

	void ClearItemDrop(ACrimItemDrop* ItemDrop);

private:

	UPROPERTY()
	TArray<ACrimItemDrop*> ItemDrops;
	// Cached off value of the ItemContainer
	UPROPERTY()
	UCrimItemContainer* ItemContainer;

	/** If at MaxItemDrops, makes enough space for one new item drop. */
	void ClearItemDrops();

	/** Creates the ItemDrop and adds it to the array. */
	ACrimItemDrop* CreateItemDrop(FGuid ItemId, FCrimItemDropParams& Params);
};
