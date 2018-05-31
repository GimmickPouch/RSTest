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

	UPROPERTY(EditDefaultsOnly, Category = "Earth Channeler Attack")
	float _attackRaycastLength;

	UClass* _earthSpike;

	//Functions
protected:
	virtual void Attack(const FVector& attackLocation) override;

	void CreateEarthSpike(const FVector& spawnLocation, const FVector& attackLocation);

	//Particles
protected:
	UParticleSystem* _attackBeamVFX;
};
