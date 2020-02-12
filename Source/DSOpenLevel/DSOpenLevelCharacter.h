// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DSOpenLevelCharacter.generated.h"

const FString MapPath = TEXT("Maps/");
const FString MapName = TEXT("MiniMap");
const FString MapPakName = TEXT("MiniMap.pak");

UCLASS(config=Game)
class ADSOpenLevelCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ADSOpenLevelCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	virtual void BeginPlay() override;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void InitializeInputBindings();

	//�ͻ����첽��Pak(����ִ��)
	void ClientAsync_Mount_MapPak(const FString& _MapPakName);

	//�������첽��Pak(Serverִ��)
	UFUNCTION(Server, unreliable, WithValidation)
	void ServerAsync_Mount_MapPak(const FString& _MapPakName);

	//�ͻ���ͬ����Pak(����ִ��)
	void ClientSync_Mount_MapPak(const FString& _MapPakName);

	//������ͬ����Pak(Serverִ��)
	UFUNCTION(Server, unreliable, WithValidation)
	void ServerSync_Mount_MapPak(const FString& _MapPakName);

	void CreateAllChildren_Client();

	void CreateAllChildren_Server();

	//����server�л���ͼ����.pak�ļ�
	UFUNCTION(Exec)
	void LoadLocalMap()
	{
		ServerChangeMap(TEXT("test"));
	}

	UFUNCTION(Exec)
	void InitLevel();


	UFUNCTION(Server, unreliable, WithValidation)
	void ServerChangeMap(const FString& MapName);

	UFUNCTION(Exec)
	void UnpackClientMap()
	{
		ClientAsync_Mount_MapPak(MapPakName);
	}

	UFUNCTION(Exec)
	void UnpackServerMap()
	{
		ServerAsync_Mount_MapPak(MapPakName);
	}

	UFUNCTION(Exec)
	void LoadServerMap()
	{
		ServerChangeMap(MapName);
	}

	//�ͻ����첽���
	UFUNCTION(Exec)
	void ClientAsyncMount();

	//�ͻ���ͬ�����
	UFUNCTION(Exec)
	void ClientSyncMount();

	UFUNCTION(Exec)
	void Test();

	//pak�ļ��е��ļ�·���б�
	TArray<FSoftObjectPath> ObjectPaths;
	TArray<TSoftObjectPtr<UObject>> ObjectPtrs;
};

