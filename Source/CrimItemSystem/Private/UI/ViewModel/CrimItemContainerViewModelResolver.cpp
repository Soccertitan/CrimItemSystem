// Copyright Soccertitan


#include "UI/ViewModel/CrimItemContainerViewModelResolver.h"

#include "ItemContainer/CrimItemContainer.h"
#include "CrimItemSettings.h"
#include "Blueprint/UserWidget.h"
#include "UI/CrimItemContainerProvider.h"
#include "UI/CrimItemUISubsystem.h"
#include "UI/ViewModel/CrimItemContainerViewModel.h"

UCrimItemContainerViewModelBase* UCrimItemContainerViewModelResolverBase::GetItemContainerViewModel(const UUserWidget* UserWidget,
                                                                                       const UMVVMView* View) const
{
	UCrimItemContainerBase* ItemContainer = GetItemContainer(UserWidget, View);
	if (IsValid(ItemContainer))
	{
		// Get the view model for the container.
		UCrimItemUISubsystem* ItemUISubsystem = UserWidget->GetWorld()->GetSubsystem<UCrimItemUISubsystem>();
		return ItemUISubsystem->GetOrCreateItemContainerViewModel(ItemContainer);
	}
	return nullptr;
}

UCrimItemContainerBase* UCrimItemContainerViewModelResolverBase::GetItemContainer(const UUserWidget* UserWidget,
	const UMVVMView* View) const
{
	return nullptr;
}

// UVMR_CrimItemContainerResolver
//--------------------------------

UVMR_CrimItemContainer::UVMR_CrimItemContainer()
{
	ContainerId = UCrimItemSettings::GetDefaultContainerId();
}

UObject* UVMR_CrimItemContainer::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget,
	const UMVVMView* View) const
{
	return GetItemContainerViewModel(UserWidget, View);
}

UCrimItemContainerBase* UVMR_CrimItemContainer::GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UCrimItemUISubsystem* ItemUISubsystem = UserWidget->GetWorld()->GetSubsystem<UCrimItemUISubsystem>();
	const FCrimItemViewContext Context(UserWidget);
	return ItemUISubsystem->GetItemContainerFromProvider(Provider, ContainerId, Context);
}
