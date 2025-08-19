// Copyright Soccertitan


#include "ItemContainer/CrimItemContainer.h"

#include "CrimItemGameplayTags.h"
#include "CrimItemDefinition.h"
#include "CrimItemManagerComponent.h"
#include "CrimItemStatics.h"
#include "ItemContainer/ItemContainerRules/CrimItemContainerRule.h"
#include "ItemDefinitionFragment/CrimItemDefFrag_QuantityLimit.h"
#include "Net/UnrealNetwork.h"
#include "UI/ViewModel/CrimItemContainerViewModel.h"

UCrimItemContainer::UCrimItemContainer()
{
	ViewModelClass = UCrimItemContainerViewModel::StaticClass();
}

void UCrimItemContainer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, CapacityLimit, Params);
}

bool UCrimItemContainer::CanAddItem(const TInstancedStruct<FCrimItem>& Item, FGameplayTag& OutError) const
{
	if (!Super::CanAddItem(Item, OutError))
	{
		return false;
	}

	for (const TObjectPtr<UCrimItemContainerRule>& Rule : ItemContainerRules)
	{
		if (!Rule->CanAddItem(this, Item, OutError))
		{
			return false;
		}
	}

	if (Item.Get<FCrimItem>().Quantity <= 0)
	{
		OutError = FCrimItemGameplayTags::Get().ItemPlan_Error_QuantityIsZero;
		return false;
	}
	return true;
}

bool UCrimItemContainer::CanRemoveItem(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!Super::CanRemoveItem(TestItem))
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

int32 UCrimItemContainer::SetMaxCapacity(int32 NewCount, bool bOverride)
{
	if (HasAuthority())
	{
		const int32 OldLimit = CapacityLimit.MaxQuantity;
		CapacityLimit.bLimitQuantity = true;
		if (bOverride && NewCount >= 0)
		{
			CapacityLimit.MaxQuantity = NewCount;
		}
		else
		{
			int32 AdjustedMaxCapacity = FMath::Max(0, CapacityLimit.MaxQuantity + NewCount);
			CapacityLimit.MaxQuantity = AdjustedMaxCapacity;
		}
		
		if (OldLimit != CapacityLimit.GetMaxQuantity())
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CapacityLimit, this);
		}
	}
	return CapacityLimit.GetMaxQuantity();
}

int32 UCrimItemContainer::GetMaxCapacity() const
{
	return CapacityLimit.GetMaxQuantity();
}

int32 UCrimItemContainer::GetConsumedCapacity() const
{
	return GetItemList().GetNum();
}

int32 UCrimItemContainer::GetRemainingCapacity() const
{
	return GetMaxCapacity() - GetConsumedCapacity();
}

bool UCrimItemContainer::IsAtMaxCapacity() const
{
	return GetConsumedCapacity() >= GetMaxCapacity();
}

int32 UCrimItemContainer::GetRemainingCollectionCapacityForItem(const TInstancedStruct<FCrimItem>& TestItem) const
{
	int32 MaxQuantity = MAX_int32;

	const FCrimItemDefFrag_QuantityLimit* Fragment = UCrimItemStatics::GetItemDefinitionFragmentByType<FCrimItemDefFrag_QuantityLimit>(TestItem);
	if (Fragment)
	{
		MaxQuantity = Fragment->CollectionLimit.GetMaxQuantity();
	}

	int32 ItemCount = GetItemManagerComponent()->GetItemsByDefinition(UCrimItemStatics::GetItemDefinition(TestItem)).Num();
	
	return MaxQuantity - ItemCount;
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
	
	for (const FFastCrimItem* Entry : GetItemsByDefinition(UCrimItemStatics::GetItemDefinition(TestItem)))
	{
		AvailableQuantity = AvailableQuantity - Entry->Item.GetPtr<FCrimItem>()->Quantity;
	}
	
	return AvailableQuantity;
}

int32 UCrimItemContainer::GetItemContainerLimit(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (TestItem.IsValid())
	{
		const FCrimItemDefFrag_QuantityLimit* Fragment = UCrimItemStatics::GetItemDefinitionFragmentByType<FCrimItemDefFrag_QuantityLimit>(TestItem);
		int32 Result = Fragment ? Fragment->ContainerLimit.GetMaxQuantity() : MAX_int32;
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
		TArray<FFastCrimItem*> Items = GetItemsByDefinition(UCrimItemStatics::GetItemDefinition(TestItem));
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
		const FCrimItemDefFrag_QuantityLimit* Fragment = UCrimItemStatics::GetItemDefinitionFragmentByType<FCrimItemDefFrag_QuantityLimit>(TestItem);
		int32 Result = Fragment ? Fragment->StackLimit.GetMaxQuantity() : MAX_int32;
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
	int32 RemainingCollectionCapacity = GetRemainingCollectionCapacityForItem(TestItem);
	
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

	FFastCrimItem* FastItem = GetItemByGuid(ItemId);

	if (FastItem == nullptr)
	{
		return;
	}

	if (!CanSplitItemStack(FastItem->Item, Quantity))
	{
		return;
	}

	FCrimItem* SourceItem = FastItem->Item.GetMutablePtr<FCrimItem>();

	TInstancedStruct<FCrimItem> NewItem = DuplicateItem(FastItem->Item);
	NewItem.GetMutablePtr<FCrimItem>()->Quantity = Quantity;
	SourceItem->Quantity = SourceItem->Quantity - Quantity;
	MarkItemDirty(*FastItem);
	
	Internal_AddItem(NewItem);
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

	FFastCrimItem* SourceFastItem = GetItemByGuid(SourceItemId);
	FFastCrimItem* TargetFastItem = GetItemByGuid(TargetItemId);

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
		Internal_RemoveItem(SourceItemPtr->GetItemGuid());
	}
}



FCrimAddItemPlan UCrimItemContainer::GetAddItemPlan(const TInstancedStruct<FCrimItem>& Item) const
{
	FCrimAddItemPlan Result(Item.Get<FCrimItem>().Quantity);

	const FCrimItemGameplayTags& Tags = FCrimItemGameplayTags::Get();

	//----------------------------------------------------------------------------------------------
	// 1. Determine how many quantities of this item we can add to this container.
	//----------------------------------------------------------------------------------------------
	int32 RemainingQuantityToAdd = FMath::Min(GetRemainingCapacityForItem(Item), Result.AmountToGive);
	const int32 ItemStackMaxQuantity = GetItemQuantityLimit(Item);

	//----------------------------------------------------------------------------------------------
	// 2. If auto stacking gather a list of all matching items in the list. We only try to auto
	// stack if the ItemStackMaxQuantity is greater than 1 and auto stacking is enabled.
	//----------------------------------------------------------------------------------------------
	TArray<FFastCrimItem*> MatchingItems = TArray<FFastCrimItem*>();
	if (bAutoStack && ItemStackMaxQuantity > 1)
	{
		MatchingItems = FindMatchingItems(Item);
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
			Result.AddEntry(FCrimAddItemPlanEntry(Match, QuantityToAdd));
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
	const int32 MaxNewItemStacks = GetRemainingCollectionCapacityForItem(Item);
	while (RemainingQuantityToAdd > 0 && NewStacksAdded < MaxNewItemStacks)
	{
		const int32 QuantityToAdd = FMath::Min(RemainingQuantityToAdd, ItemStackMaxQuantity);
		RemainingQuantityToAdd = RemainingQuantityToAdd - QuantityToAdd;
	
		Result.AddEntry(FCrimAddItemPlanEntry(nullptr, QuantityToAdd));
		NewStacksAdded++;
	}

	if (Result.Result == ECrimAddItemResult::SomeItemsAdded)
	{
		Result.Error = Tags.ItemPlan_Error_MaxStacksReached;
	}
	
	return Result;
}
