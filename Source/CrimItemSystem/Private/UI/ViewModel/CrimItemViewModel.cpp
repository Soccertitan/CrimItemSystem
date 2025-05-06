// Copyright Soccertitan


#include "UI/ViewModel/CrimItemViewModel.h"

#include "CrimItemDefinition.h"

void UCrimItemViewModel::OnItemSet()
{
	Super::OnItemSet();

	SetQuantity(GetItem().Get<FCrimItem>().Quantity);
}

void UCrimItemViewModel::OnItemChanged()
{
	Super::OnItemChanged();

	SetQuantity(GetItem().Get<FCrimItem>().Quantity);
}

void UCrimItemViewModel::OnItemDefinitionLoaded(const UCrimItemDefinition* ItemDefinition)
{
	Super::OnItemDefinitionLoaded(ItemDefinition);

	SetItemName(ItemDefinition->ItemName);
	SetItemDescription(ItemDefinition->ItemDescription);
	SetIcon(ItemDefinition->ItemIcon.Get());
}

void UCrimItemViewModel::SetItemName(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemName, InValue);
}

void UCrimItemViewModel::SetItemDescription(FText InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemDescription, InValue);
}

void UCrimItemViewModel::SetIcon(UTexture2D* InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Icon, InValue);
}

void UCrimItemViewModel::SetQuantity(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(Quantity, InValue);
}
