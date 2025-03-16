// Copyright Soccertitan


#include "CrimItemContainer.h"

#include "CrimItemGameplayTags.h"
#include "CrimItemDef.h"
#include "CrimItem.h"
#include "CrimItemManagerComponent.h"
#include "ItemContainerRules/CrimItemContainerRule.h"
#include "Net/UnrealNetwork.h"

UCrimItemContainer::UCrimItemContainer()
{
	
}

void UCrimItemContainer::PostInitProperties()
{
	UObject::PostInitProperties();

	BindToItemListDelegates();
}

void UCrimItemContainer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ContainerId, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemManagerComponent, Params);
	if (bLimitCapacity)
	{
		DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MaxCapacity, Params);
	}

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bIsParent, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, LinkedContainers, Params);
	
	DOREPLIFETIME(ThisClass, ItemList);
}

#if UE_WITH_IRIS
void UCrimItemContainer::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif

const FGameplayTag& UCrimItemContainer::GetContainerId() const
{
	return ContainerId;
}

const FGameplayTagContainer& UCrimItemContainer::GetOwnedTags() const
{
	return OwnedTags;
}

FCrimItemAddPlan UCrimItemContainer::GetAddItemPlan(UCrimItem* Item, int32 ItemQuantity, int32 MaxNewItemStacks) const
{
	FCrimItemAddPlan Result(ItemQuantity);

	const FCrimItemGameplayTags& Tags = FCrimItemGameplayTags::Get();

	if (!CanAddItem(Result, Item, ItemQuantity))
	{
		return Result;
	}

	//----------------------------------------------------------------------------------------------
	// 1. Determine how many quantities of this item we can add to this container.
	//----------------------------------------------------------------------------------------------
	int32 RemainingQuantityToAdd = FMath::Min(GetRemainingCapacityForItem(Item), ItemQuantity);
	const int32 ItemStackMaxQuantity = GetItemQuantityLimit(Item);

	//----------------------------------------------------------------------------------------------
	// 2. If auto stacking gather a list of all matching items in the list. We only try to auto
	// stack if the ItemStackMaxQuantity is greater than 1 and auto stacking is enabled.
	//----------------------------------------------------------------------------------------------
	TArray<int32> ExistingSlotsToAddItems = TArray<int32>();
	if (bAutoStack && ItemStackMaxQuantity > 1)
	{
		ExistingSlotsToAddItems = GetMatchingItemSlots(Item);
		ExistingSlotsToAddItems.Sort();
	}

	//----------------------------------------------------------------------------------------------
	// 3. We add item quantities to existing items that are not at their max capacity for a single stack.
	//----------------------------------------------------------------------------------------------
	for (const int32& Slot : ExistingSlotsToAddItems)
	{
		if (RemainingQuantityToAdd < 0)
		{
			break;
		}
		
		if (GetItemInSlot(Slot)->GetQuantity() < ItemStackMaxQuantity)
		{
			const int32 QuantityToAdd = FMath::Min(RemainingQuantityToAdd, ItemStackMaxQuantity - GetItemInSlot(Slot)->GetQuantity());
			RemainingQuantityToAdd = RemainingQuantityToAdd - QuantityToAdd;
			Result.AddSlotPlan(FCrimItemAddPlanSlot(Slot, QuantityToAdd, false));
		}
	}

	if (RemainingQuantityToAdd <= 0)
	{
		return Result;
	}

	//----------------------------------------------------------------------------------------------
	// 4. We get all valid slots the item can be in. If the TArray is empty then all slots are valid,
	// and we add items to available empty slots.
	// If the ValidSlots has an array of int32s we cycle through each one to check if the slot is empty and valid.
	//----------------------------------------------------------------------------------------------
	TArray<int32> ValidSlots = GetValidSlots(Item);
	if (ValidSlots.IsEmpty())
	{
		// Defining how to add items to empty slots in the container.
		int32 NextEmptySlot = GetNextEmptySlot();
		int32 NewStacksAdded = 0;
		while (RemainingQuantityToAdd > 0 && NewStacksAdded < MaxNewItemStacks)
		{
			const int32 QuantityToAdd = FMath::Min(RemainingQuantityToAdd, ItemStackMaxQuantity);
			RemainingQuantityToAdd = RemainingQuantityToAdd - QuantityToAdd;
		
			Result.AddSlotPlan(FCrimItemAddPlanSlot(NextEmptySlot, QuantityToAdd, true));
			NewStacksAdded++;
			NextEmptySlot = GetNextEmptySlot(++NextEmptySlot);
		}
	}
	else
	{
		int32 NewStacksAdded = 0;
		for (int32& Slot : ValidSlots)
		{
			if (RemainingQuantityToAdd <= 0 || NewStacksAdded >= MaxNewItemStacks)
			{
				break;
			}
			
			// Only add items to the slot if it's empty. As we already created a plan for existing items in the container.
			if (!GetItemInSlot(Slot))
			{
				const int32 QuantityToAdd = FMath::Min(RemainingQuantityToAdd, ItemStackMaxQuantity);
				RemainingQuantityToAdd = RemainingQuantityToAdd - QuantityToAdd;
				Result.AddSlotPlan(FCrimItemAddPlanSlot(Slot, QuantityToAdd, true));
			}
		}
	}

	if (Result.Result == ECrimItemAddResult::SomeItemsAdded)
	{
		Result.Error = Tags.Crim_ItemPlan_Error_MaxStacksReached;
	}
	
	return Result;
}

FCrimItemAddPlan UCrimItemContainer::GetAddItemPlanForSlot(UCrimItem* Item, int32 ItemQuantity, int32 MaxNewItemStacks, int32 Slot) const
{
	FCrimItemAddPlan Result(ItemQuantity);

	const FCrimItemGameplayTags& Tags = FCrimItemGameplayTags::Get();

	if (!CanAddItem(Result, Item, ItemQuantity, Slot))
	{
		return Result;
	}

	//----------------------------------------------------------------------------------------------
	// 1. Determine how many quantities of this item we can add to this container.
	//----------------------------------------------------------------------------------------------
	int32 RemainingQuantityToAdd = FMath::Min(GetRemainingCapacityForItem(Item), ItemQuantity);
	const int32 ItemStackMaxQuantity = GetItemQuantityLimit(Item);
	

	//----------------------------------------------------------------------------------------------
	// 2. If there is a matching item or empty. Will add much as much quantity as possible
	// to that slot.
	//----------------------------------------------------------------------------------------------
	UCrimItem* ItemAtSlot = GetItemInSlot(Slot);
	FCrimItemAddPlanSlot SlotPlan;
	SlotPlan.TargetSlot = Slot;
	if (IsValid(ItemAtSlot))
	{
		SlotPlan.bSlotIsEmpty = false;
		SlotPlan.QuantityToAdd = FMath::Min(ItemStackMaxQuantity - ItemAtSlot->GetQuantity(), RemainingQuantityToAdd);
	}
	else
	{
		if (MaxNewItemStacks >= 1)
		{
			SlotPlan.bSlotIsEmpty = true;
			SlotPlan.QuantityToAdd = FMath::Min(ItemStackMaxQuantity, RemainingQuantityToAdd);
		}
	}
	Result.AddSlotPlan(SlotPlan);

	if (Result.Result == ECrimItemAddResult::SomeItemsAdded)
	{
		Result.Error = Tags.Crim_ItemPlan_Error_MaxStacksReached;
	}

	return Result;
}

bool UCrimItemContainer::IsChildContainer() const
{
	return !bIsParent;
}

bool UCrimItemContainer::IsParentContainer() const
{
	return bIsParent;
}

TArray<UCrimItemContainer*> UCrimItemContainer::GetLinkedContainers() const
{
	return LinkedContainers;
}

bool UCrimItemContainer::IsContainerLinked(UCrimItemContainer* TestItemContainer) const
{
	return GetLinkedContainers().Contains(TestItemContainer);
}

int32 UCrimItemContainer::GetMaxCapacity() const
{
	if (!bLimitCapacity)
	{
		return MAX_int32;
	}
	return MaxCapacity;
}

int32 UCrimItemContainer::GetRemainingCapacity() const
{
	return GetMaxCapacity() - GetConsumedCapacity();
}

int32 UCrimItemContainer::GetConsumedCapacity() const
{
	return ItemList.GetItems().Num();
}

bool UCrimItemContainer::IsAtMaxCapacity() const
{
	return GetConsumedCapacity() >= GetMaxCapacity();
}

int32 UCrimItemContainer::SetMaxCapacity(int32 NewCount, bool bOverride)
{
	if (bOwnerIsNetAuthority)
	{
		if (bOverride && NewCount >= 1)
		{
			MaxCapacity = NewCount;
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MaxCapacity, this);
		}
		else
		{
			int32 AdjustedMaxCapacity = FMath::Max(0, MaxCapacity + NewCount);
			MaxCapacity = AdjustedMaxCapacity;
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MaxCapacity, this);
		}
	}
	return MaxCapacity;
}

const TArray<FCrimItemListEntry>& UCrimItemContainer::GetItems() const
{
	return ItemList.GetItems();
}

UCrimItem* UCrimItemContainer::GetItemInSlot(int32 Slot) const
{
	return ItemList.GetItemAtSlot(Slot);
}

UCrimItem* UCrimItemContainer::GetItemFromId(FGuid ItemId) const
{
	if (ItemId.IsValid())
	{
		for (const FCrimItemListEntry& Entry : ItemList.GetItems())
		{
			if (Entry.GetItem()->GetItemId() == ItemId)
			{
				return Entry.GetItem();
			}
		}
	}
	return nullptr;
}

UCrimItem* UCrimItemContainer::GetItemFromDef(const UCrimItemDef* ItemDef) const
{
	if (ItemDef)
	{
		for (const FCrimItemListEntry& Entry : ItemList.GetItems())
		{
			if (Entry.GetItem()->GetItemDefinition() == ItemDef)
			{
				return Entry.GetItem();
			}
		}
	}
	return nullptr;
}

TArray<UCrimItem*> UCrimItemContainer::GetItemsFromDef(const UCrimItemDef* ItemDef, bool bIgnoreDuplicates) const
{
	TArray<UCrimItem*> Items;

	if (ItemDef)
	{
		for (const FCrimItemListEntry& Entry : ItemList.GetItems())
		{
			if (Entry.GetItem()->GetItemDefinition() == ItemDef)
			{
				if (bIgnoreDuplicates)
				{
					Items.Add(Entry.GetItem());
				}
				else
				{
					Items.AddUnique(Entry.GetItem());
				}
			}
		}
	}
	return Items;
}

UCrimItem* UCrimItemContainer::GetMatchingItem(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return nullptr;
	}

	for (const FCrimItemListEntry& Entry : ItemList.GetItems())
	{
		if (Entry.GetItem()->IsMatching(TestItem))
		{
			return Entry.GetItem();
		}
	}
	return nullptr;
}

TArray<UCrimItem*> UCrimItemContainer::GetMatchingItems(const UCrimItem* TestItem, bool bIgnoreDuplicates) const
{
	if (!IsValid(TestItem))
	{
		return TArray<UCrimItem*>();
	}
	
	TArray<UCrimItem*> Result;

	for (const FCrimItemListEntry& Entry : ItemList.GetItems())
	{
		if (Entry.GetItem()->IsMatching(TestItem))
		{
			if (bIgnoreDuplicates)
			{
				Result.Add(Entry.GetItem());
			}
			else
			{
				Result.AddUnique(Entry.GetItem());
			}
		}
	}
	return Result;
}

TArray<int32> UCrimItemContainer::GetMatchingItemSlots(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return TArray<int32>();
	}

	TArray<int32> Result;

	for (const FCrimItemListEntry& Entry : ItemList.GetItems())
	{
		if (Entry.GetItem()->IsMatching(TestItem))
		{
			Result.Add(Entry.GetSlot());
		}
	}
	
	return Result;
}

TArray<int32> UCrimItemContainer::GetSlotsFromItem(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return TArray<int32>();
	}
	
	TArray<int32> Result;

	for (const FCrimItemListEntry& Entry : ItemList.GetItems())
	{
		if (Entry.GetItem() == TestItem)
		{
			Result.Add(Entry.GetSlot());
		}
	}
	
	return Result;
}

bool UCrimItemContainer::HasItem(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return false;
	}
	
	for (const FCrimItemListEntry& Entry : ItemList.GetItems())
	{
		if (Entry.GetItem() == TestItem)
		{
			return true;
		}
	}
	return false;
}

bool UCrimItemContainer::HasItemById(FGuid ItemId) const
{
	if (!ItemId.IsValid())
	{
		return false;
	}

	for (const FCrimItemListEntry& Entry : ItemList.GetItems())
	{
		if (Entry.GetItem()->GetItemId() == ItemId)
		{
			return true;
		}
	}
	
	return false;
}

int32 UCrimItemContainer::GetRemainingCapacityForItem(UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return 0;
	}

	int32 AvailableStacks;
	TArray<int32> ValidSlots = GetValidSlots(TestItem);
	if (ValidSlots.IsEmpty())
	{
		AvailableStacks = FMath::Min(GetRemainingCapacity(), GetItemContainerLimit(TestItem));
	}
	else
	{
		AvailableStacks = FMath::Min(ValidSlots.Num(), GetItemContainerLimit(TestItem));
	}
	
	const int32 StackMaxQuantity = GetItemQuantityLimit(TestItem);
	int32 AvailableQuantity = 0;
	if (!FMath::MultiplyAndCheckForOverflow(AvailableStacks, StackMaxQuantity, AvailableQuantity))
	{
		AvailableQuantity = MAX_int32;
	}
	
	for (const FCrimItemListEntry& Entry : ItemList.GetItems())
	{
		if (Entry.GetItem()->IsMatching(TestItem))
		{
			AvailableQuantity = AvailableQuantity - Entry.GetItem()->GetQuantity();
		}
	}
	
	return AvailableQuantity;
}

bool UCrimItemContainer::IsValidSlot(int32 Slot) const
{
	if (Slot < 0)
	{
		return false;
	}

	if (Slot >= GetMaxCapacity())
	{
		return false;
	}

	return true;
}

bool UCrimItemContainer::IsValidSlotForItem(const UCrimItem* TestItem, int32 Slot) const
{
	if (!IsValid(TestItem))
	{
		return false;
	}
	
	TArray<int32> ValidSlots = GetValidSlots(TestItem);
	if (ValidSlots.IsEmpty() && IsValidSlot(Slot))
	{
		return true;
	}

	if (ValidSlots.Contains(Slot))
	{
		return true;
	}

	return false;
}

int32 UCrimItemContainer::GetNextEmptySlot(int32 Slot) const
{
	if (ItemList.GetItems().Num() >= GetMaxCapacity())
	{
		return INDEX_NONE;
	}

	return ItemList.NextAvailableSlot(Slot);
}

bool UCrimItemContainer::CanContainItem(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return false;
	}

	for (const TObjectPtr<UCrimItemContainerRule>& Rule : ItemContainerRules)
	{
		if (!Rule->CanContainItem(this, TestItem))
		{
			return false;
		}
	}
	return true;
}

bool UCrimItemContainer::CanRemoveItem(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return false;
	}

	for (const TObjectPtr<UCrimItemContainerRule>& Rule : ItemContainerRules)
	{
		if (!Rule->CanRemoveItem(this, TestItem))
		{
			return false;
		}
	}
	return true;
}

TArray<int32> UCrimItemContainer::GetValidSlots(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return TArray<int32>();
	}

	TArray<int32> TempResults;
	TArray<int32> FilteredResults;

	for (const TObjectPtr<UCrimItemContainerRule>& Rule : ItemContainerRules)
	{
		TArray<int32> RuleResults = Rule->GetValidSlots(this, TestItem);
		TempResults.Append(RuleResults);
	}

	for (int32& Result : TempResults)
	{
		if (IsValidSlot(Result))
		{
			FilteredResults.AddUnique(Result);
		}
	}
	return FilteredResults;
}

int32 UCrimItemContainer::GetItemContainerLimit(const UCrimItem* TestItem) const
{
	if (IsValid(TestItem))
	{
		int32 Result = TestItem->GetItemDefinition()->ContainerLimit.GetMaxQuantity();
		for (const TObjectPtr<UCrimItemContainerRule>& Rule : ItemContainerRules)
		{
			const int32 RuleMaxQuantity = Rule->GetMaxNumberOfStacks(this, TestItem);
			if (RuleMaxQuantity >= 0)
			{
				Result = FMath::Min(Result, RuleMaxQuantity);
			}
		}
		return Result;
	}
	return 0;
}

bool UCrimItemContainer::IsItemAtContainerLimit(const UCrimItem* TestItem) const
{
	TArray<UCrimItem*> Items = GetMatchingItems(TestItem);
	int32 MaxStacks = GetItemContainerLimit(TestItem);

	if (Items.Num() >= MaxStacks)
	{
		return true;
	}
	
	return false;
}

int32 UCrimItemContainer::GetItemQuantityLimit(const UCrimItem* TestItem) const
{
	if (IsValid(TestItem))
	{
		int32 Result = TestItem->GetItemDefinition()->StackLimit.GetMaxQuantity();
		for (const TObjectPtr<UCrimItemContainerRule>& Rule : ItemContainerRules)
		{
			const int32 RuleMaxQuantity = Rule->GetItemStackMaxQuantity(this, TestItem);
			if (RuleMaxQuantity >= 0)
			{
				Result = FMath::Min(Result, RuleMaxQuantity);	
			}
		}
		return Result;
	}
	return 0;
}

bool UCrimItemContainer::IsItemAtQuantityLimit(const UCrimItem* TestItem) const
{
	if (IsValid(TestItem))
	{
		return TestItem->GetQuantity() >= GetItemQuantityLimit(TestItem);
	}
	return false;
}

void UCrimItemContainer::Initialize(UCrimItemManagerComponent* ItemManager, FGameplayTag NewContainerId, bool bInIsParent)
{
	ItemManagerComponent = ItemManager;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemManagerComponent, this);
	bOwnerIsNetAuthority = ItemManagerComponent->HasAuthority();

	ContainerId = NewContainerId;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ContainerId, this);

	if (bIsParent != bInIsParent)
	{
		bIsParent = bInIsParent;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bIsParent, this);
	}
}

void UCrimItemContainer::BindToItemListDelegates()
{
	ItemList.OnEntryAddedDelegate.AddWeakLambda(this, [this](const FCrimItemListEntry& Entry)
	{
		OnEntryAdded(Entry);
		K2_OnEntryAdded(Entry);
		OnEntryAddedDelegate.Broadcast(this, Entry);
	});
	ItemList.OnEntryUpdatedDelegate.AddWeakLambda(this, [this](const FCrimItemListEntry& Entry)
	{
		OnEntryUpdated(Entry);
		K2_OnEntryUpdated(Entry);
		OnEntryUpdatedDelegate.Broadcast(this, Entry);
	});
	ItemList.OnEntryRemovedDelegate.AddWeakLambda(this, [this](const FCrimItemListEntry& Entry)
	{
		OnEntryRemoved(Entry);
		K2_OnEntryRemoved(Entry);
		OnEntryRemovedDelegate.Broadcast(this, Entry);
	});
}

void UCrimItemContainer::RefreshChildContainers(UCrimItem* OldItem)
{
	if (IsParentContainer() && IsValid(OldItem))
	{
		for (TObjectPtr<UCrimItemContainer>& Child : LinkedContainers)
		{
			if (Child->HasItem(OldItem))
			{
				bool bNoParentsHasItem = true;
				for (UCrimItemContainer* ParentContainer : LinkedContainers)
				{
					if (ParentContainer->HasItem(OldItem))
					{
						// Found the item in our parent. Do nothing.
						bNoParentsHasItem = false;
						break;
					}
				}
				if (bNoParentsHasItem)
				{
					Child->ItemList.RemoveEntry(OldItem, true);
				}
			}
		}
	}
}

bool UCrimItemContainer::CanAddItem(FCrimItemAddPlan& ItemPlan, UCrimItem* Item, int32 ItemQuantity, int32 Slot) const
{
	const FCrimItemGameplayTags& Tags = FCrimItemGameplayTags::Get();

	if (IsChildContainer())
	{
		ItemPlan.Error = Tags.Crim_ItemPlan_Error_ItemContainerIsChild;
		return false;
	}
	if (!IsValid(Item))
	{
		ItemPlan.Error = Tags.Crim_ItemPlan_Error_InvalidItem;
		return false;
	}
	if (HasItem(Item))
	{
		ItemPlan.Error = Tags.Crim_ItemPlan_Error_ItemAlreadyExists;
		return false;
	}
	if (!CanContainItem(Item))
	{
		ItemPlan.Error = Tags.Crim_ItemPlan_Error_CantContainItem;
		return false;
	}
	if (ItemQuantity <= 0)
	{
		ItemPlan.Error = Tags.Crim_ItemPlan_Error_QuantityIsZero;
		return false;
	}

	if (Slot >= 0)
	{
		// The item is not allowed at the specified slot. Return an error.
		if (!IsValidSlotForItem(Item, Slot))
		{
			ItemPlan.Error = Tags.Crim_ItemPlan_Error_CantContainItem;
			return false;
		}

		// Have an existing item that doesn't match. Return an error.
		UCrimItem* ItemAtSlot = GetItemInSlot(Slot);
		if (ItemAtSlot && !ItemAtSlot->IsMatching(Item))
		{
			ItemPlan.Error = Tags.Crim_ItemPlan_Error_ItemAlreadyExists;
			return false;
		}
	}

	return true;
}

bool UCrimItemContainer::LinkContainers(UCrimItemContainer* TargetContainer)
{
	if (!bOwnerIsNetAuthority)
	{
		return false;
	}

	if (!IsValid(TargetContainer))
	{
		return false;
	}

	if (IsChildContainer() == TargetContainer->IsChildContainer())
	{
		// Can't link containers if the type is the same.
		return false;
	}

	LinkedContainers.AddUnique(TargetContainer);
	TargetContainer->LinkedContainers.AddUnique(this);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LinkedContainers, this);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LinkedContainers, TargetContainer);
	return true;
}

bool UCrimItemContainer::UnlinkContainers(UCrimItemContainer* TargetContainer)
{
	if (!bOwnerIsNetAuthority)
	{
		return false;
	}

	if (!IsValid(TargetContainer))
	{
		return false;
	}

	if (LinkedContainers.Contains(TargetContainer))
	{
		LinkedContainers.Remove(TargetContainer);
		TargetContainer->LinkedContainers.Remove(this);
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LinkedContainers, this);
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LinkedContainers, TargetContainer);
		return true;
	}
	return false;
}
