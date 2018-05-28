// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

UCLASS()
class RSTEST_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()
	
public:	
	ABaseEnemy();

	//Variables
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Enemy Data", meta = (ClampMin = 0))
		float _maxHealth;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy Data")
		float _invulnerabilityWindowSeconds;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy Data")
		float _movementSpeed;

private:
	FTimerHandle _invulnerableWindowHandle;

	float _health;

	bool _isDead;
	bool _canTakeDamage;

	//GettersAndSetters
public:
	UFUNCTION(BlueprintCallable, Category = "Enemy GetSet")
		float GetHealth() const { return _health; }
	UFUNCTION(BlueprintCallable, Category = "Enemy GetSet")
		void SetHealth(float newHealth) { _health = newHealth; }
	UFUNCTION(BlueprintCallable, Category = "Enemy GetSet")
		void IncreaseHealth(float increaseAmount) { _health += increaseAmount; if (_health > _maxHealth) { _health = _maxHealth; } };
	UFUNCTION(BlueprintCallable, Category = "Enemy GetSet")
		void DecreaseHealth(float decreaseAmount) { _health -= decreaseAmount;  if (_health < 0) { _health = 0; } };

	UFUNCTION(BlueprintCallable, Category = "Enemy GetSet")
		float GetMaxHealth() const { return _maxHealth; }

	//Functions
protected:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Enemy Reactions")
		virtual void OnTakeDamage(float damageAmount); //TakeDamage function name was taken by Pawn class

	virtual void EndInvulnerability();

	virtual bool CheckForDeath();

	UFUNCTION(BlueprintCallable, Category = "Enemy Actions")
		virtual void Attack(FVector attackLocation) {};

public:
	virtual void OnShot(AActor* shotBy, float attemptedDamage);
	
};
