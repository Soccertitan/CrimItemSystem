// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "CrimItemTypes.h"
#include "CrimItemGenerator.generated.h"

/**
 * An object that provide a function for generating Items from a Drop Table.
 */
UCLASS(ClassGroup = "Crim Item System", BlueprintType, Blueprintable, EditInlineNew)
class CRIMITEMSYSTEM_API UCrimItemGenerator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Generates an array of ItemSpecs that the ItemManagerComponent can use to create new items.
	 * @param UserContextData Arbitrary data to facilitate what gets chosen and how.
	 * @param OutItemSpecs 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "CrimItemGenerator")
	virtual bool GenerateItemSpecs(UObject* UserContextData, TArray<TInstancedStruct<FCrimItemSpec>>& OutItemSpecs) const;

	/**
	 * Generates an array of ItemSpecs that the ItemManagerComponent can use to create new items. 
	 * @param UserContextData 
	 * @param OutItemSpecs 
	 * @return 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "CrimItemGenerator")
	bool K2_GenerateItemSpecs(UObject* UserContextData, TArray<TInstancedStruct<FCrimItemSpec>>& OutItemSpecs) const;

protected:
	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
};
