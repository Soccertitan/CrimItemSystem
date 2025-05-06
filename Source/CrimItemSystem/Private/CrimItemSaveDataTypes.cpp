// Copyright Soccertitan


#include "CrimItemSaveDataTypes.h"

#include "CrimItemContainer.h"
#include "CrimItemDefinition.h"
#include "GameplayTagContainer.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

FCrimItemContainerSaveData::FCrimItemContainerSaveData(UCrimItemContainer* InItemContainer)
{
	if (!IsValid(InItemContainer))
	{
		return;
	}

	ContainerId = InItemContainer->GetContainerId();
	ItemContainerClass = InItemContainer->GetClass();

	FMemoryWriter MemWriter(ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	InItemContainer->Serialize(Ar);
	
	for (const FFastCrimItem& FastItem : InItemContainer->GetItems())
	{
		FFastCrimItem* Mutable = const_cast<FFastCrimItem*>(&FastItem);
		Items.Add(FCrimItemSaveData(Mutable->Item));
	}
}

FCrimItemSaveData::FCrimItemSaveData(TInstancedStruct<FCrimItem>& InItem)
{
	if (!InItem.IsValid())
	{
		return;
	}

	const FCrimItem* ItemPtr = InItem.GetPtr<FCrimItem>();
	
	ItemDef = ItemPtr->GetItemDefinition()->GetPathName();

	FMemoryWriter MemWriter(ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	InItem.Serialize(Ar);
}
