// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "RSTestGameMode.h"
#include "RSTestHUD.h"
#include "RSTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARSTestGameMode::ARSTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ARSTestHUD::StaticClass();
}
