// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CrimItemVMInterface.generated.h"

class UCrimItemViewModelBase;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCrimItemVMInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Designed to allow code to inject a ViewModel into a widget for displaying information about an item.
 */
class CRIMITEMSYSTEM_API ICrimItemVMInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/**
	 * @param CrimItemViewModel The ViewModel to assign to the widget.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetCrimItemViewModel(UCrimItemViewModelBase* CrimItemViewModel);
};
