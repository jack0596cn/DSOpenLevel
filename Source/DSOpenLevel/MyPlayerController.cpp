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
	Clear();

	UMyGameInstance* MyInstance = Cast<UMyGameInstance>(GetGameInstance());
	FPakPlatformFile* PakPlatformFile = nullptr;

	if (MyInstance){
		PakPlatformFile = MyInstance->GetClientPakPlatformFile();
	}

	TArray<FString> PakFilenames;
	PakPlatformFile->GetMountedPakFilenames(PakFilenames);

	if (PakFilenames.Num() > 0)
	{
		for (auto fileList : PakFilenames)
		{
			FString ShortMapName = FPackageName::GetShortName(fileList);
			if (ShortMapName.Equals(_MapPakName))
			{
				UE_LOG(LogTemp, Log, TEXT("%s File Has Mounted"))
				return;
			}
		}
	}

	FString PakForlderName, FileExtn;
	_MapPakName.Split(TEXT("."), &PakForlderName, &FileExtn);

	const FString PakFileFullName = FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH, PakForlderName, _MapPakName);

	//测试用MountPoint
	FString MountPoint(FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH, PakForlderName, TEXT("/")));

	FPakFile* Pak = new FPakFile(&FPlatformFileManager::Get().GetPlatformFile(), *PakFileFullName, false);

	if (Pak->IsValid())
	{
		bool bRet = PakPlatformFile->Mount(*PakFileFullName, 1000, *MountPoint);
		if (bRet)
		{
			UE_LOG(LogTemp, Log, TEXT("Mount Success..."))
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Mount Failed..."))
		}

		TArray<FString> Files;
		Pak->FindFilesAtPath(Files, *(Pak->GetMountPoint()), true, false, true);

		for (auto File : Files)
		{
			FString _Filename, _FileExtn;
			int32 LastSlashIndex;
			File.FindLastChar(*TEXT("/"), LastSlashIndex);
			FString FileOnly = File.RightChop(LastSlashIndex + 1);
			FileOnly.Split(TEXT("."), &_Filename, &_FileExtn);

			if (_FileExtn == TEXT("umap"))
			{
				File = FPaths::Combine(TEXT("/Game"), MAP_ROOT_PATH, PakForlderName, _Filename + TEXT(".") + _Filename);
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
	UE_LOG(LogTemp, Log, TEXT("finished loading assets"));
	for (int32 i = 0; i < ObjectPtrs.Num(); ++i)
	{
		UObject* LoadObject = ObjectPtrs[i].Get();
		FString _MapName, FileExtn;
		if (LoadObject != nullptr)
		{
			FString _MapPath = ObjectPtrs[i].ToString();
			FString ShortMapName = FPackageName::GetShortName(_MapPath);
			ShortMapName.Split(TEXT("."), &_MapName, &FileExtn);
			UE_LOG(LogTemp, Warning, TEXT("......Map Load %s.pak Success......"), *_MapName);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("......Map Load %s.pak Failed......"), *_MapName);
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

void AMyPlayerController::InitMountAllMapPak()
{
	TArray<FString> MapList;

	FString MapPath = FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH);
	IFileManager::Get().FindFilesRecursive(MapList, *MapPath, TEXT("*.pak"), true, false, false);

	for (auto MapPak : MapList)
	{
		FString ShortMapName = FPackageName::GetShortName(MapPak);
		ClientAsync_Mount_MapPak(ShortMapName);
	}
}