// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemTypes.h"

#include "CrimItem.generated.h"

class UCrimItemManagerComponent;
class UCrimItemContainer;

/**
 * Extends the base functionality of an item.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FCrimItemExtension
{
	GENERATED_BODY()

	FCrimItemExtension() {}
	virtual ~FCrimItemExtension() {}
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
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 Quantity;

	/** Tags representing various stats about this item, such as level, use count, remaining ammo, etc... */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FCrimItemTagStackContainer TagStats;

	/** Extends an item's capabilities. */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	TArray<TInstancedStruct<FCrimItemExtension>> Extensions;

	/** Returns true if the TestItem has the same ItemDefinition and TagStats as this item. */
	virtual bool IsMatching(const TInstancedStruct<FCrimItem>& TestItem) const;

	UCrimItemManagerComponent* GetItemManager() const;
	UCrimItemContainer* GetItemContainer() const;
	
protected:
	/** Called from the ItemContainer when it creates a new item. */
	virtual void Initialize(FGuid NewItemId, const UCrimItemDefinition* ItemDef);

	/** The ItemContainer will call this when generating a new item from a template. */
	virtual void ApplyItemSpec(const TInstancedStruct<FCrimItemSpec>& Spec);

	/** The ItemContainer will call this when generating an item from just the ItemDefinition. */
	virtual void ApplyItemDef(const UCrimItemDefinition* ItemDef);

private:
	
	/** The globally unique identifier for this item. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	FGuid ItemGuid;

	/** The static data representing this item. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true), SaveGame)
	TSoftObjectPtr<UCrimItemDefinition> ItemDefinition;
	
	/** The ItemManager that owns this item. */
	UPROPERTY(BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UCrimItemManagerComponent> ItemManager;
	
	/** The ItemContainer this item resides in. */
	UPROPERTY(BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UCrimItemContainer> ItemContainer;

	friend UCrimItemManagerComponent;
	friend UCrimItemContainer;
};

