// Copyright Soccertitan


#include "CrimItemSaveDataTypes.h"

#include "CrimItem.h"
#include "CrimItemContainer.h"
#include "CrimItemDef.h"
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
}

FCrimItemSaveData::FCrimItemSaveData(UCrimItem* InItem, int32 InSlot)
{
	if (!IsValid(InItem))
	{
		return;
	}

	ItemId = InItem->GetItemId();
	ItemDef = InItem->GetItemDefinition()->GetPathName();
	Slot = InSlot;

	FMemoryWriter MemWriter(ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	InItem->Serialize(Ar);
}

FCrimItemSaveData_Child::FCrimItemSaveData_Child(FGuid InGuid, int32 InSlot)
{
	ItemId = InGuid;
	Slot = InSlot;
}
