// Copyright Soccertitan


#include "CrimItemGameplayTags.h"

#include "GameplayTagsManager.h"

FCrimItemGameplayTags FCrimItemGameplayTags::GameplayTags;

void FCrimItemGameplayTags::InitializeNativeGameplayTags()
{
	GameplayTags.Crim_ItemContainer_Default = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Crim.ItemContainer.Default"), FString("The default ContainerId for an ItemContainer."));
	GameplayTags.Crim_ItemContainer_ItemDropManager = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Crim.ItemContainer.ItemDropManager"), FString("The tag used by the ItemDropManager for it's default ItemContainer."));
	
	GameplayTags.Crim_ItemPlan_Error = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Crim.ItemPlan.Error"), FString("Root Gameplay Tag for when an Item can't be fully added to the ItemContainer."));
	
	GameplayTags.Crim_ItemPlan_Error_InvalidItem = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Crim.ItemPlan.Error.InvalidItem"), FString("The item is invalid/nullptr."));
	GameplayTags.Crim_ItemPlan_Error_ItemContainerNotFound = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Crim.ItemPlan.Error.ItemContainerNotFound"), FString("Could not find a valid Item Container."));
	GameplayTags.Crim_ItemPlan_Error_CantContainItem = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Crim.ItemPlan.Error.CantContainItem"), FString("The item is not allowed to be in the container."));
	GameplayTags.Crim_ItemPlan_Error_QuantityIsZero = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Crim.ItemPlan.Error.QuantityIsZero"), FString("Tried to add less than 0 item quantity."));
	GameplayTags.Crim_ItemPlan_Error_MaxStacksReached = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Crim.ItemPlan.Error.MaxStacksReached"), FString("The container has reached maximum amount of stacks for the item or the container."));
}
