// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemDefinition.h"
#include "CrimItemDefFrag_UI.generated.h"

/**
 * Describes information that is shown to a user.
 */
USTRUCT(BlueprintType)
struct FCrimItemDefFrag_UI : public FCrimItemDefinitionFragment
{
	GENERATED_BODY()

	/** User facing text of the item name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI")
	FText ItemName;

	/** User facing description of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (MultiLine))
	FText ItemDescription;

	/** The user facing icon of the item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> ItemIcon;

	/** The ViewModel to use. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "UI"), NoClear)
	TSoftClassPtr<UCrimItemViewModelBase> ItemViewModelClass;

	/** The widget to display item information. This class should implement the CrimItemVMInterface. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|UI", meta = (AssetBundles = "UI"))
	TSoftClassPtr<UUserWidget> ItemWidgetClass;
};
