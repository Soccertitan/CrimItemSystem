// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "View/MVVMViewModelContextResolver.h"
#include "CrimItemContainerViewModelResolver.generated.h"

class UCrimItemContainerViewModelBase;
class UCrimItemContainerProvider;
class UCrimItemContainerBase;

/**
 * Base class for a view model resolver for CrimItems.
 */
UCLASS(Abstract)
class CRIMITEMSYSTEM_API UCrimItemContainerViewModelResolverBase : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:

	/** Get or creates a view model for the relevant CrimItemContainer. */
	virtual UCrimItemContainerViewModelBase* GetItemContainerViewModel(const UUserWidget* UserWidget, const UMVVMView* View) const;

	/** Returns the relevant CrimItemContainer. */
	virtual UCrimItemContainerBase* GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const;
};

/**
 * Resolves a CrimItemContainer view model using a provider.
 * Implement custom provider classes to retrieve containers from different actors in the world.
 */
UCLASS()
class CRIMITEMSYSTEM_API UVMR_CrimItemContainer : public UCrimItemContainerViewModelResolverBase
{
	GENERATED_BODY()

public:
	UVMR_CrimItemContainer();

	/** The item container provider to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	TSubclassOf<UCrimItemContainerProvider> Provider;

	/** The container id. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	FGameplayTag ContainerId;

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
	virtual UCrimItemContainerBase* GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const override;
};