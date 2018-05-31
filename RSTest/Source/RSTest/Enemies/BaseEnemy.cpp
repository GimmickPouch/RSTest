// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseEnemy.h"
#include "TimerManager.h"
#include "Components/LifeSystem.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	LifeSystem = CreateDefaultSubobject<ULifeSystem>(TEXT("LifeSystem"));
	AddOwnedComponent(LifeSystem);

	_movementSpeed = 1.f;
}

void ABaseEnemy::OnAttacked(AActor* attackedBy, float attemptedDamage)
{
	// CAUTION: attackedBy actor is usually destroyed after this call if it's a player projectile
	LifeSystem->OnTakeDamage(attemptedDamage);
	if (LifeSystem->GetIsDead())
	{
		Destroy();
	}
}