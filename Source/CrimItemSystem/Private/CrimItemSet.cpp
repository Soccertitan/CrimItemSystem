// Copyright Soccertitan


#include "CrimItemSet.h"

#include "CrimItemDefinition.h"
#include "ItemContainer/CrimItemContainerBase.h"
#include "UObject/ObjectSaveContext.h"

const TInstancedStruct<FCrimItem>& FCrimItemInstance::GetItem() const
{
	return Item;
}

void FCrimItemInstance::TryCreateItem()
{
	if (ItemDefinition)
	{
		if (!Item.IsValid() ||
			Item.GetPtr<FCrimItem>()->GetItemDefinition() != ItemDefinition)
		{
			Item = UCrimItemContainerBase::CreateItem(ItemDefinition);
			ItemDefName = ItemDefinition->GetName();
			bShowItem = true;
		}
	}
	else
	{
		Item = TInstancedStruct<FCrimItem>();
		bShowItem = false;
	}
}

UCrimItemSet::UCrimItemSet()
{
}

FPrimaryAssetId UCrimItemSet::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("CrimItemSet", GetFName());
}

void UCrimItemSet::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(FCrimItemInstance, ItemDefinition))
	{
		for (FCrimItemInstance& ItemInstance : ItemInstances)
		{
			ItemInstance.TryCreateItem();
		}
	}
}

void UCrimItemSet::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	// Clean up invalid Items from the array.
	for (int32 idx = ItemInstances.Num() - 1; idx >= 0; idx--)
	{
		if (!ItemInstances[idx].Item.IsValid())
		{
			ItemInstances.RemoveAt(idx);
		}
	}
}
