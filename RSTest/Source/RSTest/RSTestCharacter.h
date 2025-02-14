// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RSTestCharacter.generated.h"

UENUM(BlueprintType)
enum class EWallRunEntrySide : uint8
{
	WR_Left 	UMETA(DisplayName = "Left"),
	WR_Right 	UMETA(DisplayName = "Right"),
};

class UInputComponent;
class ULifeSystem;

UCLASS(config=Game)
class ARSTestCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	ARSTestCharacter();

protected:
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class ARSTestProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

protected:
	
	/** Fires a projectile. */
	void OnFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

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

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	//Luke aadditions from here:
	//Components
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Data")
	ULifeSystem* LifeSystem;

	//Variables
	//Jump Re-direct
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Jump Data", meta = (ClampMin = 0.1, ClampMax = 1.0))
	float _jumpStrafePowerPercentage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Jump Data", meta = (ClampMin = 0.1, ClampMax = 1.0))
	float _jumpRedirectionPenalty;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Jump Data", meta = (ClampMin = 0.1, ClampMax = 1.0))
	float _jumpConsecutivePowerPercentage;

	//Wall Running
private:
	float _holdingForward;
	float _holdingRight;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run Data")
	bool _canEverWallRun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run Data", meta = (ClampMin = "-180.0", ClampMax = "180.0"))
	float _wallRunEnterAngleLowerExclusive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run Data", meta = (ClampMin = "-180.0", ClampMax = "180.0"))
	float _wallRunEnterAngleHigherExclusive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run Data")
	float _wallRunGravityScaleChange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run Data")
	float _wallRunRotateSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run Data")
	float _wallRunPlayerRollAngleChange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run Data")
	float _wallRunVelocityAcceptance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wall Run Data")
	float _wallRunDistanceAcceptance;

private:
	AActor* _previousWallRunActor;
	FVector _wallRunMaintainTrace;
	FRotator _wallRunRotationAngle;
	FRotator _startLerpCharacterRotation;

	bool _isWallRunning;
	bool _currentWallRunIsOver;
	bool _jumpCancelsWallRun;

	float _wallRunLastJumpHeightZ;
	float _characterRotationAlpha;
	float _gravityOnWallRunStart;

	//GettersAndSetters
public:
	UFUNCTION(BlueprintCallable, Category = "Player Feature Active GetSet")
	bool GetCanWallRun() const { return _canEverWallRun; }
	UFUNCTION(BlueprintCallable, Category = "Player Feature Active GetSet")
	void SetCanWallRun(bool bSet = true) { _canEverWallRun = bSet; }

	//Functions
public:
	virtual void Jump() override;

	virtual void OnAttacked(AActor* attackedBy, float attemptedDamage);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void Landed(const FHitResult& Hit) override;

	virtual void WallRunBegin();
	virtual void WhileWallRunning();
	virtual void WallRunEnd();

	bool CheckWillWallRun(EWallRunEntrySide sideOfActivation, FVector wallRunTriggerLocation, AActor* wallRunOnActor);

	bool CheckVelocityIsAcceptableForWallRunning();

	void StartRotateCharacterForWallRun(const FRotator& startRotation);

	void RotateCharacterForWallRun(float deltaTime);

	//Visuals and Triggers
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UBoxComponent* _wallRunTriggerLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UBoxComponent* _wallRunTriggerRight;
};
