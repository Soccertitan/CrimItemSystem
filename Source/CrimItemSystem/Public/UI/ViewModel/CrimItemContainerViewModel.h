// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemContainerViewModelBase.h"
#include "CrimItemContainerViewModel.generated.h"

struct FCrimItem;
struct FFastCrimItem;
class UCrimItemContainer;
class UCrimItemViewModelBase;

/**
 * A view model for a Crim Item Container that retrieves the items and creates a ViewModel for each item.
 */
UCLASS()
class CRIMITEMSYSTEM_API UCrimItemContainerViewModel : public UCrimItemContainerViewModelBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetItemContainerName() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	TArray<UCrimItemViewModelBase*> GetItems() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	int32 GetConsumedCapacity() const;

	UFUNCTION(BlueprintPure, FieldNotify)
	int32 GetMaxCapacity() const;

protected:
	virtual void OnItemContainerSet() override;
	virtual void OnItemAdded(const TInstancedStruct<FCrimItem>& Item) override;
	virtual void OnItemRemoved(const TInstancedStruct<FCrimItem>& Item) override;
	
	virtual void BroadcastUpdates();

private:

	UPROPERTY()
	TArray<TObjectPtr<UCrimItemViewModelBase>> ItemViewModels;
};
