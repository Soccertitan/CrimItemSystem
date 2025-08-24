// Copyright Soccertitan


#include "CrimItemTypes.h"

#include "CrimItemFastTypes.h"


int32 FCrimItemQuantityLimit::GetMaxQuantity() const
{
	return bLimitQuantity ? MaxQuantity : MAX_int32;
}

bool FCrimAddItemPlanEntry::IsValid() const
{
	if (QuantityToAdd < 0)
	{
		return false;
	}
	
	return true;
}

bool FCrimAddItemPlan::IsValid() const
{
	if (Entries.Num() == 0)
	{
		return false;
	}
	for (const FCrimAddItemPlanEntry& Entry : Entries)
	{
		if (!Entry.IsValid())
		{
			return false;
		}
	}
	return true;
}

void FCrimAddItemPlan::AddEntry(const FCrimAddItemPlanEntry& InEntry)
{
	if (!InEntry.IsValid())
	{
		return;
	}
	
	for (FCrimAddItemPlanEntry& Entry : Entries)
	{
		if (Entry.FastItemPtr != nullptr &&
			Entry.FastItemPtr == InEntry.FastItemPtr)
		{
			UpdateAmountGiven(-Entry.QuantityToAdd + InEntry.QuantityToAdd);
			Entry = InEntry;
			return;
		}
	}
	UpdateAmountGiven(InEntry.QuantityToAdd);
	Entries.Add(InEntry);
}

const TArray<FCrimAddItemPlanEntry>& FCrimAddItemPlan::GetEntries() const
{
	return Entries;
}

void FCrimAddItemPlan::UpdateAmountGiven(int32 NewValue)
{
	AmountGiven = AmountGiven + NewValue;

	if (AmountGiven >= AmountToGive)
	{
		Result = ECrimAddItemResult::AllItemsAdded;
	}
	else if (AmountGiven <= 0)
	{
		Result = ECrimAddItemResult::NoItemsAdded;
	}
	else
	{
		Result = ECrimAddItemResult::SomeItemsAdded;
	}
}

FCrimAddItemResult::FCrimAddItemResult(const FCrimAddItemPlan& InPlan, const TArray<TInstancedStruct<FCrimItem>>& InItems)
{
	AmountToGive = InPlan.AmountToGive;
	AmountGiven = InPlan.AmountGiven;
	Result = InPlan.Result;
	Error = InPlan.Error;
	Items = InItems;
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
		return;
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
			}
			else
			{
				Stack.Count += DeltaCount;
			}
			return;
		}
	}

	Items.Add(FCrimItemTagStack(Tag, DeltaCount));
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
		return;
	}

	for (auto It = Items.CreateIterator(); It; ++It)
	{
		FCrimItemTagStack& Stack = *It;
		if (Stack.Tag == Tag)
		{
			if (Stack.Count - DeltaCount == 0)
			{
				// remove the tag entirely since the value is 0.
				It.RemoveCurrent();
			}
			else
			{
				// decrease the stack count
				Stack.Count -= DeltaCount;
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
			return;
		}
	}
}

int32 FCrimItemTagStackContainer::GetStackCount(FGameplayTag Tag) const
{
	for (const auto& Item : Items)
	{
		if (Item.Tag.MatchesTagExact(Tag))
		{
			return Item.Count;
		}
	}
	return 0;
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
	for (const auto& Item : Items)
	{
		if (bExactMatch)
		{
			if (Item.Tag.MatchesTagExact(Tag))
			{
				return true;
			}
		}
		else
		{
			if(Item.Tag.MatchesTag(Tag))
			{
				return true;
			}	
		}
	}
	return false;
}

void FCrimItemTagStackContainer::Empty()
{
	Items.Empty();
}
