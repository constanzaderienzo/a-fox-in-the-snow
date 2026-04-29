#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SnowChunk.generated.h"

UCLASS()
class IDP_API ASnowChunk : public AActor
{
    GENERATED_BODY()

public:
    ASnowChunk();

    // Initialize chunk at a given world position
    void InitializeChunk(FVector2D InChunkOrigin, float InChunkSize, int32 InResolution, float InMaxHeightCm);

    // Reset the render target (called when chunk is recycled)
    void ResetDeformation();

    // Get the render target for this chunk
    UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

    // Get chunk origin in world XY
    FVector2D GetChunkOrigin() const { return ChunkOrigin; }

    // Get chunk size
    float GetChunkSize() const { return ChunkSize; }

    // Convert world XY position to chunk UV (0-1)
    FVector2D WorldToChunkUV(FVector2D WorldXY) const;

    // Check if a world XY position is within this chunk
    bool ContainsWorldPosition(FVector2D WorldXY) const;

    // The dynamic material instance for this chunk's surface
    UPROPERTY()
    UMaterialInstanceDynamic* SurfaceMID = nullptr;

    // Ping pong render targets
    UPROPERTY()
    UTextureRenderTarget2D* SnowRT_A = nullptr;

    UPROPERTY()
    UTextureRenderTarget2D* SnowRT_B = nullptr;

    bool bUseA = true;

protected:
    virtual void BeginPlay() override;

private:
    void GenerateMesh();
    float SampleHeight(float X, float Y) const;

    UPROPERTY()
    UProceduralMeshComponent* ProcMesh = nullptr;

    UPROPERTY()
    UTextureRenderTarget2D* RenderTarget = nullptr;

    // Snow surface material
    UPROPERTY(EditAnywhere, Category = "Snow")
    UMaterialInterface* SnowMaterial = nullptr;

    FVector2D ChunkOrigin = FVector2D::ZeroVector;
    float ChunkSize = 1000.0f;
    int32 Resolution = 64;
    float MaxHeightCm = 150.0f;

    bool bInitialized = false;
};