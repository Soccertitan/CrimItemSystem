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
	bHasGetValidSlots = BlueprintNodeHelpers::HasBlueprintFunction(TEXT("K2_GetValidSlots"), *this, *StaticClass());
}

bool UCrimItemContainerRule::CanContainItem(const UCrimItemContainer* Container, const UCrimItem* Item) const
{
	if (IsValid(Container) && IsValid(Item))
	{
		if (bHasCanContainItem)
        {
        	return K2_CanContainItem(Container, Item);
        }
		return true;
	}
	return false;
}

bool UCrimItemContainerRule::CanRemoveItem(const UCrimItemContainer* Container, const UCrimItem* Item) const
{
	if (IsValid(Container) && IsValid(Item))
	{
		if (bHasCanRemoveItem)
		{
			return K2_CanRemoveItem(Container, Item);
		}
		return true;
	}
	return false;
}

TArray<int32> UCrimItemContainerRule::GetValidSlots(const UCrimItemContainer* Container, const UCrimItem* Item) const
{
	if (IsValid(Container) && IsValid(Item))
	{
		if (bHasGetValidSlots)
		{
			return K2_GetValidSlots(Container, Item);
		}
	}
	return TArray<int32>();
}

int32 UCrimItemContainerRule::GetMaxNumberOfStacks(const UCrimItemContainer* Container, const UCrimItem* Item) const
{
	if (IsValid(Container) && IsValid(Item))
	{
		if (bHasGetItemMaxQuantity)
		{
			return K2_GetMaxNumberOfStacks(Container, Item);
		}
		return MAX_int32;
	}
	return 0;
}

int32 UCrimItemContainerRule::GetItemStackMaxQuantity(const UCrimItemContainer* Container, const UCrimItem* Item) const
{
	if (IsValid(Container) && IsValid(Item))
	{
		if (bHasGetItemStatckMaxQuantity)
		{
			return K2_GetItemStackMaxQuantity(Container, Item);
		}
		return MAX_int32;
	}
	return 0;
}
