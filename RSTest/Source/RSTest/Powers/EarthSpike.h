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
	const float kPowerSize = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Earth Spike Data")
		float _interpAttackSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Earth Spike Data")
		float _attackPushPower;

	UPROPERTY(EditDefaultsOnly, Category = "Earth Spike Data")
		float _attackPushUp;

	FVector _attackLocation;

	float _scaleToReachTargetRoundedUp;

	//GettersAndSetter
public:
	UFUNCTION(BlueprintCallable, Category = "Earth Spike GetSet")
		FVector GetAttackLocation() const { return _attackLocation; }
	UFUNCTION(BlueprintCallable, Category = "Earth Spike GetSet")
		void SetAttackLocation(FVector location) { _attackLocation = location; }

	//Functions
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void PowerTick(float DeltaTime) override;

public:
	virtual void ActivatePower() override;

	virtual void ActivatePowerAfterDelay() override;

	//Visuals and Colliders
protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		class UStaticMeshComponent* _powerMesh;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		class UBoxComponent* _attackTrigger;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		class UStaticMeshComponent* _visualWarning;
};
