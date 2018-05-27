// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseMagicPower.h"


ABaseMagicPower::ABaseMagicPower()
{
	PrimaryActorTick.bCanEverTick = true;

	_damage = 1.f;
	_attackActivationDelay = 0.5f;
}

void ABaseMagicPower::BeginPlay()
{
	Super::BeginPlay();
	
	_powerHasBeenActivated = false;
}

void ABaseMagicPower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_powerIsActive)
	{
		PowerTick(DeltaTime);
	}
}

void ABaseMagicPower::ActivatePower()
{
	_powerHasBeenActivated = _powerIsActive = true;
}

void ABaseMagicPower::DeactivatePower()
{
	_powerIsActive = false;
}