// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemStatics.generated.h"

struct FCrimItem;
class UCrimItemDefinition;
class UCrimItemFragment;
class UCrimItemManagerComponent;

/**
 * 
 */
UCLASS(ClassGroup = "Crim Item System")
class CRIMITEMSYSTEM_API UCrimItemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the ItemManagerComponent if the actor implements the CrimItemSystemInterface. If it does not, fallbacks to
	 * checking the actor's components.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemSystem")
	static UCrimItemManagerComponent* GetCrimItemManagerComponent(const AActor* Actor);

	/**
	 * @param Item The item to retrieve the ItemDefinition from.
	 * @return The loaded ItemDef or nullptr.
	 */
	UFUNCTION(BlueprintPure)
	static const UCrimItemDefinition* GetItemDefinition(UPARAM(ref) TInstancedStruct<FCrimItem> Item);
};
