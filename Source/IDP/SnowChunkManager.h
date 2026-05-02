#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnowChunkManager.generated.h"

class ASnowChunk;

UCLASS()
class IDP_API ASnowChunkManager : public AActor
{
    GENERATED_BODY()

public:
    ASnowChunkManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, Category = "Snow")
    TSubclassOf<ASnowChunk> SnowChunkClass;

    UPROPERTY(EditAnywhere, Category = "Snow")
    UMaterialInterface* SnowMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Snow")
    UMaterialInterface* SnowWriteMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Snow")
    AActor* PlayerActor;

    UPROPERTY(EditAnywhere, Category = "Snow")
    float ChunkSize = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Snow")
    int32 ChunkResolution = 128;

    UPROPERTY(EditAnywhere, Category = "Snow")
    float MaxHeightCm = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Snow")
    int32 GridRadius = 2;

    // Find which chunk owns a world position
    ASnowChunk* GetChunkAtWorldPosition(FVector2D WorldXY) const;

private:
    FIntPoint CurrentCenterChunk = FIntPoint(INT_MAX, INT_MAX);

    UPROPERTY()
    TMap<FIntPoint, ASnowChunk*> ActiveChunks;

    // Pool of hidden chunks ready to reuse
    UPROPERTY()
    TArray<ASnowChunk*> ChunkPool;

    FIntPoint WorldToChunkCoord(const FVector& WorldLocation) const;
    void UpdateChunks();
    ASnowChunk* GetOrSpawnChunk(const FIntPoint& Coord);
    void RecycleChunk(ASnowChunk* Chunk);
};