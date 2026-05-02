#include "SnowDeformationManager.h"
#include "SnowChunk.h"
#include "SnowGameModeBase.h"

void ASnowDeformationManager::BeginPlay()
{
    Super::BeginPlay();

    if (ASnowGameModeBase* GM = GetWorld()->GetAuthGameMode<ASnowGameModeBase>())
    {
        GM->SetSnowManager(this);
    }
}
void ASnowDeformationManager::ResetSnow()
{
    // No-op
}

FVector2D ASnowDeformationManager::GetSnowUVFromHit(const FHitResult& Hit) const
{
    ASnowChunk* Chunk = Cast<ASnowChunk>(Hit.GetActor());
    if (!Chunk)
    {
        return FVector2D::ZeroVector;
    }

    return Chunk->WorldToChunkUV(
        FVector2D(Hit.Location.X, Hit.Location.Y)
    );
}

void ASnowDeformationManager::StampAtWorldHit(
    const FHitResult& Hit,
    const FVector& PrevWorld,
    const FVector2D& PrevUV,
    float RadiusX,
    float RadiusY,
    float FootYaw
)
{
    ASnowChunk* Chunk = Cast<ASnowChunk>(Hit.GetActor());
    if (!Chunk) return;

    FVector2D CurrUV = Chunk->WorldToChunkUV(
        FVector2D(Hit.Location.X, Hit.Location.Y)
    );

    const bool bPrevValid =
        !PrevWorld.IsZero() &&
        PrevUV.X >= 0.0f &&
        PrevUV.X <= 1.0f &&
        PrevUV.Y >= 0.0f &&
        PrevUV.Y <= 1.0f;

    const bool bPrevInsideThisChunk =
        bPrevValid &&
        Chunk->ContainsWorldPosition(FVector2D(PrevWorld.X, PrevWorld.Y));

    const bool bPrevNearEdge =
        PrevUV.X < 0.02f || PrevUV.X > 0.98f ||
        PrevUV.Y < 0.02f || PrevUV.Y > 0.98f;

    const bool bCurrNearEdge =
        CurrUV.X < 0.02f || CurrUV.X > 0.98f ||
        CurrUV.Y < 0.02f || CurrUV.Y > 0.98f;

    const FVector2D SafePrevUV =
        (bPrevInsideThisChunk && !bPrevNearEdge && !bCurrNearEdge)
        ? PrevUV
        : CurrUV;

    Chunk->StampAtUV(
        SafePrevUV,
        CurrUV,
        RadiusX,
        RadiusY,
        FootYaw
    );
}