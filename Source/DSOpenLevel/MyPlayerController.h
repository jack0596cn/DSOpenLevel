// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlatformFilemanager.h"
#include "IPlatformFilePak.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "DSOpenLevelCharacter.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "PlatformFilemanager.h"
#include "IPlatformFilePak.h"
#include "MyPlayerController.generated.h"

#define MAP_ROOT_PATH TEXT("Maps")

/**
 * 
 */
UCLASS()
class DSOPENLEVEL_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AMyPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	//�ͻ����첽��Pak(����ִ��)�����Ե�ʱ��ֱ���ڿ���̨����ClientAsync_Mount_MapPak
	UFUNCTION(Exec)
	void ClientAsync_Mount_MapPak(const FString& _MapPakName);
	void CreateAllChildren_Client();

	//�������첽��Pak(Serverִ��)
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAsync_Mount_MapPak(const FString& _MapPakName);
	void CreateAllChildren_Server();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerChangeMap(const FString& _MapName);

private:
	//pak�ļ��е��ļ�·���б�
	TArray<FSoftObjectPath> ObjectPaths;
	TArray<TSoftObjectPtr<UObject>> ObjectPtrs;

	TArray<FSoftObjectPath> Server_ObjectPaths;
	TArray<TSoftObjectPtr<UObject>> Server_ObjectPtrs;
};
