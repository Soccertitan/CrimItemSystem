// Copyright Soccertitan


#include "CrimItem.h"

#include "CrimItemDefinition.h"


FCrimItem::FCrimItem()
{
	ItemGuid = FGuid();
	Quantity = 0;
}

bool FCrimItem::IsMatching(const TInstancedStruct<FCrimItem>& TestItem) const
{
	if (!TestItem.IsValid())
	{
		return false;
	}
	
	if (TestItem.GetPtr<FCrimItem>()->ItemDefinition != ItemDefinition)
	{
		return false;
	}

	if (!AreFragmentsEqual(TestItem))
	{
		return false;
	}

	if (TestItem.GetPtr<FCrimItem>()->TagStats != TagStats)
	{
		return false;
	}

	return true;
}

UCrimItemManagerComponent* FCrimItem::GetItemManager() const
{
	return ItemManager.Get();
}

UCrimItemContainerBase* FCrimItem::GetItemContainer() const
{
	return ItemContainer.Get();
}

bool FCrimItem::AreFragmentsEqual(const TInstancedStruct<FCrimItem>& TestItem) const
{
	const FCrimItem* TestItemPtr = TestItem.GetPtr<FCrimItem>();
	
	// Compare this Item's extensions to the TestItem's extensions.
	for (const TInstancedStruct<FCrimItemFragment>& Fragment : Fragments)
	{
		if (Fragment.IsValid())
		{
			if (const FCrimItemFragment* FragmentPtr = TestItemPtr->GetFragmentByScriptStruct(Fragment.GetScriptStruct()))
			{
				if (!FragmentPtr->IsMatching(Fragment))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	// Compare all the TestItem's extensions with this Item's extensions.
	for (const TInstancedStruct<FCrimItemFragment>& Ext : TestItemPtr->Fragments)
	{
		if (Ext.IsValid())
		{
			if (const FCrimItemFragment* FragmentPtr = GetFragmentByScriptStruct(Ext.GetScriptStruct()))
			{
				if (!FragmentPtr->IsMatching(Ext))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

const FCrimItemFragment* FCrimItem::GetFragmentByScriptStruct(const UScriptStruct* Struct) const
{
	for (const TInstancedStruct<FCrimItemFragment>& Fragment : Fragments)
	{
		if (Fragment.GetScriptStruct() == Struct)
		{
			return Fragment.GetPtr<FCrimItemFragment>();
		}
	}
	return nullptr;
}

void FCrimItem::Initialize(const UCrimItemDefinition* ItemDef)
{
	ItemDefinition = ItemDef->GetPathName();

	for (const TTuple<FGameplayTag, int>& Pair : ItemDef->DefaultStats)
	{
		TagStats.AddStack(Pair.Key, Pair.Value);
	}
}
