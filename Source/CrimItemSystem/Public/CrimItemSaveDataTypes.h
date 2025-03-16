// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "CrimItemSaveDataTypes.generated.h"

class UCrimItem;
class UCrimItemDef;
class UCrimItemContainer;

/** Contains the save data of the ItemContainer. */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemContainerSaveData
{
	GENERATED_BODY()

	FCrimItemContainerSaveData(){}

	FCrimItemContainerSaveData(UCrimItemContainer* InItemContainer);

	/** The ContainerId */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ContainerId = FGameplayTag();

	/** The class of ItemContainer to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UCrimItemContainer> ItemContainerClass;

	/** The ItemContainer's serialized SaveGame properties. */
	UPROPERTY()
	TArray<uint8> ByteData;
};

/** Contains the save data for an item. */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemSaveData
{
	GENERATED_BODY()

	FCrimItemSaveData(){}

	FCrimItemSaveData(UCrimItem* InItem, int32 InSlot);

	/** Unique item identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ItemId = FGuid();

	/** The item definition to spawn the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UCrimItemDef> ItemDef;

	/** Slot the item occupied. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0))
	int32 Slot = 0;

	/** The item's serialized SaveGame properties. */
	UPROPERTY()
	TArray<uint8> ByteData;

	friend bool operator==(const FCrimItemSaveData& X, const FCrimItemSaveData& Y)
	{
		return X.ItemId == Y.ItemId;
	}
	friend bool operator!=(const FCrimItemSaveData& X, const FCrimItemSaveData& Y)
	{
		return X.ItemId != Y.ItemId;
	}
};

USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemContainerSaveData_Parent
{
	GENERATED_BODY()

	/** The Container save data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCrimItemContainerSaveData ItemContainer;
	
	/** The linked containers of the Parent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTag> ChildContainerIds;

	/** The item save data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCrimItemSaveData> Items;
};

USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemSaveData_Child
{
	GENERATED_BODY()

	FCrimItemSaveData_Child(){}
	FCrimItemSaveData_Child(FGuid InGuid, int32 InSlot);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ItemId = FGuid();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0))
	int32 Slot = 0;
};

USTRUCT(BlueprintType)
struct FCrimItemContainerSaveData_Child
{
	GENERATED_BODY()

	/** The Container save data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCrimItemContainerSaveData ItemContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCrimItemSaveData_Child> Items;
};

/** Contains the saved data to reconstruct an ItemManagerComponents Items and ItemContainers. */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemManagerSaveData
{
	GENERATED_BODY()

	/** Parent Container Save Data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCrimItemContainerSaveData_Parent> ParentSaveData;
	
	/** Child Container Save Data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCrimItemContainerSaveData_Child> ChildSaveData;
};