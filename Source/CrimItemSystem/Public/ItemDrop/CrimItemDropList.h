// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"

#include "Net/Serialization/FastArraySerializer.h"
#include "CrimItemDropList.generated.h"

class UCrimItem;
class ACrimItemDrop;

/**
 * Holds an ItemDrop information.
 */
USTRUCT()
struct CRIMITEMSYSTEM_API FCrimItemDropListEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ACrimItemDrop> ItemDropActor;
	
	UPROPERTY()
	TObjectPtr<UCrimItem> Item;
	
	friend bool operator==(const FCrimItemDropListEntry& X, const FCrimItemDropListEntry& Y)
	{
		return X.ItemDropActor == Y.ItemDropActor;
	}
	friend bool operator!=(const FCrimItemDropListEntry& X, const FCrimItemDropListEntry& Y)
	{
		return X.ItemDropActor != Y.ItemDropActor;
	}
};

USTRUCT()
struct CRIMITEMSYSTEM_API FCrimItemDropList : public FFastArraySerializer
{
	GENERATED_BODY()

	FCrimItemDropList() { }

	void AddEntry(FCrimItemDropListEntry& Entry);
	void RemoveEntry(ACrimItemDrop* ItemDropActor);

	const TArray<FCrimItemDropListEntry>& GetItems() const;
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FCrimItemDropListEntry, FCrimItemDropList>(Items, DeltaParms, *this);
	}

private:
	UPROPERTY()
	TArray<FCrimItemDropListEntry> Items;
};

template <>
struct TStructOpsTypeTraits<FCrimItemDropList> : public TStructOpsTypeTraitsBase2<FCrimItemDropList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
