// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemTypes.h"

#include "CrimItem.generated.h"

class UCrimItemContainerBase;
class UCrimItemManagerComponent;

/**
 * Contains custom state information for an item. Mark your UPROPERTY's with SaveGame to ensure the state saves.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemFragment
{
	GENERATED_BODY()

	FCrimItemFragment() {}
	virtual ~FCrimItemFragment() {}

	virtual bool IsMatching(const TInstancedStruct<FCrimItemFragment>& Fragment) const {return true;}
};

/**
 * The base representation of an Item. This can be extended with child structs.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItem
{
	GENERATED_BODY()

	FCrimItem();
	virtual ~FCrimItem(){}

	FGuid GetItemGuid() const { return ItemGuid; }
	TSoftObjectPtr<UCrimItemDefinition> GetItemDefinition() const { return ItemDefinition; }

	/** The quantity of this item instance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 Quantity;

	/** Tags representing various stats about this item, such as level, use count, remaining ammo, etc... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FCrimItemTagStackContainer TagStats;

	/** Extends an item's capabilities. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, SaveGame, meta = (FullyExpand=true, ExcludeBaseStruct))
	TArray<TInstancedStruct<FCrimItemFragment>> Fragments;

	/** Returns true if this Item has the same ItemDefinition, TagStats, and ItemExtensions as the TestItem. */
	virtual bool IsMatching(const TInstancedStruct<FCrimItem>& TestItem) const;

	UCrimItemManagerComponent* GetItemManager() const;
	UCrimItemContainerBase* GetItemContainer() const;

	template<typename T> requires std::derived_from<T, FCrimItemFragment>
	const T* GetFragmentByType() const;
	template<typename T> requires std::derived_from<T, FCrimItemFragment>
	T* GetMutableFragmentByType();
	
protected:
	/** Called when the ItemContainer creates a new item. */
	virtual void Initialize(const UCrimItemDefinition* ItemDef);

private:
	
	/** The globally unique identifier for this item. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	FGuid ItemGuid;

	/** The static data representing this item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	TSoftObjectPtr<UCrimItemDefinition> ItemDefinition;
	
	/** The ItemManager that owns this item. */
	UPROPERTY(BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UCrimItemManagerComponent> ItemManager;
	
	/** The ItemContainer this item resides in. */
	UPROPERTY(BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UCrimItemContainerBase> ItemContainer;

	friend UCrimItemManagerComponent;
	friend UCrimItemContainerBase;

	/**
	 * Iterates through ThisItem and TestItem extensions. And calls IsMatching on each one.
	 */
	bool AreFragmentsEqual(const TInstancedStruct<FCrimItem>& TestItem) const;
	const FCrimItemFragment* GetFragmentByScriptStruct(const UScriptStruct* Struct) const;
};

/**
 * @return A const pointer to the first FCrimItemExtension that matches the type.
 */
template <typename T> requires std::derived_from<T, FCrimItemFragment>
const T* FCrimItem::GetFragmentByType() const
{
	for (const TInstancedStruct<FCrimItemFragment>& Fragment : Fragments)
	{
		if (const T* Ptr = Fragment.GetPtr<T>())
		{
			return Ptr;
		}
	}
	return nullptr;
}

/**
 * @return A mutable pointer to the first FCrimItemExtension that matches the type.
 */
template <typename T> requires std::derived_from<T, FCrimItemFragment>
T* FCrimItem::GetMutableFragmentByType()
{
	for (TInstancedStruct<FCrimItemFragment>& Fragment : Fragments)
	{
		if (T* Ptr = Fragment.GetMutablePtr<T>())
		{
			return Ptr;
		}
	}
	return nullptr;
}
