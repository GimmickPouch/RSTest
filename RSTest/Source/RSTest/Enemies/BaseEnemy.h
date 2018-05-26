// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseEnemy.generated.h"

UCLASS()
class RSTEST_API ABaseEnemy : public AActor
{
	GENERATED_BODY()
	
	//Functions
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

	//Functions
protected:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void TakeDamage(float damageAmount);

	virtual void EndInvulnerability();

	virtual bool CheckForDeath();

public:
	virtual void OnShot(AActor* shotBy, float attemptedDamage);

	//Visuals and Colliders
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		UStaticMeshComponent* _enemyMesh;
	
};
