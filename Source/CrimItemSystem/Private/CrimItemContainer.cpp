// Copyright Soccertitan


#include "CrimItemContainer.h"

#include "CrimItemGameplayTags.h"
#include "CrimItemDefinition.h"
#include "CrimItemManagerComponent.h"
#include "CrimItemStatics.h"
#include "Engine/AssetManager.h"
#include "ItemContainerRules/CrimItemContainerRule.h"
#include "Net/UnrealNetwork.h"
#include "UI/ViewModel/CrimItemContainerViewModel.h"

UCrimItemContainer::UCrimItemContainer()
{
	ViewModelClass = UCrimItemContainerViewModel::StaticClass();
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
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemList, Params);
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

TSubclassOf<UCrimItemContainerViewModel> UCrimItemContainer::GetViewModelClass() const
{
	if (!ViewModelClass.Get())
	{
		UAssetManager::Get().LoadAssetList({ViewModelClass.ToSoftObjectPath()})->WaitUntilComplete();
	}
	return ViewModelClass.Get();
}

const FGameplayTagContainer& UCrimItemContainer::GetOwnedTags() const
{
	return OwnedTags;
}

FCrimAddItemPlanResult UCrimItemContainer::GetAddItemPlan(const TInstancedStruct<FCrimItem>& Item, int32 ItemQuantity, int32 MaxNewItemStacks) const
{
	FCrimAddItemPlanResult Result(ItemQuantity);

	const FCrimItemGameplayTags& Tags = FCrimItemGameplayTags::Get();

	if (!CanAddItem(Item, Result, ItemQuantity))
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
	TArray<FFastCrimItem*> MatchingItems = TArray<FFastCrimItem*>();
	if (bAutoStack && ItemStackMaxQuantity > 1)
	{
		MatchingItems = GetMatchingItems(Item);
	}

	//----------------------------------------------------------------------------------------------
	// 3. We add item quantities to existing items that are not at their max capacity for a single stack.
	//----------------------------------------------------------------------------------------------
	for (FFastCrimItem*& Match : MatchingItems)
	{
		if (RemainingQuantityToAdd < 0)
		{
			break;
		}

		const FCrimItem* ItemPtr = Match->Item.GetPtr<FCrimItem>();
		if (ItemPtr->Quantity < ItemStackMaxQuantity)
		{
			const int32 QuantityToAdd = FMath::Min(RemainingQuantityToAdd, ItemStackMaxQuantity - ItemPtr->Quantity);
			RemainingQuantityToAdd = RemainingQuantityToAdd - QuantityToAdd;
			Result.AddSlotPlan(FCrimAddItemPlanResultValue(Match, QuantityToAdd));
		}
	}

	if (RemainingQuantityToAdd <= 0)
	{
		return Result;
	}

	//----------------------------------------------------------------------------------------------
	// 4. We add as many new stacks as allowed to the plan.
	//----------------------------------------------------------------------------------------------
	int32 NewStacksAdded = 0;
	while (RemainingQuantityToAdd > 0 && NewStacksAdded < MaxNewItemStacks)
	{
		const int32 QuantityToAdd = FMath::Min(RemainingQuantityToAdd, ItemStackMaxQuantity);
		RemainingQuantityToAdd = RemainingQuantityToAdd - QuantityToAdd;
	
		Result.AddSlotPlan(FCrimAddItemPlanResultValue(nullptr, QuantityToAdd));
		NewStacksAdded++;
	}

	if (Result.Result == ECrimItemAddResult::SomeItemsAdded)
	{
		Result.Error = Tags.Crim_ItemPlan_Error_MaxStacksReached;
	}
	
	return Result;
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
	return ItemList.GetNum();
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

const TArray<FFastCrimItem>& UCrimItemContainer::GetItems() const
{
	return ItemList.GetItems();
}

FFastCrimItem* UCrimItemContainer::GetItemFromGuid(FGuid ItemGuid) const
{
	return ItemList.GetItem(ItemGuid);
}

TInstancedStruct<FCrimItem> UCrimItemContainer::K2_GetItemFromGuid(FGuid ItemGuid) const
{
	if (FFastCrimItem* FastItem = ItemList.GetItem(ItemGuid))
	{
		return FastItem->Item;
	}
	
	return TInstancedStruct<FCrimItem>();
}

FFastCrimItem* UCrimItemContainer::GetItemFromDefinition(const UCrimItemDefinition* ItemDef) const
{
	if (ItemDef)
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->GetItemDefinition()->GetPathName() == ItemDef->GetPathName())
			{
				return const_cast<FFastCrimItem*>(&Entry);
			}
		}
	}
	return nullptr;
}

TInstancedStruct<FCrimItem> UCrimItemContainer::K2_GetItemFromDefinition(const UCrimItemDefinition* ItemDef) const
{
	if (ItemDef)
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->GetItemDefinition()->GetPathName() == ItemDef->GetPathName())
			{
				return Entry.Item;
			}
		}
	}
	return TInstancedStruct<FCrimItem>();
}

TArray<FFastCrimItem*> UCrimItemContainer::GetItemsFromDefinition(const UCrimItemDefinition* ItemDef) const
{
	TArray<FFastCrimItem*> Items;

	if (ItemDef)
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->GetItemDefinition()->GetPathName() == ItemDef->GetPathName())
			{
				Items.Add(const_cast<FFastCrimItem*>(&Entry));
			}
		}
	}
	return Items;
}

TArray<TInstancedStruct<FCrimItem>> UCrimItemContainer::K2_GetItemsFromDefinition(const UCrimItemDefinition* ItemDef) const
{
	TArray<TInstancedStruct<FCrimItem>> Items;

	if (ItemDef)
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->GetItemDefinition()->GetPathName() == ItemDef->GetPathName())
			{
				Items.Add(Entry.Item);
			}
		}
	}
	return Items;
}

TInstancedStruct<FCrimItem> UCrimItemContainer::GetMatchingItem(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!TestItem.IsValid())
	{
		return TInstancedStruct<FCrimItem>();
	}

	for (const FFastCrimItem& Entry : ItemList.GetItems())
	{
		if (Entry.Item.GetPtr<FCrimItem>()->IsMatching(TestItem))
		{
			return Entry.Item;
		}
	}
	
	return TInstancedStruct<FCrimItem>();
}

TArray<FFastCrimItem*> UCrimItemContainer::GetMatchingItems(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!TestItem.IsValid())
	{
		return TArray<FFastCrimItem*>();
	}
	
	TArray<FFastCrimItem*> Result;

	for (const FFastCrimItem& Entry : ItemList.GetItems())
	{
		if (Entry.Item.GetPtr<FCrimItem>()->IsMatching(TestItem))
		{
			Result.Add(const_cast<FFastCrimItem*>(&Entry));
		}
	}
	return Result;
}

TArray<TInstancedStruct<FCrimItem>> UCrimItemContainer::K2_GetMatchingItems(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!TestItem.IsValid())
	{
		return TArray<TInstancedStruct<FCrimItem>>();
	}
	
	TArray<TInstancedStruct<FCrimItem>> Result;

	for (const FFastCrimItem& Entry : ItemList.GetItems())
	{
		if (Entry.Item.GetPtr<FCrimItem>()->IsMatching(TestItem))
		{
			Result.Add(Entry.Item);
		}
	}
	return Result;
}

bool UCrimItemContainer::HasItemById(FGuid ItemId) const
{
	if (!ItemId.IsValid())
	{
		return false;
	}

	for (const FFastCrimItem& Entry : ItemList.GetItems())
	{
		if (Entry.Item.GetPtr<FCrimItem>()->GetItemGuid() == ItemId)
		{
			return true;
		}
	}
	
	return false;
}

int32 UCrimItemContainer::GetRemainingCapacityForItem(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!TestItem.IsValid())
	{
		return 0;
	}

	int32 AvailableStacks = FMath::Min(GetRemainingCapacity(), GetItemContainerLimit(TestItem));
	
	const int32 StackMaxQuantity = GetItemQuantityLimit(TestItem);
	int32 AvailableQuantity = 0;
	if (!FMath::MultiplyAndCheckForOverflow(AvailableStacks, StackMaxQuantity, AvailableQuantity))
	{
		AvailableQuantity = MAX_int32;
	}
	
	for (const FFastCrimItem& Entry : ItemList.GetItems())
	{
		if (Entry.Item.GetPtr<FCrimItem>()->IsMatching(TestItem))
		{
			AvailableQuantity = AvailableQuantity - Entry.Item.GetPtr<FCrimItem>()->Quantity;
		}
	}
	
	return AvailableQuantity;
}

bool UCrimItemContainer::CanContainItem(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!TestItem.IsValid())
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

bool UCrimItemContainer::CanRemoveItem(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!TestItem.IsValid())
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

int32 UCrimItemContainer::GetItemContainerLimit(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (TestItem.IsValid())
	{
		int32 Result = UCrimItemStatics::GetItemDefinition(TestItem)->ContainerLimit.GetMaxQuantity();
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

bool UCrimItemContainer::IsItemAtContainerLimit(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (TestItem.IsValid())
	{
		TArray<FFastCrimItem*> Items = GetItemsFromDefinition(UCrimItemStatics::GetItemDefinition(TestItem));
		int32 MaxStacks = GetItemContainerLimit(TestItem);

		if (Items.Num() >= MaxStacks)
		{
			return true;
		}
	}
	
	return false;
}

int32 UCrimItemContainer::GetItemQuantityLimit(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (TestItem.IsValid())
	{
		int32 Result = UCrimItemStatics::GetItemDefinition(TestItem)->StackLimit.GetMaxQuantity();
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

bool UCrimItemContainer::HasAuthority() const
{
	return bOwnerIsNetAuthority;
}

FCrimAddItemPlanResult UCrimItemContainer::TryAddItem(const TInstancedStruct<FCrimItem>& Item, int32 Quantity)
{
	FCrimAddItemPlanResult Result;
	Result.AmountToGive = Quantity;
	if (!HasAuthority())
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error;
		return Result;
	}

	if (!Item.IsValid() ||
		Item.GetPtr<FCrimItem>()->ItemDefinition.IsNull())
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error_InvalidItem;
		return Result;
	}

	Result = GetAddItemPlan(Item, Quantity,
		ItemManagerComponent->GetRemainingCollectionCapacityForItem(UCrimItemStatics::GetItemDefinition(Item)));
	ExecuteAddItemPlan(Result,  Item);
	return Result;
}

TInstancedStruct<FCrimItem> UCrimItemContainer::AddItem(const TInstancedStruct<FCrimItem>& Item, int32 Quantity)
{
	if (!HasAuthority() || !Item.IsValid() ||
		Item.GetPtr<FCrimItem>()->ItemDefinition.IsNull())
	{
		return TInstancedStruct<FCrimItem>();
	}

	TInstancedStruct<FCrimItem> NewItemInstance = Item;
	NewItemInstance.GetMutablePtr<FCrimItem>()->ItemGuid = FGuid::NewGuid();
	NewItemInstance.GetMutablePtr<FCrimItem>()->Quantity = Quantity;
	AddItemToItemContainer(NewItemInstance);
	return NewItemInstance;
}

FCrimAddItemPlanResult UCrimItemContainer::TryAddItemFromSpec(const TInstancedStruct<FCrimItemSpec>& ItemSpec)
{
	FCrimAddItemPlanResult Result;
	if (!HasAuthority() || !ItemSpec.IsValid())
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error;
		return Result;
	}

	const FCrimItemSpec& Spec = ItemSpec.Get<FCrimItemSpec>();
	if (!IsValid(Spec.ItemDef) ||
		Spec.Quantity <= 0)
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error_InvalidItem;
		return Result;
	}

	// Create a temporary item for the ExecuteAddItemPlan.
	TInstancedStruct<FCrimItem> TempItem;
	if (CreateItem(Spec.ItemDef, FGuid::NewGuid(), TempItem))
	{
		FCrimItem* TempItemPtr = TempItem.GetMutablePtr<FCrimItem>();
		TempItemPtr->ApplyItemSpec(ItemSpec);
		Result = GetAddItemPlan(TempItem, Spec.Quantity, ItemManagerComponent->GetRemainingCollectionCapacityForItem(Spec.ItemDef));
		ExecuteAddItemPlan(Result, TempItem);
	}
	return Result;
}

TInstancedStruct<FCrimItem> UCrimItemContainer::AddItemFromSpec(const TInstancedStruct<FCrimItemSpec>& ItemSpec)
{
	if (!HasAuthority() || !ItemSpec.IsValid())
	{
		return TInstancedStruct<FCrimItem>();
	}
	
	const FCrimItemSpec* ConstSpec = ItemSpec.GetPtr<FCrimItemSpec>();
	if (!IsValid(ConstSpec->ItemDef))
	{
		return TInstancedStruct<FCrimItem>();
	}

	TInstancedStruct<FCrimItem> NewItem;
	if (CreateItem(ConstSpec->ItemDef, FGuid::NewGuid(), NewItem))
	{
		NewItem.GetMutablePtr<FCrimItem>()->ApplyItemSpec(ItemSpec);
		AddItemToItemContainer(NewItem);
		return NewItem;
	}
	return TInstancedStruct<FCrimItem>();
}

FCrimAddItemPlanResult UCrimItemContainer::TryAddItemFromDefinition(const UCrimItemDefinition* ItemDef, int32 Quantity)
{
	FCrimAddItemPlanResult Result;
	if (!HasAuthority())
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error;
		return Result;
	}

	if (!ItemDef || Quantity <= 0)
	{
		Result.Error = FCrimItemGameplayTags::Get().Crim_ItemPlan_Error_InvalidItem;
		return Result;
	}

	// Create a temporary item for copying.
	TInstancedStruct<FCrimItem> TempItem;
	if (CreateItem(ItemDef, FGuid::NewGuid(), TempItem))
	{
		TempItem.GetMutablePtr<FCrimItem>()->ApplyItemDef(ItemDef);
		for (const TInstancedStruct<FCrimItemFragment>& Fragment : ItemDef->Fragments)
		{
			Fragment.Get<FCrimItemFragment>().SetDefaultValues(TempItem);
		}
		Result = GetAddItemPlan(TempItem, Quantity, ItemManagerComponent->GetRemainingCollectionCapacityForItem(ItemDef));
		ExecuteAddItemPlan(Result, TempItem);
	}
	return Result;
}

TInstancedStruct<FCrimItem> UCrimItemContainer::AddItemFromDefinition(const UCrimItemDefinition* ItemDef, int32 Quantity)
{
	if (!HasAuthority() || !ItemDef || Quantity <= 0)
	{
		return TInstancedStruct<FCrimItem>();
	}

	TInstancedStruct<FCrimItem> NewItem;
	if (CreateItem(ItemDef, FGuid::NewGuid(), NewItem))
	{
		FCrimItem* ItemPtr = NewItem.GetMutablePtr<FCrimItem>();
		ItemPtr->Quantity = Quantity;
		ItemPtr->ApplyItemDef(ItemDef);
		for (const TInstancedStruct<FCrimItemFragment>& Fragment : ItemDef->Fragments)
		{
			Fragment.Get<FCrimItemFragment>().SetDefaultValues(NewItem);
		}
		AddItemToItemContainer(NewItem);
		return NewItem;
	}

	return TInstancedStruct<FCrimItem>();
}

void UCrimItemContainer::ConsumeItem(const FGuid& ItemId, int32 Quantity)
{
	if (!ItemId.IsValid() || !HasAuthority() || Quantity <= 0)
	{
		return;
	}
	
	FFastCrimItem* FastItem = ItemList.GetItem(ItemId);

	if (FastItem == nullptr)
	{
		return;
	}

	FCrimItem* MutableItem = FastItem->Item.GetMutablePtr<FCrimItem>();
	
	const int32 NewQuantity = FMath::Max(MutableItem->Quantity - Quantity, 0);
	MutableItem->Quantity = NewQuantity;
	if (NewQuantity <= 0)
	{
		RemoveFromItemList(ItemId);
	}
	else
	{
		MarkItemDirty(*FastItem);
	}
}

void UCrimItemContainer::ConsumeItemsByDefinition(const UCrimItemDefinition* ItemDef, int32 Quantity)
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
			RemoveFromItemList(Item->GetItemGuid());
		}
		else
		{
			MarkItemDirty(*FastItem);
		}

		if (QuantityRemaining == 0)
		{
			return;
		}
	}
}

bool UCrimItemContainer::RemoveItem(const FGuid& ItemId)
{
	if (!HasAuthority() || !ItemId.IsValid())
	{
		return false;
	}

	FFastCrimItem* FastItem = GetItemFromGuid(ItemId);
	if (FastItem == nullptr)
	{
		return false;
	}

	if (!CanRemoveItem(FastItem->Item))
	{
		return false;
	}

	RemoveFromItemList(ItemId);
	return true;
}

bool UCrimItemContainer::CanMoveItemToContainer(const TInstancedStruct<FCrimItem>& TestItem,
	const UCrimItemContainer* TargetContainer, int32 Quantity, FCrimAddItemPlanResult& OutPlan) const
{
	//--------------------------------------------------------------------------------
	// 1. Validation checks to ensure everything that needs to be valid is and the TestItem and TargetContainer are
	// owned by this ItemManager.
	//--------------------------------------------------------------------------------
	if (!TestItem.IsValid() || !IsValid(TargetContainer))
	{
		return false;
	}

	const FCrimItem* TestItemPtr = TestItem.GetPtr<FCrimItem>();

	if (TestItemPtr->GetItemContainer() == TargetContainer)
	{
		return false;
	}

	//--------------------------------------------------------------------------------
	// 2. Get the plan on how to add the item to the TargetContainer.
	//--------------------------------------------------------------------------------
	OutPlan = FCrimAddItemPlanResult(Quantity);

	int32 RemainingCollectionLimit = ItemManagerComponent->GetRemainingCollectionCapacityForItem(
		UCrimItemStatics::GetItemDefinition(TestItem));
	if (RemainingCollectionLimit == 0)
	{
		// If the collection limit is at max. Check to see if we can move all of the item to the TargetContainer.
		OutPlan = TargetContainer->GetAddItemPlan(TestItem, Quantity, 1);
		if (OutPlan.Result != ECrimItemAddResult::AllItemsAdded)
		{
			return false;
		}
	}
	else
	{
		OutPlan = TargetContainer->GetAddItemPlan(TestItem, Quantity, RemainingCollectionLimit);
	}

	if (!OutPlan.IsValid())
	{
		return false;
	}

	return true;
}

void UCrimItemContainer::MoveItemToContainer(const FGuid& ItemId, UCrimItemContainer* TargetContainer, int32 Quantity)
{
	if (!HasAuthority() || !ItemId.IsValid())
	{
		return;
	}

	FFastCrimItem* FastItem = GetItemFromGuid(ItemId);
	if (FastItem == nullptr)
	{
		return;
	}

	FCrimAddItemPlanResult ItemAddPlan;
	if (!CanMoveItemToContainer(FastItem->Item, TargetContainer, Quantity, ItemAddPlan))
	{
		return;
	}
	
	FCrimItem* MutableItem = FastItem->Item.GetMutablePtr<FCrimItem>();

	//--------------------------------------------------------------------------------
	// Execute the valid plan. Do we just update quantities for both? Do we destroy the Item when it's
	// quantity reaches 0? Or do we copy the item over to the TargetContainer.
	//--------------------------------------------------------------------------------
	for (const FCrimAddItemPlanResultValue& Entry : ItemAddPlan.GetSlotPlans())
	{
		if (Entry.FastItemPtr == nullptr)
		{
			if (MutableItem->Quantity - Entry.QuantityToAdd == 0)
			{
				// The amount added to the TargetContainer would result in 0 in the Item's current container.
				// Make an exact copy of the item for the TargetContainer.
				TInstancedStruct<FCrimItem> NewItem = FastItem->Item;
				RemoveFromItemList(ItemId);
				TargetContainer->AddItemToItemContainer(NewItem);
			}
			else
			{
				// The amount added would result in a leftover amount in the original Container make a copy of the item.
				MutableItem->Quantity = MutableItem->Quantity - Entry.QuantityToAdd;
				MarkItemDirty(*FastItem);
				TInstancedStruct<FCrimItem> NewItem = FastItem->Item;
				NewItem.GetMutablePtr<FCrimItem>()->ItemGuid = FGuid::NewGuid();
				NewItem.GetMutablePtr<FCrimItem>()->Quantity = Entry.QuantityToAdd;
				TargetContainer->AddItemToItemContainer(NewItem);
			}
		}
		else
		{
			// The TargetContainer has a matching item.
			FCrimItem* TargetItemPtr = Entry.FastItemPtr->Item.GetMutablePtr<FCrimItem>();
			TargetItemPtr->Quantity = TargetItemPtr->Quantity + Entry.QuantityToAdd;
			MutableItem->Quantity = MutableItem->Quantity - Entry.QuantityToAdd;

			MarkItemDirty(*FastItem);
			TargetContainer->MarkItemDirty(*Entry.FastItemPtr);
			
			if (MutableItem->Quantity <= 0)
			{
				RemoveFromItemList(ItemId);
			}
		}
	}
}

bool UCrimItemContainer::CanSplitItemStack(const TInstancedStruct<FCrimItem>& TestItem, int32 Quantity) const
{
	if (!TestItem.IsValid())
	{
		return false;
	}

	const FCrimItem* TestItemPtr = TestItem.GetPtr<FCrimItem>();

	if (TestItemPtr->Quantity <= 1)
	{
		return false;
	}

	if (TestItemPtr->Quantity <= Quantity)
	{
		return false;
	}

	//--------------------------------------------------------
	// Check if at max capacity for item.
	//--------------------------------------------------------
	int32 RemainingCollectionCapacity = ItemManagerComponent->GetRemainingCollectionCapacityForItem(
		UCrimItemStatics::GetItemDefinition(TestItem));
	
	if (RemainingCollectionCapacity <= 0)
	{
		return false;
	}

	if (IsAtMaxCapacity() || IsItemAtContainerLimit(TestItem))
	{
		return false;
	}

	return true;
}

void UCrimItemContainer::SplitItemStack(const FGuid& ItemId, int32 Quantity)
{
	if (!HasAuthority() || !ItemId.IsValid())
	{
		return;
	}

	FFastCrimItem* FastItem = GetItemFromGuid(ItemId);

	if (FastItem == nullptr)
	{
		return;
	}

	if (!CanSplitItemStack(FastItem->Item, Quantity))
	{
		return;
	}

	FCrimItem* SourceItem = FastItem->Item.GetMutablePtr<FCrimItem>();

	TInstancedStruct<FCrimItem> NewItem = FastItem->Item;
	NewItem.GetMutablePtr<FCrimItem>()->ItemGuid = FGuid::NewGuid();
	NewItem.GetMutablePtr<FCrimItem>()->Quantity = Quantity;
	
	SourceItem->Quantity = SourceItem->Quantity - Quantity;
	MarkItemDirty(*FastItem);
	
	AddItemToItemContainer(NewItem);
}

bool UCrimItemContainer::CanStackItems(const TInstancedStruct<FCrimItem>& SourceItem,
	const TInstancedStruct<FCrimItem>& TargetItem, int32& OutMaxQuantity) const
{
	OutMaxQuantity = 0;

	if (!SourceItem.IsValid() || !TargetItem.IsValid())
	{
		return false;
	}

	if (SourceItem == TargetItem)
	{
		return false;
	}

	const FCrimItem* SourceItemPtr = SourceItem.GetPtr<FCrimItem>();
	const FCrimItem* TargetItemPtr = TargetItem.GetPtr<FCrimItem>();

	int32 MaxQuantity = GetItemQuantityLimit(SourceItem);
	if (MaxQuantity <= 1)
	{
		return false;
	}

	int32 TargetAvailableSpace = MaxQuantity - TargetItemPtr->Quantity;
	OutMaxQuantity = FMath::Min(TargetAvailableSpace, SourceItemPtr->Quantity);
	return OutMaxQuantity > 0;
}

void UCrimItemContainer::StackItems(const FGuid& SourceItemId, const FGuid& TargetItemId, int32 Quantity)
{
	if (!HasAuthority() || !SourceItemId.IsValid() || !TargetItemId.IsValid())
	{
		return;
	}

	FFastCrimItem* SourceFastItem = GetItemFromGuid(SourceItemId);
	FFastCrimItem* TargetFastItem = GetItemFromGuid(TargetItemId);

	if (SourceFastItem == nullptr || TargetFastItem == nullptr)
	{
		return;
	}

	int32 MaxTransferAmount = 0;
	if (!CanStackItems(SourceFastItem->Item, TargetFastItem->Item, MaxTransferAmount))
	{
		return;
	}

	int32 TransferAmount = FMath::Min(MaxTransferAmount, Quantity);
	
	FCrimItem* SourceItemPtr = SourceFastItem->Item.GetMutablePtr<FCrimItem>();
	FCrimItem* TargetItemPtr = TargetFastItem->Item.GetMutablePtr<FCrimItem>();
	SourceItemPtr->Quantity = SourceItemPtr->Quantity - TransferAmount;
	TargetItemPtr->Quantity = TargetItemPtr->Quantity + TransferAmount;
	MarkItemDirty(*SourceFastItem);
	MarkItemDirty(*TargetFastItem);

	if (SourceItemPtr->Quantity <= 0)
	{
		RemoveFromItemList(SourceItemPtr->GetItemGuid());
	}
}

void UCrimItemContainer::MarkItemDirty(FFastCrimItem& FastItem)
{
	if (HasAuthority() &&
		FastItem.Item.GetPtr<FCrimItem>()->ItemContainer == this)
	{
		ItemList.MarkItemDirty(FastItem);
		ItemList.OnItemChangedDelegate.Broadcast(FastItem);
		FastItem.PreReplicatedChangeItem = FastItem.Item;
	}
	else
	{
		// We are client so we mark the array dirty to force rebuild.
		ItemList.MarkArrayDirty();
	}
}

void UCrimItemContainer::Initialize(UCrimItemManagerComponent* ItemManager, FGameplayTag NewContainerId)
{
	ItemManagerComponent = ItemManager;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemManagerComponent, this);
	bOwnerIsNetAuthority = ItemManagerComponent->HasAuthority();

	ContainerId = NewContainerId;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ContainerId, this);
}

bool UCrimItemContainer::CreateItem(const UCrimItemDefinition* ItemDef, FGuid ItemId, TInstancedStruct<FCrimItem>& OutItemInstance)
{
	if (!HasAuthority() || !ItemDef)
	{
		return false;
	}

	if (!ItemId.IsValid())
	{
		ItemId = FGuid::NewGuid();
	}

	OutItemInstance.InitializeAsScriptStruct(ItemDef->ItemClass.GetScriptStruct());
	OutItemInstance.GetMutablePtr<FCrimItem>()->Initialize(ItemId, ItemDef);
	return true;
}

void UCrimItemContainer::ExecuteAddItemPlan(FCrimAddItemPlanResult& Plan, const TInstancedStruct<FCrimItem>& TemplateItem)
{
	if (Plan.IsValid())
	{
		for (const FCrimAddItemPlanResultValue& Slot : Plan.GetSlotPlans())
		{
			if (Slot.FastItemPtr)
			{
				FCrimItem* SlotItem = Slot.FastItemPtr->Item.GetMutablePtr<FCrimItem>();
				SlotItem->Quantity = SlotItem->Quantity + Slot.QuantityToAdd;
				MarkItemDirty(*Slot.FastItemPtr);
			}
			else
			{
				TInstancedStruct<FCrimItem> NewItem = TemplateItem;
				NewItem.GetMutablePtr<FCrimItem>()->ItemGuid = FGuid::NewGuid();
				NewItem.GetMutablePtr<FCrimItem>()->Quantity = Slot.QuantityToAdd;
				AddItemToItemContainer(NewItem);
			}
		}
	}
}

bool UCrimItemContainer::CanAddItem(const TInstancedStruct<FCrimItem>& Item, FCrimAddItemPlanResult& ItemPlan, int32 ItemQuantity) const
{
	const FCrimItemGameplayTags& Tags = FCrimItemGameplayTags::Get();

	if (!Item.IsValid())
	{
		ItemPlan.Error = Tags.Crim_ItemPlan_Error_InvalidItem;
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

	return true;
}

void UCrimItemContainer::AddItemToItemContainer(TInstancedStruct<FCrimItem>& Item)
{
	Item.GetMutablePtr<FCrimItem>()->ItemContainer = this;
	Item.GetMutablePtr<FCrimItem>()->ItemManager = ItemManagerComponent;
	ItemList.AddItem(Item);
}

void UCrimItemContainer::RemoveFromItemList(const FGuid& ItemId)
{
	ItemList.RemoveItem(ItemId);
}

void UCrimItemContainer::BindToItemListDelegates()
{
	ItemList.OnItemAddedDelegate.AddWeakLambda(this, [this](const FFastCrimItem& Item)
	{
		OnItemAdded(Item);
		K2_OnItemAdded(Item);
		OnItemAddedDelegate.Broadcast(this, Item);
	});
	ItemList.OnItemRemovedDelegate.AddWeakLambda(this, [this](const FFastCrimItem& Item)
	{
		OnItemRemoved(Item);
		K2_OnItemRemoved(Item);
		OnItemRemovedDelegate.Broadcast(this, Item);
	});
	ItemList.OnItemChangedDelegate.AddWeakLambda(this, [this](const FFastCrimItem& Item)
	{
		OnItemChanged(Item);
		K2_OnItemChanged(Item);
		OnItemChangedDelegate.Broadcast(this, Item);
	});
}
