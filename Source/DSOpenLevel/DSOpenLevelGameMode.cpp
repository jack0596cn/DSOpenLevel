// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DSOpenLevelGameMode.h"
#include "DSOpenLevelCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADSOpenLevelGameMode::ADSOpenLevelGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
