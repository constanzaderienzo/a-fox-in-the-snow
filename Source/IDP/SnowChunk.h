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

    void InitializeChunk(
        FVector2D InChunkOrigin,
        float InChunkSize,
        int32 InResolution,
        float InMaxHeightCm,
        UMaterialInterface* InSnowMaterial,
        UMaterialInterface* InSnowWriteMaterial
    );

    void ResetDeformation();

    FVector2D GetChunkOrigin() const { return ChunkOrigin; }
    float GetChunkSize() const { return ChunkSize; }
    FVector2D WorldToChunkUV(FVector2D WorldXY) const;
    bool ContainsWorldPosition(FVector2D WorldXY) const;

    // Stamp deformation — writes to RT and deforms vertices
    void StampAtUV(
        const FVector2D& PrevUV,
        const FVector2D& CurrUV,
        float RadiusX,
        float RadiusY,
        float FootYaw
    );

    // Called by ChunkManager every frame if bHasDeformation
    void UpdateDeformation(float DeltaTime);

    bool bHasDeformation = false;

    // Param names for write material
    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName SurfaceRTParamName = TEXT("SnowDeformationRT");

    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName PreviousRTParamName = TEXT("PreviousSnowRT");

    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName PrevStampUParamName = TEXT("PrevStampU");

    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName PrevStampVParamName = TEXT("PrevStampV");

    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName RadiusXParamName = TEXT("RadiusX");

    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName RadiusYParamName = TEXT("RadiusY");

    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName FootYawParamName = TEXT("FootYaw");

    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName StampUParamName = TEXT("StampU");

    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    FName StampVParamName = TEXT("StampV");

    UPROPERTY()
    UMaterialInstanceDynamic* WriteMID = nullptr;

    UPROPERTY()
    UMaterialInstanceDynamic* SurfaceMID = nullptr;

    UPROPERTY()
    UTextureRenderTarget2D* SnowRT_A = nullptr;

    UPROPERTY()
    UTextureRenderTarget2D* SnowRT_B = nullptr;

    bool bUseA = true;

    // Max depth a stamp can deform in cm
    UPROPERTY(EditAnywhere, Category = "Snow|Deformation")
    float MaxDepthCm = 30.0f;

protected:
    virtual void BeginPlay() override;

private:
    void GenerateMesh();
    float SampleHeight(float X, float Y) const;
    void RecalculateNormals();
    void ApplyVertexDeformation(const FVector2D& PrevUV, const FVector2D& CurrUV,
        float RadiusX, float RadiusY, float DepthCm);

    UPROPERTY()
    UProceduralMeshComponent* ProcMesh = nullptr;

    UPROPERTY()
    UMaterialInterface* SnowMaterial = nullptr;

    UPROPERTY()
    UMaterialInterface* SnowWriteMaterial = nullptr;

    FVector2D ChunkOrigin = FVector2D::ZeroVector;
    float ChunkSize = 1000.0f;
    int32 Resolution = 128;
    float MaxHeightCm = 100.0f;

    bool bInitialized = false;
    bool bVertexDirty = false;

    // Vertex arrays
    TArray<FVector> OriginalVertices;
    TArray<FVector> CurrentVertices;
    TArray<float> DeformationDepth;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<int32> Triangles;
    TArray<FColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;
    TArray<float> RidgeHeight;
};