// Copyright Soccertitan


#include "ItemDrop/CrimItemDrop.h"

#include "CrimItem.h"
#include "ItemContainer/CrimItemContainerBase.h"
#include "CrimItemManagerComponent.h"
#include "Net/UnrealNetwork.h"


ACrimItemDrop::ACrimItemDrop()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetNetUpdateFrequency(1.f);
}

void ACrimItemDrop::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ACrimItemDrop, ItemGuid, Params);
}

void ACrimItemDrop::TakeItem(UCrimItemContainerBase* ItemContainer)
{
	if (!HasAuthority() ||
		!ItemGuid.IsValid() ||
		!IsValid(ItemContainer) ||
		ItemContainer == ItemDropItemContainer)
	{
		// The existing ItemContainer can't take its own item.
		return;
	}

	FFastCrimItem* FastItem = ItemDropItemContainer->GetItemByGuid(ItemGuid);
	if (FastItem == nullptr)
	{
		return;
	}

	FCrimItem* Item = FastItem->Item.GetMutablePtr<FCrimItem>();

	FCrimAddItemResult Result = ItemContainer->TryAddItem(FastItem->Item);
	if (Result.AmountGiven >= 0)
	{
		Item->Quantity = Item->Quantity - Result.AmountGiven;
		ItemDropItemContainer->MarkItemDirty(*FastItem);
	}

	if (Item->Quantity <= 0)
	{
		ItemDropItemContainer->RemoveItem(ItemGuid);
		OnAllItemsTaken.Broadcast(this);
	}
}

TInstancedStruct<FCrimItem> ACrimItemDrop::GetItem() const
{
	return ItemDropItemContainer->K2_GetItemByGuid(ItemGuid);
}

bool ACrimItemDrop::HasValidItem() const
{
	if (!ItemGuid.IsValid())
	{
		return false;
	}

	FFastCrimItem* FastItem = ItemDropItemContainer->GetItemByGuid(ItemGuid);

	if (FastItem == nullptr)
	{
		return false;
	}

	return true;
}

UCrimItemContainerBase* ACrimItemDrop::GetItemContainer() const
{
	return ItemDropItemContainer;
}

void ACrimItemDrop::InitializeItemDrop(FGuid InItemGuid, UObject* Context)
{
	if (HasAuthority())
	{
		ItemGuid = InItemGuid;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemGuid, this);
		K2_InitializeItemDrop(ItemGuid, Context);
	}
}

bool ACrimItemDrop::CanTakeItem_Implementation(UCrimItemManagerComponent* ItemManager) const
{
	return HasValidItem();
}




