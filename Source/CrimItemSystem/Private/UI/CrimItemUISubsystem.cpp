// Copyright Soccertitan


#include "UI/CrimItemUISubsystem.h"

#include "ItemContainer/CrimItemContainer.h"
#include "CrimItemManagerComponent.h"
#include "CrimItemStatics.h"
#include "Engine/AssetManager.h"
#include "UI/CrimItemContainerProvider.h"
#include "UI/ViewModel/CrimItemContainerViewModel.h"

UCrimItemContainerViewModelBase* UCrimItemUISubsystem::GetOrCreateItemContainerViewModel(UCrimItemContainerBase* Container)
{
	if (!IsValid(Container))
	{
		return nullptr;
	}

	TObjectPtr<UCrimItemContainerViewModelBase>* ContainerViewModel = ItemContainerViewModels.FindByPredicate([Container](UCrimItemContainerViewModelBase* ViewModel)
	{
		return ViewModel && ViewModel->GetItemContainer() == Container;
	});

	if (ContainerViewModel)
	{
		return *ContainerViewModel;
	}

	UCrimItemContainerViewModelBase* NewVM = CreateContainerViewModel(Container);
	ItemContainerViewModels.Add(NewVM);
	return NewVM;
}

UCrimItemContainerViewModelBase* UCrimItemUISubsystem::GetOrCreateItemContainerViewModelForActor(AActor* Actor, FGameplayTag ContainerId)
{
	if (UCrimItemManagerComponent* ItemManager = UCrimItemStatics::GetCrimItemManagerComponent(Actor))
	{
		return GetOrCreateItemContainerViewModel(ItemManager->GetItemContainerByGuid(ContainerId));
	}
	return nullptr;
}

UCrimItemContainerBase* UCrimItemUISubsystem::GetItemContainerFromProvider(TSubclassOf<UCrimItemContainerProvider> Provider,
	FGameplayTag ContainerId, const FCrimItemViewContext& Context)
{
	if (Provider)
	{
		if (const UCrimItemContainerProvider* ProviderCDO = GetDefault<UCrimItemContainerProvider>(Provider))
		{
			return ProviderCDO->ProvideContainer(ContainerId, Context);
		}
	}
	return nullptr;
}

UCrimItemContainerViewModelBase* UCrimItemUISubsystem::CreateContainerViewModel(UCrimItemContainerBase* Container)
{
	if (!Container->GetViewModelClass().Get())
	{
		UAssetManager::Get().LoadAssetList({Container->GetViewModelClass().ToSoftObjectPath()})->WaitUntilComplete();
	}
	UCrimItemContainerViewModelBase* NewVM = NewObject<UCrimItemContainerViewModelBase>(this, Container->GetViewModelClass().Get());
	NewVM->SetItemContainer(Container);
	return NewVM;
}
