// Fill out your copyright notice in the Description page of Project Settings.

#include "EEarthChanneler.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "CollisionQueryParams.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Powers/EarthSpike.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

AEEarthChanneler::AEEarthChanneler()
{
	//EarthSpike.cpp
	static ConstructorHelpers::FObjectFinder<UClass> earthSpike(TEXT("Class'/Game/Blueprints/Attacks/EarthSpike.EarthSpike_C'"));
	if (earthSpike.Object)
	{
		_earthSpike = earthSpike.Object;
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> beamParticle (TEXT("/Game/VFX/EarthSpikeBeam.EarthSpikeBeam"));
	if (beamParticle.Succeeded())
	{
		_attackBeamVFX = beamParticle.Object;
	}

	_attackRaycastLength = 5000.0f;
}

void AEEarthChanneler::Attack(const FVector& attackLocation)
{
	Super::Attack(attackLocation);

	UWorld* const world = GetWorld();
	if (world)
	{
		FVector* closestSpawnPosition = nullptr;
		FHitResult hitData(ForceInit);

		FCollisionQueryParams traceParams(FName(TEXT("AttackTracer")), false, this);

		for (int i = 1; i <= 4; i++)
		{
			FVector traceHitLocation;
			FVector traceDirection;
			if (i < 3)
			{
				traceDirection = FVector::RightVector;
			}
			else
			{
				traceDirection = FVector::ForwardVector;
			}

			if (i % 2 == 0)
			{
				traceDirection *= -1;
			}

			world->LineTraceSingleByChannel(
				hitData, //result
				attackLocation, //start
				attackLocation + (traceDirection * _attackRaycastLength), //end
				ECC_Visibility, //collison channel
				traceParams
			);

			if (hitData.GetActor() && 
				!hitData.GetActor()->IsA(ACharacter::StaticClass()) && 
				(closestSpawnPosition == nullptr || 
				(*closestSpawnPosition - attackLocation).Size() > (hitData.Location - attackLocation).Size()))
			{
				traceHitLocation = FVector(hitData.Location);
				closestSpawnPosition = &traceHitLocation;
			}
		}

		if (closestSpawnPosition != nullptr)
		{
			CreateEarthSpike(*closestSpawnPosition, attackLocation);
		}
	}
}

// This function would ideally be extracted so that the Earth Spike power was easier to be equipped and used by multiple Actors/Characters and enemies could more easily fire any BaseMagicPower.cpp
void AEEarthChanneler::CreateEarthSpike(const FVector& spawnLocation, const FVector& attackLocation)
{
	UWorld* const world = GetWorld();
	if (world && _earthSpike)
	{
		FActorSpawnParameters spawnParams;
		AEarthSpike* newEarthSpike = world->SpawnActor<AEarthSpike>(
			_earthSpike,
			spawnLocation,
			UKismetMathLibrary::FindLookAtRotation(spawnLocation, attackLocation) + FRotator(-90.f, 0, 0),
			spawnParams
			);
		newEarthSpike->SetAttackLocation(attackLocation);
		newEarthSpike->ActivatePowerAfterDelay();

		if (_attackBeamVFX)
		{
			UParticleSystemComponent* shootBeam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				_attackBeamVFX,
				GetActorLocation()
			);
			shootBeam->SetBeamSourcePoint(0, GetActorLocation(), 0);
			shootBeam->SetBeamTargetPoint(0, spawnLocation, 0);
		}

	}
}