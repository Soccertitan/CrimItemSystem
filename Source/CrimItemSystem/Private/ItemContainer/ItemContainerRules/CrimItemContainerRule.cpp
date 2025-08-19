// Copyright Soccertitan


#include "ItemContainer/ItemContainerRules/CrimItemContainerRule.h"

#include "BlueprintNodeHelpers.h"
#include "ItemContainer/CrimItemContainer.h"
#include "CrimItem.h"
#include "CrimItemGameplayTags.h"

UCrimItemContainerRule::UCrimItemContainerRule()
{
	bHasCanContainItem = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_CanAddItem"), *this, *StaticClass());
	bHasCanRemoveItem = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_CanRemoveItem"), *this, *StaticClass());
	bHasGetItemMaxQuantity = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_GetMaxNumberOfStacks"), *this, *StaticClass());
	bHasGetItemStatckMaxQuantity = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_GetItemStackMaxQuantity"), *this, *StaticClass());
}

bool UCrimItemContainerRule::CanAddItem(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item, FGameplayTag& OutError) const
{
	if (IsValid(Container) && Item.IsValid())
	{
		if (bHasCanContainItem)
        {
        	return K2_CanAddItem(Container, Item, OutError);
        }
		return true;
	}
	OutError = FCrimItemGameplayTags::Get().ItemPlan_Error;
	return false;
}

bool UCrimItemContainerRule::CanRemoveItem(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const
{
	if (IsValid(Container) && Item.IsValid())
	{
		if (bHasCanRemoveItem)
		{
			return K2_CanRemoveItem(Container, Item);
		}
		return true;
	}
	return false;
}

int32 UCrimItemContainerRule::GetMaxNumberOfStacks(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const
{
	if (IsValid(Container) && Item.IsValid())
	{
		if (bHasGetItemMaxQuantity)
		{
			return K2_GetMaxNumberOfStacks(Container, Item);
		}
		return MAX_int32;
	}
	return 0;
}

int32 UCrimItemContainerRule::GetItemStackMaxQuantity(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const
{
	if (IsValid(Container) && Item.IsValid())
	{
		if (bHasGetItemStatckMaxQuantity)
		{
			return K2_GetItemStackMaxQuantity(Container, Item);
		}
		return MAX_int32;
	}
	return 0;
}
