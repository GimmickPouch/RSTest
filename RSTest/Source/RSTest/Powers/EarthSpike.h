// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Powers/BaseMagicPower.h"
#include "EarthSpike.generated.h"

/**
 * 
 */
UCLASS()
class RSTEST_API AEarthSpike : public ABaseMagicPower
{
	GENERATED_BODY()

	AEarthSpike();
	
	//Variables
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Earth Spike Data")
		float _lerpIncreaseRate;

	const float kPowerSize = 100.f;

	FVector _attackLocation;

	float _startingZScale;
	float _scaleToReachTargetRoundedUp;
	float _scaleLerpAlpha;

	//GettersAndSetter
public:
	UFUNCTION(BlueprintCallable, Category = "Earth Spike GetSet")
		FVector GetAttackLocation() const { return _attackLocation; }
	UFUNCTION(BlueprintCallable, Category = "Earth Spike GetSet")
		void SetAttackLocation(FVector location) { _attackLocation = location; }

	//Functions
protected:
	virtual void PowerTick(float DeltaTime) override;

public:
	virtual void ActivatePower() override;
};
