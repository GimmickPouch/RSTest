// Fill out your copyright notice in the Description page of Project Settings.

#include "EarthSpike.h"

AEarthSpike::AEarthSpike()
{
	_lerpIncreaseRate = 0.5f;
}

void AEarthSpike::ActivatePower()
{
	Super::ActivatePower();

	_scaleLerpAlpha = 0.f;
	_startingZScale = GetActorScale().Z;

	//_attackLocation needs to be set before activating the Earth Spike
	_scaleToReachTargetRoundedUp = FMath::CeilToInt(((_attackLocation - GetActorLocation()).Size()) / kPowerSize);

	SetActorScale3D(FVector(1, 1, _scaleToReachTargetRoundedUp));
}

void AEarthSpike::PowerTick(float DeltaTime)
{
	_scaleLerpAlpha += _lerpIncreaseRate * DeltaTime;

	if (_scaleLerpAlpha > 1)
	{
		_scaleLerpAlpha = 1;
		DeactivatePower();
	}

	SetActorScale3D(FVector(1, 1, FMath::Lerp(_startingZScale, _scaleToReachTargetRoundedUp, _scaleLerpAlpha))); // TODO lerp and same speed - not percentage over specific time
}
