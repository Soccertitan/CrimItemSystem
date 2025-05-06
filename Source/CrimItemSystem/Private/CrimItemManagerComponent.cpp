// Copyright Soccertitan


#include "CrimItemManagerComponent.h"

#include "CrimItemContainer.h"
#include "CrimItemDefinition.h"
#include "CrimItemSettings.h"
#include "CrimItemSystem.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


UCrimItemManagerComponent::UCrimItemManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	StartupItemContainers.Add(UCrimItemSettings::GetDefaultContainerId(), UCrimItemSettings::GetDefaultItemContainerClass());
}

void UCrimItemManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheIsNetSimulated();
	InitializeStartupItemContainers();
}

void UCrimItemManagerComponent::PostInitProperties()
{
	Super::PostInitProperties();

	BindToItemContainerListDelegates();
}

void UCrimItemManagerComponent::PreNetReceive()
{
	Super::PreNetReceive();
	
	CacheIsNetSimulated();
}

void UCrimItemManagerComponent::OnRegister()
{
	Super::OnRegister();

	CacheIsNetSimulated();
}

void UCrimItemManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemContainerList, Params);
}

UCrimItemContainer* UCrimItemManagerComponent::GetItemContainer(FGameplayTag ContainerId) const
{
	if (ContainerId.IsValid())
	{
		for (const FFastCrimItemContainer& Entry : ItemContainerList.GetItemContainers())
		{
			if (Entry.GetItemContainer()->GetContainerId().MatchesTagExact(ContainerId))
			{
				return Entry.GetItemContainer();
			}
		}
	}
	return nullptr;
}

bool UCrimItemManagerComponent::HasItemContainer(const UCrimItemContainer* ItemContainer) const
{
	if (!IsValid(ItemContainer))
	{
		return false;
	}

	for (const FFastCrimItemContainer& Entry : GetItemContainers())
	{
		if (Entry.GetItemContainer() == ItemContainer)
		{
			return true;
		}
	}

	return false;
}

const TArray<FFastCrimItemContainer>& UCrimItemManagerComponent::GetItemContainers() const
{
	return ItemContainerList.GetItemContainers();
}

FFastCrimItem* UCrimItemManagerComponent::GetItemById(FGuid ItemId) const
{
	if (ItemId.IsValid())
	{
		for (const FFastCrimItemContainer& Entry : GetItemContainers())
		{
			FFastCrimItem* Item = Entry.GetItemContainer()->GetItemFromGuid(ItemId);
			if (Item != nullptr)
			{
				return Item;
			}
		}
	}
	return nullptr;
}

TInstancedStruct<FCrimItem> UCrimItemManagerComponent::K2_GetItemById(FGuid ItemId) const
{
	if (ItemId.IsValid())
	{
		for (const FFastCrimItemContainer& Entry : GetItemContainers())
		{
			TInstancedStruct<FCrimItem> Item = Entry.GetItemContainer()->K2_GetItemFromGuid(ItemId);
			if (Item.IsValid())
			{
				return Item;
			}
		}
	}
	return TInstancedStruct<FCrimItem>();
}

TArray<FFastCrimItem*> UCrimItemManagerComponent::GetItemsFromDefinition(const UCrimItemDefinition* ItemDef) const
{
	TArray<FFastCrimItem*> Result;

	if (ItemDef)
	{
		for (const FFastCrimItemContainer& Entry : GetItemContainers())
		{
			Result.Append(Entry.GetItemContainer()->GetItemsFromDefinition(ItemDef));
		}
	}
	return Result;
	
}
TArray<TInstancedStruct<FCrimItem>> UCrimItemManagerComponent::K2_GetItemsFromDefinition(const UCrimItemDefinition* ItemDef) const
{
	TArray<TInstancedStruct<FCrimItem>> Result;

	if (ItemDef)
	{
		for (const FFastCrimItemContainer& Entry : GetItemContainers())
		{
			Result.Append(Entry.GetItemContainer()->K2_GetItemsFromDefinition(ItemDef));
		}
	}
	return Result;
}

int32 UCrimItemManagerComponent::GetRemainingCollectionCapacityForItem(const UCrimItemDefinition* ItemDef) const
{
	if (!ItemDef)
	{
		return 0;
	}

	TArray<TInstancedStruct<FCrimItem>> MatchingItems;
	for (const FFastCrimItemContainer& Entry : GetItemContainers())
	{
		MatchingItems.Append(Entry.GetItemContainer()->K2_GetItemsFromDefinition(ItemDef));
	}

	return ItemDef->CollectionLimit.GetMaxQuantity() - MatchingItems.Num();
}

void UCrimItemManagerComponent::ConsumeItemsByDefinition(const UCrimItemDefinition* ItemDef, int32 Quantity)
{
	if (!HasAuthority() || !ItemDef || Quantity <= 0)
	{
		return;
	}

	int32 QuantityRemaining = Quantity;
	for (FFastCrimItem*& FastItem : GetItemsFromDefinition(ItemDef))
	{
		FCrimItem* Item = FastItem->Item.GetMutablePtr<FCrimItem>();
		const int32 NewQuantity = FMath::Max(Item->Quantity - QuantityRemaining, 0);
		QuantityRemaining = FMath::Max(QuantityRemaining - Item->Quantity, 0);
		Item->Quantity = NewQuantity;

		if (NewQuantity <= 0)
		{
			Item->GetItemContainer()->RemoveFromItemList(Item->GetItemGuid());
		}
		else
		{
			Item->GetItemContainer()->MarkItemDirty(*FastItem);	
		}

		if (QuantityRemaining == 0)
		{
			return;
		}
	}
}

UCrimItemContainer* UCrimItemManagerComponent::CreateItemContainer(FGameplayTag ContainerId,
	TSubclassOf<UCrimItemContainer> ItemContainerClass)
{
	if (!HasAuthority() || !ItemContainerClass || !ContainerId.IsValid())
	{
		return nullptr;
	}

	const FFastCrimItemContainer* ExistingContainer = ItemContainerList.GetItemContainers().FindByPredicate(
		[ContainerId](const FFastCrimItemContainer& Entry)
		{
			return Entry.GetItemContainer()->GetContainerId().MatchesTagExact(ContainerId);
		});
	if (ExistingContainer)
	{
		UE_LOG(LogCrimItemSystem, Verbose,
			   TEXT("CreateItemContainer already has %s as a ContainerId. Skip creating the container."),
			   *ContainerId.ToString());
		return nullptr;
	}

	UCrimItemContainer* NewContainer = NewObject<UCrimItemContainer>(this, ItemContainerClass);
	NewContainer->Initialize(this, ContainerId);
	AddReplicatedSubObject(NewContainer);
	ItemContainerList.AddItemContainer(NewContainer);
	return NewContainer;
}

void UCrimItemManagerComponent::DestroyItemContainer(UCrimItemContainer* ItemContainer)
{
	if (!HasItemContainer(ItemContainer))
	{
		return;
	}
	
	ItemContainer->ItemList.Reset();

	ItemContainerList.RemoveItemContainer(ItemContainer);
	RemoveReplicatedSubObject(ItemContainer);
	ItemContainer->MarkAsGarbage();
}

FCrimItemManagerSaveData UCrimItemManagerComponent::GetSaveData() const
{
	FCrimItemManagerSaveData SaveData;

	for (const FFastCrimItemContainer& Entry : GetItemContainers())
	{
		UCrimItemContainer* ItemContainer = Entry.GetItemContainer();
		if (IsValid(ItemContainer))
		{
			SaveData.ItemContainerSaveData.Add(ItemContainer);
		}
	}

	return SaveData;
}

void UCrimItemManagerComponent::LoadSavedData(const FCrimItemManagerSaveData& SaveData)
{
	if (!HasAuthority())
	{
		return;
	}
	
	//----------------------------------------------------------
	// 1. Remove all existing Items and ItemContainers
	//----------------------------------------------------------
	TArray<UCrimItemContainer*> ContainersToDestroy;
	for (const FFastCrimItemContainer& Container : ItemContainerList.GetItemContainers())
	{
		if (IsValid(Container.GetItemContainer()))
		{
			ContainersToDestroy.Add(Container.GetItemContainer());
		}
	}
	
	for (UCrimItemContainer* Container : ContainersToDestroy)
	{
		DestroyItemContainer(Container);
	}
	
	//----------------------------------------------------------
	// 2. Restore Containers and their Items
	//----------------------------------------------------------
	for (const FCrimItemContainerSaveData& ContainerData : SaveData.ItemContainerSaveData)
	{
		if (!ContainerData.ItemContainerClass.Get())
		{
			UAssetManager::Get().LoadAssetList({ContainerData.ItemContainerClass.ToSoftObjectPath()})->WaitUntilComplete();
		}
		
		UCrimItemContainer* NewContainer = CreateItemContainer(
			ContainerData.ContainerId,
			ContainerData.ItemContainerClass.Get()
		);
		
		if (NewContainer)
		{
			// Serialize ItemContainer properties
			FMemoryReader MemoryReader(ContainerData.ByteData);
			FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
			Archive.ArIsSaveGame = true;
			NewContainer->Serialize(Archive);
			
			for (const FCrimItemSaveData& ItemData : ContainerData.Items)
			{
				// Do not restore the item's data if the ItemDef is invalid.
				if (ItemData.ItemDef.IsNull())
				{
					continue;
				}
				
				TInstancedStruct<FCrimItem> NewItem;
				FMemoryReader MemReader(ItemData.ByteData);
				FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
				Ar.ArIsSaveGame = true;
				NewItem.Serialize(Ar);
				if (NewItem.IsValid())
				{
					NewContainer->AddItemToItemContainer(NewItem);
				}
			}
		}
	}
}

bool UCrimItemManagerComponent::HasAuthority() const
{
	return !bCachedIsNetSimulated;
}

void UCrimItemManagerComponent::CacheIsNetSimulated()
{
	bCachedIsNetSimulated = IsNetSimulating();
}

void UCrimItemManagerComponent::InitializeStartupItemContainers()
{
	if (!HasAuthority())
	{
		return;
	}

	for (const TTuple<FGameplayTag, TSubclassOf<UCrimItemContainer>>& Container : StartupItemContainers)
	{
		CreateItemContainer(Container.Key, Container.Value);
	}
}

void UCrimItemManagerComponent::BindToItemContainerListDelegates()
{
	if (!ItemContainerList.OnItemContainerAddedDelegate.IsBoundToObject(this))
	{
		ItemContainerList.OnItemContainerAddedDelegate.AddWeakLambda(this, [this](const FFastCrimItemContainer& Entry)
		{
			BindToItemContainerDelegates(Entry.GetItemContainer());
		});
	}
}

void UCrimItemManagerComponent::BindToItemContainerDelegates(UCrimItemContainer* Container)
{
	Container->OnItemAddedDelegate.AddWeakLambda(
		this, [this](UCrimItemContainer* ItemContainer, const FFastCrimItem& Item)
		{
			OnItemAddedDelegate.Broadcast(this, ItemContainer, Item);
		});
	Container->OnItemRemovedDelegate.AddWeakLambda(
		this, [this](UCrimItemContainer* ItemContainer, const FFastCrimItem& Item)
		{
			OnItemRemovedDelegate.Broadcast(this, ItemContainer, Item);
		});
	Container->OnItemChangedDelegate.AddWeakLambda(
		this, [this](UCrimItemContainer* ItemContainer, const FFastCrimItem& Item)
		{
			OnItemChangedDelegate.Broadcast(this, ItemContainer, Item);
		});
}
