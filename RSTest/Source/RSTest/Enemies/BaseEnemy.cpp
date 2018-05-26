// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseEnemy.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	_enemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnemyMesh"));
	_enemyMesh->SetupAttachment(RootComponent);

	_maxHealth = 100;
	_invulnerabilityWindowSeconds = 0.5f;

	_isDead = false;
	_canTakeDamage = true;
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	_health = _maxHealth;
	CheckForDeath(); // In case they start at 0 health
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::OnShot(AActor* shotBy, float attemptedDamage)
{
	// CAREFUL: shotBy is destroyed after this call
	TakeDamage(attemptedDamage);
}

void ABaseEnemy::TakeDamage(float damageAmount)
{
	if (!_isDead && _canTakeDamage)
	{
		DecreaseHealth(damageAmount);
		if (!CheckForDeath())
		{
			_canTakeDamage = false;
			GetWorldTimerManager().SetTimer(_invulnerableWindowHandle, this, &ABaseEnemy::EndInvulnerability, _invulnerabilityWindowSeconds);
		}
	}
}

void ABaseEnemy::EndInvulnerability()
{
	_canTakeDamage = true;
}

bool ABaseEnemy::CheckForDeath()
{
	if (_health <= 0)
	{
		PrimaryActorTick.bCanEverTick = false;
		_isDead = true;
	}
	return _isDead;
}