// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemDropManager.generated.h"

struct FCrimItem;
struct FCrimItemSpec;
struct FCrimItemDropParams;
class ACrimItemDrop;
class UCrimItemManagerComponent;
class UCrimItemContainerBase;

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
	 * Takes the passed in Item and tries to represent it in the world.
	 * @param Item The item to drop.
	 * @param Params Defines the ItemDropActor.
	 * @return The newly created ItemDrop.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDropManager")
	ACrimItemDrop* DropItem(UPARAM(ref) const TInstancedStruct<FCrimItem>& Item, FCrimItemDropParams Params);

	/**
	 * Creates a new item drop from an ItemSpec.
	 * @param ItemSpec 
	 * @param Params 
	 * @return The newly created ItemDrop.
	 */
	// UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "CrimItemDropManager")
	// ACrimItemDrop* DropItemBySpec(TInstancedStruct<FCrimItemSpec> ItemSpec, FCrimItemDropParams Params);

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
	UCrimItemContainerBase* ItemContainer;

	/** If at MaxItemDrops, makes enough space for one new item drop. */
	void ClearItemDrops();

	/** Creates the ItemDrop and adds it to the array. */
	ACrimItemDrop* CreateItemDrop(FGuid ItemId, FCrimItemDropParams& Params);
};
