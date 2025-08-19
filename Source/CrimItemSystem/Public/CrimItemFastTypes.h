// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItem.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "StructUtils/InstancedStruct.h"

#include "CrimItemFastTypes.generated.h"

class UCrimItemContainerBase;
struct FFastCrimItemContainerList;
class UCrimItemManagerComponent;
struct FFastCrimItem;
struct FFastCrimItemList;

/**
 * FastArraySerializerItem wrapper for a CrimItem.
 * 
 * This is needed to avoid data slicing an FCrimItem as a direct FastArraySerializerItem, it needs to be handled with a TInstancedStruct.
 * TInstancedStruct does not support FastArray Serialization. Therefore, we need to wrap it in our own type.
 * 
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FFastCrimItem : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	friend class UCrimItemManagerComponent;
	friend class UCrimItemContainerBase;
	friend struct FFastCrimItemList;

	void Initialize(const TInstancedStruct<FCrimItem>& InItem);

	//~ Begin of FFastArraySerializerItem
	void PostReplicatedAdd(const FFastCrimItemList& InItemList);
	void PostReplicatedChange(const FFastCrimItemList& InItemList);
	void PreReplicatedRemove(const FFastCrimItemList& InItemList);
	//~ End of FFastArraySerializerItem

	/* The replicated Item. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	TInstancedStruct<FCrimItem> Item;

	/** Holds the previous value of the item during a broadcast event.*/
	TInstancedStruct<FCrimItem> GetPreReplicatedItem() const { return PreReplicatedChangeItem; }
private:

	/* A copy of the Item which we use as a lookup for the previous values of changed properties. */
	UPROPERTY(NotReplicated, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TInstancedStruct<FCrimItem> PreReplicatedChangeItem;
};

/**
 * FastArraySerializer of CrimItem.
 *  
 * This should only be used by UCrimItemManagerComponent.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FFastCrimItemList : public FFastArraySerializer
{
    GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_OneParam(FFastCrimItemListChangedSignature, const FFastCrimItem&);

	FFastCrimItemListChangedSignature OnItemAddedDelegate;
	FFastCrimItemListChangedSignature OnItemChangedDelegate;
	FFastCrimItemListChangedSignature OnItemRemovedDelegate;

    FFastCrimItemList(){}

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
    {
        return FastArrayDeltaSerialize<FFastCrimItem, FFastCrimItemList>(Items, DeltaParams, *this);
    }

    /** Adds an Item to the list. */
    void AddItem(const TInstancedStruct<FCrimItem>& Item);

    /** Removes an Item from the list. */
    bool RemoveItem(const FGuid& ItemGuid);

    /** Returns a const reference of all the Items within the container. */
    const TArray<FFastCrimItem>& GetItems() const;

	/** Returns a pointer to an Item. */
	FFastCrimItem* GetItem(const FGuid& ItemGuid) const;

    /** Returns the number of Items in the container. */
    int32 GetNum() const;

	/** Removes all items from this container. */
	void Reset();

private:
	UPROPERTY()
	TArray<FFastCrimItem> Items;
};

template<>
struct TStructOpsTypeTraits<FFastCrimItemList> : public TStructOpsTypeTraitsBase2<FFastCrimItemList>
{
    enum
    {
        WithNetDeltaSerializer = true
    };
};


/**
 * Holds an ItemContainer.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FFastCrimItemContainerItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FString ToDebugString() const;

	UCrimItemContainerBase* GetItemContainer() const { return ItemContainer; }

	//~ Begin of FFastArraySerializerItem
	void PostReplicatedAdd(const FFastCrimItemContainerList& InItemContainerList);
	void PreReplicatedRemove(const FFastCrimItemContainerList& InItemContainerList);
	//~ End of FFastArraySerializerItem

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCrimItemContainerBase> ItemContainer = nullptr;

	friend FFastCrimItemContainerList;

	friend bool operator==(const FFastCrimItemContainerItem& X, const FFastCrimItemContainerItem& Y)
	{
		return X.ItemContainer == Y.ItemContainer;
	}
	friend bool operator!=(const FFastCrimItemContainerItem& X, const FFastCrimItemContainerItem& Y)
	{
		return X.ItemContainer != Y.ItemContainer;
	}
};

/**
 * Holds a list of item containers.
 */
USTRUCT(BlueprintType)
struct CRIMITEMSYSTEM_API FFastCrimItemContainerList : public FFastArraySerializer
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_OneParam(FCrimItemContainerListUpdatedSignature, const FFastCrimItemContainerItem& );
	
	FFastCrimItemContainerList() {}

	/** Called when an ItemContainer has been added to the list. */
	FCrimItemContainerListUpdatedSignature OnItemContainerAddedDelegate;
	/** Called when an ItemContainer is removed from the list. */
	FCrimItemContainerListUpdatedSignature OnItemContainerRemovedDelegate;
	
	/** Adds a new item container the list. */
	void AddItemContainer(UCrimItemContainerBase* ItemContainer);

	/**
	 * Removes the item container from the list.
	 */
	void RemoveItemContainer(UCrimItemContainerBase* ItemContainer);
	
	/** Gets a const reference to the item containers. */
	const TArray<FFastCrimItemContainerItem>& GetItemContainers() const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FFastCrimItemContainerItem, FFastCrimItemContainerList>(Items, DeltaParms, *this);
	}

private:
	/** Replicated list of item containers. */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<FFastCrimItemContainerItem> Items;
};

template <>
struct TStructOpsTypeTraits<FFastCrimItemContainerList> : public TStructOpsTypeTraitsBase2<FFastCrimItemContainerList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
