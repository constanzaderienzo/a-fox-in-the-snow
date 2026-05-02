// Fill out your copyright notice in the Description page of Project Settings.

#include "SnowCharacter.h"
#include "SnowGameModeBase.h"
#include "SnowDeformationManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ASnowCharacter::ASnowCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASnowCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ASnowGameModeBase* GM = GetWorld()->GetAuthGameMode<ASnowGameModeBase>())
	{
		SnowManager = GM->SnowManager;
	}

	if (FootContacts.Num() == 0)
	{
		FFootContactState LeftFoot;
		LeftFoot.SocketName = TEXT("foot_l_Socket");
		FootContacts.Add(LeftFoot);

		FFootContactState RightFoot;
		RightFoot.SocketName = TEXT("foot_r_Socket");
		FootContacts.Add(RightFoot);
	}
}

void ASnowCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!SnowManager)
	{
		SnowManager = Cast<ASnowDeformationManager>(
			UGameplayStatics::GetActorOfClass(this, ASnowDeformationManager::StaticClass())
		);

		if (!SnowManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("SnowManager is null"));
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("SnowManager found: %s"), *SnowManager->GetName());
	}

	UpdateFootContacts();
}

bool ASnowCharacter::TraceFootToGround(const FFootContactState& Foot, FHitResult& OutHit, FVector& OutSocketLocation) const
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	UWorld* World = GetWorld();

	if (!MeshComp || !World) return false;
	if (!MeshComp->DoesSocketExist(Foot.SocketName)) return false;

	OutSocketLocation = MeshComp->GetSocketLocation(Foot.SocketName);

	const FVector Start = OutSocketLocation + FVector(0.0f, 0.0f, Foot.TraceStartOffset);
	const FVector End = OutSocketLocation - FVector(0.0f, 0.0f, Foot.TraceLength);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FootTrace), false, this);
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = true;

	const FCollisionShape Shape = FCollisionShape::MakeSphere(Foot.TraceRadius);

	const bool bHit = World->SweepSingleByChannel(
		OutHit,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		Shape,
		Params
	);

	return bHit;
}


void ASnowCharacter::UpdateFootContacts()
{
	if (!SnowManager) {
		UE_LOG(LogTemp, Warning, TEXT("SnowManager is null"));
		return;
	}

	for (FFootContactState& Foot : FootContacts)
	{
		FHitResult Hit;
		FVector SocketLocation = FVector::ZeroVector;
		const bool bHit = TraceFootToGround(Foot, Hit, SocketLocation);

		bool bGroundedNow = false;

		if (bHit && Hit.GetActor())
		{
			const float ContactHeight = FMath::Abs(SocketLocation.Z - Hit.ImpactPoint.Z);
			bGroundedNow = ContactHeight <= 30.0f;
			
			if (bGroundedNow)
			{
				const bool bFarEnough =
					Foot.LastStampWorld.IsZero() ||
					FVector::Dist2D(Foot.LastStampWorld, Hit.Location) >= Foot.MinStampDistanceCm;

				if (bFarEnough)
				{
					float FootYaw = (GetActorRotation().Yaw + 90.0f)/ 360.0f;

					FVector2D CurrUV = SnowManager->GetSnowUVFromHit(Hit);

					SnowManager->StampAtWorldHit(Hit, Foot.LastStampWorld, Foot.LastStampUV, Foot.RadiusX, Foot.RadiusY, FootYaw);

					Foot.LastStampWorld = Hit.Location;
					Foot.LastStampUV = CurrUV;
				}
			}
			
		}

		// Reset UV when foot lifts so next landing starts fresh
		if (!bGroundedNow && Foot.bWasGroundedLastFrame)
		{
			Foot.LastStampUV = FVector2D(-1.0f, -1.0f);
		}

		Foot.bWasGroundedLastFrame = bGroundedNow;
	}
}
