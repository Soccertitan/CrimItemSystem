// Copyright Soccertitan


#include "CrimItemDefinition.h"

#include "UI/ViewModel/CrimItemViewModel.h"
#include "UObject/AssetRegistryTagsContext.h"

UCrimItemDefinition::UCrimItemDefinition()
{
	ItemClass.InitializeAsScriptStruct(FCrimItem::StaticStruct());
}

FPrimaryAssetId UCrimItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("CrimItemDefinition", GetFName());
}

void UCrimItemDefinition::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

	// Adding OwnedTags to the AssetRegistry.
	FAssetRegistryTag RegistryTag;
	RegistryTag.Type = FAssetRegistryTag::TT_Hidden;
	RegistryTag.Name = "OwnedTags";
	RegistryTag.Value = OwnedTags.ToString();
	Context.AddTag(RegistryTag);

	for (const TInstancedStruct<FCrimItemDefinitionFragment>& Fragment : Fragments)
	{
		if (const FCrimItemDefinitionFragment* Ptr = Fragment.GetPtr<FCrimItemDefinitionFragment>())
		{
			Ptr->GetAssetRegistryTags(Context);
		}
	}
}
