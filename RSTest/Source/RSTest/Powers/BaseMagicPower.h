// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseMagicPower.generated.h"

UCLASS()
class RSTEST_API ABaseMagicPower : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseMagicPower();

	//Variables
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Magic Power Data")
		float _damage;

	UPROPERTY(EditDefaultsOnly, Category = "Magic Power Data")
		float _attackActivationDelay;

	bool _powerHasBeenActivated;
	bool _powerIsActive;

	//Functions
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void PowerTick(float DeltaTime) {};

public:
	virtual void ActivatePower();

	virtual void DeactivatePower();
};
