// Copyright Soccertitan


#include "ItemContainer/CrimItemContainerBase.h"

#include "CrimItemDefinition.h"
#include "CrimItemGameplayTags.h"
#include "CrimItemManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "UI/ViewModel/CrimItemContainerViewModel.h"

UCrimItemContainerBase::UCrimItemContainerBase()
{
	ViewModelClass = UCrimItemContainerViewModelBase::StaticClass();
}

void UCrimItemContainerBase::PostInitProperties()
{
	UObject::PostInitProperties();

	BindToItemListDelegates();
}

void UCrimItemContainerBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ContainerGuid, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemManagerComponent, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemList, Params);
}

#if UE_WITH_IRIS
void UCrimItemContainerBase::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,
	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif

const FGameplayTag& UCrimItemContainerBase::GetContainerGuid() const
{
	return ContainerGuid;
}

const FGameplayTagContainer& UCrimItemContainerBase::GetOwnedTags() const
{
	return OwnedTags;
}

const FFastCrimItemList& UCrimItemContainerBase::GetItemList() const
{
	return ItemList;
}

const TArray<FFastCrimItem>& UCrimItemContainerBase::GetItems() const
{
	return ItemList.GetItems();
}

FFastCrimItem* UCrimItemContainerBase::GetItemByGuid(FGuid ItemGuid) const
{
	return ItemList.GetItem(ItemGuid);
}

TInstancedStruct<FCrimItem> UCrimItemContainerBase::K2_GetItemByGuid(FGuid ItemGuid) const
{
	if (FFastCrimItem* FastItem = ItemList.GetItem(ItemGuid))
	{
		return FastItem->Item;
	}
	
	return TInstancedStruct<FCrimItem>();
}

FFastCrimItem* UCrimItemContainerBase::GetItemByDefinition(const UCrimItemDefinition* ItemDefinition) const
{
	if (ItemDefinition)
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->GetItemDefinition()->GetPathName() == ItemDefinition->GetPathName())
			{
				return const_cast<FFastCrimItem*>(&Entry);
			}
		}
	}
	return nullptr;
}

TInstancedStruct<FCrimItem> UCrimItemContainerBase::K2_GetItemByDefinition(const UCrimItemDefinition* ItemDefinition) const
{
	if (ItemDefinition)
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->GetItemDefinition()->GetPathName() == ItemDefinition->GetPathName())
			{
				return Entry.Item;
			}
		}
	}
	return TInstancedStruct<FCrimItem>();
}

TArray<FFastCrimItem*> UCrimItemContainerBase::GetItemsByDefinition(const UCrimItemDefinition* ItemDefinition) const
{
	TArray<FFastCrimItem*> Items;

	if (ItemDefinition)
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->GetItemDefinition()->GetPathName() == ItemDefinition->GetPathName())
			{
				Items.Add(const_cast<FFastCrimItem*>(&Entry));
			}
		}
	}
	return Items;
}

TArray<TInstancedStruct<FCrimItem>> UCrimItemContainerBase::K2_GetItemsByDefinition(
	const UCrimItemDefinition* ItemDefinition) const
{
	TArray<TInstancedStruct<FCrimItem>> Items;

	if (ItemDefinition)
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->GetItemDefinition()->GetPathName() == ItemDefinition->GetPathName())
			{
				Items.Add(Entry.Item);
			}
		}
	}
	return Items;
}

FFastCrimItem* UCrimItemContainerBase::FindMatchingItem(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (TestItem.IsValid())
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->IsMatching(TestItem))
			{
				return const_cast<FFastCrimItem*>(&Entry);
			}
		}
	}
	return nullptr;
}

TInstancedStruct<FCrimItem> UCrimItemContainerBase::K2_FindMatchingItem( const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (TestItem.IsValid())
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->IsMatching(TestItem))
			{
				return Entry.Item;
			}
		}
	}
	return TInstancedStruct<FCrimItem>();
}

TArray<FFastCrimItem*> UCrimItemContainerBase::FindMatchingItems(const TInstancedStruct<FCrimItem>& TestItem) const
{
	TArray<FFastCrimItem*> Result;
	if (TestItem.IsValid())
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->IsMatching(TestItem))
			{
				Result.Add(const_cast<FFastCrimItem*>(&Entry));
			}
		}
	}
	return Result;
}

TArray<TInstancedStruct<FCrimItem>> UCrimItemContainerBase::K2_FindMatchingItems( const TInstancedStruct<FCrimItem>& TestItem) const
{
	TArray<TInstancedStruct<FCrimItem>> Result;
	if (TestItem.IsValid())
	{
		for (const FFastCrimItem& Entry : ItemList.GetItems())
		{
			if (Entry.Item.GetPtr<FCrimItem>()->IsMatching(TestItem))
			{
				Result.Add(Entry.Item);
			}
		}
	}
	return Result;
}

TInstancedStruct<FCrimItem> UCrimItemContainerBase::CreateItem(const UCrimItemDefinition* ItemDefinition, const int32 Quantity)
{
	if (!ItemDefinition ||
		!ItemDefinition->ItemClass.GetScriptStruct())
	{
		return TInstancedStruct<FCrimItem>();
	}

	TInstancedStruct<FCrimItem> Result;
	Result.InitializeAsScriptStruct(ItemDefinition->ItemClass.GetScriptStruct());
	FCrimItem* ItemPtr = Result.GetMutablePtr<FCrimItem>();
	ItemPtr->ItemGuid = FGuid::NewGuid();
	ItemPtr->Initialize(ItemDefinition);
	ItemPtr->Quantity = FMath::Max(1, Quantity);
	for (const TInstancedStruct<FCrimItemDefinitionFragment>& Fragment : ItemDefinition->Fragments)
	{
		Fragment.Get<FCrimItemDefinitionFragment>().SetDefaultValues(Result);
	}
	return Result;
}

TInstancedStruct<FCrimItem> UCrimItemContainerBase::DuplicateItem(const TInstancedStruct<FCrimItem>& Item)
{
	TInstancedStruct<FCrimItem> Result;
	if (!Item.IsValid())
	{
		return Result;
	}

	if (Item.Get<FCrimItem>().GetItemDefinition().IsNull())
	{
		return Result;
	}

	Result = Item;
	Result.GetMutablePtr<FCrimItem>()->ItemGuid = FGuid::NewGuid();
	return Result;
}

FCrimAddItemResult UCrimItemContainerBase::TryAddItem(const TInstancedStruct<FCrimItem>& Item)
{
	FCrimAddItemResult Result;

	if (!CanAddItem(Item, Result.Error))
	{
		return Result;
	}

	FCrimAddItemPlan AddItemPlan = GetAddItemPlan(Item);
	Result = FCrimAddItemResult(AddItemPlan, ExecuteAddItemPlan(Item, AddItemPlan));
	return Result;
}

bool UCrimItemContainerBase::CanAddItem(const TInstancedStruct<FCrimItem>& Item, FGameplayTag& OutError) const
{
	if (!HasAuthority())
	{
		OutError = FCrimItemGameplayTags::Get().ItemPlan_Error;
		return false;
	}

	if (!Item.IsValid() ||
		Item.GetPtr<FCrimItem>()->ItemDefinition.IsNull())
	{
		OutError = FCrimItemGameplayTags::Get().ItemPlan_Error_InvalidItem;
		return false;
	}

	return true;
}

int32 UCrimItemContainerBase::ConsumeItem(const FGuid ItemGuid, const int32 Quantity, bool bRemoveItem)
{
	if (ItemGuid.IsValid() || !HasAuthority() || Quantity <= 0)
	{
		return 0;
	}

	FFastCrimItem* FastItem = ItemList.GetItem(ItemGuid);
	if (FastItem == nullptr)
	{
		return 0;
	}

	FCrimItem* MutableItem = FastItem->Item.GetMutablePtr<FCrimItem>();
	const int32 NewQuantity = FMath::Max(MutableItem->Quantity - Quantity, 0);
	const int32 Delta = MutableItem->Quantity - NewQuantity;
	MutableItem->Quantity = NewQuantity;
	if (NewQuantity <= 0 && bRemoveItem)
	{
		Internal_RemoveItem(ItemGuid);
	}
	else
	{
		MarkItemDirty(*FastItem);
	}
	return Delta;
}

int32 UCrimItemContainerBase::ConsumeItemsByDefinition(const UCrimItemDefinition* ItemDefinition, const int32 Quantity, bool bRemoveItem)
{
	if (!HasAuthority() || !ItemDefinition || Quantity <= 0)
	{
		return 0;
	}

	int32 QuantityRemaining = Quantity;
	for (FFastCrimItem*& FastItem : GetItemsByDefinition(ItemDefinition))
	{
		FCrimItem* Item = FastItem->Item.GetMutablePtr<FCrimItem>();
		const int32 NewQuantity = FMath::Max(Item->Quantity - QuantityRemaining, 0);
		QuantityRemaining = FMath::Max(QuantityRemaining - Item->Quantity, 0);
		Item->Quantity = NewQuantity;

		if (NewQuantity <= 0 && bRemoveItem)
		{
			Internal_RemoveItem(Item->GetItemGuid());
		}
		else
		{
			MarkItemDirty(*FastItem);
		}

		if (QuantityRemaining == 0)
		{
			return Quantity;
		}
	}
	return Quantity - QuantityRemaining;
}

TInstancedStruct<FCrimItem> UCrimItemContainerBase::RemoveItem(const FGuid ItemGuid)
{
	TInstancedStruct<FCrimItem> Result;
	FFastCrimItem* FastItem = GetItemByGuid(ItemGuid);

	if (!CanRemoveItem(FastItem->Item))
	{
		return Result;
	}

	Result = FastItem->Item;
	Internal_RemoveItem(ItemGuid);
	return Result;
}

TArray<TInstancedStruct<FCrimItem>> UCrimItemContainerBase::RemoveItemsByDefinition(const UCrimItemDefinition* ItemDefinition)
{
	TArray<TInstancedStruct<FCrimItem>> Result;
	TArray<FFastCrimItem*> FastItems = GetItemsByDefinition(ItemDefinition);
	
	for (FFastCrimItem* FastItem : FastItems)
	{
		if (CanRemoveItem(FastItem->Item))
		{
			Result.Add(FastItem->Item);
			Internal_RemoveItem(FastItem->Item.Get<FCrimItem>().GetItemGuid());	
		}
	}
	return Result;
}

bool UCrimItemContainerBase::CanRemoveItem(const TInstancedStruct<FCrimItem>& Item) const
{
	if (!HasAuthority() || !Item.IsValid())
	{
		return false;
	}

	FGuid ItemGuid = Item.Get<FCrimItem>().GetItemGuid();
	if (!ItemGuid.IsValid())
	{
		return false;
	}

	FFastCrimItem* FastItem = GetItemByGuid(ItemGuid);
	if (FastItem == nullptr)
	{
		return false;
	}
	return true;
}

void UCrimItemContainerBase::MarkItemDirty(FFastCrimItem& FastItem)
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
		// We are a client so we mark the array dirty to force rebuild.
		ItemList.MarkArrayDirty();
	}
}

void UCrimItemContainerBase::Initialize(UCrimItemManagerComponent* ItemManager, FGameplayTag NewContainerGuid)
{
	ItemManagerComponent = ItemManager;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemManagerComponent, this);
	bOwnerIsNetAuthority = ItemManagerComponent->HasAuthority();

	ContainerGuid = NewContainerGuid;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ContainerGuid, this);
}

FCrimAddItemPlan UCrimItemContainerBase::GetAddItemPlan(const TInstancedStruct<FCrimItem>& Item) const
{
	const FCrimItem* ItemPtr = Item.GetPtr<FCrimItem>();
	FCrimAddItemPlan Result(ItemPtr->Quantity);

	Result.AddEntry(FCrimAddItemPlanEntry(nullptr, ItemPtr->Quantity));

	return Result;
}

void UCrimItemContainerBase::Internal_AddItem(TInstancedStruct<FCrimItem>& Item)
{
	if (HasAuthority())
	{
		Item.GetMutablePtr<FCrimItem>()->ItemContainer = this;
		Item.GetMutablePtr<FCrimItem>()->ItemManager = ItemManagerComponent;
		ItemList.AddItem(Item);
	}
}

void UCrimItemContainerBase::Internal_RemoveItem(const FGuid& ItemGuid)
{
	if (HasAuthority())
	{
		ItemList.RemoveItem(ItemGuid);
	}
}

TArray<TInstancedStruct<FCrimItem>> UCrimItemContainerBase::ExecuteAddItemPlan(const TInstancedStruct<FCrimItem>& Item,
	const FCrimAddItemPlan& AddItemPlan)
{
	TArray<TInstancedStruct<FCrimItem>> Result;

	// Check for a duplicate ItemGuid. The Item being added will retain its original ItemGuid if an existing Guid is not found.
	bool bFoundDuplicateItemGuid = false;
	if (GetItemByGuid(Item.Get<FCrimItem>().GetItemGuid()))
	{
		bFoundDuplicateItemGuid = true;
	}
	
	if (AddItemPlan.IsValid())
	{
		for (const FCrimAddItemPlanEntry& Entry : AddItemPlan.GetEntries())
		{
			if (Entry.FastItemPtr)
			{
				// Modify the quantity of an existing item.
				FCrimItem* EntryItem = Entry.FastItemPtr->Item.GetMutablePtr<FCrimItem>();
				EntryItem->Quantity = EntryItem->Quantity + Entry.QuantityToAdd;
				OnPreModifyItem(Entry.FastItemPtr->Item, Item);
				MarkItemDirty(*Entry.FastItemPtr);
				Result.Add(Entry.FastItemPtr->Item);
			}
			else
			{
				// Creating a duplicate of the item.
				TInstancedStruct<FCrimItem> NewItem = Item;
				if (bFoundDuplicateItemGuid)
				{
					NewItem.GetMutablePtr<FCrimItem>()->ItemGuid = FGuid::NewGuid();
				}
				else
				{
					bFoundDuplicateItemGuid = true;
				}
				NewItem.GetMutablePtr<FCrimItem>()->Quantity = Entry.QuantityToAdd;
				OnPreAddItem(NewItem);
				Internal_AddItem(NewItem);
				Result.Add(NewItem);
			}
		}
	}

	return Result;
}

void UCrimItemContainerBase::BindToItemListDelegates()
{
	ItemList.OnItemAddedDelegate.AddUObject(this, &UCrimItemContainerBase::Internal_OnItemAdded);
	ItemList.OnItemRemovedDelegate.AddUObject(this, &UCrimItemContainerBase::Internal_OnItemRemoved);
	ItemList.OnItemChangedDelegate.AddUObject(this, &UCrimItemContainerBase::Internal_OnItemChanged);
}

void UCrimItemContainerBase::Internal_OnItemAdded(const FFastCrimItem& FastItem)
{
	OnItemAdded(FastItem);
	K2_OnItemAdded(FastItem);
	OnItemAddedDelegate.Broadcast(this, FastItem);
}

void UCrimItemContainerBase::Internal_OnItemRemoved(const FFastCrimItem& FastItem)
{
	OnItemRemoved(FastItem);
	K2_OnItemRemoved(FastItem);
	OnItemRemovedDelegate.Broadcast(this, FastItem);
}

void UCrimItemContainerBase::Internal_OnItemChanged(const FFastCrimItem& FastItem)
{
	OnItemChanged(FastItem);
	K2_OnItemChanged(FastItem);
	OnItemChangedDelegate.Broadcast(this, FastItem);
}

