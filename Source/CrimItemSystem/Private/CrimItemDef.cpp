// Copyright Soccertitan


#include "CrimItemDef.h"

#include "UObject/ObjectSaveContext.h"

UCrimItemDef::UCrimItemDef()
{
	
}

FPrimaryAssetId UCrimItemDef::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("CrimItemDef", GetFName());
}