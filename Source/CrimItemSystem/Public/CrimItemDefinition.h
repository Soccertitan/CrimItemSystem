// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "CrimItemDefinition.generated.h"


class UCrimItemViewModelBase;

// Represents a fragment of an item definition. Contains static information and way to initialize a new item.
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemDefinitionFragment
{
	GENERATED_BODY()

	FCrimItemDefinitionFragment(){}
	virtual ~FCrimItemDefinitionFragment() {}

	/** Called when an item is created from an ItemDefinition. */
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

	/** The default stats for the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ForceInlineRow, ClampMin=1))
	TMap<FGameplayTag, int32> DefaultStats;

	/**
	 * Defines custom item functionality.
	 * @note Soft Class/Object nested in a UPROPERTY for InstancedStruct currently (UE5.5) does not load AssetBundles when marked.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (FullyExpand=true, ExcludeBaseStruct))
	TArray<TInstancedStruct<FCrimItemDefinitionFragment>> Fragments;

	/** If set to false, this item should not be created. Useful for marking a deprecated item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", AdvancedDisplay, AssetRegistrySearchable)
	bool bSpawnable = true;

	/** The Item class to use to create from this ItemDef */
	UPROPERTY(BlueprintReadOnly)
	TInstancedStruct<FCrimItem> ItemClass;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	template<typename T> requires std::derived_from<T, FCrimItemDefinitionFragment>
	const T* GetFragmentByType() const;
};

/**
 * @return A const pointer to the first FCrimItemFragment that matches the type.
 */
template <typename T> requires std::derived_from<T, FCrimItemDefinitionFragment>
const T* UCrimItemDefinition::GetFragmentByType() const
{
	for (const TInstancedStruct<FCrimItemDefinitionFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}