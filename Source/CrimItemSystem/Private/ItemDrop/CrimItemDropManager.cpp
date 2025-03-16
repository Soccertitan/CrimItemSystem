// Copyright Soccertitan


#include "ItemDrop/CrimItemDropManager.h"

#include "CrimItemContainer.h"
#include "CrimItemGameplayTags.h"
#include "CrimItemManagerComponent.h"
#include "ItemDrop/CrimItemDrop.h"
#include "Kismet/GameplayStatics.h"


ACrimItemDropManager::ACrimItemDropManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	ItemManagerComponent = CreateDefaultSubobject<UCrimItemManagerComponent>(TEXT("ItemManagerComponent"));
}

ACrimItemDrop* ACrimItemDropManager::DropItem(UCrimItem* ItemToDrop, int32 Quantity, FCrimItemDropParams Params)
{
	if (!HasAuthority() || !IsValid(ItemToDrop) || Quantity <= 0 || !Params.ItemDropClass)
	{
		return nullptr;
	}

	if (ItemToDrop->GetItemManagerComponent() == ItemManagerComponent)
	{
		// Can't drop items already in the ItemManagerComponent.
		return nullptr;
	}

	ClearItemDrops();

	int32 ActualQuantityToDrop = FMath::Min(ItemToDrop->GetQuantity(), Quantity);
	UCrimItem* NewItem = ItemManagerComponent->AddItem(ItemContainerId, ItemToDrop, ActualQuantityToDrop);
	ItemToDrop->GetItemManagerComponent()->ConsumeItem(ItemToDrop, ActualQuantityToDrop);

	ACrimItemDrop* NewItemDrop = CreateItemDrop(NewItem, Params);
	return NewItemDrop;
}

ACrimItemDrop* ACrimItemDropManager::DropItemBySpec(TInstancedStruct<FCrimItemSpec> ItemSpec, FCrimItemDropParams Params)
{
	if (!HasAuthority() || !ItemSpec.IsValid() || !Params.ItemDropClass)
	{
		return nullptr;
	}

	UCrimItem* NewItem = ItemManagerComponent->AddItemFromSpec(ItemContainerId, ItemSpec);
	if (!NewItem)
	{
		return nullptr;
	}

	ClearItemDrops();
	ACrimItemDrop* NewItemDrop = CreateItemDrop(NewItem, Params);
	return NewItemDrop;
}

void ACrimItemDropManager::BeginPlay()
{
	Super::BeginPlay();

	ItemContainerId = FCrimItemGameplayTags::Get().Crim_ItemContainer_ItemDropManager;
	ItemManagerComponent->CreateItemContainer(ItemContainerId, UCrimItemContainer::StaticClass());
}

void ACrimItemDropManager::ClearItemDrop(ACrimItemDrop* ItemDrop)
{
	ItemDrop->OnAllItemsTaken.RemoveAll(this);
	ItemDrops.Remove(ItemDrop);
	ItemManagerComponent->ReleaseItem(ItemDrop->GetItem());
	ItemDrop->Destroy();
}

void ACrimItemDropManager::OnItemDropTaken(ACrimItemDrop* ItemDrop)
{
	ClearItemDrop(ItemDrop);
}

void ACrimItemDropManager::ClearItemDrops()
{
	while (ItemDrops.Num() >= MaxItemDrops)
	{
		if (IsValid(ItemDrops[0]))
		{
			ClearItemDrop(ItemDrops[0]);
		}
	}
}

ACrimItemDrop* ACrimItemDropManager::CreateItemDrop(UCrimItem* Item, FCrimItemDropParams& Params)
{
	FTransform Transform;
	Transform.SetLocation(Params.SpawnLocation);
	ACrimItemDrop* NewItemDrop = GetWorld()->SpawnActorDeferred<ACrimItemDrop>(
		Params.ItemDropClass,
		Transform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	NewItemDrop->ItemDropManager = this;
	NewItemDrop->InitializeItemDrop(Item, Params.Context);
	ItemDrops.Add(NewItemDrop);
	NewItemDrop->OnAllItemsTaken.AddUObject(this, &ACrimItemDropManager::OnItemDropTaken);
	UGameplayStatics::FinishSpawningActor(NewItemDrop, Transform);
	return NewItemDrop;
}
