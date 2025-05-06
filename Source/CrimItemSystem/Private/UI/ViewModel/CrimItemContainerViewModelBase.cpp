// Copyright Soccertitan


#include "UI/ViewModel/CrimItemContainerViewModelBase.h"

#include "CrimItemContainer.h"
#include "CrimItemDefinition.h"
#include "CrimItemStatics.h"
#include "Engine/AssetManager.h"
#include "UI/ViewModel/CrimItemViewModelBase.h"

void UCrimItemContainerViewModelBase::SetItemContainer(UCrimItemContainer* InItemContainer)
{
	if (InItemContainer != GetItemContainer())
	{
		if (IsValid(GetItemContainer()))
		{
			GetItemContainer()->OnItemAddedDelegate.RemoveAll(this);
			GetItemContainer()->OnItemRemovedDelegate.RemoveAll(this);
		}
		
		ItemContainer = InItemContainer;
		
		if (IsValid(InItemContainer))
		{
			GetItemContainer()->OnItemAddedDelegate.AddUObject(this, &UCrimItemContainerViewModelBase::Internal_OnItemAdded);
			GetItemContainer()->OnItemRemovedDelegate.AddUObject(this, &UCrimItemContainerViewModelBase::Internal_OnItemRemoved);
			OnItemContainerSet();
		}
	}
}

UCrimItemContainer* UCrimItemContainerViewModelBase::GetItemContainer() const
{
	return ItemContainer.Get();
}

UCrimItemViewModelBase* UCrimItemContainerViewModelBase::CreateItemViewModel(const TInstancedStruct<FCrimItem>& Item)
{
	const UCrimItemDefinition* ItemDef = UCrimItemStatics::GetItemDefinition(Item);
	if (!ItemDef->ItemViewModelClass.Get())
	{
		UAssetManager::Get().LoadAssetList({ItemDef->ItemViewModelClass.ToSoftObjectPath()})->WaitUntilComplete();
	}
	UCrimItemViewModelBase* NewVM = NewObject<UCrimItemViewModelBase>(this, ItemDef->ItemViewModelClass.Get());
	NewVM->SetItem(Item);
	return NewVM;
}

void UCrimItemContainerViewModelBase::Internal_OnItemAdded(UCrimItemContainer* InItemContainer, const FFastCrimItem& InItem)
{
	OnItemAdded(InItem.Item);
}

void UCrimItemContainerViewModelBase::Internal_OnItemRemoved(UCrimItemContainer* InItemContainer, const FFastCrimItem& InItem)
{
	OnItemRemoved(InItem.Item);
}
