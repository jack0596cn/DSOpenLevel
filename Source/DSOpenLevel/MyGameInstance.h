// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PlatformFilemanager.h"
#include "IPlatformFilePak.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DSOPENLEVEL_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMyGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init() override;

	void MountDSAllPak();

	void ServerAsync_Mount_MapPak(const FString& _MapPakName);
	void CreateAllChildren_Server();
	
	void InitMountListDS();

	FPakPlatformFile* GetClientPakPlatformFile()
	{
		return ClientPakPlatformFile;
	}

private:
	TArray<FSoftObjectPath> ObjectPaths;
	TArray<TSoftObjectPtr<UObject>> ObjectPtrs;
	TArray<FString> PakList;

	FPakPlatformFile* ClientPakPlatformFile;
};
