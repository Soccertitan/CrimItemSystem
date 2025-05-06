// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "CrimItemTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CrimItemDefinition.generated.h"


class UCrimItemViewModelBase;

// Represents a fragment of an item definition. Can contain custom information and initialization.
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemFragment
{
	GENERATED_BODY()

	FCrimItemFragment(){}
	virtual ~FCrimItemFragment() {}

	virtual void SetDefaultValues(TInstancedStruct<FCrimItem>& ItemInstance) const {}
};

/**
 * The base definition for any item.
 */
UCLASS(ClassGroup = "Crim Item System")
class CRIMITEMSYSTEM_API UCrimItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UCrimItemDefinition();

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

	/** The ViewModel to use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "UI"), NoClear)
	TSoftClassPtr<UCrimItemViewModelBase> ItemViewModelClass;

	/** The widget to display item information. This class should implement the CrimItemVMInterface. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "UI"))
	TSoftClassPtr<UUserWidget> ItemWidgetClass;

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

	/** Defines custom item functionality. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (NoResetToDefault, FullyExpand=true, ExcludeBaseStruct))
	TArray<TInstancedStruct<FCrimItemFragment>> Fragments;

	/** If set to false, this item should not be created. Useful for marking a deprecated item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", AdvancedDisplay, AssetRegistrySearchable)
	bool bSpawnable = true;

	/** The Item class to use to create from this ItemDef */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", AdvancedDisplay, NoClear)
	TInstancedStruct<FCrimItem> ItemClass;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/**
	 * @tparam T Must be of type FCrimItemFragment.
	 * @return A const pointer to the first FCrimItemFragment that matches the type.
	 */
	template <typename T>
	const T* FindFragmentByClass() const
	{
		for (const TInstancedStruct<FCrimItemFragment>& Fragment : Fragments)
		{
			if (Fragment.IsValid() && Fragment.GetPtr<T>())
			{
				return Fragment.GetPtr<T>();
			}
		}
		return nullptr;
	}
};
