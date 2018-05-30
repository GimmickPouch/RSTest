// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemies/BaseEnemy.h"
#include "EEarthChanneler.generated.h"

/**
 * 
 */
UCLASS()
class RSTEST_API AEEarthChanneler : public ABaseEnemy
{
	GENERATED_BODY()
	
public:
	AEEarthChanneler();

	//Variables
protected:

	UClass* _earthSpike;

	UPROPERTY(EditDefaultsOnly, Category = "Earth Channeler Attack")
		float _attackRaycastLength;

	//Functions
protected:
	virtual void Attack(FVector attackLocation) override;

	void CreateEarthSpike(FVector& spawnLocation, FVector& attackLocation);

	//Particles
protected:
	UParticleSystem* _attackBeamVFX;
};
