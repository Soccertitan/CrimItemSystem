// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemContainer.h"
#include "UObject/Object.h"
#include "CrimItemContainerRule.generated.h"

class UCrimItem;
class UCrimItemContainer;
/**
 * Defines container specific conditions. For example, stack limitations for items. These will override item specific
 * conditions.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class CRIMITEMSYSTEM_API UCrimItemContainerRule : public UObject
{
	GENERATED_BODY()

public:
	UCrimItemContainerRule();

	/**
	 * Returns true if the item is allowed in the container.
	 */
	UFUNCTION(BlueprintPure)
	virtual bool CanContainItem(const UCrimItemContainer* Container, const UCrimItem* Item) const;

	UFUNCTION(BlueprintPure)
	virtual bool CanRemoveItem(const UCrimItemContainer* Container, const UCrimItem* Item) const;

	/**
	 * Returns the valid slots the item can be in. If empty, all slots are valid.
	 */
	UFUNCTION(BlueprintPure)
	virtual TArray<int32> GetValidSlots(const UCrimItemContainer* Container, const UCrimItem* Item) const;

	/** Return the maximum number of unique item instances the item can occupy in the container */
	UFUNCTION(BlueprintPure)
	virtual int32 GetMaxNumberOfStacks(const UCrimItemContainer* Container, const UCrimItem* Item) const;

	/** Return the maximum allowed quantity for a single stack of an item. */
	UFUNCTION(BlueprintPure)
	virtual int32 GetItemStackMaxQuantity(const UCrimItemContainer* Container, const UCrimItem* Item) const;

protected:
	/**
	 * Returns true if the item is allowed in the container.
	 * The Container and Item is guaranteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "CanContainItem")
	bool K2_CanContainItem(const UCrimItemContainer* Container, const UCrimItem* Item) const;

	/**
	 * Return true if the item is allowed to be removed from the container.
	 * The Container and Item is guarnteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "CanRemoveItem")
	bool K2_CanRemoveItem(const UCrimItemContainer* Container, const UCrimItem* Item) const;

	/**
	 * Returns the valid slots the item can be in. If empty, all slots are valid.
	 * The container and item is guaranteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "GetValidSlots")
	TArray<int32> K2_GetValidSlots(const UCrimItemContainer* Container, const UCrimItem* Item) const;

	/**
	 * Return the maximum number of unique item instances the item can occupy in the container.
	 * The Container and Item is guaranteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "GetMaxNumberOfStacks")
	int32 K2_GetMaxNumberOfStacks(const UCrimItemContainer* Container, const UCrimItem* Item) const;

	/**
	 * Return the maximum allowed quantity for a single stack of an item.
	 * The Container and Item is guaranteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "GetItemQuantityLimit")
	bool K2_GetItemStackMaxQuantity(const UCrimItemContainer* Container, const UCrimItem* Item) const;

private:

	uint8 bHasCanContainItem : 1;
	uint8 bHasCanRemoveItem : 1;
	uint8 bHasGetItemMaxQuantity : 1;
	uint8 bHasGetItemStatckMaxQuantity : 1;
	uint8 bHasGetValidSlots : 1;
};
