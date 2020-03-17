// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "DSOpenLevelCharacter.h"
#include "MyPlayerController.h"
#include "FileHelper.h"

UMyGameInstance::UMyGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UMyGameInstance::Init()
{
	ClientPakPlatformFile = new FPakPlatformFile();
	UE_LOG(LogTemp, Warning, TEXT("--------------------Client Init--------------------"));
	ClientPakPlatformFile->Initialize(&FPlatformFileManager::Get().GetPlatformFile(), TEXT(""));
	FPlatformFileManager::Get().SetPlatformFile(*ClientPakPlatformFile);
}