// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemViewModelBase.h"
#include "Engine/StreamableManager.h"
#include "CrimItemViewModel.generated.h"

/**
 * A basic implementation displaying item name, description, icon, and quantity.
 */
UCLASS()
class CRIMITEMSYSTEM_API UCrimItemViewModel : public UCrimItemViewModelBase
{
	GENERATED_BODY()

public:

	UCrimItemViewModel();

	FText GetItemName() const {return ItemName;}
	FText GetItemDescription() const {return ItemDescription;}
	UTexture2D* GetIcon() const {return Icon;}
	int32 GetQuantity() const {return Quantity;}
	
protected:

	virtual void OnItemSet() override;
	/** Called after the ItemDefinition is loaded.*/
	virtual void OnItemDefinitionLoaded(const UCrimItemDefinition* ItemDefinition);

	void SetItemName(FText InValue);
	void SetItemDescription(FText InValue);
	void SetIcon(UTexture2D* InValue);
	void SetQuantity(int32 InValue);

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify, Getter, meta = (AllowPrivateAccess = "true"))
	int32 Quantity = 0;

	/** Bundles to load when async loading the ItemDefinition. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> Bundles;

	/** Cached handle for the ItemDef. */
	TSharedPtr<FStreamableHandle> ItemDefStreamableHandle;

	/** Cached reference to the ItemContainer where the delegate was bound. */
	UPROPERTY()
	TWeakObjectPtr<UCrimItemContainerBase> ItemContainerBase;
	FDelegateHandle ItemChangedDelegateHandle;

	/** Called after the ItemDefinition is loaded.*/
	void Internal_OnItemDefinitionLoaded(TSoftObjectPtr<UCrimItemDefinition> ItemDefinition);

	void Internal_OnIconLoaded(TSoftObjectPtr<UTexture2D> InIcon);

	/** Checks if the ItemGuid from the FastItem matches the cached ItemGuid before calling OnItemChanged. */
	void Internal_OnItemChanged(UCrimItemContainerBase* ItemContainer, const FFastCrimItem& FastItem);
};
