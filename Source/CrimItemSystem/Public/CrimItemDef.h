// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "CrimItemTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CrimItemDef.generated.h"

class UCrimItem;

/**
 * The base definition for any item. Can be subclassed for specific information.
 */
UCLASS(ClassGroup = "Crim Item System")
class CRIMITEMSYSTEM_API UCrimItemDef : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UCrimItemDef();

	/** The tags that this item has. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FGameplayTagContainer OwnedTags;
	
	/** User facing text of the item name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI")
	FText ItemName;

	/** User facing description of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI")
	FText ItemDescription;

	/** The user facing icon of the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> ItemIcon;

	/** The default stats for the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ForceInlineRow, ClampMin=1))
	TMap<FGameplayTag, int32> DefaultStats;

	/** The maximum number of unique item instances allowed to be managed by the ItemManager. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FCrimItemQuantityLimit CollectionLimit;

	/** The maximum number of unique item instances allowed in a single container. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FCrimItemQuantityLimit ContainerLimit;

	/** The maximum quantity of this item allowed in a single stack. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FCrimItemQuantityLimit StackLimit;

	/** If set to false, this item should not be created. Useful for marking an item is deprecated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", AdvancedDisplay)
	bool bSpawnable = true;

	/** The Item class to use to create from this ItemDef */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", AdvancedDisplay, NoClear)
	TSoftClassPtr<UCrimItem> ItemClass = UCrimItem::StaticClass();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
