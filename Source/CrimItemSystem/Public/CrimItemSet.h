// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "Engine/DataAsset.h"
#include "CrimItemSet.generated.h"

class UCrimItemDefinition;

/**
 * Used internally by the UCrimItemSet class.
 */
USTRUCT()
struct CRIMITEMSYSTEM_API FCrimItemInstance
{
	GENERATED_BODY()

	const TInstancedStruct<FCrimItem>& GetItem() const;

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCrimItemDefinition> ItemDefinition;

	UPROPERTY(EditAnywhere, meta = (StructTypeConst, FullyExpand=true, EditCondition=bShowItem,
		EditConditionHides, HideEditConditionToggle, AllowPrivateAccess, ShowOnlyInnerProperties))
	TInstancedStruct<FCrimItem> Item;
	
	UPROPERTY()
	bool bShowItem = false;

	/** Hidden used as a TitleProperty in the UCrimItemSet. */
	UPROPERTY(VisibleDefaultsOnly, meta = (EditCondition=bShowProperty, EditConditionHides, HideEditConditionToggle))
	FString ItemDefName;
	UPROPERTY()
	bool bShowProperty = false;

	/** Creates the item based on the ItemDefinition. Or clears it out when ItemDefinition is null. */
	void TryCreateItem();
	
	friend class UCrimItemSet;
};

/**
 * Contains an array of items that can be designed in editor. Typically used as adding default startup items to an
 * ItemContainer.
 */
UCLASS(const, ClassGroup = "Crim Item System", meta = (DisplayName = "Crim Item Set",
	ToolTip = "A collection of manually created items."))
class CRIMITEMSYSTEM_API UCrimItemSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UCrimItemSet();
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	UPROPERTY(EditAnywhere, meta = (TitleProperty="ItemDefName"))
	TArray<FCrimItemInstance> ItemInstances;
};
