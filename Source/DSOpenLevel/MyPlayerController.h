// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlatformFilemanager.h"
#include "IPlatformFilePak.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "DSOpenLevelCharacter.h"
#include "MyPlayerController.generated.h"

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

	UFUNCTION(Exec)
	void LoadMapPakTestTest()
	{
		ClientAsync_Mount_MapPak(TEXT("MiniMap.pak"));
		ServerChangeMap(TEXT("MiniMap"));
	}
	/*
	 * �ͻ����첽Mount .Pak������
	 * @ PakName����׺�ĵ�ͼPak
	 * ��ʽ���ƣ�TEXT("MiniMap.pak")
	 */
	void LoadMap(const FString& PakName)
	{
		ClientAsync_Mount_MapPak(PakName);
		FString _Filename, _FileExtn;
		PakName.Split(TEXT("."), &_Filename, &_FileExtn);
		ServerChangeMap(_Filename);
	}

	//�ͻ����첽��Pak(����ִ��)
	void ClientAsync_Mount_MapPak(const FString& _MapPakName);
	void CreateAllChildren_Client();

	FORCEINLINE void Clear()
	{
		ObjectPaths.Empty();
		ObjectPtrs.Empty();
	}
	/*
	 * ֪ͨDS�л���ͼ
	 */
	UFUNCTION(Server, unreliable, WithValidation)
	void ServerChangeMap(const FString& MapName);

	/*
	 * �����ͻ������е�ͼPak��Mount
	 */
	void InitMountAllMapPak();

private:
	TArray<FSoftObjectPath> ObjectPaths;
	TArray<TSoftObjectPtr<UObject>> ObjectPtrs;
};
