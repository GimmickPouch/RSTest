// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LifeSystem.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RSTEST_API ULifeSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULifeSystem();

	//Variables
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Life System Data", meta = (ClampMin = 0))
	float _maxHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Life System Data")
	float _invulnerabilityWindowSeconds;

private:
	float _health;

	bool _isDead;
	bool _canTakeDamage;

	//GettersAndSetters
public:
	UFUNCTION(BlueprintCallable, Category = "Life System GetSet")
	float GetHealth() const { return _health; }
	UFUNCTION(BlueprintCallable, Category = "Life System GetSet")
	void SetHealth(float newHealth) { _health = newHealth; }

	UFUNCTION(BlueprintCallable, Category = "Life System GetSet")
	void IncreaseHealth(float increaseAmount) { _health += increaseAmount; if (_health > _maxHealth) { _health = _maxHealth; } };
	UFUNCTION(BlueprintCallable, Category = "Life System GetSet")
	void DecreaseHealth(float decreaseAmount) { _health -= decreaseAmount;  if (_health < 0) { _health = 0; } };

	UFUNCTION(BlueprintCallable, Category = "Life System GetSet")
	float GetMaxHealth() const { return _maxHealth; }

	UFUNCTION(BlueprintCallable, Category = "Life System GetSet")
	bool GetIsDead() const { return _isDead; }

	UFUNCTION(BlueprintCallable, Category = "Enemy Reactions")
	virtual void OnTakeDamage(float damageAmount); // TakeDamage is being used by Pawn class

protected:
	virtual void BeginPlay() override;

	virtual void EndInvulnerability();

	virtual bool CheckForDeath();	
	
};
