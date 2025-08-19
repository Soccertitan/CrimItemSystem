// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "MVVMViewModelBase.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemViewModelBase.generated.h"

/**
 * A generic implementation of the Item ViewModel designed to be subclassed.
 */
UCLASS(Abstract)
class CRIMITEMSYSTEM_API UCrimItemViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:

	UCrimItemViewModelBase();

	/** Initializes the ViewModel with the specified item. */
	void SetItem(const TInstancedStruct<FCrimItem>& InItem);

	UFUNCTION(BlueprintPure)
	const TInstancedStruct<FCrimItem>& GetItem() const;

	UFUNCTION(BlueprintPure)
	FGuid GetItemGuid() const {return ItemGuid;}

protected:

	/** Called from SetItem when a valid Item has been set. */
	virtual void OnItemSet(){}

private:

	/** Cached copy of the item. */
	UPROPERTY()
	TInstancedStruct<FCrimItem> Item;

	/** Cached for quick retrieval. */
	UPROPERTY()
	FGuid ItemGuid;
};
