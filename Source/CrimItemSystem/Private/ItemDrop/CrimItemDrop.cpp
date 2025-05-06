// Copyright Soccertitan


#include "ItemDrop/CrimItemDrop.h"

#include "CrimItem.h"
#include "CrimItemContainer.h"
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
	DOREPLIFETIME_WITH_PARAMS_FAST(ACrimItemDrop, ItemId, Params);
}

void ACrimItemDrop::TakeItem(UCrimItemContainer* ItemContainer)
{
	if (!HasAuthority() ||
		!ItemId.IsValid() ||
		!IsValid(ItemContainer) ||
		ItemContainer == ItemDropItemContainer)
	{
		// The existing ItemContainer can't take its own item.
		return;
	}

	FFastCrimItem* FastItem = ItemDropItemContainer->GetItemFromGuid(ItemId);
	if (FastItem == nullptr)
	{
		return;
	}

	FCrimItem* Item = FastItem->Item.GetMutablePtr<FCrimItem>();

	FCrimAddItemPlanResult Result = ItemContainer->TryAddItem(FastItem->Item, Item->Quantity);
	if (Result.IsValid())
	{
		Item->Quantity = Item->Quantity - Result.AmountGiven;
		ItemDropItemContainer->MarkItemDirty(*FastItem);
	}

	if (Item->Quantity <= 0)
	{
		ItemDropItemContainer->RemoveItem(ItemId);
		OnAllItemsTaken.Broadcast(this);
	}
}

TInstancedStruct<FCrimItem> ACrimItemDrop::GetItem() const
{
	return ItemDropItemContainer->K2_GetItemFromGuid(ItemId);
}

bool ACrimItemDrop::HasValidItem() const
{
	if (!ItemId.IsValid())
	{
		return false;
	}

	FFastCrimItem* FastItem = ItemDropItemContainer->GetItemFromGuid(ItemId);

	if (FastItem == nullptr)
	{
		return false;
	}

	return true;
}

UCrimItemContainer* ACrimItemDrop::GetItemContainer() const
{
	return ItemDropItemContainer;
}

void ACrimItemDrop::InitializeItemDrop(FGuid InItemId, UObject* Context)
{
	if (HasAuthority())
	{
		ItemId = InItemId;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemId, this);
		K2_InitializeItemDrop(ItemId, Context);
	}
}

bool ACrimItemDrop::CanTakeItem_Implementation(UCrimItemManagerComponent* ItemManager) const
{
	return HasValidItem();
}




