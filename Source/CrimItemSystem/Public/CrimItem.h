// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "CrimItemTypes.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif
#include "CrimItem.generated.h"

class UCrimItemManagerComponent;
class UCrimItemDef;
class UCrimItemContainer;

DECLARE_MULTICAST_DELEGATE_TwoParams(FCrimItemQuantityUpdatedSignature, int32 /*NewCount*/, int32 /*OldCount*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCrimItemTagStatUpdatedSignature, const FGameplayTag& /*Tag*/, int32 /*NewValue*/, int32 /*OldValue*/);

/**
 * An item!
 */
UCLASS(ClassGroup = "Crim Item System", BlueprintType, Blueprintable)
class CRIMITEMSYSTEM_API UCrimItem : public UObject
{
	GENERATED_BODY()
	
	friend UCrimItemContainer;
	friend UCrimItemManagerComponent;
	
public:
	virtual void PostInitProperties() override;
	virtual bool IsSupportedForNetworking() const override {return true;}
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#if UE_WITH_IRIS
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif

public:
	/** Returns this item's item definition. */
	UFUNCTION(BlueprintPure, DisplayName = "GetItemDefinition", Category = "CrimItem")
	const UCrimItemDef* GetItemDefinition() const;

	/** Returns the item's unique identifier */
	UFUNCTION(BlueprintPure, Category = "CrimItem")
	FGuid GetItemId() const {return ItemId;}

	/** Returns the item's description. */
	UFUNCTION(BlueprintNativeEvent, Category = "CrimItem")
	FText GetDescription() const;

	/** Returns the owning item's ItemManagerComponent. */
	UFUNCTION(BlueprintPure, Category = "CrimItem")
	UCrimItemManagerComponent* GetItemManagerComponent() const { return ItemManagerComponent; }

	/** Return the owned tags of the item definition. */
	UFUNCTION(BlueprintPure, Category="CrimItem")
	const FGameplayTagContainer& GetOwnedTags() const;

	/** Set's the item's quantity to the new quantity. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="CrimItem")
	void SetQuantity(int32 NewQuantity);
	
	/** Returns the number of items in this instance. */
	UFUNCTION(BlueprintPure, Category="CrimItem")
	int32 GetQuantity() const { return Quantity; }

	/** Called when the quantity of this item stack has changed. */
	FCrimItemQuantityUpdatedSignature OnQuantityUpdatedDelegate;

	/** Return the stack count for a tag, or 0 if the tag is not present. */
	UFUNCTION(BlueprintPure, Category = "CrimItem")
	int32 GetTagStat(FGameplayTag Tag) const;

	/** Add stacks to a tag. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="CrimItem")
	void AddTagStat(FGameplayTag Tag, int32 Delta);

	/** Remove stacks from a tag. If Delta is -1 removes all. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="CrimItem")
	void RemoveTagStat(FGameplayTag Tag, int32 Delta);

	/** Called when a tag stat of this item has changed. */
	FCrimItemTagStatUpdatedSignature OnTagStatUpdatedDelegate;
	
	/**
	 * Return true if the TestItem has the same item definition and TagStats as this item.
	 */
	UFUNCTION(BlueprintPure, Category = "CrimItem")
	bool IsMatching(const UCrimItem* TestItem) const;

protected:
	/**
	 * Called once by the ItemManagerComponent upon constructing this item.
	 */
	virtual void Initialize(UCrimItemManagerComponent* ItemManager, const UCrimItemDef* ItemDef, FGuid NewItemId);

	/**
	 * Copy properties from an item to this item. Currently, it just copies the item's TagStats property.
	 * @param Item The item to copy properties from.
	 */
	virtual void CopyItemProperties(const UCrimItem* Item);

	/**
	 * Called in the ItemManager when generating brand-new items that are not copied from existing items.
	 * @param Spec Applies an ItemSpec to this item.
	 */
	virtual void ApplyItemSpec(const TInstancedStruct<FCrimItemSpec>& Spec);

	/**
	 * Called in the ItemManager upon constructing an item with just an ItemDefinition.
	 * The ItemDef is valid upon being called.
	 */
	virtual void ApplyItemDef(const UCrimItemDef* ItemDef);
	
private:
	
	/** The unique identifier for this item. */
	UPROPERTY(Replicated)
	FGuid ItemId;

	/** The item definition */
	UPROPERTY(Replicated)
	TSoftObjectPtr<UCrimItemDef> ItemDefinition;
	
	/** The quantity of this item in this instance */
	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_Quantity)
	int32 Quantity = 0;

	/** Tags representing various stats about this item, such as level, use count, remaining ammo, etc. */
	UPROPERTY(SaveGame, Replicated)
	FCrimItemTagStackContainer TagStats;

	/** The owning item manager component. */
	UPROPERTY(Replicated)
	TObjectPtr<UCrimItemManagerComponent> ItemManagerComponent;
	UPROPERTY()
	bool bOwnerIsNetAuthority = false;
	
	UFUNCTION()
	void OnRep_Quantity(int32 OldValue);
	
public:
	UFUNCTION(BlueprintPure, Category = "CrimItem")
	FString ToDebugString() const;
};

