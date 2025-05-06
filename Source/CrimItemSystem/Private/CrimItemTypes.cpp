// Copyright Soccertitan


#include "CrimItemTypes.h"

#include "CrimItemFastTypes.h"


int32 FCrimItemQuantityLimit::GetMaxQuantity() const
{
	return bLimitQuantity ? MaxQuantity : MAX_int32;
}

bool FCrimAddItemPlanResultValue::IsValid() const
{
	if (QuantityToAdd <= 0)
	{
		return false;
	}
	
	return true;
}

bool FCrimAddItemPlanResult::IsValid() const
{
	if (SlotPlans.Num() == 0)
	{
		return false;
	}
	for (const FCrimAddItemPlanResultValue& Entry : SlotPlans)
	{
		if (!Entry.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FCrimAddItemPlanResult::AddSlotPlan(const FCrimAddItemPlanResultValue& InSlotPlan)
{
	if (!InSlotPlan.IsValid())
	{
		return;
	}
	
	for (FCrimAddItemPlanResultValue& Plan : SlotPlans)
	{
		if (Plan.FastItemPtr != nullptr &&
			Plan.FastItemPtr == InSlotPlan.FastItemPtr)
		{
			UpdateAmountGiven(-Plan.QuantityToAdd + InSlotPlan.QuantityToAdd);
			Plan = InSlotPlan;
			return;
		}
	}
	UpdateAmountGiven(InSlotPlan.QuantityToAdd);
	SlotPlans.Add(InSlotPlan);
}

const TArray<FCrimAddItemPlanResultValue>& FCrimAddItemPlanResult::GetSlotPlans() const
{
	return SlotPlans;
}

void FCrimAddItemPlanResult::UpdateAmountGiven(int32 NewValue)
{
	AmountGiven = AmountGiven + NewValue;

	if (AmountGiven <= 0)
	{
		Result = ECrimItemAddResult::NoItemsAdded;
	}
	else if (AmountGiven < AmountToGive)
	{
		Result = ECrimItemAddResult::SomeItemsAdded;
	}
	else
	{
		Result = ECrimItemAddResult::AllItemsAdded;
	}
}

//----------------------------------------------------------------------------------------
// CrimItemTagStack
//----------------------------------------------------------------------------------------
FString FCrimItemTagStack::ToDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), Count);
}

const FGameplayTag& FCrimItemTagStack::GetTag() const
{
	return Tag;
}

int32 FCrimItemTagStack::GetCount() const
{
	return Count;
}

//----------------------------------------------------------------------------------------
// CrimItemTagStackContainer
//----------------------------------------------------------------------------------------

void FCrimItemTagStackContainer::AddStack(FGameplayTag Tag, int32 DeltaCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}

	if (DeltaCount == 0)
	{
		FFrame::KismetExecutionMessage(TEXT("Unable to add 0 to AddStack"), ELogVerbosity::Warning);
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FCrimItemTagStack& Stack = *It;
		
		if (Stack.Tag == Tag)
		{
			if (Stack.Count + DeltaCount == 0)
			{
				// remove the tag entirely since the value is 0.
				It.RemoveCurrent();
				StackCountMap.Remove(Tag);
				OnTagCountUpdatedDelegate.Broadcast(Stack.Tag, 0, Stack.LastObservedCount);
				MarkArrayDirty();
			}
			else
			{
				Stack.LastObservedCount = Stack.Count;
				Stack.Count += DeltaCount;
				StackCountMap[Tag] = Stack.Count;
				OnTagCountUpdatedDelegate.Broadcast(Stack.Tag, Stack.Count, Stack.LastObservedCount);
				MarkItemDirty(Stack);
			}
			return;
		}
	}

	FCrimItemTagStack& NewStack = Items.Emplace_GetRef(Tag, DeltaCount);
	NewStack.LastObservedCount = 0;
	OnTagCountUpdatedDelegate.Broadcast(NewStack.Tag, NewStack.Count, 0);
	MarkItemDirty(NewStack);
	StackCountMap.Add(Tag, DeltaCount);
}

void FCrimItemTagStackContainer::SubtractStack(FGameplayTag Tag, int32 DeltaCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to SubtractStack"), ELogVerbosity::Warning);
		return;
	}

	if (DeltaCount == 0)
	{
		FFrame::KismetExecutionMessage(TEXT("Unable to subtract 0 to SubtractStack"), ELogVerbosity::Warning);
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FCrimItemTagStack& Stack = *It;
		if (Stack.Tag == Tag)
		{
			Stack.LastObservedCount = Stack.Count;
			if (Stack.Count - DeltaCount == 0)
			{
				// remove the tag entirely since the value is 0.
				It.RemoveCurrent();
				StackCountMap.Remove(Tag);
				OnTagCountUpdatedDelegate.Broadcast(Stack.Tag, 0, Stack.LastObservedCount);
				MarkArrayDirty();
			}
			else
			{
				// decrease the stack count
				Stack.Count -= DeltaCount;
				StackCountMap[Tag] = Stack.Count;
				OnTagCountUpdatedDelegate.Broadcast(Stack.Tag, Stack.Count, Stack.LastObservedCount);
				MarkItemDirty(Stack);
			}
			return;
		}
	}
}

void FCrimItemTagStackContainer::RemoveStack(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FCrimItemTagStack& Stack = *It;
		if (Stack.Tag == Tag)
		{
			It.RemoveCurrent();
			StackCountMap.Remove(Tag);
			OnTagCountUpdatedDelegate.Broadcast(Stack.Tag, 0, Stack.LastObservedCount);
			MarkArrayDirty();
			return;
		}
	}
}

void FCrimItemTagStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Idx : RemovedIndices)
	{
		FCrimItemTagStack& Stack = Items[Idx];
		StackCountMap.Remove(Stack.Tag);
		OnTagCountUpdatedDelegate.Broadcast(Stack.Tag, 0, Stack.Count);
	}
}

void FCrimItemTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		FCrimItemTagStack& Stack = Items[Idx];
		StackCountMap.Add(Stack.Tag, Stack.Count);
		Stack.LastObservedCount = Stack.Count;
		OnTagCountUpdatedDelegate.Broadcast(Stack.Tag, Stack.Count, 0);
	}
}

void FCrimItemTagStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Idx : ChangedIndices)
	{
		FCrimItemTagStack& Stack = Items[Idx];
		StackCountMap[Stack.Tag] = Stack.Count;
		OnTagCountUpdatedDelegate.Broadcast(Stack.Tag, Stack.Count, Stack.LastObservedCount);
		Stack.LastObservedCount = Stack.Count;
	}
}

void FCrimItemTagStackContainer::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading())
	{
		// update StackCountMap after load
		StackCountMap.Reset();
		for (const FCrimItemTagStack& Stack : Items)
		{
			StackCountMap.Add(Stack.Tag, Stack.Count);
		}
	}
}

FString FCrimItemTagStackContainer::ToDebugString() const
{
	TArray<FString> StackStrings;
	for (const FCrimItemTagStack& Stack : Items)
	{
		StackStrings.Add(Stack.ToDebugString());
	}
	return FString::Join(StackStrings, TEXT(", "));
}

const TArray<FCrimItemTagStack>& FCrimItemTagStackContainer::GetTagStats() const
{
	return Items;
}

bool FCrimItemTagStackContainer::ContainsTag(FGameplayTag Tag, bool bExactMatch) const
{
	if (bExactMatch)
	{
		return StackCountMap.Contains(Tag);
	}

	for (const auto& Entry : StackCountMap)
	{
		if(Entry.Key.MatchesTag(Tag))
		{
			return true;
		}
	}
	return false;
}

void FCrimItemTagStackContainer::Empty()
{
	Items.Empty();
	MarkArrayDirty();
	StackCountMap.Empty();
}
