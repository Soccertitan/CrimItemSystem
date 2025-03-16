// Copyright Soccertitan


#include "CrimItemStatics.h"

#include "CrimItemManagerComponent.h"
#include "CrimItemSystemInterface.h"

UCrimItemManagerComponent* UCrimItemStatics::GetCrimItemManagerComponent(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	if (Actor->Implements<UCrimItemSystemInterface>())
	{
		return ICrimItemSystemInterface::Execute_GetCrimItemManagerComponent(Actor);
	}

	return Actor->FindComponentByClass<UCrimItemManagerComponent>();
}
