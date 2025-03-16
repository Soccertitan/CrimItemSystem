// Copyright Soccertitan


#include "CrimItemManagerComponent.h"

#include "CrimItemContainer.h"
#include "CrimItemDef.h"
#include "CrimItemGameplayTags.h"
#include "CrimItemSystem.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


UCrimItemManagerComponent::UCrimItemManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
	
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UCrimItemManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	const UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		return;
	}

	CacheIsNetSimulated();
	InitializeStartupItemContainers();
}

void UCrimItemManagerComponent::PostInitProperties()
{
	Super::PostInitProperties();

	BindToItemContainerListDelegates();
}

void UCrimItemManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemContainerList);
}

void UCrimItemManagerComponent::PreNetReceive()
{
	Super::PreNetReceive();

	// Update the cached IsNetSimulated value here if this component is still considered authority.
	// Even though the value is also cached in OnRegister and BeginPlay, clients may
	// receive properties before OnBeginPlay, so this ensures the role is correct for that case.
	if (!bCachedIsNetSimulated)
	{
		CacheIsNetSimulated();
	}
}

void UCrimItemManagerComponent::OnRegister()
{
	Super::OnRegister();

	// Cached off net role to avoid constant checking on owning actor.
	CacheIsNetSimulated();
}

void UCrimItemManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheIsNetSimulated();
}

void UCrimItemManagerComponent::InitializeStartupItemContainers()
{
	if (!HasAuthority())
	{
		return;
	}

	for (const TTuple<FGameplayTag, TSubclassOf<UCrimItemContainer>>& Container : StartupParentItemContainers)
	{
		CreateItemContainer(Container.Key, Container.Value, true);
	}

	for (const TTuple<FGameplayTag, TSubclassOf<UCrimItemContainer>>& Container : StartupChildItemContainers)
	{
		CreateItemContainer(Container.Key, Container.Value, false);
	}

	for (const FCrimItemContainerLinkSpec& Spec : StartupItemContainerLinkSpecs)
	{
		if (UCrimItemContainer* ParentContainer = GetItemContainer(Spec.ParentContainerId))
		{
			for (const FGameplayTag& ChildId : Spec.ChildContainerIds)
			{
				if (UCrimItemContainer* ChildContainer = GetItemContainer(ChildId))
				{
					ParentContainer->LinkContainers(ChildContainer);
					ChildContainer->LinkContainers(ParentContainer);
				}
			}
		}
	}
}

UCrimItemContainer* UCrimItemManagerComponent::CreateItemContainer(FGameplayTag ContainerId,
	TSubclassOf<UCrimItemContainer> ItemContainerClass, bool bIsParent)
{
	if (!HasAuthority() || !ItemContainerClass || !ContainerId.IsValid())
	{
		return nullptr;
	}

	const FCrimItemContainerListEntry* ExistingContainer = ItemContainerList.GetItemContainers().FindByPredicate([ContainerId](const FCrimItemContainerListEntry& Entry)
	{
		return Entry.GetItemContainer()->GetContainerId().MatchesTagExact(ContainerId);
	});
	if (ExistingContainer)
	{
		UE_LOG(LogCrimItemSystem, Verbose, TEXT("CreateItemContainer already has %s as a ContainerId. Skip creating the container."), *ContainerId.ToString());
		return nullptr;
	}

	UCrimItemContainer* NewContainer = NewObject<UCrimItemContainer>(this, ItemContainerClass);
	NewContainer->Initialize(this, ContainerId, bIsParent);
	AddReplicatedSubObject(NewContainer);
	ItemContainerList.AddEntry(NewContainer);
	return NewContainer;
}

void UCrimItemManagerComponent::DestroyItemContainerById(FGameplayTag ContainerId)
{
	if (!HasAuthority() || !ContainerId.IsValid())
	{
		return;
	}
	
	UCrimItemContainer* TargetContainer = GetItemContainer(ContainerId);
	if (!IsValid(TargetContainer))
	{
		return;
	}

	DestroyItemContainer(TargetContainer);
}

void UCrimItemManagerComponent::DestroyItemContainer(UCrimItemContainer* ItemContainer)
{
	if (ItemContainer->IsParentContainer())
	{
		// Parent container is responsible for holding instances of items. Get all the items and store them for reference.
		TArray<UCrimItem*> OldItems;
		for (const FCrimItemListEntry& Entry : ItemContainer->GetItems())
		{
			if (IsValid(Entry.GetItem()))
			{
				OldItems.Add(Entry.GetItem());
			}
		}
		// Remove all items in the list.
		ItemContainer->ItemList.Reset();
		
		for (UCrimItem* Item : OldItems)
		{
			// Remove all the old items that our children reference.
			for (UCrimItemContainer* Child : ItemContainer->LinkedContainers)
			{
				Child->ItemList.RemoveEntry(Item, true);
			}
			DestroyItem(Item);
		}
	}
	else
	{
		// Child container, we just need to remove the references to items. The parent is responsible for holding items.
		ItemContainer->ItemList.Reset();
	}
	
	TArray<UCrimItemContainer*> LinkedContainers = ItemContainer->GetLinkedContainers();
	for (UCrimItemContainer* Link : LinkedContainers)
	{
		ItemContainer->UnlinkContainers(Link);
	}
	ItemContainerList.RemoveEntry(ItemContainer);
	RemoveReplicatedSubObject(ItemContainer);
	ItemContainer->MarkAsGarbage();
}

void UCrimItemManagerComponent::LinkItemContainers(FGameplayTag ParentContainerId, FGameplayTag ChildContainerId)
{
	if (!HasAuthority() || !ParentContainerId.IsValid() || !ChildContainerId.IsValid())
	{
		return;
	}

	UCrimItemContainer* ParentContainer = GetItemContainer(ParentContainerId);
	UCrimItemContainer* ChildContainer = GetItemContainer(ChildContainerId);
	if (!ParentContainer || !ChildContainer)
	{
		return;
	}
	ParentContainer->LinkContainers(ChildContainer);
}

UCrimItemContainer* UCrimItemManagerComponent::GetItemContainer(FGameplayTag ContainerId) const
{
	if (ContainerId.IsValid())
	{
		for  (const FCrimItemContainerListEntry& Entry : ItemContainerList.GetItemContainers())
		{
			if (Entry.GetItemContainer()->GetContainerId().MatchesTagExact(ContainerId))
			{
				return Entry.GetItemContainer();
			}
		}
	}
	return nullptr;
}

const TArray<FCrimItemContainerListEntry>& UCrimItemManagerComponent::GetItemContainers() const
{
	return ItemContainerList.GetItemContainers();
}

TArray<UCrimItemContainer*> UCrimItemManagerComponent::GetItemContainersWithItem(UCrimItem* Item) const
{
	TArray<UCrimItemContainer*> Result;

	if (IsValid(Item))
	{
		for (const FCrimItemContainerListEntry& Entry : ItemContainerList.GetItemContainers())
		{
			if (Entry.GetItemContainer()->HasItem(Item))
			{
				Result.Add(Entry.GetItemContainer());
			}
		}
	}
	return Result;
}

TArray<UCrimItem*> UCrimItemManagerComponent::GetItemsByDefinition(const UCrimItemDef* ItemDef) const
{
	TArray<UCrimItem*> Result;

	if (IsValid(ItemDef))
	{
		for (const FCrimItemContainerListEntry& Entry : ItemContainerList.GetItemContainers())
		{
			if (Entry.GetItemContainer()->IsChildContainer())
			{
				continue;
			}
			
			for (const FCrimItemListEntry& Item : Entry.GetItemContainer()->GetItems())
			{
				if (IsValid(Item.GetItem()) &&
					Item.GetItem()->GetItemDefinition() == ItemDef)
				{
					Result.Add(Item.GetItem());
				}
			}
		}
	}
	
	return Result;
}

UCrimItem* UCrimItemManagerComponent::GetItemById(FGuid ItemId) const
{
	UCrimItem* Result = nullptr;
	if (ItemId.IsValid())
	{
		for (const FCrimItemContainerListEntry& Entry : ItemContainerList.GetItemContainers())
		{
			Result = Entry.GetItemContainer()->GetItemFromId(ItemId);
			if (IsValid(Result))
			{
				break;
			}
		}
	}
	return Result;
}

int32 UCrimItemManagerComponent::GetRemainingCollectionCapacityForItem(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return 0;
	}

	TArray<UCrimItem*> MatchingItems;
	for (const FCrimItemContainerListEntry& Entry : ItemContainerList.GetItemContainers())
	{
		// Only count parent containers towards remaining capacity.
		if (Entry.GetItemContainer()->IsParentContainer())
		{
			MatchingItems.Append(Entry.GetItemContainer()->GetMatchingItems(TestItem));
		}
	}
	
	return TestItem->GetItemDefinition()->CollectionLimit.GetMaxQuantity() - MatchingItems.Num();
}

bool UCrimItemManagerComponent::CanSplitItemStack(FGameplayTag ContainerId, const UCrimItem* TestItem, int32& Slot) const
{
	if (!ContainerId.IsValid() || !IsValid(TestItem))
	{
		return false;
	}
	
	if (TestItem->GetQuantity() <= 1)
	{
		return false;
	}
	
	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	if (!IsValid(ItemContainer) || ItemContainer->IsChildContainer())
	{
		return false;
	}

	if (!ItemContainer->HasItem(TestItem))
	{
		return false;
	}

	//--------------------------------------------------------
	// Check if at max capacity for item.
	//--------------------------------------------------------
	if (GetRemainingCollectionCapacityForItem(TestItem) <= 0)
	{
		return false;
	}

	if (ItemContainer->IsAtMaxCapacity() || ItemContainer->IsItemAtContainerLimit(TestItem))
	{
		return false;
	}

	//--------------------------------------------------------
	// Checking for valid slot to place the item. The slot must be empty.
	//--------------------------------------------------------
	if (Slot < 0)
	{
		TArray<int32> ValidSlots = ItemContainer->GetValidSlots(TestItem);
		if (ValidSlots.IsEmpty())
		{
			Slot = ItemContainer->GetNextEmptySlot(0);
		}
		else
		{
			for (int32& ValidSlot : ValidSlots)
			{
				if (!ItemContainer->GetItemInSlot(ValidSlot))
				{
					Slot = ValidSlot;
					break;
				}
			}
			if (Slot < 0)
			{
				return false;
			}
		}
	}
	else
	{
		if (IsValid(ItemContainer->GetItemInSlot(Slot)))
		{
			// The item is valid, can't place in an existing slot.
			return false;
		}

		if (!ItemContainer->IsValidSlotForItem(TestItem, Slot))
		{
			return false;
		}
	}
	
	return true;
}

bool UCrimItemManagerComponent::CanStackItems(FGameplayTag ContainerId, int32 SourceSlot, int32 TargetSlot, int32& OutMaxQuantity) const
{
	OutMaxQuantity = 0;
	if (!ContainerId.IsValid() || SourceSlot < 0 || TargetSlot < 0)
	{
		return false;
	}

	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	if (!IsValid(ItemContainer) || ItemContainer->IsChildContainer())
	{
		return false;
	}

	UCrimItem* SourceItem = ItemContainer->GetItemInSlot(SourceSlot);
	UCrimItem* TargetItem = ItemContainer->GetItemInSlot(TargetSlot);
	if (!IsValid(SourceItem) || !IsValid(TargetItem))
	{
		return false;
	}

	int32 MaxQuantity = ItemContainer->GetItemQuantityLimit(SourceItem);
	if (MaxQuantity <= 1)
	{
		return false;
	}

	int32 TargetAvailableSpace = MaxQuantity - TargetItem->GetQuantity();
	OutMaxQuantity = FMath::Min(TargetAvailableSpace, SourceItem->GetQuantity());
	return OutMaxQuantity > 0;
}

bool UCrimItemManagerComponent::CanAssignItemToContainer(FGameplayTag ParentContainerId, int32 ParentSlot,
	FGameplayTag ChildContainerId, int32& TargetSlot) const
{
	if (!ParentContainerId.IsValid() || !ChildContainerId.IsValid() || ParentSlot < 0)
	{
		return false;
	}

	if (ParentContainerId.MatchesTagExact(ChildContainerId))
	{
		return false;
	}

	UCrimItemContainer* ParentContainer = GetItemContainer(ParentContainerId);
	UCrimItemContainer* ChildContainer = GetItemContainer(ChildContainerId);

	if (!IsValid(ParentContainer) || !IsValid(ChildContainer))
	{
		return false;
	}

	if (ParentContainer->IsChildContainer() || ChildContainer->IsParentContainer())
	{
		return false;
	}

	if (!ParentContainer->LinkedContainers.Contains(ChildContainer))
	{
		return false;
	}

	UCrimItem* Item = ParentContainer->GetItemInSlot(ParentSlot);
	if (!IsValid(Item))
	{
		return false;
	}
	
	UCrimItem* TargetSlotItem = ChildContainer->GetItemInSlot(TargetSlot);
	if (Item == TargetSlotItem)
	{
		return false;
	}

	if (!ChildContainer->CanContainItem(Item))
	{
		return false;
	}

	if (TargetSlot >= 0)
	{
		if (!ChildContainer->IsValidSlotForItem(Item, TargetSlot))
		{
			return false;
		}
	}
	else
	{
		TArray<int32> ValidSlots = ChildContainer->GetValidSlots(Item);
		if (ValidSlots.IsEmpty())
		{
			TargetSlot = 0;
		}
		else
		{
			TargetSlot = ValidSlots[0];
		}	
	}

	return true;
}

void UCrimItemManagerComponent::SwapItemsBySlot(FGameplayTag ContainerId, int32 FirstSlot, int32 SecondSlot)
{
	if (!ContainerId.IsValid() || FirstSlot < 0 || SecondSlot < 0)
	{
		return;
	}

	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	
	if (!IsValid(ItemContainer))
	{
		return;
	}

	UCrimItem* FirstItem = ItemContainer->GetItemInSlot(FirstSlot);
	UCrimItem* SecondItem = ItemContainer->GetItemInSlot(SecondSlot);

	if (!FirstItem && !SecondItem)
	{
		// Both items are null.
		return;
	}

	if (!ItemContainer->IsValidSlot(FirstSlot) || !ItemContainer->IsValidSlot(SecondSlot))
	{
		return;
	}

	if (!HasAuthority())
	{
		Server_SwapItemsBySlot(ContainerId, FirstSlot, SecondSlot);
		return;
	}
	
	if (FirstItem && SecondItem)
	{
		// Both items are valid so we try to swap them.
		const bool bFirstItemCanSwap = ItemContainer->IsValidSlotForItem(FirstItem, SecondSlot);
		const bool bSecondItemCanSwap = ItemContainer->IsValidSlotForItem(SecondItem, FirstSlot);
		if (bFirstItemCanSwap && bSecondItemCanSwap)
		{
			ItemContainer->ItemList.RemoveEntryAt(FirstSlot);
			ItemContainer->ItemList.RemoveEntryAt(SecondSlot);
			ItemContainer->ItemList.AddEntryAt(SecondItem, FirstSlot);
			ItemContainer->ItemList.AddEntryAt(FirstItem, SecondSlot);
		}
	}
	else if (FirstItem)
	{
		if (ItemContainer->IsValidSlotForItem(FirstItem, SecondSlot))
		{
			ItemContainer->ItemList.RemoveEntryAt(FirstSlot);
			ItemContainer->ItemList.AddEntryAt(FirstItem, SecondSlot);
		}
	}
	else if (SecondItem)
	{
		if (ItemContainer->IsValidSlotForItem(SecondItem, SecondSlot))
		{
			ItemContainer->ItemList.RemoveEntryAt(SecondSlot);
			ItemContainer->ItemList.AddEntryAt(SecondItem, FirstSlot);
		}
	}
}

void UCrimItemManagerComponent::MoveItemToContainer(FGameplayTag SourceContainerId, int32 SourceSlot,
	FGameplayTag TargetContainerId, int32 TargetSlot, int32 Quantity)
{
	if (!SourceContainerId.IsValid() || !TargetContainerId.IsValid() || SourceSlot < 0 || Quantity < 1)
	{
		return;
	}
	
	if (SourceContainerId.MatchesTagExact(TargetContainerId))
	{
		UE_LOG(LogCrimItemSystem, Verbose, TEXT("Can't move item between the same item container. Use SwapItemsBySlot instead."));
		return;
	}

	if (!HasAuthority())
	{
		Server_MoveItemToContainer(SourceContainerId, SourceSlot, TargetContainerId, TargetSlot, Quantity);
		return;
	}

	//--------------------------------------------------------------------------------
	// 1. Validation checks to ensure everything that needs to be valid is. Ensuring the ItemContainers are valid, the
	// SourceItem doesn't already exist, and the Quantities are valid.
	//--------------------------------------------------------------------------------
	UCrimItemContainer* SourceContainer = GetItemContainer(SourceContainerId);
	UCrimItemContainer* TargetContainer = GetItemContainer(TargetContainerId);
	if (!SourceContainer || !TargetContainer)
	{
		return;
	}

	if (SourceContainer->IsChildContainer() || TargetContainer->IsChildContainer())
	{
		return;
	}

	UCrimItem* SourceItem = SourceContainer->GetItemInSlot(SourceSlot);
	if (!IsValid(SourceItem))
	{
		UE_LOG(LogCrimItemSystem, Verbose, TEXT("No item exists on %s at slot: %d."), *SourceContainer->GetName(), SourceSlot);
		return;
	}
	Quantity = FMath::Min(SourceItem->GetQuantity(), Quantity);
	if (Quantity < 1)
	{
		UE_LOG(LogCrimItemSystem, Verbose, TEXT("The item %s has 0 quantity left. Unable to move."), *SourceItem->GetName());
		return;
	}

	//--------------------------------------------------------------------------------
	// 2. Get the plan on how to add the item to the TargetContainer.
	//--------------------------------------------------------------------------------
	int32 RemainingCollectionLimit = GetRemainingCollectionCapacityForItem(SourceItem);
	FCrimItemAddPlan MoveItemPlan;
	if (TargetSlot >= 0)
	{
		// Try and move item to the specified slot.
		MoveItemPlan = TargetContainer->GetAddItemPlanForSlot(SourceItem, Quantity, TargetSlot, RemainingCollectionLimit);
	}
	else
	{
		// Try and move item to the first available slot.
		MoveItemPlan = TargetContainer->GetAddItemPlan(SourceItem, Quantity, RemainingCollectionLimit);
	}

	if (!MoveItemPlan.IsValid())
	{
		return;
	}

	//--------------------------------------------------------------------------------
	// 3. Execute the valid plan. Do we just update quantities for both? Do we destroy the SourceItem when it's
	// quantity reaches 0? Or do we copy the item over to the TargetContainer.
	//--------------------------------------------------------------------------------
	for (const FCrimItemAddPlanSlot& SlotPlan : MoveItemPlan.GetSlotPlans())
	{
		if (SlotPlan.bSlotIsEmpty)
		{
			// The slot is empty.
			if (SourceItem->GetQuantity() - SlotPlan.QuantityToAdd == 0)
			{
				// The amount added would result in 0 at the SourceContainer. Directly move the item to the TargetContainer.
				SourceContainer->ItemList.RemoveEntryAt(SourceSlot);
				TargetContainer->ItemList.AddEntryAt(SourceItem, SlotPlan.TargetSlot);
				SourceContainer->RefreshChildContainers(SourceItem);
			}
			else
			{
				// The amount added would result in a leftover amount in the SourceContainer make a copy of the item.
				SourceItem->SetQuantity(SourceItem->GetQuantity() - SlotPlan.QuantityToAdd);
				UCrimItem* NewItem = CreateItem(SourceItem->GetItemDefinition(), FGuid::NewGuid());
				NewItem->CopyItemProperties(SourceItem);
				NewItem->SetQuantity(SlotPlan.QuantityToAdd);
				TargetContainer->ItemList.AddEntryAt(NewItem, SlotPlan.TargetSlot);
			}
		}
		else
		{
			// The slot has an existing matching item.
			UCrimItem* TargetItem = TargetContainer->GetItemInSlot(SlotPlan.TargetSlot);
			TargetItem->SetQuantity(TargetItem->GetQuantity() + SlotPlan.QuantityToAdd);
			SourceItem->SetQuantity(SourceItem->GetQuantity() - SlotPlan.QuantityToAdd);
			
			if (SourceItem->GetQuantity() == 0 && SourceContainer->CanRemoveItem(SourceItem))
			{
				DestroyItem(SourceItem);
			}
		}
	}
}

void UCrimItemManagerComponent::AssignItemToContainer(FGameplayTag ParentContainerId, int32 ParentSlot, FGameplayTag ChildContainerId, int32 TargetSlot)
{
	if (!CanAssignItemToContainer(ParentContainerId, ParentSlot, ChildContainerId, TargetSlot))
	{
		return;
	}
	
	if (!HasAuthority())
	{
		Server_AssignItemToContainer_Implementation(ParentContainerId, ParentSlot, ChildContainerId, TargetSlot);
		return;
	}
	
	UCrimItem* Item = GetItemContainer(ParentContainerId)->GetItemInSlot(ParentSlot);
	GetItemContainer(ChildContainerId)->ItemList.AddEntryAt(Item, TargetSlot);
}

void UCrimItemManagerComponent::SplitItemStack(FGameplayTag ContainerId, int32 SourceSlot, int32 TargetSlot, int32 Quantity)
{
	UCrimItemContainer* Container = GetItemContainer(ContainerId);
	if (!Container)
	{
		return;
	}
	
	UCrimItem* Item = Container->GetItemInSlot(SourceSlot);
	if (!CanSplitItemStack(ContainerId, Item, TargetSlot))
	{
		return;
	}

	if (Item->GetQuantity() <= Quantity)
	{
		// Can't split an item if the quantity is greater than the Item's quantity.
		return;
	}

	if (!HasAuthority())
	{
		Server_SplitItemStack(ContainerId, SourceSlot, TargetSlot, Quantity);
		return;
	}

	UCrimItem* NewItem = CreateItem(Item->GetItemDefinition(), FGuid::NewGuid());
	NewItem->CopyItemProperties(Item);
	NewItem->SetQuantity(Quantity);
	Item->SetQuantity(Item->GetQuantity() - Quantity);
	Container->ItemList.AddEntryAt(NewItem, TargetSlot);
}

void UCrimItemManagerComponent::StackItems(FGameplayTag ContainerId, int32 SourceSlot, int32 TargetSlot, int32 Quantity)
{
	if (!ContainerId.IsValid() || SourceSlot < 0 || TargetSlot < 0 || Quantity < 1)
	{
		return;
	}
	
	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	if (!ItemContainer)
	{
		return;
	}

	int32 MaxTransferAmount = 0;
	if (!CanStackItems(ContainerId, SourceSlot, TargetSlot, MaxTransferAmount))
	{
		return;
	}

	if (!HasAuthority())
	{
		Server_StackItems(ContainerId, SourceSlot, TargetSlot, Quantity);
		return;
	}

	int32 TransferAmount = FMath::Min(MaxTransferAmount, Quantity);
	UCrimItem* SourceItem = ItemContainer->GetItemInSlot(SourceSlot);
	UCrimItem* TargetItem = ItemContainer->GetItemInSlot(TargetSlot);

	SourceItem->SetQuantity(SourceItem->GetQuantity() - TransferAmount);
	TargetItem->SetQuantity(TargetItem->GetQuantity() + TransferAmount);

	if (SourceItem->GetQuantity() <= 0 && ItemContainer->CanRemoveItem(SourceItem))
	{
		DestroyItem(SourceItem);
	}
}

bool UCrimItemManagerComponent::ReleaseItem(UCrimItem* Item)
{
	if (!HasAuthority())
	{
		return false;
	}
	
	if (!IsValid(Item))
	{
		return false;
	}

	if (Item->GetItemManagerComponent() != this)
	{
		return false;
	}

	DestroyItem(Item);
	return true;
}

FCrimItemAddPlan UCrimItemManagerComponent::TryAddItem(FGameplayTag ContainerId, UCrimItem* Item, int32 Quantity)
{
	FCrimItemAddPlan Result;
	Result.AmountToGive = Quantity;
	if (!HasAuthority())
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error;
		return Result;
	}
	
	UCrimItemContainer* TargetContainer = GetItemContainer(ContainerId);
	if (!TargetContainer)
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error_ItemContainerNotFound;
		return Result;
	}

	Result = TargetContainer->GetAddItemPlan(Item, Quantity, GetRemainingCollectionCapacityForItem(Item));
	ExecuteAddItemPlan(Result, TargetContainer, Item);
	return Result;
}

UCrimItem* UCrimItemManagerComponent::AddItem(FGameplayTag ContainerId, UCrimItem* Item, int32 Quantity, int32 Slot)
{
	if (!HasAuthority() || !IsValid(Item) || !Item->GetItemDefinition())
	{
		return nullptr;
	}

	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	if (!IsValid(ItemContainer))
	{
		return nullptr;
	}

	if (ItemContainer->IsChildContainer())
	{
		return nullptr;
	}

	UCrimItem* NewItem = CreateItem(Item->GetItemDefinition(), FGuid::NewGuid());
	NewItem->CopyItemProperties(Item);
	NewItem->SetQuantity(Quantity);
	AddItemToContainer(ItemContainer, NewItem, Slot);

	return NewItem;
}

FCrimItemAddPlan UCrimItemManagerComponent::TryAddItemFromSpec(FGameplayTag ContainerId, const TInstancedStruct<FCrimItemSpec>& ItemSpec)
{
	FCrimItemAddPlan Result;
	if (!HasAuthority() || !ItemSpec.IsValid() || !ContainerId.IsValid())
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error;
		return Result;
	}

	const FCrimItemSpec& Spec = ItemSpec.Get<FCrimItemSpec>();
	if (!Spec.ItemDef.IsValid() ||
		Spec.Quantity <= 0)
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error_InvalidItem;
		return Result;
	}

	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	if (!IsValid(ItemContainer))
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error_ItemContainerNotFound;
		return Result;
	}

	// Create a temporary item. Don't need to add this to a replicated SubObject list as this item will be removed later.
	UCrimItem* TempItem = NewObject<UCrimItem>(this,Spec.ItemDef->ItemClass.LoadSynchronous());
	TempItem->Initialize(this, Spec.ItemDef.LoadSynchronous(), FGuid::NewGuid());
	TempItem->ApplyItemSpec(ItemSpec);

	Result = ItemContainer->GetAddItemPlan(TempItem, Spec.Quantity, GetRemainingCollectionCapacityForItem(TempItem));
	ExecuteAddItemPlan(Result, ItemContainer, TempItem);
	return Result;
}

UCrimItem* UCrimItemManagerComponent::AddItemFromSpec(FGameplayTag ContainerId, const TInstancedStruct<FCrimItemSpec>& Spec, int32 Slot)
{
	if (!HasAuthority() || !Spec.IsValid())
	{
		return nullptr;
	}

	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	if (!IsValid(ItemContainer))
	{
		return nullptr;
	}

	if (ItemContainer->IsChildContainer())
	{
		return nullptr;
	}

	const FCrimItemSpec* ConstSpec = Spec.GetPtr<FCrimItemSpec>();
	if (!ConstSpec->ItemDef.IsValid())
	{
		return nullptr;
	}

	UCrimItem* NewItem = CreateItem(ConstSpec->ItemDef.LoadSynchronous(), FGuid::NewGuid());
	NewItem->ApplyItemSpec(Spec);
	AddItemToContainer(ItemContainer, NewItem, Slot);
	
	return NewItem;
}

FCrimItemAddPlan UCrimItemManagerComponent::TryAddItemFromDefinition(FGameplayTag ContainerId,
	const UCrimItemDef* ItemDef, int32 Quantity)
{
	FCrimItemAddPlan Result;
	if (!HasAuthority() || !ContainerId.IsValid())
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error;
		return Result;
	}
	
	if (!ItemDef || Quantity <= 0)
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error_InvalidItem;
		return Result;
	}

	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	if (!IsValid(ItemContainer))
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error_ItemContainerNotFound;
		return Result;
	}

	// Create a temporary item. Don't need to add this to a replicated SubObject list as this item will be removed later.
	UCrimItem* TempItem = NewObject<UCrimItem>(this, ItemDef->ItemClass.LoadSynchronous());
	TempItem->Initialize(this, ItemDef, FGuid::NewGuid());
	TempItem->ApplyItemDef(ItemDef);

	Result = ItemContainer->GetAddItemPlan(TempItem, Quantity, GetRemainingCollectionCapacityForItem(TempItem));
	ExecuteAddItemPlan(Result, ItemContainer, TempItem);
	return Result;
}

UCrimItem* UCrimItemManagerComponent::AddItemFromDefinition(FGameplayTag ContainerId, const UCrimItemDef* ItemDef, int32 Quantity, int32 Slot)
{
	if (!HasAuthority() || !ItemDef || Quantity <= 0)
	{
		return nullptr;
	}

	UCrimItemContainer* ItemContainer = GetItemContainer(ContainerId);
	if (!IsValid(ItemContainer))
	{
		return nullptr;
	}

	if (ItemContainer->IsChildContainer())
	{
		return nullptr;
	}

	UCrimItem* NewItem = CreateItem(ItemDef, FGuid::NewGuid());
	NewItem->SetQuantity(Quantity);
	NewItem->ApplyItemDef(ItemDef);
	AddItemToContainer(ItemContainer, NewItem, Slot);
	
	return NewItem;
}

void UCrimItemManagerComponent::ConsumeItem(UCrimItem* Item, int32 Quantity)
{
	if (!IsValid(Item) || !HasAuthority() || Quantity <= 0 || Item->GetItemManagerComponent() != this)
	{
		return;
	}

	const int32 NewQuantity = FMath::Max(Item->GetQuantity() - Quantity, 0);
	Item->SetQuantity(NewQuantity);
	if (NewQuantity == 0)
	{
		TArray<UCrimItemContainer*> Containers = GetItemContainersWithItem(Item);
		bool bCanDestroyItem = true;
		for (const TObjectPtr<UCrimItemContainer>& Container : Containers)
		{
			if (!Container->CanRemoveItem(Item))
			{
				bCanDestroyItem = false;
				break;
			}
		}

		if (bCanDestroyItem)
		{
			DestroyItem(Item);
		}
	}
}

void UCrimItemManagerComponent::ConsumeItemByDefinition(const UCrimItemDef* ItemDef, int32 Quantity)
{
	if (!HasAuthority() || !IsValid(ItemDef) || Quantity <= 0)
	{
		return;
	}

	int32 QuantityRemaining = Quantity;
	for (UCrimItem*& Item : GetItemsByDefinition(ItemDef))
	{
		const int32 NewQuantity = FMath::Max(Item->GetQuantity() - QuantityRemaining, 0);
		QuantityRemaining = FMath::Max(QuantityRemaining - Item->GetQuantity(), 0);
		Item->SetQuantity(NewQuantity);
		if (NewQuantity == 0)
		{
			TArray<UCrimItemContainer*> Containers = GetItemContainersWithItem(Item);
			bool bCanDestroyItem = true;
			for (const TObjectPtr<UCrimItemContainer>& Container : Containers)
			{
				if (!Container->CanRemoveItem(Item))
				{
					bCanDestroyItem = false;
					break;
				}
			}

			if (bCanDestroyItem)
			{
				DestroyItem(Item);
			}
		}

		if (QuantityRemaining == 0)
		{
			return;
		}
	}
}

UCrimItem* UCrimItemManagerComponent::CreateItem(const UCrimItemDef* ItemDef, FGuid ItemId)
{
	if (!HasAuthority() || !ItemDef)
	{
		return nullptr;
	}

	if (!ItemId.IsValid())
	{
		ItemId = FGuid::NewGuid();
	}
	
	UCrimItem* NewItem = NewObject<UCrimItem>(
		this,
		ItemDef->ItemClass.LoadSynchronous()
	);
	NewItem->Initialize(this, ItemDef, ItemId);
	AddReplicatedSubObject(NewItem);
	return NewItem;
}

void UCrimItemManagerComponent::ExecuteAddItemPlan(const FCrimItemAddPlan& Plan, UCrimItemContainer* ItemContainer, UCrimItem* Item)
{
	if (Plan.IsValid())
	{
		for (const FCrimItemAddPlanSlot& Slot : Plan.GetSlotPlans())
		{
			if (Slot.bSlotIsEmpty)
			{
				UCrimItem* NewItem = CreateItem(Item->GetItemDefinition(), FGuid::NewGuid());
				NewItem->SetQuantity(Slot.QuantityToAdd);
				NewItem->CopyItemProperties(Item);
				ItemContainer->ItemList.AddEntryAt(NewItem, Slot.TargetSlot);
			}
			else
			{
				UCrimItem* ExistingItem = ItemContainer->ItemList.GetItemAtSlot(Slot.TargetSlot);
				ExistingItem->SetQuantity(ExistingItem->GetQuantity() + Slot.QuantityToAdd);
			}
		}
	}
}

bool UCrimItemManagerComponent::HasAuthority() const
{
	return !bCachedIsNetSimulated;
}

FCrimItemManagerSaveData UCrimItemManagerComponent::GetSaveData() const
{
	FCrimItemManagerSaveData SaveData;

	for (const FCrimItemContainerListEntry& Container : GetItemContainers())
	{
		if (!IsValid(Container.GetItemContainer()))
		{
			continue;
		}
		UCrimItemContainer* ItemContainer = Container.GetItemContainer();
		if (ItemContainer->IsParentContainer())
		{
			FCrimItemContainerSaveData_Parent NewData;
			NewData.ItemContainer = FCrimItemContainerSaveData(ItemContainer);
			for (const auto& Item : ItemContainer->GetItems())
			{
				if (IsValid(Item.GetItem()))
				{
					NewData.Items.Add(FCrimItemSaveData(Item.GetItem(), Item.GetSlot()));
				}
			}
			for (const TObjectPtr<UCrimItemContainer>& Child : ItemContainer->GetLinkedContainers())
			{
				if (IsValid(Child))
				{
					NewData.ChildContainerIds.Add(Child->GetContainerId());
				}
			}
			SaveData.ParentSaveData.Add(NewData);
		}
		else
		{
			FCrimItemContainerSaveData_Child NewData;
			NewData.ItemContainer = FCrimItemContainerSaveData(ItemContainer);
			for (const FCrimItemListEntry& Item : ItemContainer->GetItems())
			{
				if (IsValid(Item.GetItem()))
				{
					NewData.Items.Add(FCrimItemSaveData_Child(Item.GetItem()->GetItemId(), Item.GetSlot()));
				}
			}
			SaveData.ChildSaveData.Add(NewData);
		}
	}
	return SaveData;
}

void UCrimItemManagerComponent::RestoreSavedData(const FCrimItemManagerSaveData& SaveData)
{
	if (!HasAuthority())
	{
		return;
	}

	//----------------------------------------------------------
	// 1. Remove all existing Items and ItemContainers
	//----------------------------------------------------------
	TArray<UCrimItemContainer*> ItemContainers;
	for (const FCrimItemContainerListEntry& Container : ItemContainerList.GetItemContainers())
	{
		if (IsValid(Container.GetItemContainer()))
		{
			ItemContainers.Add(Container.GetItemContainer());
		}
	}
	
	for (UCrimItemContainer* Container : ItemContainers)
	{
		DestroyItemContainer(Container);
	}

	//----------------------------------------------------------
	// 2. Restore Parent Containers and their Items
	//----------------------------------------------------------
	TArray<UCrimItemContainer*> RestoredParentContainers;
	for (const FCrimItemContainerSaveData_Parent& ParentSaveData : SaveData.ParentSaveData)
	{
		UCrimItemContainer* NewContainer = CreateItemContainer(
			ParentSaveData.ItemContainer.ContainerId,
			ParentSaveData.ItemContainer.ItemContainerClass.LoadSynchronous(),
			true);
		if (NewContainer)
		{
			// Serialize ItemContainer properties
			FMemoryReader MemoryReader(ParentSaveData.ItemContainer.ByteData);
			FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
			Archive.ArIsSaveGame = true;
			NewContainer->Serialize(Archive);
			
			RestoredParentContainers.Add(NewContainer);
			for (const FCrimItemSaveData& ItemSaveData : ParentSaveData.Items)
			{
				if (NewContainer->GetItemInSlot(ItemSaveData.Slot))
				{
					// Have an existing item restored in the slot. Skip.
					continue;
				}
				
				UCrimItem* NewItem = CreateItem(ItemSaveData.ItemDef.LoadSynchronous(), ItemSaveData.ItemId);
				if (NewItem)
				{
					FMemoryReader MemReader(ItemSaveData.ByteData);
					FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
					Ar.ArIsSaveGame = true;
					NewItem->Serialize(Ar);
					NewContainer->ItemList.AddEntryAt(NewItem, ItemSaveData.Slot);
				}
			}
		}
	}

	//----------------------------------------------------------
	// 3. Restore Child Containers
	//----------------------------------------------------------
	TArray<UCrimItemContainer*> RestoredChildContainers;
	for (const FCrimItemContainerSaveData_Child& ChildSaveData : SaveData.ChildSaveData)
	{
		UCrimItemContainer* NewContainer = CreateItemContainer(
			ChildSaveData.ItemContainer.ContainerId,
			ChildSaveData.ItemContainer.ItemContainerClass.LoadSynchronous(),
			false);
		if (NewContainer)
		{
			// Serialize ItemContainer properties
			FMemoryReader MemReader(ChildSaveData.ItemContainer.ByteData);
			FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
			Ar.ArIsSaveGame = true;
			NewContainer->Serialize(Ar);
			RestoredChildContainers.Add(NewContainer);
		}
	}

	//----------------------------------------------------------
	// 4. Restore Container Links
	//----------------------------------------------------------
	for (const FCrimItemContainerSaveData_Parent& ParentSaveData : SaveData.ParentSaveData)
	{
		if (UCrimItemContainer* ParentContainer = GetItemContainer(ParentSaveData.ItemContainer.ContainerId))
		{
			for (const FGameplayTag& ChildId : ParentSaveData.ChildContainerIds)
			{
				if (UCrimItemContainer* ChildContainer = GetItemContainer(ChildId))
				{
					ParentContainer->LinkContainers(ChildContainer);
				}
			}
		}
	}
	
	//----------------------------------------------------------
	// 5. Restore Items in the ChildContainers from Parent
	//----------------------------------------------------------
	for (const FCrimItemContainerSaveData_Child& ChildSaveData : SaveData.ChildSaveData)
	{
		if (UCrimItemContainer* ChildContainer = GetItemContainer(ChildSaveData.ItemContainer.ContainerId))
		{
			for (const auto& ItemSaveData : ChildSaveData.Items)
			{
				for (UCrimItemContainer* Link : ChildContainer->GetLinkedContainers())
				{
					if (UCrimItem* Item = Link->GetItemFromId(ItemSaveData.ItemId))
					{
						ChildContainer->ItemList.AddEntryAt(Item, ItemSaveData.Slot);
						break;
					}
				}
			}
		}
	}
}

void UCrimItemManagerComponent::CacheIsNetSimulated()
{
	bCachedIsNetSimulated = IsNetSimulating();
}

void UCrimItemManagerComponent::BindToItemContainerListDelegates()
{
	if (!ItemContainerList.OnEntryAddedDelegate.IsBoundToObject(this))
	{
		ItemContainerList.OnEntryAddedDelegate.AddWeakLambda(this, [this](const FCrimItemContainerListEntry& Entry)
		{
			BindToItemContainerDelegates(Entry.GetItemContainer());
		});
	}
}

void UCrimItemManagerComponent::BindToItemContainerDelegates(UCrimItemContainer* Container)
{
	Container->OnEntryAddedDelegate.AddWeakLambda(this, [this](UCrimItemContainer* ItemContainer, const FCrimItemListEntry& Item)
	{
		OnItemAddedDelegate.Broadcast(this, ItemContainer, Item);
	});
	Container->OnEntryRemovedDelegate.AddWeakLambda(this, [this](UCrimItemContainer* ItemContainer, const FCrimItemListEntry& Item)
	{
		OnItemRemovedDelegate.Broadcast(this, ItemContainer, Item);
	});
	Container->OnEntryUpdatedDelegate.AddWeakLambda(this, [this](UCrimItemContainer* ItemContainer, const FCrimItemListEntry& Item)
	{
		OnItemUpdatedDelegate.Broadcast(this, ItemContainer, Item);
	});
}

void UCrimItemManagerComponent::RemoveItemFromAllItemContainers(UCrimItem* Item)
{
	if (HasAuthority())
	{
		TArray<UCrimItemContainer*> ItemContainers = GetItemContainersWithItem(Item);
		for (UCrimItemContainer*& Container : ItemContainers)
		{
			Container->ItemList.RemoveEntry(Item, true);
		}
	}
}

void UCrimItemManagerComponent::AddItemToContainer(UCrimItemContainer* ItemContainer, UCrimItem* NewItem, int32 Slot)
{
	if (Slot < 0)
	{
		ItemContainer->ItemList.AddEntry(NewItem);
	}
	else
	{
		UCrimItem* OldItem = ItemContainer->GetItemInSlot(Slot);
		if (IsValid(OldItem))
		{
			ItemContainer->ItemList.RemoveEntryAt(Slot);
			DestroyItem(OldItem);
		}
		ItemContainer->ItemList.AddEntryAt(NewItem, Slot);
	}
}

bool UCrimItemManagerComponent::DestroyItem(UCrimItem* Item)
{
	if (!HasAuthority())
	{
		return false;
	}
	
	RemoveItemFromAllItemContainers(Item);
	RemoveReplicatedSubObject(Item);
	Item->MarkAsGarbage();
	return true;
}

//---------------------------------------------------------------------------
// Server RPC Functions
//---------------------------------------------------------------------------
#pragma region Server RPC Functions

void UCrimItemManagerComponent::Server_SwapItemsBySlot_Implementation(FGameplayTag ContainerId, int32 FirstSlot, int32 SecondSlot)
{
	SwapItemsBySlot(ContainerId, FirstSlot, SecondSlot);
}

void UCrimItemManagerComponent::Server_MoveItemToContainer_Implementation(FGameplayTag SourceContainerId,
	int32 SourceSlot, FGameplayTag TargetContainerId, int32 TargetSlot, int32 Quantity)
{
	MoveItemToContainer(SourceContainerId, SourceSlot, TargetContainerId, TargetSlot, Quantity);
}

void UCrimItemManagerComponent::Server_AssignItemToContainer_Implementation(FGameplayTag ParentContainerId, int32 ParentSlot,
	FGameplayTag ChildContainerId, int32 TargetSlot)
{
	AssignItemToContainer(ParentContainerId, ParentSlot, ChildContainerId, TargetSlot);
}

void UCrimItemManagerComponent::Server_SplitItemStack_Implementation(FGameplayTag ContainerId, int32 SourceSlot,
	int32 TargetSlot, int32 Quantity)
{
	SplitItemStack(ContainerId, SourceSlot, TargetSlot, Quantity);
}

void UCrimItemManagerComponent::Server_StackItems_Implementation(FGameplayTag ContainerId, int32 SourceSlot,
	int32 TargetSlot, int32 Quantity)
{
	StackItems(ContainerId, SourceSlot, TargetSlot, Quantity);
}

#pragma endregion
