// Copyright Soccertitan


#include "ItemDrop/CrimItemDrop.h"

#include "CrimItem.h"
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
	DOREPLIFETIME_WITH_PARAMS_FAST(ACrimItemDrop, Item, Params);
}

void ACrimItemDrop::TakeItem(UCrimItemManagerComponent* ItemManagerComponent, FGameplayTag ContainerId)
{
	if (!HasAuthority() ||
		!IsValid(Item) ||
		!IsValid(ItemManagerComponent) ||
		!ContainerId.IsValid() ||
		ItemManagerComponent == Item->GetItemManagerComponent())
	{
		// ItemManagerComponent can't be the same as the one that owns the item already.
		return;
	}

	FCrimItemAddPlan Result = ItemManagerComponent->TryAddItem(ContainerId, Item, Item->GetQuantity());
	if (Result.IsValid())
	{
		Item->SetQuantity(Item->GetQuantity() - Result.AmountGiven);
	}

	if (Item->GetQuantity() <= 0)
	{
		OnAllItemsTaken.Broadcast(this);
	}
}

UCrimItem* ACrimItemDrop::GetItem() const
{
	return Item;
}

bool ACrimItemDrop::HasValidItemInstance() const
{
	return IsValid(Item);
}

void ACrimItemDrop::InitializeItemDrop(UCrimItem* InItem, UObject* Context)
{
	if (IsValid(InItem) && HasAuthority())
	{
		Item = InItem;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Item, this);
		K2_InitializeItemDrop(InItem, Context);
	}
}

bool ACrimItemDrop::CanTakeItem_Implementation(UCrimItemManagerComponent* ItemManager) const
{
	return HasValidItemInstance();
}




