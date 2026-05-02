#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnowDeformationManager.generated.h"

UCLASS()
class IDP_API ASnowDeformationManager : public AActor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Snow")
    void ResetSnow();

    UFUNCTION(BlueprintCallable, Category = "Snow")
    void StampAtWorldHit(
        const FHitResult& Hit,
        const FVector& PrevWorld,
        const FVector2D& PrevUV,
        float RadiusX,
        float RadiusY,
        float FootYaw
    );

    UFUNCTION(BlueprintCallable, Category = "Snow")
    FVector2D GetSnowUVFromHit(const FHitResult& Hit) const;

protected:
    virtual void BeginPlay() override;
};