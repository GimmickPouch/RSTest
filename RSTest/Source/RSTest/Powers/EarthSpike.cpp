// Fill out your copyright notice in the Description page of Project Settings.

#include "EarthSpike.h"
#include "Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "RSTestCharacter.h"

AEarthSpike::AEarthSpike()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	_powerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	_powerMesh->SetupAttachment(RootComponent);

	_attackTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackTrigger"));
	_attackTrigger->SetupAttachment(_powerMesh);
	_attackTrigger->SetCanEverAffectNavigation(false);
	_attackTrigger->bGenerateOverlapEvents = true;

	_interpAttackSpeed = 25.f;
}

void AEarthSpike::BeginPlay()
{
	Super::BeginPlay();

	_attackTrigger->OnComponentBeginOverlap.AddDynamic(this, &AEarthSpike::OnAttackOverlapBegin);
}

void AEarthSpike::ActivatePower()
{
	//_attackLocation needs to be set before activating the Earth Spike
	_scaleToReachTargetRoundedUp = FMath::CeilToInt(((_attackLocation - GetActorLocation()).Size()) / kPowerSize);

	Super::ActivatePower();
}

void AEarthSpike::PowerTick(float DeltaTime)
{
	SetActorScale3D(FVector(1, 1, FMath::FInterpConstantTo(GetActorScale().Z, _scaleToReachTargetRoundedUp, DeltaTime, _interpAttackSpeed)));

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

		//player->OnTakeDamage(_damage);
		//if (OtherComp->IsSimulatingPhysics())
		//{
		//	OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		//}
	}
}
