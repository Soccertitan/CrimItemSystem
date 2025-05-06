// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"

#include "CrimItemSaveDataTypes.generated.h"

struct FCrimItem;
class UCrimItemDefinition;
class UCrimItemContainer;

/** Contains the save data for an item. */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemSaveData
{
	GENERATED_BODY()

	FCrimItemSaveData(){}

	FCrimItemSaveData(TInstancedStruct<FCrimItem>& InItem);

	/** The item definition to check if it's valid before restoring the item. */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UCrimItemDefinition> ItemDef;

	/** The item's serialized SaveGame properties. */
	UPROPERTY()
	TArray<uint8> ByteData;
};

/** Contains the save data of the ItemContainer. */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemContainerSaveData
{
	GENERATED_BODY()

	FCrimItemContainerSaveData(){}

	FCrimItemContainerSaveData(UCrimItemContainer* InItemContainer);

	/** The ContainerId */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ContainerId = FGameplayTag();

	/** The class of ItemContainer to spawn. */
	UPROPERTY(BlueprintReadOnly)
	TSoftClassPtr<UCrimItemContainer> ItemContainerClass;

	/** The ItemContainer's serialized SaveGame properties. */
	UPROPERTY()
	TArray<uint8> ByteData;
	
	/** The item save data. */
	UPROPERTY(BlueprintReadOnly)
	TArray<FCrimItemSaveData> Items;
};

/** Contains the saved data to reconstruct an ItemManager's ItemContainers and their Items. */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemManagerSaveData
{
	GENERATED_BODY()

	/** Container Save Data */
	UPROPERTY(BlueprintReadOnly)
	TArray<FCrimItemContainerSaveData> ItemContainerSaveData;
};