// Fill out your copyright notice in the Description page of Project Settings.

#include "EarthSpike.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "RSTestCharacter.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"

AEarthSpike::AEarthSpike()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	_powerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	_powerMesh->SetupAttachment(RootComponent);

	_attackTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackTrigger"));
	_attackTrigger->SetupAttachment(_powerMesh);
	_attackTrigger->SetCanEverAffectNavigation(false);
	_attackTrigger->bGenerateOverlapEvents = true;

	//This would be better as VFX
	_visualWarning = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualWarning"));
	_visualWarning->SetupAttachment(_powerMesh);
	_visualWarning->SetCollisionProfileName("NoCollision");
	_visualWarning->bGenerateOverlapEvents = false;

	_attackActivationDelay = 0.5f;

	_interpAttackSpeed = 25.f;
	_attackPushPower = 2000.f;
	_attackPushUp = 150.f;
}

void AEarthSpike::BeginPlay()
{
	Super::BeginPlay();

	_attackTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEarthSpike::OnAttackOverlapBegin);

	_baseScale = GetActorScale();
}

void AEarthSpike::ActivatePower()
{
	//_attackLocation needs to be set before activating the Earth Spike
	_scaleToReachTargetRoundedUp = FMath::CeilToInt(((_attackLocation - GetActorLocation()).Size()) / kPowerSize);
	_scaleToReachTargetRoundedUp += 1; //Just to make sure the block is going to try and go right through the player - adds more near-miss tension

	if (_visualWarning != nullptr)
	{
		_visualWarning->DestroyComponent();
		_visualWarning = nullptr;
	}

	Super::ActivatePower();
}

void AEarthSpike::ActivatePowerAfterDelay()
{
	SetActorScale3D(FVector(_baseScale.X, _baseScale.Y, 0.04f));

	Super::ActivatePowerAfterDelay();
}

void AEarthSpike::PowerTick(float DeltaTime)
{
	SetActorScale3D(FVector(_baseScale.X, _baseScale.Y, FMath::FInterpConstantTo(GetActorScale().Z, _scaleToReachTargetRoundedUp, DeltaTime, _interpAttackSpeed)));

	if (FMath::IsNearlyEqual(GetActorScale().Z, _scaleToReachTargetRoundedUp, FLT_EPSILON))
	{
		DeactivatePower();
	}
}

void AEarthSpike::OnAttackOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!_powerIsActive)
	{
		return;
	}

	if (OtherActor && OtherActor->IsA(ARSTestCharacter::StaticClass()))
	{
		ARSTestCharacter* player = Cast<ARSTestCharacter>(OtherActor);

		player->OnAttacked(this, _damage);
		if (player && _attackTrigger)
		{
			FVector pushDirection = OtherActor->GetActorLocation() - GetActorLocation();
			FVector pushNormal = FVector(pushDirection.X, pushDirection.Y, _attackPushUp).GetSafeNormal();

			if (!player->GetCharacterMovement()->IsFalling())
			{
				player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			}
			player->GetCharacterMovement()->Velocity = (pushNormal * _attackPushPower);
		}
	}
}
