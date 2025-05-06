// Copyright Soccertitan


#include "ItemContainerRules/CrimItemContainerRule.h"

#include "BlueprintNodeHelpers.h"
#include "CrimItemContainer.h"
#include "CrimItem.h"

UCrimItemContainerRule::UCrimItemContainerRule()
{
	bHasCanContainItem = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_CanContainItem"), *this, *StaticClass());
	bHasCanRemoveItem = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_CanRemoveItem"), *this, *StaticClass());
	bHasGetItemMaxQuantity = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_GetMaxNumberOfStacks"), *this, *StaticClass());
	bHasGetItemStatckMaxQuantity = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_GetItemStackMaxQuantity"), *this, *StaticClass());
}

bool UCrimItemContainerRule::CanContainItem(const UCrimItemContainer* Container, const TInstancedStruct<FCrimItem>& Item) const
{
	if (IsValid(Container) && Item.IsValid())
	{
		if (bHasCanContainItem)
        {
        	return K2_CanContainItem(Container, Item);
        }
		return true;
	}
	return false;
}

bool UCrimItemContainerRule::CanRemoveItem(const UCrimItemContainer* Container, const TInstancedStruct<FCrimItem>& Item) const
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

int32 UCrimItemContainerRule::GetMaxNumberOfStacks(const UCrimItemContainer* Container, const TInstancedStruct<FCrimItem>& Item) const
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

int32 UCrimItemContainerRule::GetItemStackMaxQuantity(const UCrimItemContainer* Container, const TInstancedStruct<FCrimItem>& Item) const
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
