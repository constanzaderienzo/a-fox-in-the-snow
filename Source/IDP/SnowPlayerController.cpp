// Fill out your copyright notice in the Description page of Project Settings.


#include "SnowPlayerController.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "SnowGameModeBase.h"
void ASnowPlayerController::BeginPlay()
{
	Super::BeginPlay();

    bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	SetInputMode(FInputModeGameAndUI());

    ASnowGameModeBase* GM = GetWorld()->GetAuthGameMode<ASnowGameModeBase>();
    if (GM)
    {
        SnowManager = GM->SnowManager;
    }

}

void ASnowPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
