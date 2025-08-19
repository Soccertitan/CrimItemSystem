// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "ItemContainer/CrimItemContainer.h"
#include "UObject/Object.h"
#include "CrimItemContainerRule.generated.h"

class UCrimItemContainerBase;

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

	/** Returns true if the item is allowed in the container. */
	UFUNCTION(BlueprintPure)
	virtual bool CanAddItem(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item, FGameplayTag& OutError) const;

	/** Returns true if the item is allowed to be removed directly. */
	UFUNCTION(BlueprintPure)
	virtual bool CanRemoveItem(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const;

	/** Return the maximum number of unique item instances the item can occupy in the container */
	UFUNCTION(BlueprintPure)
	virtual int32 GetMaxNumberOfStacks(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const;

	/** Return the maximum allowed quantity for a single stack of an item. */
	UFUNCTION(BlueprintPure)
	virtual int32 GetItemStackMaxQuantity(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const;

protected:
	/**
	 * Returns true if the item is allowed in the container.
	 * The Container and Item is guaranteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "CanAddItem")
	bool K2_CanAddItem(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item, FGameplayTag& OutError) const;

	/**
	 * Returns true if the item is allowed to be removed without being consumed.
	 * The Container and Item is guaranteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "CanRemoveItem")
	bool K2_CanRemoveItem(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const;

	/**
	 * Return the maximum number of unique item instances the item can occupy in the container.
	 * The Container and Item is guaranteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "GetMaxNumberOfStacks")
	int32 K2_GetMaxNumberOfStacks(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const;

	/**
	 * Return the maximum allowed quantity for a single stack of an item.
	 * The Container and Item is guaranteed to be valid.
	 */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "GetItemQuantityLimit")
	bool K2_GetItemStackMaxQuantity(const UCrimItemContainerBase* Container, const TInstancedStruct<FCrimItem>& Item) const;

private:

	uint8 bHasCanContainItem : 1;
	uint8 bHasCanRemoveItem : 1;
	uint8 bHasGetItemMaxQuantity : 1;
	uint8 bHasGetItemStatckMaxQuantity : 1;
};
