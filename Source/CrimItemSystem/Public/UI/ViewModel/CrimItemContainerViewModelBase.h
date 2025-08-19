// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "StructUtils/InstancedStruct.h"
#include "CrimItemContainerViewModelBase.generated.h"

class UCrimItemViewModelBase;
struct FFastCrimItem;
struct FCrimItem;
class UCrimItemContainerBase;

/**
 * A generic implementation of the ItemContainer ViewModel designed to be subclassed.
 */
UCLASS(Abstract)
class CRIMITEMSYSTEM_API UCrimItemContainerViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** Updates the ItemContainer for this ViewModel. Triggers OnItemContainerSet if a new one is set. */
	UFUNCTION(BlueprintCallable)
	void SetItemContainer(UCrimItemContainerBase* InItemContainer);

	UFUNCTION(BlueprintPure)
	UCrimItemContainerBase* GetItemContainer() const;

protected:

	/** Called when a valid item container is set. */
	virtual void OnItemContainerSet() {}

	/** Called whenever an item is added to the ItemContainer. */
	virtual void OnItemAdded(const TInstancedStruct<FCrimItem>& Item) {}
	/** Called whenever an item is removed from the ItemContainer. */
	virtual void OnItemRemoved(const TInstancedStruct<FCrimItem>& Item) {}
	/** Called whenever an item is changed in the ItemContainer. */
	virtual void OnItemChanged(const FFastCrimItem& InItem) {}

	/**
	 * Creates an ItemViewModel from the Item from the Item's ItemDef and initializes it with the Item.
	 * This will synchronously load the ItemDef if it's not already loaded.
	 */
	UCrimItemViewModelBase* CreateItemViewModel(const TInstancedStruct<FCrimItem>& Item);

private:
	UPROPERTY()
	TWeakObjectPtr<UCrimItemContainerBase> ItemContainer;

	void Internal_OnItemAdded(UCrimItemContainerBase* InItemContainer, const FFastCrimItem& InItem);
	void Internal_OnItemRemoved(UCrimItemContainerBase* InItemContainer, const FFastCrimItem& InItem);
	void Internal_OnItemChanged(UCrimItemContainerBase* InItemContainer, const FFastCrimItem& InItem);
};
