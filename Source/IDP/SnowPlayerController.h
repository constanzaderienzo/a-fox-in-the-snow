// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SnowDeformationManager.h"
#include "SnowPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class IDP_API ASnowPlayerController : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Snow")
	ASnowDeformationManager* SnowManager = nullptr;

	UPROPERTY(EditAnywhere, Category="Snow")
	UTextureRenderTarget2D* SnowRT_A;

	UPROPERTY(EditAnywhere, Category = "Snow")
	UTextureRenderTarget2D* SnowRT_B;

	UPROPERTY(EditAnywhere, Category = "Snow")
	UMaterialInterface* SnowWriteMaterial;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* SnowMID; 

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* SnowWriteMID;

	UPROPERTY(EditAnywhere, Category = "Snow", meta = (ClampMin = "1", ClampMax = "100"))
	float RadiusCm = 15.0f;

	float CachedRadiusUV = 0.01f;
	float CachedTileSizeCm = 1000.0f;
	FVector CachedMin;
	FVector CachedMax;

	bool useA = true;
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
};
