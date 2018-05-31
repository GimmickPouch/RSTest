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

	FTimerHandle _powerActivationDelayHandle;

	bool _powerHasBeenActivated;
	bool _powerIsActive;

	//GettersAndSetter
public:
	UFUNCTION(BlueprintCallable, Category = "Magic Power GetSet")
		bool GetPowerHasBeenActivated() const { return _powerHasBeenActivated; }
	UFUNCTION(BlueprintCallable, Category = "Magic Power GetSet")
		void SetPowerHasBeenActivated(bool value) { _powerHasBeenActivated = value; }

	UFUNCTION(BlueprintCallable, Category = "Magic Power GetSet")
		bool GetPowerIsActive() const { return _powerIsActive; }
	UFUNCTION(BlueprintCallable, Category = "Magic Power GetSet")
		void SetPowerIsActive(bool value) { _powerIsActive = value; }

	//Functions
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void PowerTick(float DeltaTime) {};

	void PowerBecomeActive();

public:
	virtual void ActivatePower();

	virtual void ActivatePowerAfterDelay();

	virtual void DeactivatePower();
};
