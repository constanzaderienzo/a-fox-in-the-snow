#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "SnowDeformationManager.generated.h"

UCLASS()
class IDP_API ASnowDeformationManager : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Snow|RT")
    UTextureRenderTarget2D* SnowRT_A = nullptr;

    UPROPERTY(EditAnywhere, Category = "Snow|RT")
    UTextureRenderTarget2D* SnowRT_B = nullptr;

    UPROPERTY(EditAnywhere, Category = "Snow|Write")
    UMaterialInterface* SnowWriteMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Snow|Surface")
    AActor* SnowPlaneActor = nullptr;

    UPROPERTY(EditAnywhere, Category = "Snow|Names")
    FName PreviousRTParamName = TEXT("PreviousSnowRT");

    UPROPERTY(EditAnywhere, Category = "Snow|Names")
    FName SurfaceRTParamName = TEXT("SnowDeformationRT");

    UPROPERTY(EditAnywhere) FName StampUParamName = TEXT("StampU");
    UPROPERTY(EditAnywhere) FName StampVParamName = TEXT("StampV");
    UPROPERTY(EditAnywhere) FName RadiusXParamName = TEXT("RadiusX");
    UPROPERTY(EditAnywhere) FName RadiusYParamName = TEXT("RadiusY");
    UPROPERTY(EditAnywhere) FName FootYawParamName = TEXT("FootYaw");

    UPROPERTY(EditAnywhere) FName PrevStampUParamName = TEXT("PrevStampU");
    UPROPERTY(EditAnywhere) FName PrevStampVParamName = TEXT("PrevStampV");
    void ResetSnow();
    void StampAtUV(const FVector2D& PrevUV, const FVector2D& CurrUV, float RadiusX, float RadiusY, float FootYaw);
    void StampAtWorldHit(const FHitResult& Hit, const FVector2D& PrevUV, float RadiusX, float RadiusY, float FootYaw);
    FVector2D GetSnowUVFromHit(const FHitResult& Hit) const;

protected:
    virtual void BeginPlay() override;

private:
    bool bUseA = true;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* WriteMID = nullptr;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* SurfaceMID = nullptr;
};
