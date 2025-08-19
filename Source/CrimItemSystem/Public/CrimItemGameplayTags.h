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

	FGameplayTag ItemContainer_Default;

	FGameplayTag ItemContainer_ItemDropManager;
	
	/**
	 * Generic Root Gameplay Tags
	 */
	FGameplayTag ItemPlan_Error;
	
	/**
	 * Add Item Plan Errors
	 */
	FGameplayTag ItemPlan_Error_InvalidItem;
	FGameplayTag ItemPlan_Error_ItemContainerNotFound;
	FGameplayTag ItemPlan_Error_CantContainItem;
	FGameplayTag ItemPlan_Error_QuantityIsZero;
	FGameplayTag ItemPlan_Error_MaxStacksReached;
};
