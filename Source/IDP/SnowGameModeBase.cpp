// Fill out your copyright notice in the Description page of Project Settings.


#include "SnowGameModeBase.h"
#include "SnowPlayerController.h"


ASnowGameModeBase::ASnowGameModeBase()
{
    PlayerControllerClass = ASnowPlayerController::StaticClass();
}
