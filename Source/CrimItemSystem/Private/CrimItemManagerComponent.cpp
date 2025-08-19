// Copyright Soccertitan


#include "CrimItemManagerComponent.h"

#include "ItemContainer/CrimItemContainer.h"
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

	StartupItems.Add(UCrimItemSettings::GetDefaultContainerId(), FCrimStartupItems(UCrimItemSettings::GetDefaultItemContainerClass(), {}));
}

void UCrimItemManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	BindToItemContainerListDelegates();
	CacheIsNetSimulated();
	InitializeStartupItems();
}

void UCrimItemManagerComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// Actor components can't bind to delegates properly in PostInitProperties when creating this component in the
	// constructor on an actor using CreateDefaultSubobject. 
	// BindToItemContainerListDelegates();
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

UCrimItemContainerBase* UCrimItemManagerComponent::GetItemContainerByGuid(FGameplayTag ContainerGuid) const
{
	if (ContainerGuid.IsValid())
	{
		for (const FFastCrimItemContainerItem& Entry : ItemContainerList.GetItemContainers())
		{
			if (Entry.GetItemContainer()->GetContainerGuid().MatchesTagExact(ContainerGuid))
			{
				return Entry.GetItemContainer();
			}
		}
	}
	return nullptr;
}

bool UCrimItemManagerComponent::HasItemContainer(const UCrimItemContainerBase* ItemContainer) const
{
	if (!IsValid(ItemContainer))
	{
		return false;
	}

	for (const FFastCrimItemContainerItem& Entry : GetItemContainers())
	{
		if (Entry.GetItemContainer() == ItemContainer)
		{
			return true;
		}
	}

	return false;
}

const TArray<FFastCrimItemContainerItem>& UCrimItemManagerComponent::GetItemContainers() const
{
	return ItemContainerList.GetItemContainers();
}

FFastCrimItem* UCrimItemManagerComponent::GetItemByGuid(FGuid ItemGuid) const
{
	if (ItemGuid.IsValid())
	{
		for (const FFastCrimItemContainerItem& Entry : GetItemContainers())
		{
			FFastCrimItem* Item = Entry.GetItemContainer()->GetItemByGuid(ItemGuid);
			if (Item != nullptr)
			{
				return Item;
			}
		}
	}
	return nullptr;
}

TInstancedStruct<FCrimItem> UCrimItemManagerComponent::K2_GetItemByGuid(FGuid ItemGuid) const
{
	if (ItemGuid.IsValid())
	{
		for (const FFastCrimItemContainerItem& Entry : GetItemContainers())
		{
			TInstancedStruct<FCrimItem> Item = Entry.GetItemContainer()->K2_GetItemByGuid(ItemGuid);
			if (Item.IsValid())
			{
				return Item;
			}
		}
	}
	return TInstancedStruct<FCrimItem>();
}

TArray<FFastCrimItem*> UCrimItemManagerComponent::GetItemsByDefinition(const UCrimItemDefinition* ItemDefinition) const
{
	TArray<FFastCrimItem*> Result;

	if (ItemDefinition)
	{
		for (const FFastCrimItemContainerItem& Entry : GetItemContainers())
		{
			Result.Append(Entry.GetItemContainer()->GetItemsByDefinition(ItemDefinition));
		}
	}
	return Result;
	
}
TArray<TInstancedStruct<FCrimItem>> UCrimItemManagerComponent::K2_GetItemsByDefinition(const UCrimItemDefinition* ItemDefinition) const
{
	TArray<TInstancedStruct<FCrimItem>> Result;

	if (ItemDefinition)
	{
		for (const FFastCrimItemContainerItem& Entry : GetItemContainers())
		{
			Result.Append(Entry.GetItemContainer()->K2_GetItemsByDefinition(ItemDefinition));
		}
	}
	return Result;
}

void UCrimItemManagerComponent::ConsumeItemsByDefinition(const UCrimItemDefinition* ItemDefinition, int32 Quantity)
{
	if (!HasAuthority() || !ItemDefinition || Quantity <= 0)
	{
		return;
	}

	int32 QuantityRemaining = Quantity;
	for (FFastCrimItem*& FastItem : GetItemsByDefinition(ItemDefinition))
	{
		FCrimItem* Item = FastItem->Item.GetMutablePtr<FCrimItem>();
		const int32 NewQuantity = FMath::Max(Item->Quantity - QuantityRemaining, 0);
		QuantityRemaining = FMath::Max(QuantityRemaining - Item->Quantity, 0);
		Item->Quantity = NewQuantity;

		if (NewQuantity <= 0)
		{
			Item->GetItemContainer()->Internal_RemoveItem(Item->GetItemGuid());
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

UCrimItemContainerBase* UCrimItemManagerComponent::CreateItemContainer(FGameplayTag ContainerGuid,
	TSubclassOf<UCrimItemContainerBase> ItemContainerClass)
{
	if (!HasAuthority() || !ItemContainerClass)
	{
		return nullptr;
	}

	if (ContainerGuid.IsValid())
	{
		const FFastCrimItemContainerItem* ExistingContainer = ItemContainerList.GetItemContainers().FindByPredicate(
		   [ContainerGuid](const FFastCrimItemContainerItem& Entry)
		   {
			   return Entry.GetItemContainer()->GetContainerGuid().MatchesTagExact(ContainerGuid);
		   });
		if (ExistingContainer)
		{
			UE_LOG(LogCrimItemSystem, Verbose,
				   TEXT("CreateItemContainer already has %s as a ContainerId. Skip creating the container."),
				   *ContainerGuid.ToString());
			return nullptr;
		}
	}

	UCrimItemContainerBase* NewContainer = NewObject<UCrimItemContainerBase>(this, ItemContainerClass);
	NewContainer->Initialize(this, ContainerGuid);
	AddReplicatedSubObject(NewContainer);
	ItemContainerList.AddItemContainer(NewContainer);
	return NewContainer;
}

void UCrimItemManagerComponent::RemoveItemContainer(UCrimItemContainerBase* ItemContainer)
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

	for (const FFastCrimItemContainerItem& Entry : GetItemContainers())
	{
		UCrimItemContainerBase* ItemContainer = Entry.GetItemContainer();
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
	TArray<UCrimItemContainerBase*> ContainersToDestroy;
	for (const FFastCrimItemContainerItem& Container : ItemContainerList.GetItemContainers())
	{
		if (IsValid(Container.GetItemContainer()))
		{
			ContainersToDestroy.Add(Container.GetItemContainer());
		}
	}
	
	for (UCrimItemContainerBase* Container : ContainersToDestroy)
	{
		RemoveItemContainer(Container);
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
		
		UCrimItemContainerBase* NewContainer = CreateItemContainer(
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
				
				// Do not restore the item if it's been depreciated.
				if (!ItemData.ItemDef.Get())
				{
					UAssetManager::Get().LoadAssetList({ItemData.ItemDef.ToSoftObjectPath()})->WaitUntilComplete();
				}
				if (!ItemData.ItemDef.Get()->bSpawnable)
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
					NewContainer->Internal_AddItem(NewItem);
				}
			}
		}
	}
}

bool UCrimItemManagerComponent::HasAuthority() const
{
	return !bCachedIsNetSimulated;
}

void UCrimItemManagerComponent::OnItemContainerAdded(const FFastCrimItemContainerItem& Entry)
{
	OnItemContainerAddedDelegate.Broadcast(this, Entry.GetItemContainer());
	BindToItemContainerDelegates(Entry.GetItemContainer());
}

void UCrimItemManagerComponent::OnItemContainerRemoved(const FFastCrimItemContainerItem& Entry)
{
	OnItemContainerRemovedDelegate.Broadcast(this, Entry.GetItemContainer());
}

void UCrimItemManagerComponent::OnItemAdded(UCrimItemContainerBase* ItemContainer, const FFastCrimItem& Item)
{
	OnItemAddedDelegate.Broadcast(this, ItemContainer, Item);
}

void UCrimItemManagerComponent::OnItemRemoved(UCrimItemContainerBase* ItemContainer, const FFastCrimItem& Item)
{
	OnItemRemovedDelegate.Broadcast(this, ItemContainer, Item);
}

void UCrimItemManagerComponent::OnItemChanged(UCrimItemContainerBase* ItemContainer, const FFastCrimItem& Item)
{
	OnItemChangedDelegate.Broadcast(this, ItemContainer, Item);
}

void UCrimItemManagerComponent::CacheIsNetSimulated()
{
	bCachedIsNetSimulated = IsNetSimulating();
}

void UCrimItemManagerComponent::InitializeStartupItems()
{
	if (!HasAuthority())
	{
		return;
	}

	for (const TTuple<FGameplayTag, FCrimStartupItems>& Startup : StartupItems)
	{
		UCrimItemContainerBase* ItemContainer = CreateItemContainer(Startup.Key, Startup.Value.ItemContainerClass);
		if (ItemContainer)
		{
			for (const TInstancedStruct<FCrimItem>& Item : Startup.Value.Items)
			{
				ItemContainer->TryAddItem(Item);
			}
		}
	}
}

void UCrimItemManagerComponent::BindToItemContainerListDelegates()
{
	ItemContainerList.OnItemContainerAddedDelegate.AddUObject(this, &UCrimItemManagerComponent::OnItemContainerAdded);
	ItemContainerList.OnItemContainerRemovedDelegate.AddUObject(this, &UCrimItemManagerComponent::OnItemContainerRemoved);
}

void UCrimItemManagerComponent::BindToItemContainerDelegates(UCrimItemContainerBase* Container)
{
	Container->OnItemAddedDelegate.AddUObject(this, &UCrimItemManagerComponent::OnItemAdded);
	Container->OnItemRemovedDelegate.AddUObject(this, &UCrimItemManagerComponent::OnItemRemoved);
	Container->OnItemChangedDelegate.AddUObject(this, &UCrimItemManagerComponent::OnItemChanged);
}
