// Copyright Soccertitan


#include "UI/ViewModel/CrimItemViewModelBase.h"

#include "CrimItemDefinition.h"

UCrimItemViewModelBase::UCrimItemViewModelBase()
{
}

void UCrimItemViewModelBase::SetItem(const TInstancedStruct<FCrimItem>& InItem)
{
	checkf(InItem.IsValid(), TEXT("The passed in Item is invalid in %s"), *GetName());

	Item = InItem;
	const FCrimItem* ItemPtr = Item.GetPtr<FCrimItem>();
	ItemGuid = ItemPtr->GetItemGuid();
	OnItemSet();
}

const TInstancedStruct<FCrimItem>& UCrimItemViewModelBase::GetItem() const
{
	return Item;
}
