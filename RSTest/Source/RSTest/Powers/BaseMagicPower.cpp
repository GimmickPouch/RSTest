// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseMagicPower.h"
#include "TimerManager.h"

ABaseMagicPower::ABaseMagicPower()
{
	PrimaryActorTick.bCanEverTick = true;

	_damage = 1.f;
	_attackActivationDelay = 0.0f;
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
	PowerBecomeActive();
}

void ABaseMagicPower::ActivatePowerAfterDelay()
{
	if (_attackActivationDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(_powerActivationDelayHandle, this, &ABaseMagicPower::ActivatePower, _attackActivationDelay);
	}
	else
	{
		ActivatePower();
	}
}

void ABaseMagicPower::PowerBecomeActive()
{
	_powerHasBeenActivated = _powerIsActive = true;
}

void ABaseMagicPower::DeactivatePower()
{
	_powerIsActive = false;
}