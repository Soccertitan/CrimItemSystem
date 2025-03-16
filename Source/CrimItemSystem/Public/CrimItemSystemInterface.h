// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CrimItemSystemInterface.generated.h"

class UCrimItemManagerComponent;
// This class does not need to be modified.
UINTERFACE()
class UCrimItemSystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CRIMITEMSYSTEM_API ICrimItemSystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Returns the ItemManagerComponent that can live on an actor such as a pawn or PlayerState.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "CrimItemSystem")
	UCrimItemManagerComponent* GetCrimItemManagerComponent() const;
};
