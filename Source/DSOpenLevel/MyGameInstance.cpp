// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "DSOpenLevelCharacter.h"
#include "FileHelper.h"

UMyGameInstance::UMyGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UMyGameInstance::Init()
{
	if (IsDedicatedServerInstance())
	{
		InitMountListDS();
		UE_LOG(LogTemp, Warning, TEXT("--------------------1--------------------"));
		AsyncTask(ENamedThreads::GameThread, [=]()
		{
			MountDSAllPak();
		});
	}
	else
	{
		//InnerPlatform = FPlatformFileManager::Get().GetPlatformFile();
		ClientPakPlatformFile = new FPakPlatformFile();

		ClientPakPlatformFile->Initialize(&FPlatformFileManager::Get().GetPlatformFile(), TEXT(""));
		FPlatformFileManager::Get().SetPlatformFile(*ClientPakPlatformFile);
	}
}

void UMyGameInstance::MountDSAllPak()
{
	for (auto MapPak : PakList)
	{
		ServerAsync_Mount_MapPak(MapPak);
	}
}

void UMyGameInstance::ServerAsync_Mount_MapPak(const FString& _MapPakName)
{
	UE_LOG(LogTemp, Warning, TEXT("--------------------ServerAsync_Mount_MapPak--------------------"));
	IPlatformFile& InnerPlatform = FPlatformFileManager::Get().GetPlatformFile();

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();

	PakPlatformFile->Initialize(&InnerPlatform, TEXT(""));

	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	FString PakForlderName, FileExtn;
	_MapPakName.Split(TEXT("."), &PakForlderName, &FileExtn);

	const FString PakFileFullName = FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH, PakForlderName, _MapPakName);

	//测试用MountPoint
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

		UAssetManager::GetStreamableManager().RequestAsyncLoad(ObjectPaths, FStreamableDelegate::CreateUObject(this, &UMyGameInstance::CreateAllChildren_Server));
	}
}

void UMyGameInstance::CreateAllChildren_Server()
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

void UMyGameInstance::InitMountListDS()
{
	PakList.Empty();
	TArray<FString> MapList;

	FString MapPath = FPaths::Combine(FPaths::ProjectContentDir(), MAP_ROOT_PATH);
	IFileManager::Get().FindFilesRecursive(MapList, *MapPath, TEXT("*.pak"), true, false, false);

	for (auto MapPak : MapList)
	{
		FString ShortMapName = FPackageName::GetShortName(MapPak);
		PakList.Add(ShortMapName);
	}
}