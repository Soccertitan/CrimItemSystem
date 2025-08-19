// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "CrimItemContainerProvider.generated.h"

class UCrimItemContainerBase;

/** Context passed into a UCrimItemContainerProvider. */
USTRUCT(BlueprintType)
struct FCrimItemViewContext
{
	GENERATED_BODY()

	/** The user widget requesting the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	const UUserWidget* UserWidget = nullptr;

	/** Additional context tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ContextTags;
};

/**
 * Provides an ItemContainer, for use in view model resolvers or similar situations.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const)
class CRIMITEMSYSTEM_API UCrimItemContainerProvider : public UObject
{
	GENERATED_BODY()

public:
	/** Returns the relevant item container. */
	UFUNCTION(BlueprintNativeEvent)
	UCrimItemContainerBase* ProvideContainer(FGameplayTag ContainerId, FCrimItemViewContext Context) const;
};

/**
 * Provides a game item container from the widget's owning player.
 * Checks the player pawn, player state, and player controller in that order.
 */
UCLASS(DisplayName = "Player")
class UCrimItemContainerProvider_Player : public UCrimItemContainerProvider
{
	GENERATED_BODY()

public:
	virtual UCrimItemContainerBase* ProvideContainer_Implementation(FGameplayTag ContainerId, FCrimItemViewContext Context) const override;
};
