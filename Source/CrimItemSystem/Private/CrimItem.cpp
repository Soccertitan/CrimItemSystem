// Copyright Soccertitan


#include "CrimItem.h"

#include "CrimItemDef.h"
#include "CrimItemManagerComponent.h"
#include "Net/UnrealNetwork.h"

void UCrimItem::PostInitProperties()
{
	UObject::PostInitProperties();

	TagStats.OnTagCountUpdatedDelegate.AddWeakLambda(this, [this](const FGameplayTag& Tag, int32 NewCount, int32 OldCount)
	{
		OnTagStatUpdatedDelegate.Broadcast(Tag, NewCount, OldCount);
	});
}

void UCrimItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemDefinition, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemId, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemManagerComponent, Params);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Quantity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ThisClass, TagStats);
}

#if UE_WITH_IRIS
void UCrimItem::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif

const UCrimItemDef* UCrimItem::GetItemDefinition() const
{
	return ItemDefinition.LoadSynchronous();
}

FText UCrimItem::GetDescription_Implementation() const
{
	return ItemDefinition.LoadSynchronous()->ItemDescription;
}

const FGameplayTagContainer& UCrimItem::GetOwnedTags() const
{
	return ItemDefinition.LoadSynchronous()->OwnedTags;
}

void UCrimItem::SetQuantity(int32 NewQuantity)
{
	if (bOwnerIsNetAuthority)
	{
		if (Quantity != NewQuantity)
		{
			const int32 OldQuantity = Quantity;
			Quantity = NewQuantity;

			OnRep_Quantity(OldQuantity);
		}
	}
}

int32 UCrimItem::GetTagStat(FGameplayTag Tag) const
{
	return TagStats.GetStackCount(Tag);
}

void UCrimItem::AddTagStat(FGameplayTag Tag, int32 Delta)
{
	if (bOwnerIsNetAuthority)
	{
		TagStats.AddStack(Tag, Delta);
	}
}

void UCrimItem::RemoveTagStat(FGameplayTag Tag, int32 Delta)
{
	if (bOwnerIsNetAuthority)
	{
		TagStats.RemoveStack(Tag, Delta);
	}
}

bool UCrimItem::IsMatching(const UCrimItem* TestItem) const
{
	if (!IsValid(TestItem))
	{
		return false;
	}

	if (TestItem->GetItemDefinition() != this->GetItemDefinition())
	{
		return false;
	}

	if (TagStats != TestItem->TagStats)
	{
		return false;
	}
	
	return true;
}

void UCrimItem::Initialize(UCrimItemManagerComponent* ItemManager, const UCrimItemDef* ItemDef, FGuid NewItemId)
{
	ItemManagerComponent = ItemManager;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemManagerComponent, this);
	bOwnerIsNetAuthority = ItemManager->HasAuthority();

	ItemId = NewItemId;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemId, this);

	ItemDefinition = ItemDef->GetPathName();
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemDefinition, this);
}

void UCrimItem::CopyItemProperties(const UCrimItem* Item)
{
	TagStats.Empty();
	for (const FCrimItemTagStack& TagStat : Item->TagStats.GetTagStats())
	{
		TagStats.AddStack(TagStat.GetTag(), TagStat.GetCount());
	}
}

void UCrimItem::ApplyItemSpec(const TInstancedStruct<FCrimItemSpec>& Spec)
{
	const FCrimItemSpec* ConstSpec = Spec.GetPtr<FCrimItemSpec>();
	SetQuantity(ConstSpec->Quantity);

	if (ConstSpec->bApplyDefaultStats)
	{
		for (const TTuple<FGameplayTag, int>& Pair : ConstSpec->ItemDef->DefaultStats)
		{
			TagStats.AddStack(Pair.Key, Pair.Value);
		}
	}

	for (const TTuple<FGameplayTag, int>& Pair : ConstSpec->ItemStats)
	{
		TagStats.AddStack(Pair.Key, Pair.Value);
	}
}

void UCrimItem::ApplyItemDef(const UCrimItemDef* ItemDef)
{
	for (const TTuple<FGameplayTag, int>& Pair : ItemDef->DefaultStats)
	{
		TagStats.AddStack(Pair.Key, Pair.Value);
	}
}

void UCrimItem::OnRep_Quantity(int32 OldValue)
{
	OnQuantityUpdatedDelegate.Broadcast(Quantity, OldValue);
}

FString UCrimItem::ToDebugString() const
{
	return FString::Printf(TEXT("%s (%sx%d)"), *GetName(), *ItemDefinition.ToString(), Quantity);
}
