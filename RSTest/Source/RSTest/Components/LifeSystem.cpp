// Fill out your copyright notice in the Description page of Project Settings.

#include "LifeSystem.h"
#include "TimerManager.h"
#include "Engine.h"

ULifeSystem::ULifeSystem()
{
	PrimaryComponentTick.bCanEverTick = false;

	_maxHealth = 5;
	_invulnerabilityWindowSeconds = 0.5f;
}

void ULifeSystem::BeginPlay()
{
	Super::BeginPlay();

	_isDead = false;
	_canTakeDamage = true;
	_health = _maxHealth;
	CheckForDeath(); // In case they start at 0 health
}

void ULifeSystem::OnTakeDamage(float damageAmount)
{
	if (!_isDead && _canTakeDamage)
	{
		DecreaseHealth(damageAmount);
		if (!CheckForDeath())
		{
			_canTakeDamage = false;
			FTimerHandle invulnerableWindowHandle;
			GetWorld()->GetTimerManager().SetTimer(invulnerableWindowHandle, this, &ULifeSystem::EndInvulnerability, _invulnerabilityWindowSeconds);
		}
	}
}

void ULifeSystem::EndInvulnerability()
{
	_canTakeDamage = true;
}

bool ULifeSystem::CheckForDeath()
{
	if (_health <= 0)
	{
		_isDead = true;
	}
	return _isDead;
}

