// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemViewModelBase.h"
#include "CrimItemViewModel.generated.h"

/**
 * A basic implementation displaying item name, description, icon, and quantity.
 */
UCLASS()
class CRIMITEMSYSTEM_API UCrimItemViewModel : public UCrimItemViewModelBase
{
	GENERATED_BODY()

public:

	FText GetItemName() const {return ItemName;}
	FText GetItemDescription() const {return ItemDescription;}
	UTexture2D* GetIcon() const {return Icon;}
	int32 GetQuantity() const {return Quantity;}
	
protected:

	virtual void OnItemSet() override;
	virtual void OnItemChanged() override;
	virtual void OnItemDefinitionLoaded(const UCrimItemDefinition* ItemDefinition) override;

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
	
};
