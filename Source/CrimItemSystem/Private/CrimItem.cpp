// Copyright Soccertitan


#include "CrimItem.h"

#include "CrimItemDefinition.h"


FCrimItem::FCrimItem()
{
	ItemGuid = FGuid();
	Quantity = 0;
}

bool FCrimItem::IsMatching(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!TestItem.IsValid())
	{
		return false;
	}
	
	if (TestItem.GetPtr<FCrimItem>()->ItemDefinition != ItemDefinition)
	{
		return false;
	}

	if (TestItem.GetPtr<FCrimItem>()->TagStats != TagStats)
	{
		return false;
	}

	return true;
}

UCrimItemManagerComponent* FCrimItem::GetItemManager() const
{
	return ItemManager.Get();
}

UCrimItemContainer* FCrimItem::GetItemContainer() const
{
	return ItemContainer.Get();
}

void FCrimItem::Initialize(FGuid NewItemId, const UCrimItemDefinition* ItemDef)
{
	ItemGuid = NewItemId;
	ItemDefinition = ItemDef->GetPathName();
}

void FCrimItem::ApplyItemSpec(const TInstancedStruct<FCrimItemSpec>& Spec)
{
	const FCrimItemSpec* ConstSpec = Spec.GetPtr<FCrimItemSpec>();
	Quantity = ConstSpec->Quantity;
	Extensions = ConstSpec->Extensions;
	
	for (const TTuple<FGameplayTag, int>& Pair : ConstSpec->ItemStats)
	{
		TagStats.AddStack(Pair.Key, Pair.Value);
	}
}

void FCrimItem::ApplyItemDef(const UCrimItemDefinition* ItemDef)
{
	for (const TTuple<FGameplayTag, int>& Pair : ItemDef->DefaultStats)
	{
		TagStats.AddStack(Pair.Key, Pair.Value);
	}
}
