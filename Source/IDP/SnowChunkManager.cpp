#include "SnowChunkManager.h"
#include "SnowChunk.h"
#include "Kismet/GameplayStatics.h"

ASnowChunkManager::ASnowChunkManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASnowChunkManager::BeginPlay()
{
    Super::BeginPlay();

    PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);

    if (!PlayerActor || !SnowChunkClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SnowChunkManager missing PlayerActor or SnowChunkClass"));
        return;
    }

    if (!SnowMaterial || !SnowWriteMaterial)
    {
        UE_LOG(LogTemp, Error, TEXT("SnowChunkManager missing SnowMaterial or SnowWriteMaterial"));
        return;
    }

    // Force initial spawn
    CurrentCenterChunk = FIntPoint(INT_MAX, INT_MAX);
    UpdateChunks();
}

void ASnowChunkManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!PlayerActor) return;

    FIntPoint NewCenter = WorldToChunkCoord(PlayerActor->GetActorLocation());
    if (NewCenter != CurrentCenterChunk)
    {
        CurrentCenterChunk = NewCenter;
        UpdateChunks();
    }

    // Update deformation on active chunks that have been stamped
    for (const TPair<FIntPoint, ASnowChunk*>& Pair : ActiveChunks)
    {
        if (Pair.Value && Pair.Value->bHasDeformation)
        {
            Pair.Value->UpdateDeformation(DeltaTime);
        }
    }
}

FIntPoint ASnowChunkManager::WorldToChunkCoord(const FVector& WorldLocation) const
{
    return FIntPoint(
        FMath::FloorToInt(WorldLocation.X / ChunkSize),
        FMath::FloorToInt(WorldLocation.Y / ChunkSize)
    );
}

void ASnowChunkManager::UpdateChunks()
{
    TSet<FIntPoint> NeededCoords;

    for (int32 Y = -GridRadius; Y <= GridRadius; Y++)
    {
        for (int32 X = -GridRadius; X <= GridRadius; X++)
        {
            FIntPoint Coord = CurrentCenterChunk + FIntPoint(X, Y);
            NeededCoords.Add(Coord);

            if (!ActiveChunks.Contains(Coord))
            {
                ASnowChunk* Chunk = GetOrSpawnChunk(Coord);
                if (Chunk)
                {
                    ActiveChunks.Add(Coord, Chunk);
                }
            }
        }
    }

    // Recycle out of range chunks
    TArray<FIntPoint> ToRemove;
    for (const TPair<FIntPoint, ASnowChunk*>& Pair : ActiveChunks)
    {
        if (!NeededCoords.Contains(Pair.Key))
        {
            ToRemove.Add(Pair.Key);
            RecycleChunk(Pair.Value);
        }
    }

    for (const FIntPoint& Coord : ToRemove)
    {
        ActiveChunks.Remove(Coord);
    }
}

ASnowChunk* ASnowChunkManager::GetOrSpawnChunk(const FIntPoint& Coord)
{
    FVector2D ChunkOrigin(
        Coord.X * ChunkSize,
        Coord.Y * ChunkSize
    );

    ASnowChunk* Chunk = nullptr;

    // Reuse from pool if available
    if (ChunkPool.Num() > 0)
    {
        Chunk = ChunkPool.Pop();
        Chunk->SetActorHiddenInGame(false);
        Chunk->SetActorEnableCollision(true);
    }
    else
    {
        Chunk = GetWorld()->SpawnActor<ASnowChunk>(
            SnowChunkClass,
            FVector(ChunkOrigin.X, ChunkOrigin.Y, 0.0f),
            FRotator::ZeroRotator
        );
    }

    if (!Chunk) return nullptr;

    Chunk->InitializeChunk(
        ChunkOrigin,
        ChunkSize,
        ChunkResolution,
        MaxHeightCm,
        SnowMaterial,
        SnowWriteMaterial
    );

    return Chunk;
}

void ASnowChunkManager::RecycleChunk(ASnowChunk* Chunk)
{
    if (!Chunk) return;
    Chunk->SetActorHiddenInGame(true);
    Chunk->SetActorEnableCollision(false);
    ChunkPool.Add(Chunk);
}

ASnowChunk* ASnowChunkManager::GetChunkAtWorldPosition(FVector2D WorldXY) const
{
    FIntPoint Coord = FIntPoint(
        FMath::FloorToInt(WorldXY.X / ChunkSize),
        FMath::FloorToInt(WorldXY.Y / ChunkSize)
    );

    ASnowChunk* const* Found = ActiveChunks.Find(Coord);
    return Found ? *Found : nullptr;
}