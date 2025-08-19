// Copyright Soccertitan


#include "UI/CrimItemContainerProvider.h"

#include "ItemContainer/CrimItemContainer.h"
#include "CrimItemManagerComponent.h"
#include "CrimItemStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState.h"

UCrimItemContainerBase* UCrimItemContainerProvider::ProvideContainer_Implementation(FGameplayTag ContainerId,
                                                                                FCrimItemViewContext Context) const
{
	return nullptr;
}

UCrimItemContainerBase* UCrimItemContainerProvider_Player::ProvideContainer_Implementation(FGameplayTag ContainerId,
	FCrimItemViewContext Context) const
{
	if (!Context.UserWidget)
	{
		return nullptr;
	}
	APlayerController* Player = Context.UserWidget->GetOwningPlayer();
	if (!Player)
	{
		return nullptr;
	}

	UCrimItemContainerBase* Result = nullptr;

	if (UCrimItemManagerComponent* ItemManager = UCrimItemStatics::GetCrimItemManagerComponent(Player->GetPawn()))
	{
		Result = ItemManager->GetItemContainerByGuid(ContainerId);
	}

	if (!IsValid(Result))
	{
		if (UCrimItemManagerComponent* ItemManager = UCrimItemStatics::GetCrimItemManagerComponent(Player->PlayerState))
		{
			Result = ItemManager->GetItemContainerByGuid(ContainerId);
		}
	}

	if (!IsValid(Result))
	{
		if (UCrimItemManagerComponent* ItemManager = UCrimItemStatics::GetCrimItemManagerComponent(Player))
		{
			Result = ItemManager->GetItemContainerByGuid(ContainerId);
		}
	}

	return Result;
}
