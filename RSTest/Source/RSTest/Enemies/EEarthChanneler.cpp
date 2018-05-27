// Fill out your copyright notice in the Description page of Project Settings.

#include "EEarthChanneler.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "CollisionQueryParams.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"

AEEarthChanneler::AEEarthChanneler()
{
	//Earth Spike
	static ConstructorHelpers::FObjectFinder<UClass> earthSpike(TEXT("Class'/Game/Blueprints/Environment/EarthSpike.EarthSpike_C'"));
	if (earthSpike.Object)
	{
		_earthSpike = earthSpike.Object;
	}

	_attackActivationDelay = 0.5f;
	_attackRaycastLength = 5000.0f;
}

void AEEarthChanneler::Attack(FVector attackLocation)
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

			if (hitData.GetActor() && !hitData.GetActor()->IsA(ACharacter::StaticClass()))
			{
				closestSpawnPosition = &hitData.Location;
			}
		}

		if (closestSpawnPosition != nullptr)
		{
			CreateEarthSpike(*closestSpawnPosition, attackLocation);
		}
	}
}

void AEEarthChanneler::CreateEarthSpike(FVector& spawnLocation, FVector& attackLocation)
{
	UWorld* const world = GetWorld();
	if (world && _earthSpike)
	{
		FActorSpawnParameters spawnParams;
		AActor* newEarthSpike = world->SpawnActor<AActor>(
			_earthSpike,
			spawnLocation,
			UKismetMathLibrary::FindLookAtRotation(spawnLocation, attackLocation) + FRotator(-90.f, 0, 0), //TODO rotate 90 degrees so top is facing player
			spawnParams
			);
	}
}