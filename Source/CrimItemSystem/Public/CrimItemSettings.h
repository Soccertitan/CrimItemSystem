// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "CrimItemSettings.generated.h"

class UCrimItemContainerBase;
/**
 * 
 */
UCLASS(Config = Game, DefaultConfig)
class CRIMITEMSYSTEM_API UCrimItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UCrimItemSettings();

	/** The default guid for item containers. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	FGameplayTag DefaultContainerId;

	/** The default ItemContainer to use. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UCrimItemContainerBase> DefaultItemContainerClass;

	virtual FName GetCategoryName() const override;

	static FGameplayTag GetDefaultContainerId();
	static TSubclassOf<UCrimItemContainerBase> GetDefaultItemContainerClass();
};
