// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "CrimItemDefinition.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemStatics.generated.h"

struct FCrimItemDefinitionFragment;
class UCrimItemDefinition;
class UCrimItemFragment;
class UCrimItemManagerComponent;

/**
 * 
 */
UCLASS(ClassGroup = "Crim Item System")
class CRIMITEMSYSTEM_API UCrimItemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the ItemManagerComponent if the actor implements the CrimItemSystemInterface. If it does not, fallbacks to
	 * checking the actor's components.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemSystem", meta = (DefaultToSelf = "Actor"))
	static UCrimItemManagerComponent* GetCrimItemManagerComponent(const AActor* Actor);

	/**
	 * Synchronously loads the ItemDefinition from the Item. Does not flush any current AsyncLoads.
	 * @param Item The item to retrieve the ItemDefinition from.
	 * @return The loaded ItemDef or nullptr.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemSystem")
	static const UCrimItemDefinition* GetItemDefinition(UPARAM(ref) const TInstancedStruct<FCrimItem>& Item);

	template<typename T> requires std::derived_from<T, FCrimItemFragment>
	static const T* GetItemFragmentByType(const TInstancedStruct<FCrimItem>& Item);
	template<typename T> requires std::derived_from<T, FCrimItemFragment>
	static T* GetMutableItemFragmentByType(TInstancedStruct<FCrimItem>& Item);
	template<typename T> requires std::derived_from<T, FCrimItemDefinitionFragment>
	static const T* GetItemDefinitionFragmentByType(const TInstancedStruct<FCrimItem>& Item);

	/**
	 * Iterates through an Item's Fragments and finds the one that is a child of FragmentType.
	 * @param Item The item to get the ItemFragment from.
	 * @param FragmentType The type of item fragment to search for.
	 * @return An InstancedStruct of type CrimItemFragment.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemSystem", DisplayName = "Get Item Fragment")
	static TInstancedStruct<FCrimItemFragment> K2_GetItemFragment(UPARAM(ref) const TInstancedStruct<FCrimItem>& Item, const UScriptStruct* FragmentType);

	/**
	 * Iterates through an ItemDefinition's Fragments and finds the one that is a child of FragmentType.
	 * @param ItemDefinition The ItemDefinition to get the ItemDefinitionFragment from.
	 * @param FragmentType The type of fragment to search for.
	 * @return An instanced struct of type CrimItemDefinitionFragment.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemSystem", DisplayName = "Get Item Definition Fragment")
	static TInstancedStruct<FCrimItemDefinitionFragment> K2_GetItemDefinitionFragment(const UCrimItemDefinition* ItemDefinition, const UScriptStruct* FragmentType);
};

/**
 * @return A const pointer to the first FCrimItemExtension that matches the type.
 */
template <typename T> requires std::derived_from<T, FCrimItemFragment>
const T* UCrimItemStatics::GetItemFragmentByType(const TInstancedStruct<FCrimItem>& Item)
{
	if (Item.IsValid())
	{
		return Item.Get<FCrimItem>().GetFragmentByType<T>();
	}
	return nullptr;
}

/**
 * @return A mutable pointer to the first FCrimItemExtension that matches the type.
 */
template <typename T> requires std::derived_from<T, FCrimItemFragment>
T* UCrimItemStatics::GetMutableItemFragmentByType(TInstancedStruct<FCrimItem>& Item)
{
	if (Item.IsValid())
	{
		return Item.GetMutable<FCrimItem>().GetMutableFragmentByType<T>();
	}
	return nullptr;
}

/**
 * @return A const pointer to the first FCrimItemFragment that matches the type.
 */
template <typename T> requires std::derived_from<T, FCrimItemDefinitionFragment>
const T* UCrimItemStatics::GetItemDefinitionFragmentByType(const TInstancedStruct<FCrimItem>& Item)
{
	if (Item.IsValid())
	{
		if (const UCrimItemDefinition* ItemDefinition = GetItemDefinition(Item))
		{
			return ItemDefinition->GetFragmentByType<T>();
		}
	}
	return nullptr;
}
