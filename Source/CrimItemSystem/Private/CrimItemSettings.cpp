// Copyright Soccertitan


#include "CrimItemSettings.h"

#include "ItemContainer/CrimItemContainer.h"
#include "CrimItemGameplayTags.h"
#include "CrimItemSystem.h"
#include "Engine/AssetManager.h"

UCrimItemSettings::UCrimItemSettings()
{
	DefaultContainerId = FCrimItemGameplayTags::Get().ItemContainer_Default;
	DefaultItemContainerClass = UCrimItemContainer::StaticClass();
}

FName UCrimItemSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FGameplayTag UCrimItemSettings::GetDefaultContainerId()
{
	const UCrimItemSettings* Settings = GetDefault<UCrimItemSettings>();

	if (!Settings->DefaultContainerId.IsValid())
	{
		UE_LOG(LogCrimItemSystem, Error, TEXT("UCrimItemSettings.DefaultContainerId is not valid. "
			"Set a value in the project settings."));
	}

	return Settings->DefaultContainerId;
}

TSubclassOf<UCrimItemContainerBase> UCrimItemSettings::GetDefaultItemContainerClass()
{
	const UCrimItemSettings* Settings = GetDefault<UCrimItemSettings>();

	if (!Settings->DefaultItemContainerClass)
	{
		UE_LOG(LogCrimItemSystem, Error, TEXT("UCrimItemSettings.DefaultItemContainerClass is not valid. "
			"Set a value in the project settings."));
	}

	if (!Settings->DefaultItemContainerClass.Get())
	{
		UAssetManager::Get().LoadAssetList({Settings->DefaultItemContainerClass.ToSoftObjectPath()})->WaitUntilComplete();
	}

	return Settings->DefaultItemContainerClass.Get();
}


