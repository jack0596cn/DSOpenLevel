// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "MyPlayerController.h"

AMyGameModeBase::AMyGameModeBase()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	//if (PlayerPawnBPClass.Class != NULL)
	{
		//DefaultPawnClass = PlayerPawnBPClass.Class;
		PlayerControllerClass = AMyPlayerController::StaticClass();
		//GameStateClass = AREGameStateBase::StaticClass();
		//GameSessionClass = AREGameSession::StaticClass();
		//HUDClass = AREHUD::StaticClass();
	}
}