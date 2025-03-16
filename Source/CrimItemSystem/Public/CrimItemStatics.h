// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CrimItemStatics.generated.h"

class UCrimItemManagerComponent;
/**
 * 
 */
UCLASS(ClassGroup = "Crim Item System")
class CRIMITEMSYSTEM_API UCrimItemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Get the ItemManagerComponent if the actor implements the CrimItemSystemInterface. If it does not, fallbacks to
	 * checking the actor's components.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItemSystem")
	static UCrimItemManagerComponent* GetCrimItemManagerComponent(const AActor* Actor);
};
