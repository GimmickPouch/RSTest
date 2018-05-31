// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

class ULifeSystem;

UCLASS()
class RSTEST_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()
	
public:	
	ABaseEnemy();

	//Components
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Data")
		ULifeSystem* LifeSystem;

	//Variables
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Enemy Data")
		float _movementSpeed;

	//Functions
protected:
	UFUNCTION(BlueprintCallable, Category = "Enemy Actions")
		virtual void Attack(const FVector& attackLocation) {};

public:
	virtual void OnAttacked(AActor* attackedBy, float attemptedDamage);
	
};
