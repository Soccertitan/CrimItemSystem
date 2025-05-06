// Copyright Soccertitan


#include "UI/CrimItemContainerProvider.h"

#include "CrimItemContainer.h"
#include "CrimItemManagerComponent.h"
#include "CrimItemStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState.h"

UCrimItemContainer* UCrimItemContainerProvider::ProvideContainer_Implementation(FGameplayTag ContainerId,
                                                                                FCrimItemViewContext Context) const
{
	return nullptr;
}

UCrimItemContainer* UCrimItemContainerProvider_Player::ProvideContainer_Implementation(FGameplayTag ContainerId,
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

	UCrimItemContainer* Result = nullptr;

	if (UCrimItemManagerComponent* ItemManager = UCrimItemStatics::GetCrimItemManagerComponent(Player->GetPawn()))
	{
		Result = ItemManager->GetItemContainer(ContainerId);
	}

	if (!IsValid(Result))
	{
		if (UCrimItemManagerComponent* ItemManager = UCrimItemStatics::GetCrimItemManagerComponent(Player->PlayerState))
		{
			Result = ItemManager->GetItemContainer(ContainerId);
		}
	}

	if (!IsValid(Result))
	{
		if (UCrimItemManagerComponent* ItemManager = UCrimItemStatics::GetCrimItemManagerComponent(Player))
		{
			Result = ItemManager->GetItemContainer(ContainerId);
		}
	}

	return Result;
}
