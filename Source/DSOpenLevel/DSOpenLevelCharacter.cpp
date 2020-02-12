// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DSOpenLevelCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "PlatformFilemanager.h"
#include "IPlatformFilePak.h"
#include "GameFramework/PlayerInput.h"

//////////////////////////////////////////////////////////////////////////
// ADSOpenLevelCharacter

ADSOpenLevelCharacter::ADSOpenLevelCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ADSOpenLevelCharacter::BeginPlay()
{
	//FString AssetName = FPaths::Combine(TEXT("/Game"), MapPath, MapName, MapPakName);

	Super::BeginPlay();
	//UGameplayStatics::OpenLevel(GWorld, *FString("127.0.0.1:7777"));
}

void ADSOpenLevelCharacter::InitializeInputBindings()
{
	static bool bBindingsAdded = false;
	if (!bBindingsAdded)
	{
		bBindingsAdded = true;
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("FPress", EKeys::F));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("GPress", EKeys::G));
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADSOpenLevelCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	InitializeInputBindings();

	PlayerInputComponent->BindAction("GPress", IE_Pressed, this, &ADSOpenLevelCharacter::LoadLocalMap);
	PlayerInputComponent->BindAction("FPress", IE_Pressed, this, &ADSOpenLevelCharacter::InitLevel);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ADSOpenLevelCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADSOpenLevelCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ADSOpenLevelCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ADSOpenLevelCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ADSOpenLevelCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ADSOpenLevelCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ADSOpenLevelCharacter::OnResetVR);
}


void ADSOpenLevelCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ADSOpenLevelCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ADSOpenLevelCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ADSOpenLevelCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ADSOpenLevelCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ADSOpenLevelCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ADSOpenLevelCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

bool ADSOpenLevelCharacter::ServerChangeMap_Validate(const FString& _MapName)
{
	return true;
}

void ADSOpenLevelCharacter::ServerChangeMap_Implementation(const FString& _MapName)
{
	GWorld->ServerTravel(_MapName);
}

bool ADSOpenLevelCharacter::ServerSync_Mount_MapPak_Validate(const FString& _MapPakName)
{
	return true;
}

void ADSOpenLevelCharacter::ServerSync_Mount_MapPak_Implementation(const FString& _MapPakName)
{
	FString SaveContentDir = FPaths::Combine(FPaths::ProjectContentDir(), MapPath, MapName, _MapPakName);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();
	PakPlatformFile->Initialize(&PlatformFile, TEXT(""));
	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	FPakFile PakFile(&PlatformFile, *SaveContentDir, false);
	//FString MountPoint(FPaths::ProjectContentDir() + TEXT("DLC/"));
	FString MountPoint(FPaths::Combine(FPaths::ProjectContentDir(), MapPath, MapName));
	PakFile.SetMountPoint(*MountPoint);

	if (PakPlatformFile->Mount(*SaveContentDir, 0, *MountPoint))
	{
		TArray<FString> FileList;
		PakFile.FindFilesAtPath(FileList, *PakFile.GetMountPoint(), true, false, true);
		FStreamableManager StreamableManager;

		for (int32 Index = 0; Index < FileList.Num(); Index++)
		{
			FString AssetName = FileList[Index];
			FString AssetShortName = FPackageName::GetShortName(AssetName);
			FString LeftStr, FileExtn;
			AssetShortName.Split(TEXT("."), &LeftStr, &FileExtn);

			if (FileExtn == TEXT("umap"))
			{
				AssetName = FPaths::Combine(TEXT("/Game"), MapPath, MapName, LeftStr + TEXT(".") + LeftStr);
				//AssetName = FPaths::Combine(TEXT("/Game"), MapPath, _MapPakName);
				//AssetName = TEXT("/Game/DLC/") + LeftStr + TEXT(".") + LeftStr;
				FStringAssetReference reference = AssetName;
				//Load UObject  
				UObject* LoadObject = StreamableManager.LoadSynchronous(reference);
				if (LoadObject != nullptr)
				{
					UE_LOG(LogTemp, Log, TEXT("Object Load Success..."))
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Can not Load asset..."))
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mount Failed"));
	}
}

void ADSOpenLevelCharacter::InitLevel()
{
	UGameplayStatics::OpenLevel(GWorld, *FString("127.0.0.1:7777"));
}

void ADSOpenLevelCharacter::ClientSyncMount()
{
	ClientSync_Mount_MapPak(MapPakName);
	UGameplayStatics::OpenLevel(GWorld, *MapName);
}

void ADSOpenLevelCharacter::ClientAsyncMount()
{
	ClientAsync_Mount_MapPak(MapPakName);
}

void ADSOpenLevelCharacter::Test()
{
// 	//客户端连接DS
// 	InitLevel();
	//客户端解pak,以备Server调用
	ClientSync_Mount_MapPak(MapPakName);
	UE_LOG(LogTemp, Log, TEXT("Client Mount MiniMap Finished!!"));
	//服务器解pak
	ServerAsync_Mount_MapPak(MapPakName);
	UE_LOG(LogTemp, Log, TEXT("Server Mount MiniMap Finished!!"));
	//服务器端执行ServerTravel
	//ServerChangeMap(MapName);
	//UE_LOG(LogTemp, Log, TEXT("ServerTravel Finished!!"));
}


void ADSOpenLevelCharacter::ClientSync_Mount_MapPak(const FString& _MapPakName)
{	

	FString SaveContentDir = FPaths::Combine(FPaths::ProjectContentDir(), MapPath, MapName, _MapPakName);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();
	PakPlatformFile->Initialize(&PlatformFile, TEXT(""));
	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	FPakFile PakFile(&PlatformFile, *SaveContentDir, false);
	//FString MountPoint(FPaths::ProjectContentDir() + TEXT("DLC/"));
	FString MountPoint(FPaths::Combine(FPaths::ProjectContentDir(), MapPath, MapName));
	PakFile.SetMountPoint(*MountPoint);

	if (PakPlatformFile->Mount(*SaveContentDir, 0, *MountPoint))
	{
		TArray<FString> FileList;
		PakFile.FindFilesAtPath(FileList, *PakFile.GetMountPoint(), true, false, true);
		FStreamableManager StreamableManager;

		for (int32 Index = 0; Index < FileList.Num(); Index++)
		{
			FString AssetName = FileList[Index];
			FString AssetShortName = FPackageName::GetShortName(AssetName);
			FString LeftStr, FileExtn;
			AssetShortName.Split(TEXT("."), &LeftStr, &FileExtn);

			if (FileExtn == TEXT("umap"))
			{
				AssetName = FPaths::Combine(TEXT("/Game"), MapPath, MapName, LeftStr + TEXT(".") + LeftStr);
				//AssetName = FPaths::Combine(TEXT("/Game"), MapPath, _MapPakName);
				//AssetName = TEXT("/Game/DLC/") + LeftStr + TEXT(".") + LeftStr;
				FStringAssetReference reference = AssetName;
				//Load UObject  
				UObject* LoadObject = StreamableManager.LoadSynchronous(reference);
				if (LoadObject != nullptr)
				{
					UE_LOG(LogTemp, Log, TEXT("Object Load Success..."))
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Can not Load asset..."))
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Mount Failed"));
	}
}

void ADSOpenLevelCharacter::ClientAsync_Mount_MapPak(const FString& _MapPakName)
{
	//第一步
	//FPlatformFileManager::Get()返回单例
	//GetPlatformFile()返回相应平台的PlatformFile，即处理相应平台文件读写的对象
	//因此在Windows平台，这里返回的是FWindowsPlatformFile的实例
	IPlatformFile& InnerPlatform = FPlatformFileManager::Get().GetPlatformFile();

	//第二步
	//这里创建了一个FPakPlatformFile，但是未指定当前使用什么平台去读写这个文件
	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();

	//第三步
	//使用相应平台的PlatformFile去初始化PakPlatformFile
	//第二个参数是命令行参数，一般都为空
	PakPlatformFile->Initialize(&InnerPlatform, TEXT(""));

	//第四步
	//再将当前PlatformFile设置为"相应平台下pak文件读写"的模式
	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	//const FString PakFileFullName = TEXT("F:\\test_unreal\PakTest\Content\DLC\MiniMap.pak");
	//const FString PakFileFullName = FPaths::ProjectContentDir() + TEXT("ThirdPersonCPP/Maps") + _MapPakName;
	const FString PakFileFullName = FPaths::Combine(FPaths::ProjectContentDir(), MapPath, MapName, _MapPakName)/* FPaths::ProjectContentDir() + TEXT("ThirdPersonCPP/Maps") + _MapPakName*/;

	//测试用MountPoint
	FString MountPoint(FPaths::Combine(FPaths::ProjectContentDir(), MapPath, MapName, TEXT("/")));
	//FString MountPoint(FPaths::ProjectContentDir() + TEXT("DLC/")/*FPaths::EngineContentDir()*/);

	//创建FPakFile对象,同样使用相应平台的PlatformFile初始化
	//第二个参数是pak文件的完整路径+名称
	//第三个参数是是否有符号？
	FPakFile* Pak = new FPakFile(&InnerPlatform, *PakFileFullName, false);

	if (Pak->IsValid())
	{
		//具体Mount方法可以参考函数 FPakPlatformFile::Mount
		//但是其中有大量多余内容(例如版本编号处理)
		//Pak->SetMountPoint(*MountPoint);

		PakPlatformFile->Mount(*PakFileFullName, 1000, *MountPoint);

		TArray<FString> Files;
		Pak->FindFilesAtPath(Files, *(Pak->GetMountPoint()), true, false, true);

		//FStreamableManager StreamableManager;

		for (auto File : Files)
		{
			FString Filename, FileExtn;
			int32 LastSlashIndex;
			File.FindLastChar(*TEXT("/"), LastSlashIndex);
			FString FileOnly = File.RightChop(LastSlashIndex + 1);
			FileOnly.Split(TEXT("."), &Filename, &FileExtn);

			if (FileExtn == TEXT("umap"))
			{
				//File = FileOnly.Replace(TEXT("uasset"), *Filename);
				//File = TEXT("/Game/DLC/") + File;
				File = FPaths::Combine(TEXT("/Game"), MapPath, MapName, Filename + TEXT(".") + Filename);
				//File = TEXT("/Game/DLC/") + Filename + TEXT(".") + Filename;
				//File = TEXT("/Game/DLC/") + Filename + TEXT(".") + Filename;
				ObjectPaths.AddUnique(FSoftObjectPath(File));

				//将FSoftObjectPath直接转换为TSoftObjectPtr<UObject>并储存
				ObjectPtrs.AddUnique(TSoftObjectPtr<UObject>(ObjectPaths[ObjectPaths.Num() - 1]));
			}
		}

		UAssetManager::GetStreamableManager().RequestAsyncLoad(ObjectPaths, FStreamableDelegate::CreateUObject(this, &ADSOpenLevelCharacter::CreateAllChildren_Client));
	}

}

void ADSOpenLevelCharacter::CreateAllChildren_Client()
{
	UE_LOG(LogTemp, Log, TEXT("finished loading assets"));
	for (int32 i = 0; i < ObjectPtrs.Num(); ++i)
	{
		UObject* LoadObject = ObjectPtrs[i].Get();
		if (LoadObject != nullptr)
		{
			FString _MapPath = ObjectPtrs[i].ToString();
			FString ShortMapName = FPackageName::GetShortName(_MapPath);
			FString _MapName, FileExtn;
			ShortMapName.Split(TEXT("."), &_MapName, &FileExtn);
			UGameplayStatics::OpenLevel(GWorld, *_MapName);
			UE_LOG(LogTemp, Log, TEXT("Object Load Success..."))
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Can not Load asset..."))
		}
		// 		UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this);
		// 		if (!NewComp)
		// 		{
		// 			return;
		// 		}
		// 		UStaticMesh* staticMesh = Cast<UStaticMesh>(ObjectPtrs[i].Get());

		// 		NewComp->SetStaticMesh(staticMesh);
		// 		NewComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
		// 		NewComp->SetRelativeLocation(FVector(0, (i + 1) * 100.0f, 0));
		// 		NewComp->RegisterComponent();
	}
}


bool ADSOpenLevelCharacter::ServerAsync_Mount_MapPak_Validate(const FString& _MapPakName)
{
	return true;
}

void ADSOpenLevelCharacter::ServerAsync_Mount_MapPak_Implementation(const FString& _MapPakName)
{
	//第一步
	//FPlatformFileManager::Get()返回单例
	//GetPlatformFile()返回相应平台的PlatformFile，即处理相应平台文件读写的对象
	//因此在Windows平台，这里返回的是FWindowsPlatformFile的实例
	IPlatformFile& InnerPlatform = FPlatformFileManager::Get().GetPlatformFile();

	//第二步
	//这里创建了一个FPakPlatformFile，但是未指定当前使用什么平台去读写这个文件
	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();

	//第三步
	//使用相应平台的PlatformFile去初始化PakPlatformFile
	//第二个参数是命令行参数，一般都为空
	PakPlatformFile->Initialize(&InnerPlatform, TEXT(""));

	//第四步
	//再将当前PlatformFile设置为"相应平台下pak文件读写"的模式
	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	//const FString PakFileFullName = TEXT("F:\\test_unreal\PakTest\Content\DLC\MiniMap.pak");
	//const FString PakFileFullName = FPaths::ProjectContentDir() + TEXT("ThirdPersonCPP/Maps") + _MapPakName;
	const FString PakFileFullName = FPaths::Combine(FPaths::ProjectContentDir(),MapPath,MapName,_MapPakName)/* FPaths::ProjectContentDir() + TEXT("ThirdPersonCPP/Maps") + _MapPakName*/;

	//测试用MountPoint
	FString MountPoint(FPaths::Combine(FPaths::ProjectContentDir(), MapPath, MapName, TEXT("/")));
	//FString MountPoint(FPaths::ProjectContentDir() + TEXT("DLC/")/*FPaths::EngineContentDir()*/);

	//创建FPakFile对象,同样使用相应平台的PlatformFile初始化
	//第二个参数是pak文件的完整路径+名称
	//第三个参数是是否有符号？
	FPakFile* Pak = new FPakFile(&InnerPlatform, *PakFileFullName, false);

	if (Pak->IsValid())
	{
		//具体Mount方法可以参考函数 FPakPlatformFile::Mount
		//但是其中有大量多余内容(例如版本编号处理)
		//Pak->SetMountPoint(*MountPoint);

		PakPlatformFile->Mount(*PakFileFullName, 1000, *MountPoint);

		TArray<FString> Files;
		Pak->FindFilesAtPath(Files, *(Pak->GetMountPoint()), true, false, true);

		//FStreamableManager StreamableManager;

		for (auto File : Files)
		{
			FString Filename, FileExtn;
			int32 LastSlashIndex;
			File.FindLastChar(*TEXT("/"), LastSlashIndex);
			FString FileOnly = File.RightChop(LastSlashIndex + 1);
			FileOnly.Split(TEXT("."), &Filename, &FileExtn);

			if (FileExtn == TEXT("umap"))
			{
				//File = FileOnly.Replace(TEXT("uasset"), *Filename);
				//File = TEXT("/Game/DLC/") + File;
				File = FPaths::Combine(TEXT("/Game"), MapPath, MapName, Filename + TEXT(".") + Filename);
				//File = TEXT("/Game/DLC/") + Filename + TEXT(".") + Filename;
				//File = TEXT("/Game/DLC/") + Filename + TEXT(".") + Filename;
				ObjectPaths.AddUnique(FSoftObjectPath(File));

				//将FSoftObjectPath直接转换为TSoftObjectPtr<UObject>并储存
				ObjectPtrs.AddUnique(TSoftObjectPtr<UObject>(ObjectPaths[ObjectPaths.Num() - 1]));
			}

		}

		UAssetManager::GetStreamableManager().RequestAsyncLoad(ObjectPaths, FStreamableDelegate::CreateUObject(this, &ADSOpenLevelCharacter::CreateAllChildren_Server));
	}
}

void ADSOpenLevelCharacter::CreateAllChildren_Server()
{
	UE_LOG(LogTemp, Log, TEXT("finished loading assets"));
	for (int32 i = 0; i < ObjectPtrs.Num(); ++i)
	{
		UObject* LoadObject = ObjectPtrs[i].Get();
		if (LoadObject != nullptr)
		{
			FString _MapPath = ObjectPtrs[i].ToString();
			FString ShortMapName = FPackageName::GetShortName(_MapPath);
			FString _MapName, FileExtn;
			ShortMapName.Split(TEXT("."), &_MapName, &FileExtn);

			ServerChangeMap(_MapName);
			UE_LOG(LogTemp, Log, TEXT("Object Load Success..."))
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Can not Load asset..."))
		}
	}
}