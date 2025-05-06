// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "MVVMViewModelBase.h"
#include "Engine/StreamableManager.h"
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

	/** Called when the Item has been updated in the ItemContainer. */
	virtual void OnItemChanged(){}

	/** Called after the ItemDefinition is loaded.*/
	virtual void OnItemDefinitionLoaded(const UCrimItemDefinition* ItemDefinition){}

private:

	/** Cached copy of the item. */
	UPROPERTY()
	TInstancedStruct<FCrimItem> Item;

	/** Cached for quick retrieval. */
	UPROPERTY()
	FGuid ItemGuid;

	/** Bundles to load when async loading the ItemDefinition. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> Bundles;

	/** Cached handle for the ItemDef. */
	TSharedPtr<FStreamableHandle> ItemDefStreamableHandle; 

	/** Checks if the ItemGuid from the FastItem matches the cached ItemGuid before calling OnItemChanged. */
	void Internal_OnItemChanged(UCrimItemContainer* ItemContainer, const FFastCrimItem& FastItem);

	/** Called after the ItemDefinition is loaded.*/
	void Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UCrimItemDefinition> ItemDefinition);
};
