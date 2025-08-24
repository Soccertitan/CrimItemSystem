// Copyright Soccertitan


#include "ItemDrop/CrimItemDropManager.h"

#include "ItemContainer/CrimItemContainer.h"
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

void ACrimItemDropManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ItemContainer = ItemManagerComponent->CreateItemContainer(
			FCrimItemGameplayTags::Get().ItemContainer_ItemDropManager, UCrimItemContainerBase::StaticClass());
	}
}

ACrimItemDrop* ACrimItemDropManager::DropItem(const TInstancedStruct<FCrimItem>& Item, FCrimItemDropParams Params)
{
	if (!HasAuthority() || !Item.IsValid() || !Params.ItemDropClass)
	{
		return nullptr;
	}

	const FCrimItem* ItemPtr = Item.GetPtr<FCrimItem>();
	if (ItemPtr->GetItemContainer() == ItemContainer)
	{
		return nullptr;
	}

	if (ItemPtr->Quantity <= 0)
	{
		return nullptr;
	}

	ClearItemDrops();
	TInstancedStruct<FCrimItem> RemovedItem = ItemPtr->GetItemContainer()->RemoveItem(ItemPtr->GetItemGuid());
	if (!RemovedItem.IsValid())
	{
		return nullptr;
	}

	FCrimAddItemResult Result = ItemContainer->TryAddItem(RemovedItem);
	FGuid ItemGuid = Result.Items[0].GetPtr<FCrimItem>()->GetItemGuid();
	return CreateItemDrop(ItemGuid, Params);
}

void ACrimItemDropManager::ClearItemDrop(ACrimItemDrop* ItemDrop)
{
	ItemDrop->OnAllItemsTaken.RemoveAll(this);
	ItemDrops.Remove(ItemDrop);
	(void)ItemContainer->RemoveItem(ItemDrop->ItemGuid);
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

ACrimItemDrop* ACrimItemDropManager::CreateItemDrop(FGuid ItemId, FCrimItemDropParams& Params)
{
	FTransform Transform;
	Transform.SetLocation(Params.SpawnLocation);
	ACrimItemDrop* NewItemDrop = GetWorld()->SpawnActorDeferred<ACrimItemDrop>(
		Params.ItemDropClass,
		Transform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	NewItemDrop->ItemDropItemContainer = ItemContainer;
	NewItemDrop->InitializeItemDrop(ItemId, Params.Context);
	ItemDrops.Add(NewItemDrop);
	NewItemDrop->OnAllItemsTaken.AddUObject(this, &ACrimItemDropManager::OnItemDropTaken);
	UGameplayStatics::FinishSpawningActor(NewItemDrop, Transform);
	return NewItemDrop;
}
