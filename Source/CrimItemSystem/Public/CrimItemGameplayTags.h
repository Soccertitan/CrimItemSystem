// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

/**
 * CrimItemGameplayTags
 * Singleton containing native gameplay tags.
 */

struct CRIMITEMSYSTEM_API FCrimItemGameplayTags
{
	static const FCrimItemGameplayTags& Get() {return GameplayTags;}
	static void InitializeNativeGameplayTags();

private:
	static FCrimItemGameplayTags GameplayTags;
	
public:

	FGameplayTag Crim_ItemContainer_ItemDropManager;
	
	/**
	 * Generic Root Gameplay Tags
	 */
	FGameplayTag Crim_ItemPlan_Error;

	/**
	 * Add Item Plan Errors
	 */
	FGameplayTag Crim_ItemPlan_Error_InvalidItem;
	FGameplayTag Crim_ItemPlan_Error_ItemAlreadyExists;
	FGameplayTag Crim_ItemPlan_Error_ItemContainerNotFound;
	FGameplayTag Crim_ItemPlan_Error_ItemContainerIsChild;
	FGameplayTag Crim_ItemPlan_Error_CantContainItem;
	FGameplayTag Crim_ItemPlan_Error_QuantityIsZero;
	FGameplayTag Crim_ItemPlan_Error_MaxStacksReached;
};
