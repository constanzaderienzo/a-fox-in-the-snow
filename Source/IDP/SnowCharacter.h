#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SnowCharacter.generated.h"

class ASnowDeformationManager;

USTRUCT(BlueprintType)
struct FFootContactState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	float RadiusX = 0.02f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	float RadiusY = 0.018f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	float TraceStartOffset = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	float TraceLength = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	float MinStampDistanceCm = 0.0f;

	// How much depth accumulates per stamp (in cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	float DepthPerStamp = 10.0f;

	// Maximum depth this foot can sink (should match MaxDepthCm in M_Snow)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	float MaxDepthCm = 30.0f;

	bool bWasGroundedLastFrame = false;
	FVector LastStampWorld = FVector::ZeroVector;
	FVector2D LastStampUV = FVector2D(-1.0f, -1.0f);
	float CurrentDepthCm = 0.0f;
};

UCLASS()
class IDP_API ASnowCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASnowCharacter();

	// Scale factor — tune until sinking looks correct relative to visual displacement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow IK")
	float DepthIKScale = 0.5f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	bool TraceFootToGround(const FFootContactState& Foot, FHitResult& OutHit, FVector& OutSocketLocation) const;
	void UpdateFootContacts();
	void UpdateMeshSink();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow")
	TArray<FFootContactState> FootContacts;

	UPROPERTY(Transient)
	ASnowDeformationManager* SnowManager = nullptr;

private:
	bool bDefaultMeshLocationCaptured = false;

	float CurrentMeshSinkOffset = 0.0f;
	FVector DefaultMeshRelativeLocation = FVector::ZeroVector;
};