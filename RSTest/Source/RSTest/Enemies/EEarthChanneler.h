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
	
protected:
	virtual void Attack(FVector attackLocation) override;
	
	
};
