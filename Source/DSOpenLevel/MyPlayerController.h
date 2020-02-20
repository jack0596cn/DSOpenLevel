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

	void MountAndServerChange(const FString& PakName)
	{
		MountMap(PakName);
		LoadMap(PakName);
	}
	/*
	 * 建议LoadMap之前必须先调用MountMap.表示想要切换的关卡pak已经Mount到了本地内存中。不然无法正常切换
	 */
	UFUNCTION(Exec)
	void MountMap(const FString& PakName)
	{
		ClientAsync_Mount_MapPak(PakName);
	}
	/*
	 * 客户端异步Mount .Pak到本地
	 * @ PakName带后缀的地图Pak
	 * 格式类似：TEXT("MiniMap.pak")
	 */
	void LoadMap(const FString& PakName)
	{
		FString _Filename, _FileExtn;
		PakName.Split(TEXT("."), &_Filename, &_FileExtn);
		ServerChangeMap(_Filename);
	}
	
	//客户端异步解Pak(本地执行)
	void ClientAsync_Mount_MapPak(const FString& _MapPakName);
	void CreateAllChildren_Client();

	FORCEINLINE void Clear()
	{
		ObjectPaths.Empty();
		ObjectPtrs.Empty();
	}
	/*
	 * 通知DS切换地图
	 */
	UFUNCTION(Server, unreliable, WithValidation)
	void ServerChangeMap(const FString& MapName);

	/*
	 * 遍历客户端所有地图Pak并Mount
	 */
	void InitMountAllMapPak();

private:
	TArray<FSoftObjectPath> ObjectPaths;
	TArray<TSoftObjectPtr<UObject>> ObjectPtrs;
};
