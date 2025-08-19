// Copyright Soccertitan


#include "UI/ViewModel/CrimItemContainerViewModelBase.h"

#include "ItemContainer/CrimItemContainerBase.h"
#include "CrimItemDefinition.h"
#include "CrimItemStatics.h"
#include "Engine/AssetManager.h"
#include "ItemDefinitionFragment/CrimItemDefFrag_UI.h"
#include "UI/ViewModel/CrimItemViewModelBase.h"

void UCrimItemContainerViewModelBase::SetItemContainer(UCrimItemContainerBase* InItemContainer)
{
	if (!IsValid(InItemContainer))
	{
		return;
	}
	
	if (InItemContainer != GetItemContainer())
	{
		if (IsValid(GetItemContainer()))
		{
			GetItemContainer()->OnItemAddedDelegate.RemoveAll(this);
			GetItemContainer()->OnItemRemovedDelegate.RemoveAll(this);
			GetItemContainer()->OnItemChangedDelegate.RemoveAll(this);
		}
		
		ItemContainer = InItemContainer;
		
		GetItemContainer()->OnItemAddedDelegate.AddUObject(this, &UCrimItemContainerViewModelBase::Internal_OnItemAdded);
		GetItemContainer()->OnItemRemovedDelegate.AddUObject(this, &UCrimItemContainerViewModelBase::Internal_OnItemRemoved);
		GetItemContainer()->OnItemChangedDelegate.AddUObject(this, &UCrimItemContainerViewModelBase::Internal_OnItemChanged);
		OnItemContainerSet();
	}
}

UCrimItemContainerBase* UCrimItemContainerViewModelBase::GetItemContainer() const
{
	return ItemContainer.Get();
}

UCrimItemViewModelBase* UCrimItemContainerViewModelBase::CreateItemViewModel(const TInstancedStruct<FCrimItem>& Item)
{
	const UCrimItemDefinition* ItemDef = UCrimItemStatics::GetItemDefinition(Item);
	const FCrimItemDefFrag_UI* UIFrag = ItemDef->GetFragmentByType<FCrimItemDefFrag_UI>();
	if (!UIFrag->ItemViewModelClass.Get())
	{
		UAssetManager::Get().LoadAssetList({UIFrag->ItemViewModelClass.ToSoftObjectPath()})->WaitUntilComplete();
	}
	UCrimItemViewModelBase* NewVM = NewObject<UCrimItemViewModelBase>(this, UIFrag->ItemViewModelClass.Get());
	NewVM->SetItem(Item);
	return NewVM;
}

void UCrimItemContainerViewModelBase::Internal_OnItemAdded(UCrimItemContainerBase* InItemContainer, const FFastCrimItem& InItem)
{
	OnItemAdded(InItem.Item);
}

void UCrimItemContainerViewModelBase::Internal_OnItemRemoved(UCrimItemContainerBase* InItemContainer, const FFastCrimItem& InItem)
{
	OnItemRemoved(InItem.Item);
}

void UCrimItemContainerViewModelBase::Internal_OnItemChanged(UCrimItemContainerBase* InItemContainer, const FFastCrimItem& InItem)
{
	OnItemChanged(InItem);
}
