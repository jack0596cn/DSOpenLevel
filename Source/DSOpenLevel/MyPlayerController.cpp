// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyGameInstance.h"
#include "FileHelper.h"

AMyPlayerController::AMyPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPlayerController::ClientAsync_Mount_MapPak(const FString& _MapPakName)
{
	UE_LOG(LogTemp, Warning, TEXT("--------------------ClientAsync_Mount_MapPak--------------------"));
	IPlatformFile& InnerPlatform = FPlatformFileManager::Get().GetPlatformFile();

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();
	PakPlatformFile->Initialize(&InnerPlatform, TEXT(""));

	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	FString PakForlderName, FileExtn;
	_MapPakName.Split(TEXT("."), &PakForlderName, &FileExtn);

	const FString PakFileFullName = FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH, PakForlderName, _MapPakName);

	FString MountPoint(FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH, PakForlderName, TEXT("/")));
	FPakFile* Pak = new FPakFile(&InnerPlatform, *PakFileFullName, false);

	if (Pak->IsValid())
	{
		PakPlatformFile->Mount(*PakFileFullName, 1000, *MountPoint);

		TArray<FString> Files;
		Pak->FindFilesAtPath(Files, *(Pak->GetMountPoint()), true, false, true);

		for (auto File : Files)
		{
			FString Filename, _FileExtn;
			int32 LastSlashIndex;
			File.FindLastChar(*TEXT("/"), LastSlashIndex);
			FString FileOnly = File.RightChop(LastSlashIndex + 1);
			FileOnly.Split(TEXT("."), &Filename, &_FileExtn);

			if (_FileExtn == TEXT("umap"))
			{
				File = FPaths::Combine(TEXT("/Game"), MAP_ROOT_PATH, PakForlderName, Filename + TEXT(".") + Filename);
				ObjectPaths.AddUnique(FSoftObjectPath(File));
				//将FSoftObjectPath直接转换为TSoftObjectPtr<UObject>并储存
				ObjectPtrs.AddUnique(TSoftObjectPtr<UObject>(ObjectPaths[ObjectPaths.Num() - 1]));
			}
		}
		UAssetManager::GetStreamableManager().RequestAsyncLoad(ObjectPaths, FStreamableDelegate::CreateUObject(this, &AMyPlayerController::CreateAllChildren_Client));
	}
}

void AMyPlayerController::CreateAllChildren_Client()
{
	UE_LOG(LogTemp, Log, TEXT("client finished loading assets"));
	for (int32 i = 0; i < ObjectPtrs.Num(); ++i)
	{
		UObject* LoadObject = ObjectPtrs[i].Get();
		if (LoadObject != nullptr)
		{
			FString _MapPath = ObjectPtrs[i].ToString();
			FString ShortMapName = FPackageName::GetShortName(_MapPath);
			FString _MapName, FileExtn;
			ShortMapName.Split(TEXT("."), &_MapName, &FileExtn);
			FString PakName = _MapName + TEXT(".pak");
			ServerAsync_Mount_MapPak(PakName);
			UE_LOG(LogTemp, Log, TEXT("client Object Load Success...ShortMapName is %s"), *ShortMapName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("client Can not Load asset..."))
		}
	}
}

bool AMyPlayerController::ServerAsync_Mount_MapPak_Validate(const FString& _MapPakName)
{
	return true;
}

void AMyPlayerController::ServerAsync_Mount_MapPak_Implementation(const FString& _MapPakName)
{
	UE_LOG(LogTemp, Warning, TEXT("--------------------ServerAsync_Mount_MapPak--------------------: %s"), *_MapPakName);
	IPlatformFile& InnerPlatform = FPlatformFileManager::Get().GetPlatformFile();

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();
	PakPlatformFile->Initialize(&InnerPlatform, TEXT(""));

	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	FString PakForlderName, FileExtn;
	_MapPakName.Split(TEXT("."), &PakForlderName, &FileExtn);

	const FString PakFileFullName = FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH, PakForlderName, _MapPakName);
	UE_LOG(LogTemp, Log, TEXT("-PakFileFullName is: %s"), *PakFileFullName);

	FString MountPoint(FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH, PakForlderName, TEXT("/")));
	UE_LOG(LogTemp, Log, TEXT("-MountPoint is: %s"), *MountPoint);
	FPakFile* Pak = new FPakFile(&InnerPlatform, *PakFileFullName, false);

	if (Pak->IsValid())
	{
		PakPlatformFile->Mount(*PakFileFullName, 1000, *MountPoint);

		TArray<FString> Files;
		Pak->FindFilesAtPath(Files, *(Pak->GetMountPoint()), true, false, true);

		for (auto File : Files)
		{
			FString Filename, _FileExtn;
			int32 LastSlashIndex;
			File.FindLastChar(*TEXT("/"), LastSlashIndex);
			FString FileOnly = File.RightChop(LastSlashIndex + 1);
			FileOnly.Split(TEXT("."), &Filename, &_FileExtn);

			if (_FileExtn == TEXT("umap"))
			{
				File = FPaths::Combine(TEXT("/Game"), MAP_ROOT_PATH, PakForlderName, Filename + TEXT(".") + Filename);
				Server_ObjectPaths.AddUnique(FSoftObjectPath(File));
				//将FSoftObjectPath直接转换为TSoftObjectPtr<UObject>并储存
				Server_ObjectPtrs.AddUnique(TSoftObjectPtr<UObject>(Server_ObjectPaths[Server_ObjectPaths.Num() - 1]));
			}
		}
		UAssetManager::GetStreamableManager().RequestAsyncLoad(Server_ObjectPaths, FStreamableDelegate::CreateUObject(this, &AMyPlayerController::CreateAllChildren_Server));
	}
}

void AMyPlayerController::CreateAllChildren_Server()
{
	UE_LOG(LogTemp, Log, TEXT("server finished loading assets"));
	for (int32 i = 0; i < Server_ObjectPtrs.Num(); ++i)
	{
		UObject* LoadObject = Server_ObjectPtrs[i].Get();
		if (LoadObject != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("ServerMap Object is: %s"),*Server_ObjectPtrs[i].ToString());
			FString _MapPath = Server_ObjectPtrs[i].ToString();
			FString ShortMapName = FPackageName::GetShortName(_MapPath);
			FString _MapName, FileExtn;
			ShortMapName.Split(TEXT("."), &_MapName, &FileExtn);
			ServerChangeMap(_MapName);
			UE_LOG(LogTemp, Log, TEXT("server Object Load Success..."))
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("server Can not Load asset..."))
		}
	}
}

bool AMyPlayerController::ServerChangeMap_Validate(const FString& _MapName)
{
	return true;
}

void AMyPlayerController::ServerChangeMap_Implementation(const FString& _MapName)
{
	GWorld->ServerTravel(_MapName);
}