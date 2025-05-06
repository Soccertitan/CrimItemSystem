// Copyright Soccertitan


#include "CrimItemDefinition.h"

#include "UI/ViewModel/CrimItemViewModel.h"

UCrimItemDefinition::UCrimItemDefinition()
{
	ItemClass.InitializeAsScriptStruct(FCrimItem::StaticStruct());

	ItemViewModelClass = UCrimItemViewModel::StaticClass();
}

FPrimaryAssetId UCrimItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("CrimItemDefinition", GetFName());
}
