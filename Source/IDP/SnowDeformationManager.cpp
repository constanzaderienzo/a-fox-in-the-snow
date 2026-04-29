#include "SnowDeformationManager.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "SnowGameModeBase.h"

void ASnowDeformationManager::BeginPlay()
{
    Super::BeginPlay();

    if (!SnowRT_A || !SnowRT_B || !SnowWriteMaterial || !SnowPlaneActor)
    {
        UE_LOG(LogTemp, Error, TEXT("SnowDeformationManager missing assignments"));
        return;
    }

    if (ASnowGameModeBase* GM = GetWorld()->GetAuthGameMode<ASnowGameModeBase>())
    {
        GM->SetSnowManager(this);
    }

    WriteMID = UMaterialInstanceDynamic::Create(SnowWriteMaterial, this);

    UStaticMeshComponent* SnowMesh = SnowPlaneActor->FindComponentByClass<UStaticMeshComponent>();
    if (!SnowMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("SnowPlaneActor has no StaticMeshComponent"));
        return;
    }
    SurfaceMID = SnowMesh->CreateAndSetMaterialInstanceDynamic(0);

    ResetSnow();
}

void ASnowDeformationManager::ResetSnow()
{
    UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT_A, FLinearColor(1, 0, 0, 1));
    UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT_B, FLinearColor(1, 0, 0, 1));

    bUseA = true;

    // Bind surface to a known RT at start
    if (SurfaceMID)
        SurfaceMID->SetTextureParameterValue(SurfaceRTParamName, SnowRT_A);
}

void ASnowDeformationManager::StampAtUV(const FVector2D& PrevUV, const FVector2D& CurrUV, float RadiusX, float RadiusY, float FootYaw)
{
    if (!WriteMID || !SurfaceMID) return;

    UTextureRenderTarget2D* ReadRT = bUseA ? SnowRT_A : SnowRT_B;
    UTextureRenderTarget2D* WriteRT = bUseA ? SnowRT_B : SnowRT_A;

    // Previous stamp UV (for capsule)
    WriteMID->SetScalarParameterValue(PrevStampUParamName, PrevUV.X);
    WriteMID->SetScalarParameterValue(PrevStampVParamName, PrevUV.Y);

    // Current stamp UV
    WriteMID->SetScalarParameterValue(TEXT("StampU"), CurrUV.X);
    WriteMID->SetScalarParameterValue(TEXT("StampV"), CurrUV.Y);

    WriteMID->SetTextureParameterValue(PreviousRTParamName, ReadRT);

    WriteMID->SetScalarParameterValue(RadiusXParamName, RadiusX);
    WriteMID->SetScalarParameterValue(RadiusYParamName, RadiusY);
    WriteMID->SetScalarParameterValue(FootYawParamName, FootYaw);

    UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, WriteRT, WriteMID);

    SurfaceMID->SetTextureParameterValue(SurfaceRTParamName, WriteRT);

    bUseA = !bUseA;
}

FVector2D ASnowDeformationManager::GetSnowUVFromHit(const FHitResult& Hit) const
{
    UPrimitiveComponent* HitComp = Hit.GetComponent();
    UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(HitComp);
    if (!MeshComp) return FVector2D::ZeroVector;

    const FTransform& MeshTransform = MeshComp->GetComponentTransform();
    FVector LocalHit = MeshTransform.InverseTransformPosition(Hit.Location);

    FVector Min, Max;
    MeshComp->GetLocalBounds(Min, Max);

    float U = (LocalHit.X - Min.X) / (Max.X - Min.X);
    float V = (LocalHit.Y - Min.Y) / (Max.Y - Min.Y);

    U = FMath::Clamp(U, 0.0f, 1.0f);
    V = FMath::Clamp(V, 0.0f, 1.0f);

    return FVector2D(U, V);
}

void ASnowDeformationManager::StampAtWorldHit(const FHitResult& Hit, const FVector2D& PrevUV, float RadiusX, float RadiusY, float FootYaw)
{
    if (!Hit.GetComponent()) return;

    FVector2D CurrUV = GetSnowUVFromHit(Hit);
    StampAtUV(PrevUV, CurrUV, RadiusX, RadiusY, FootYaw);
}
