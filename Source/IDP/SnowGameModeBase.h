// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowGameModeBase.generated.h"

class ASnowDeformationManager;
/**
 * 
 */
UCLASS()
class IDP_API ASnowGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowGameModeBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	TObjectPtr<ASnowDeformationManager> SnowManager = nullptr;

	void SetSnowManager(ASnowDeformationManager* InManager) { SnowManager = InManager; }
	
};
