// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "CrimItemSettings.generated.h"

class UCrimItemContainer;
/**
 * 
 */
UCLASS(Config = Game, DefaultConfig)
class CRIMITEMSYSTEM_API UCrimItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UCrimItemSettings();

	/** The id to use for default item containers. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	FGameplayTag DefaultContainerId;

	/** The default ItemContainer to use. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UCrimItemContainer> DefaultItemContainerClass;

	virtual FName GetCategoryName() const override;

	static FGameplayTag GetDefaultContainerId();
	static TSubclassOf<UCrimItemContainer> GetDefaultItemContainerClass();
};
