// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "RSTestCharacter.h"
#include "RSTestProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "TimerManager.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Powers/BaseMagicPower.h"
#include "Components/LifeSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ARSTestCharacter

ARSTestCharacter::ARSTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;

	// Luke added from here:

	_wallRunTriggerLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("WallRunOverlapTriggerLeft"));
	_wallRunTriggerLeft->SetupAttachment(RootComponent);
	_wallRunTriggerLeft->SetCollisionProfileName("OverlapAll");
	_wallRunTriggerLeft->bGenerateOverlapEvents = true;

	_wallRunTriggerRight = CreateDefaultSubobject<UBoxComponent>(TEXT("WallRunOverlapTriggerRight"));
	_wallRunTriggerRight->SetupAttachment(RootComponent);
	_wallRunTriggerRight->SetCollisionProfileName("OverlapAll");
	_wallRunTriggerRight->bGenerateOverlapEvents = true;

	LifeSystem = CreateDefaultSubobject<ULifeSystem>(TEXT("LifeSystem"));
	AddOwnedComponent(LifeSystem);

	_jumpStrafePowerPercentage = 0.6f;
	_jumpRedirectionPenalty = 0.75f;
	_jumpConsecutivePowerPercentage = 1.0f;

	_canEverWallRun = true;
	_wallRunEnterAngleLowerExclusive = 0.f;
	_wallRunEnterAngleHigherExclusive = 80.f;
	_wallRunGravityScaleChange = 0.4f;
	_wallRunRotateSpeed = 5.f;
	_wallRunPlayerRollAngleChange = 20.f;
	_wallRunVelocityAcceptance = 0.f; // 0 allows any velocity to start a wall run
	_wallRunDistanceAcceptance = 100.f;
}

void ARSTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	// Luke added from here:

	_wallRunTriggerLeft->OnComponentBeginOverlap.AddDynamic(this, &ARSTestCharacter::OnOverlapBegin);
	_wallRunTriggerRight->OnComponentBeginOverlap.AddDynamic(this, &ARSTestCharacter::OnOverlapBegin);

	// Wall run variables
	_previousWallRunActor = nullptr;
	_wallRunMaintainTrace = FVector();

	_isWallRunning = false;
	_currentWallRunIsOver = false;
	_jumpCancelsWallRun = false;

	_wallRunLastJumpHeightZ = MAX_FLT; 
	_characterRotationAlpha = 1.f; // Start at 1 because we don't want this to start straight away (as it plays on Tick is < 1)
	_gravityOnWallRunStart = GetCharacterMovement()->GravityScale;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARSTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ARSTestCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ARSTestCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ARSTestCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ARSTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARSTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARSTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARSTestCharacter::LookUpAtRate);
}

void ARSTestCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<ARSTestProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<ARSTestProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ARSTestCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ARSTestCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ARSTestCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void ARSTestCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void ARSTestCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
	else if (_isWallRunning && Value < 0) // Stop wall running if we hold back
	{
		WallRunEnd();
	}
	_holdingForward = Value;
}

void ARSTestCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
	_holdingRight = Value;
}

void ARSTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARSTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool ARSTestCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ARSTestCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &ARSTestCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &ARSTestCharacter::TouchUpdate);
		return true;
	}

	return false;
}

// Luke added from here

void ARSTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_characterRotationAlpha < 1.f)
	{
		RotateCharacterForWallRun(DeltaTime);
	}

	if (_isWallRunning)
	{
		WhileWallRunning();
	}
}

// Override jumping to allow for redirecting mid-air on second jump to help avoid obstacles
void ARSTestCharacter::Jump()
{
	UCharacterMovementComponent* characterMovement = GetCharacterMovement();

	if (!characterMovement)
	{
		return;
	}

	if (characterMovement->CanEverJump())
	{
		FVector newVelocity = GetVelocity();

		if (JumpCurrentCount < JumpMaxCount)
		{
			JumpCurrentCount++;

			if (JumpCurrentCount > 1 && (_holdingForward != 0 || _holdingRight != 0)) // Double jump specifics if you're pressing any direction
			{
				FVector currentVelocityAbs = GetVelocity().GetAbs();
				float velocityPower = currentVelocityAbs.X > currentVelocityAbs.Y ? currentVelocityAbs.X : currentVelocityAbs.Y;

				newVelocity = ((GetActorRightVector() * _holdingRight) + (GetActorForwardVector() * _holdingForward)) * velocityPower;

				// If you're strafing apply a penalty (or else you'll zoom too far!) - 0.8f because controllers on strafe will only go to about 0.6f each and we can use normal algorithm
				if (FMath::Abs(_holdingForward) >= 0.8f && FMath::Abs(_holdingRight) >= 0.8f)
				{
					newVelocity *= _jumpStrafePowerPercentage;
				}

				FVector currentVelNorm = GetVelocity().GetSafeNormal();
				FVector newVelNorm = newVelocity.GetSafeNormal();

				if (FVector::DotProduct(currentVelNorm, newVelNorm) < 0.45f)
				{
					newVelocity *= _jumpRedirectionPenalty; // If we're redirecting our double jump quite radically we can apply a penalty of velocity
				}
				newVelocity.Z = characterMovement->JumpZVelocity * _jumpConsecutivePowerPercentage;
			}
			else
			{
				newVelocity.Z = characterMovement->JumpZVelocity;
			}

			characterMovement->SetMovementMode(MOVE_Falling);
			characterMovement->Velocity = newVelocity;
		}

		if (_isWallRunning && _jumpCancelsWallRun)
		{
			_wallRunLastJumpHeightZ = GetActorLocation().Z;
			WallRunEnd();
		}
	}
}

void ARSTestCharacter::OnAttacked(AActor* attackedBy, float attemptedDamage)
{
	// CAUTION: Projeciles are likely to be destroyed after hitting player (attackedBy)
	LifeSystem->OnTakeDamage(attemptedDamage);
}

void ARSTestCharacter::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComp && (OverlappedComp == _wallRunTriggerLeft || OverlappedComp == _wallRunTriggerRight))
	{
		EWallRunEntrySide wallRunSide = OverlappedComp == _wallRunTriggerLeft ? EWallRunEntrySide::WR_Left : EWallRunEntrySide::WR_Right;
		CheckWillWallRun(wallRunSide, OverlappedComp->GetComponentTransform().GetLocation(), OtherActor);
	}
}

bool ARSTestCharacter::CheckWillWallRun(EWallRunEntrySide sideOfActivation, FVector wallRunTriggerLocation, AActor* wallRunOnActor)
{
	if (_isWallRunning ||
		!GetCharacterMovement()->IsFalling() ||
		!wallRunOnActor ||
		wallRunOnActor->IsA(APawn::StaticClass()) ||
		!CheckVelocityIsAcceptableForWallRunning())
	{
		return false;
	}

	if (wallRunOnActor->IsA(ABaseMagicPower::StaticClass()))
	{
		ABaseMagicPower* power = Cast<ABaseMagicPower>(wallRunOnActor);
		if (!power->GetPowerHasBeenActivated())
		{
			// Stops earth spikes from spawning next to you on the wall and counting as a new wall run which provides a new jump
			return false;
		}
	}

	bool result = false;

	FVector directionOfWallRun = FirstPersonCameraComponent->GetRightVector();
	if (sideOfActivation == EWallRunEntrySide::WR_Left)
	{
		directionOfWallRun *= -1;
	}

	FHitResult hitData(ForceInit);
	FCollisionQueryParams traceParams(FName(TEXT("WallRunTracer")), false, this);

	GetWorld()->LineTraceSingleByChannel(hitData, wallRunTriggerLocation, wallRunTriggerLocation + (directionOfWallRun * 400), ECC_Visibility, traceParams);

	if (hitData.GetActor())
	{
		FVector vectorPerpendicularToWall = FRotator(hitData.ImpactNormal.Rotation() + FRotator(0.f, 90.f, 0.f)).Vector(); // Line of wall run direction

		_wallRunRotationAngle = FRotator(0.f, 0.f, -_wallRunPlayerRollAngleChange);

		_wallRunMaintainTrace = directionOfWallRun * _wallRunDistanceAcceptance; // How far can you get from the wall until you're no longer wall running
		if (sideOfActivation == EWallRunEntrySide::WR_Left)
		{
			vectorPerpendicularToWall *= -1;
			_wallRunRotationAngle *= -1;
		}

		// Make sure that you cannot jump off a wall and then re-enter the same wall at a higher height - stops exploit
		if (!_currentWallRunIsOver || !_previousWallRunActor || (_previousWallRunActor != wallRunOnActor) || _wallRunLastJumpHeightZ > GetActorLocation().Z)
		{
			FVector2D normVelNoZ = FVector2D(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y).GetSafeNormal();
			// Get the angle you're travelling compared to the line perpendicular to the wall
			float wallRunAttemptAngle = FMath::RadiansToDegrees(acosf(FVector2D::DotProduct(normVelNoZ, FVector2D(vectorPerpendicularToWall.X, vectorPerpendicularToWall.Y))));

			if (wallRunAttemptAngle > _wallRunEnterAngleLowerExclusive && wallRunAttemptAngle < _wallRunEnterAngleHigherExclusive) // A check to make sure you're entering at an accepted angle
			{
				_previousWallRunActor = wallRunOnActor;
				WallRunBegin();
				result = true;
			}
			else
			{
				_currentWallRunIsOver = true;
			}
		}
	}

	return result;
}

bool ARSTestCharacter::CheckVelocityIsAcceptableForWallRunning()
{
	bool result = false;
	float currentVelocity = GetVelocity().GetAbs().Size();
	if (currentVelocity >= _wallRunVelocityAcceptance)
	{
		result =  true;
	}
	return result;
}

void ARSTestCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (_isWallRunning)
	{
		WallRunEnd();
	}

	_currentWallRunIsOver = false;
}

void ARSTestCharacter::WallRunBegin()
{
	if (_isWallRunning)
	{
		return;
	}

	_isWallRunning = true;
	if (JumpCurrentCount >= JumpMaxCount)
	{
		JumpCurrentCount--; // You get an extra jump when you enter a wall run so that you can jump off the wall
	}

	_gravityOnWallRunStart = GetCharacterMovement()->GravityScale;
	GetCharacterMovement()->GravityScale = _gravityOnWallRunStart - _wallRunGravityScaleChange; // How the "wall run" feel and physics is simulated
	StartRotateCharacterForWallRun(GetController()->GetControlRotation());
}

// Constantly fire traces in the direction the wall run started until there's nothing to wall run on anymore (allows for spinning around as much as you want!)
void ARSTestCharacter::WhileWallRunning()
{
	FHitResult hitData(ForceInit);
	FCollisionQueryParams traceParams(FName(TEXT("WallRunningMaintainTracer")), false, this);

	GetWorld()->LineTraceSingleByChannel(hitData, GetActorLocation(), GetActorLocation() + _wallRunMaintainTrace, ECC_Visibility, traceParams);

	if (!hitData.GetActor())
	{
		WallRunEnd();
	}
}

void ARSTestCharacter::WallRunEnd()
{
	_isWallRunning = _jumpCancelsWallRun = false;

	_currentWallRunIsOver = true;

	GetCharacterMovement()->GravityScale = _gravityOnWallRunStart;
	StartRotateCharacterForWallRun(GetController()->GetControlRotation());
}

void ARSTestCharacter::StartRotateCharacterForWallRun(const FRotator& startRotation)
{
	_startLerpCharacterRotation = startRotation;
	_characterRotationAlpha = 0; // Starts the ticking of the lerp
}

void ARSTestCharacter::RotateCharacterForWallRun(float deltaTime)
{
	if (_characterRotationAlpha == 1)
	{
		return;
	}

	_characterRotationAlpha += (1 * _wallRunRotateSpeed) * deltaTime;

	if (_characterRotationAlpha >= 1)
	{
		_jumpCancelsWallRun = true;
		_characterRotationAlpha = 1.f;
	}

	FRotator controlRotation = GetController()->GetControlRotation();
	if (_isWallRunning)
	{
		GetController()->SetControlRotation(FMath::Lerp(
			FRotator(controlRotation.Pitch, controlRotation.Yaw, _startLerpCharacterRotation.Roll),
			FRotator(controlRotation.Pitch, controlRotation.Yaw, _wallRunRotationAngle.Roll),
			_characterRotationAlpha)
		);
	}
	else
	{
		GetController()->SetControlRotation(FMath::Lerp(
			FRotator(controlRotation.Pitch, controlRotation.Yaw, _startLerpCharacterRotation.Roll),
			FRotator(controlRotation.Pitch, controlRotation.Yaw, 0.f),
			_characterRotationAlpha)
		);
	}
}
