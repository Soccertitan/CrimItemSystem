// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "UObject/AssetRegistryTagsContext.h"
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

	/**
	 * Called from CrimItemDefinition when gathering the AssetTags for the AssetRegistrySearch functionality. Follow this
	 * design pattern to avoid clobbering AssetTag Names across different Fragments.
	 * For example if your ItemDefFragment is called "CrimItemDefFrag_Quantity" your RegistryTag Name should be
	 * "CrimItemDefFrag_Quantity_{Property}" Example code is below.
	 *
	 * @note 
	 * UObject::FAssetRegistryTag RegistryTag;
	 * RegistryTag.Type = UObject::FAssetRegistryTag::TT_Alphabetical;
	 * RegistryTag.Name = "CrimItemDefFrag_Quantity_{Property}";
	 * RegistryTag.Value = {Property in String Format};
	 * Context.AddTag(RegistryTag);
	 *
	 * @bug As of UE5.6 RegistryTags added this way will not show up in the Editor Window. But they are still being
	 * added successfully.
	 */
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const {}
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
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;

	/** The tags that this item has.
	 * @note You can search for items with specific tags through the AssetRegistry.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FGameplayTagContainer OwnedTags;

	/** The default stats for the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (ForceInlineRow, ClampMin=1))
	TMap<FGameplayTag, int32> DefaultStats;

	/**
	 * Defines custom item functionality.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (FullyExpand=true, ExcludeBaseStruct))
	TArray<TInstancedStruct<FCrimItemDefinitionFragment>> Fragments;

	/** If set to false, this item should not be created. Useful for marking a deprecated item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", AdvancedDisplay, AssetRegistrySearchable)
	bool bSpawnable = true;

	/** The Item class to use to create from this ItemDef */
	UPROPERTY(BlueprintReadOnly)
	TInstancedStruct<FCrimItem> ItemClass;

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