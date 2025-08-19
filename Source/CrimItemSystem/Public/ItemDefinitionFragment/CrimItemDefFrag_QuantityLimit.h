// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemDefinition.h"
#include "CrimItemDefFrag_QuantityLimit.generated.h"

/**
 * Describes the limitation for stacking and adding items.
 */
USTRUCT(BlueprintType)
struct FCrimItemDefFrag_QuantityLimit : public FCrimItemDefinitionFragment
{
	GENERATED_BODY()

	/** The maximum number of unique item instances allowed to be managed by the ItemManager. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FCrimItemQuantityLimit CollectionLimit;

	/** The maximum number of unique item instances allowed in a single container. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FCrimItemQuantityLimit ContainerLimit;

	/** The maximum quantity of this item allowed in a single stack. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FCrimItemQuantityLimit StackLimit;
};
