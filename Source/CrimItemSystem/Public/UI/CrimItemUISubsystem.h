// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrimItemUISubsystem.generated.h"

class UCrimItemContainerViewModelBase;
struct FCrimItemViewContext;
class UCrimItemContainerProvider;
class UCrimItemContainerBase;

/**
 * Subsystem for working with game items UI.
 */
UCLASS()
class CRIMITEMSYSTEM_API UCrimItemUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Get a view model for an ItemContainer, reusing an existing one if it already exists. */
	UFUNCTION(BlueprintCallable, Category = "CrimItemUISubsystem")
	UCrimItemContainerViewModelBase* GetOrCreateItemContainerViewModel(UCrimItemContainerBase* Container);

	/** Get a view model for an ItemContainer, reusing an existing one if it already exists. */
	UFUNCTION(BlueprintCallable, Category = "CrimItemUISubsystem")
	UCrimItemContainerViewModelBase* GetOrCreateItemContainerViewModelForActor(AActor* Actor, FGameplayTag ContainerId);

	/** Retrieve an ItemContainer from a provider class, given view context. */
	UFUNCTION(BlueprintCallable, Category = "CrimItemUISubsystem")
	UCrimItemContainerBase* GetItemContainerFromProvider(TSubclassOf<UCrimItemContainerProvider> Provider, FGameplayTag ContainerId, const FCrimItemViewContext& Context);

protected:
	/** All container view models that have been created. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCrimItemContainerViewModelBase>> ItemContainerViewModels;

	UCrimItemContainerViewModelBase* CreateContainerViewModel(UCrimItemContainerBase* Container);
};
