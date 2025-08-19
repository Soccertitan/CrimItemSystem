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

const UCrimItemDefinition* UCrimItemStatics::GetItemDefinition(const TInstancedStruct<FCrimItem>& Item)
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

TInstancedStruct<FCrimItemFragment> UCrimItemStatics::K2_GetItemFragment(const TInstancedStruct<FCrimItem>& Item, const UScriptStruct* FragmentType)
{
	if (Item.IsValid() && FragmentType)
	{
		const FCrimItem* ItemPtr = Item.GetPtr<FCrimItem>();
		for (const TInstancedStruct<FCrimItemFragment>& Fragment : ItemPtr->Fragments)
		{
			if (Fragment.IsValid() && Fragment.GetScriptStruct()->IsChildOf(FragmentType))
			{
				return Fragment;
			}
		}
	}
	return TInstancedStruct<FCrimItemFragment>();
}

TInstancedStruct<FCrimItemDefinitionFragment> UCrimItemStatics::K2_GetItemDefinitionFragment(const UCrimItemDefinition* ItemDefinition, const UScriptStruct* FragmentType)
{
	if (ItemDefinition && FragmentType)
	{
		for (const TInstancedStruct<FCrimItemDefinitionFragment>& Fragment : ItemDefinition->Fragments)
		{
			if (Fragment.IsValid() && Fragment.GetScriptStruct()->IsChildOf(FragmentType))
			{
				return Fragment;
			}
		}
	}
	return TInstancedStruct<FCrimItemDefinitionFragment>();
}
