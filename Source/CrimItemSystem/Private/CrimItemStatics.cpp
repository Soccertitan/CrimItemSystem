// Copyright Soccertitan


#include "CrimItemStatics.h"

#include "CrimItemDefinition.h"
#include "CrimItemManagerComponent.h"
#include "CrimItemSystemInterface.h"
#include "Engine/AssetManager.h"

UCrimItemManagerComponent* UCrimItemStatics::GetCrimItemManagerComponent(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	if (Actor->Implements<UCrimItemSystemInterface>())
	{
		return ICrimItemSystemInterface::Execute_GetCrimItemManagerComponent(Actor);
	}

	return Actor->FindComponentByClass<UCrimItemManagerComponent>();
}

const UCrimItemDefinition* UCrimItemStatics::GetItemDefinition(TInstancedStruct<FCrimItem> Item)
{
	if (Item.IsValid())
	{
		if (!Item.Get<FCrimItem>().GetItemDefinition().Get())
		{
			UAssetManager::Get().LoadAssetList(
				{Item.Get<FCrimItem>().GetItemDefinition().ToSoftObjectPath()})->WaitUntilComplete();
		}
		return Item.Get<FCrimItem>().GetItemDefinition().Get();
	}
	return nullptr;
}
