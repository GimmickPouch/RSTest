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
//#include "Runtime/Engine/Public/DrawDebugHelpers.h" // For debugging

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
	_wallRunTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("WallRunOverlapTrigger"));
	_wallRunTrigger->SetupAttachment(RootComponent);
	_wallRunTrigger->SetCollisionProfileName("OverlapAll");
	_wallRunTrigger->bGenerateOverlapEvents = true;

	_maxHealth = 5.f;
	_invulnerabilityWindowSeconds = 0.5f;

	_jumpStrafePowerPercentage = 0.6f;
	_jumpRedirectionPenalty = 0.75f;
	_jumpConsecutivePowerPercentage = 1.0f;

	_canWallRun = true;
	_wallRunEnterAngleLowerExclusive = 0.f;
	_wallRunEnterAngleHigherExclusive = 80.f;
	_wallRunRotateSpeed = 5.f;
	_wallRunGravityScaleChange = 0.5f;
	_wallRunPlayerRollAngleChange = 20.f;
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

	_wallRunTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARSTestCharacter::OnOverlapBegin);

	_health = _maxHealth;
	_canTakeDamage = true;

	//Wall run variables
	_isWallRunning = false;
	_currentWallRunIsOver = false;
	_jumpCancelsWallRun = false;
	_characterRotationAlpha = 1.f; // Start at 1 because we don't want this to start straight away (as it plays on Tick is < 1)
	_wallRunVelocityAcceptance = 0.f; // 0 allows any velocity to start a wall run
	_wallRunMaintainTrace = FVector();
	_previousWallRunActor = nullptr;
	_wallRunJumpHeightZ = 999999.f;
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

void ARSTestCharacter::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OverlappedComp ||
		OverlappedComp != _wallRunTrigger ||
		_isWallRunning ||
		!GetCharacterMovement()->IsFalling() ||
		!OtherActor ||
		OtherActor->IsA(APawn::StaticClass()) ||
		!CheckVelocityIsAcceptableForWallRunning())
	{
		return;
	}

	FVector normDirectionToWall = OtherActor->GetActorLocation() - FirstPersonCameraComponent->GetComponentLocation().GetSafeNormal(); // This check would need to be refined for assets that aren't straight walls
	bool activateFromRight = FVector::DotProduct(normDirectionToWall, FirstPersonCameraComponent->GetRightVector()) > 0 ? true : false;
	FVector directionOfWallRun = activateFromRight ? FirstPersonCameraComponent->GetRightVector() : FirstPersonCameraComponent->GetRightVector() * -1;

	FHitResult HitData(ForceInit);
	FCollisionQueryParams TraceParams(FName(TEXT("WallRunTracer")), false, this);

	FVector componentLocation = OverlappedComp->GetComponentTransform().GetLocation();
	GetWorld()->LineTraceSingleByChannel(HitData, componentLocation, componentLocation + (directionOfWallRun * 400), ECC_Visibility, TraceParams);

	if (HitData.GetActor())
	{
		FVector vectorPerpendicularToWall = FRotator(HitData.ImpactNormal.Rotation() + FRotator(0.f, 90.f, 0.f)).Vector(); //Line of wall run direction

		_wallRunRotationAngle = FRotator(0.f, 0.f, -_wallRunPlayerRollAngleChange);
		_wallRunMaintainTrace = directionOfWallRun * _wallRunDistanceAcceptance; //How far can you get from the wall until you're no longer wall running
		if (!activateFromRight)
		{
			vectorPerpendicularToWall *= -1;
			_wallRunRotationAngle *= -1;
		}

		FVector2D NormVelNoZ = FVector2D(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y).GetSafeNormal();

		// Make sure that you cannot jump off a wall and then re-enter the same wall at a higher height - stops exploit
		if (!_currentWallRunIsOver || !_previousWallRunActor || (_previousWallRunActor != OtherActor) || _wallRunJumpHeightZ > GetActorLocation().Z)
		{
			// Get the angle
			float wallRunAttemptAngle = FMath::RadiansToDegrees(acosf(FVector2D::DotProduct(NormVelNoZ, FVector2D(vectorPerpendicularToWall.X, vectorPerpendicularToWall.Y))));

			if (wallRunAttemptAngle > _wallRunEnterAngleLowerExclusive && wallRunAttemptAngle < _wallRunEnterAngleHigherExclusive) //A check to make sure you're entering at an accepted angle
			{
				_previousWallRunActor = OtherActor;
 				WallRunBegin(); //Finally, all checks have passed
			}
			else
			{
				_currentWallRunIsOver = true;
			}
		}
	}
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
			_wallRunJumpHeightZ = GetActorLocation().Z;
			WallRunEnd();
		}
	}
}

//Overridden landed logic
void ARSTestCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (_isWallRunning)
	{
		WallRunEnd();
	}

	_currentWallRunIsOver = false;
}

void ARSTestCharacter::OnTakeDamage(float damageAmount)
{
	if (_canTakeDamage)
	{
		_health -= damageAmount;
		_canTakeDamage = false;
		GetWorldTimerManager().SetTimer(_invulnerableWindowHandle, this, &ARSTestCharacter::EndInvulnerability, _invulnerabilityWindowSeconds);
	}
}

void ARSTestCharacter::EndInvulnerability()
{
	_canTakeDamage = true;
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

	GetCharacterMovement()->GravityScale = _wallRunGravityScaleChange; // How the "wall run" feel and physics is simulated
	StartRotateCharacterForWallRun(GetController()->GetControlRotation()); // Starts camera lean when wall running
}

//Constantly fire traces in the direction the wall run started until there's nothing to wall run on anymore (allows for spinning around as much as you want!)
void ARSTestCharacter::WhileWallRunning()
{
	FHitResult HitData(ForceInit);
	FCollisionQueryParams TraceParams(FName(TEXT("WhileWallRunningTracer")), false, this);

	GetWorld()->LineTraceSingleByChannel(HitData, GetActorLocation(), GetActorLocation() + _wallRunMaintainTrace, ECC_Visibility, TraceParams);

	if (!HitData.GetActor())
	{
		WallRunEnd();
	}
}

void ARSTestCharacter::WallRunEnd()
{
	_isWallRunning = _jumpCancelsWallRun = false;

	_currentWallRunIsOver = true;

	GetCharacterMovement()->GravityScale = 1.f; //Return to normal gravity - may want to store this in a variable to allow for varied gravity scenarios
	StartRotateCharacterForWallRun(GetController()->GetControlRotation()); // Starts camera lean back to normal
}

//Variables set to start rotation lerp
void ARSTestCharacter::StartRotateCharacterForWallRun(const FRotator& startRotation)
{
	_startLerpCharacterRotation = startRotation;
	_characterRotationAlpha = 0; //Starts the ticking of the lerp
}

//Logic for lerping the rotation of the character - works on a "dynamic" or not where "dynamic" represents if more than pure rotation logic is required
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
