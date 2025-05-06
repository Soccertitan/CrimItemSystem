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

void ACrimItemDropManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ItemContainer = ItemManagerComponent->CreateItemContainer(
			FCrimItemGameplayTags::Get().Crim_ItemContainer_ItemDropManager, UCrimItemContainer::StaticClass());
	}
}

ACrimItemDrop* ACrimItemDropManager::DropItem(UCrimItemContainer* SourceContainer, const FGuid& SourceItemId,
	int32 Quantity, FCrimItemDropParams Params)
{
	if (!HasAuthority() ||
		!IsValid(SourceContainer) ||
		Quantity <= 0 ||
		!Params.ItemDropClass ||
		SourceContainer == ItemContainer)
	{
		return nullptr;
	}

	FFastCrimItem* SourceFastItem = SourceContainer->GetItemFromGuid(SourceItemId);
	if (SourceFastItem == nullptr)
	{
		return nullptr;
	}

	ClearItemDrops();

	int32 ActualQuantityToDrop = FMath::Min(SourceFastItem->Item.GetPtr<FCrimItem>()->Quantity, Quantity);
	TInstancedStruct<FCrimItem> NewItem = ItemContainer->AddItem(SourceFastItem->Item, ActualQuantityToDrop);
	SourceContainer->ConsumeItem(SourceItemId, ActualQuantityToDrop);

	return CreateItemDrop(NewItem.GetPtr<FCrimItem>()->GetItemGuid(), Params);
}

ACrimItemDrop* ACrimItemDropManager::DropItemBySpec(TInstancedStruct<FCrimItemSpec> ItemSpec, FCrimItemDropParams Params)
{
	if (!HasAuthority() || !ItemSpec.IsValid() || !Params.ItemDropClass)
	{
		return nullptr;
	}

	TInstancedStruct<FCrimItem> NewItem = ItemContainer->AddItemFromSpec(ItemSpec);
	if (!NewItem.IsValid())
	{
		return nullptr;
	}
	
	ClearItemDrops();
	return CreateItemDrop(NewItem.GetPtr<FCrimItem>()->GetItemGuid(), Params);
}

void ACrimItemDropManager::ClearItemDrop(ACrimItemDrop* ItemDrop)
{
	ItemDrop->OnAllItemsTaken.RemoveAll(this);
	ItemDrops.Remove(ItemDrop);
	ItemContainer->RemoveItem(ItemDrop->ItemId);
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
