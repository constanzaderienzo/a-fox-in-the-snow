#include "SnowChunk.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

ASnowChunk::ASnowChunk()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    RootComponent = ProcMesh;
    ProcMesh->bUseComplexAsSimpleCollision = true;
}

void ASnowChunk::BeginPlay()
{
    Super::BeginPlay();
}

void ASnowChunk::InitializeChunk(
    FVector2D InChunkOrigin,
    float InChunkSize,
    int32 InResolution,
    float InMaxHeightCm,
    UMaterialInterface* InSnowMaterial,
    UMaterialInterface* InSnowWriteMaterial)
{
    ChunkOrigin = InChunkOrigin;
    ChunkSize = InChunkSize;
    Resolution = InResolution;
    MaxHeightCm = InMaxHeightCm;
    SnowMaterial = InSnowMaterial;
    SnowWriteMaterial = InSnowWriteMaterial;

    SetActorLocation(FVector(ChunkOrigin.X, ChunkOrigin.Y, 0.0f));

    SnowRT_A = UKismetRenderingLibrary::CreateRenderTarget2D(
        this, 1024, 1024, ETextureRenderTargetFormat::RTF_RGBA8);
    SnowRT_B = UKismetRenderingLibrary::CreateRenderTarget2D(
        this, 1024, 1024, ETextureRenderTargetFormat::RTF_RGBA8);

    // Clear immediately after creation
    UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT_A, FLinearColor(1, 0, 0, 1));
    UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT_B, FLinearColor(1, 0, 0, 1));

    if (SnowWriteMaterial)
        WriteMID = UMaterialInstanceDynamic::Create(SnowWriteMaterial, this);

    GenerateMesh();
    ResetDeformation();

    bInitialized = true;
}

float ASnowChunk::SampleHeight(float X, float Y) const
{
    float WorldX = ChunkOrigin.X + X;
    float WorldY = ChunkOrigin.Y + Y;

    float Scale1 = 0.002f;
    float Scale2 = 0.005f;

    float H1 = FMath::Sin(WorldX * Scale1) * FMath::Cos(WorldY * Scale1);
    float H2 = FMath::Sin(WorldX * Scale2 + 1.3f) * FMath::Cos(WorldY * Scale2 + 0.7f);

    float Height = (H1 * 0.85f + H2 * 0.15f);
    return Height * MaxHeightCm;
}

void ASnowChunk::GenerateMesh()
{
    int32 VertCount = Resolution + 1;
    float Step = ChunkSize / Resolution;

    OriginalVertices.Empty();
    CurrentVertices.Empty();
    DeformationDepth.Empty();
    Normals.Empty();
    UVs.Empty();
    Triangles.Empty();
    VertexColors.Empty();
    Tangents.Empty();
    RidgeHeight.Empty();

    // Generate vertices
    for (int32 Y = 0; Y <= Resolution; Y++)
    {
        for (int32 X = 0; X <= Resolution; X++)
        {
            float LocalX = X * Step;
            float LocalY = Y * Step;
            float Z = SampleHeight(LocalX, LocalY);

            FVector Vert(LocalX, LocalY, Z);
            OriginalVertices.Add(Vert);
            CurrentVertices.Add(Vert);
            DeformationDepth.Add(0.0f);

            UVs.Add(FVector2D((float)X / Resolution, (float)Y / Resolution));
            Normals.Add(FVector(0, 0, 1));
            VertexColors.Add(FColor::White);
            RidgeHeight.Add(0.0f);
        }
    }

    // Generate triangles
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            int32 BottomLeft = Y * VertCount + X;
            int32 BottomRight = BottomLeft + 1;
            int32 TopLeft = BottomLeft + VertCount;
            int32 TopRight = TopLeft + 1;

            Triangles.Add(BottomLeft);
            Triangles.Add(TopLeft);
            Triangles.Add(BottomRight);

            Triangles.Add(BottomRight);
            Triangles.Add(TopLeft);
            Triangles.Add(TopRight);
        }
    }

    RecalculateNormals();

    ProcMesh->CreateMeshSection(
        0,
        CurrentVertices,
        Triangles,
        Normals,
        UVs,
        VertexColors,
        Tangents,
        true
    );

    if (SnowMaterial)
    {
        SurfaceMID = UMaterialInstanceDynamic::Create(SnowMaterial, this);
        ProcMesh->SetMaterial(0, SurfaceMID);

        SurfaceMID->SetScalarParameterValue(TEXT("ChunkOriginX"), ChunkOrigin.X);
        SurfaceMID->SetScalarParameterValue(TEXT("ChunkOriginY"), ChunkOrigin.Y);
        SurfaceMID->SetScalarParameterValue(TEXT("ChunkSize"), ChunkSize);
        SurfaceMID->SetTextureParameterValue(TEXT("SnowDeformationRT"), SnowRT_A);
    }
}

void ASnowChunk::RecalculateNormals()
{
    // Zero out normals
    for (FVector& N : Normals)
        N = FVector::ZeroVector;

    // Accumulate face normals
    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        FVector V0 = CurrentVertices[Triangles[i]];
        FVector V1 = CurrentVertices[Triangles[i + 1]];
        FVector V2 = CurrentVertices[Triangles[i + 2]];

        FVector FaceNormal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();

        Normals[Triangles[i]] += FaceNormal;
        Normals[Triangles[i + 1]] += FaceNormal;
        Normals[Triangles[i + 2]] += FaceNormal;
    }

    // Normalize
    for (FVector& N : Normals)
        N = N.GetSafeNormal();
}

void ASnowChunk::ApplyVertexDeformation(
    const FVector2D& PrevUV,
    const FVector2D& CurrUV,
    float RadiusX,
    float RadiusY,
    float DepthCm)
{
    int32 VertCount = Resolution + 1;
    float Step = ChunkSize / Resolution;

    // Convert UV radii to local space
    float WorldRadiusX = RadiusX * ChunkSize;
    float WorldRadiusY = RadiusY * ChunkSize;

    // Capsule endpoints in local space
    FVector2D PrevLocal = PrevUV * ChunkSize;
    FVector2D CurrLocal = CurrUV * ChunkSize;

    // Compute bounding box to avoid iterating all vertices
    float MinX = FMath::Min(PrevLocal.X, CurrLocal.X) - WorldRadiusX;
    float MaxX = FMath::Max(PrevLocal.X, CurrLocal.X) + WorldRadiusX;
    float MinY = FMath::Min(PrevLocal.Y, CurrLocal.Y) - WorldRadiusY;
    float MaxY = FMath::Max(PrevLocal.Y, CurrLocal.Y) + WorldRadiusY;

    // Clamp to chunk bounds
    int32 XStart = FMath::Max(0, FMath::FloorToInt(MinX / Step));
    int32 XEnd = FMath::Min(Resolution, FMath::CeilToInt(MaxX / Step));
    int32 YStart = FMath::Max(0, FMath::FloorToInt(MinY / Step));
    int32 YEnd = FMath::Min(Resolution, FMath::CeilToInt(MaxY / Step));

    for (int32 Y = YStart; Y <= YEnd; Y++)
    {
        for (int32 X = XStart; X <= XEnd; X++)
        {
            int32 Idx = Y * VertCount + X;
            FVector2D VertLocal(X * Step, Y * Step);

            // Capsule distance calculation
            FVector2D AB = CurrLocal - PrevLocal;
            FVector2D AP = VertLocal - PrevLocal;

            float ABdotAB = FVector2D::DotProduct(AB, AB) + 0.0001f;
            float T = FMath::Clamp(
                FVector2D::DotProduct(AP, AB) / ABdotAB,
                0.0f, 1.0f
            );
            FVector2D ClosestPoint = PrevLocal + T * AB;
            FVector2D Offset = VertLocal - ClosestPoint;

            // Ellipse distance
            float EllipseX = Offset.X / WorldRadiusX;
            float EllipseY = Offset.Y / WorldRadiusY;
            float EllipseDist = FMath::Sqrt(EllipseX * EllipseX + EllipseY * EllipseY);

            if (EllipseDist <= 1.0f)
            {
                // Inside stamp — push down with smooth falloff
                //float Influence = FMath::SmoothStep(0.0f, 1.0f, 1.0f - EllipseDist);
                float Influence = FMath::Pow(1.0f - EllipseDist, 2.0f);
                
                float TargetDepth = DepthCm * Influence;

                if (TargetDepth > DeformationDepth[Idx])
                {
                    DeformationDepth[Idx] = TargetDepth;
                    CurrentVertices[Idx].Z = OriginalVertices[Idx].Z - DeformationDepth[Idx];
                    bVertexDirty = true;
                }

               /* float NewDepth = FMath::Min(
                    DeformationDepth[Idx] + DepthCm * Influence,
                    MaxDepthCm
                );

                if (NewDepth > DeformationDepth[Idx])
                {
                    DeformationDepth[Idx] = NewDepth;
                    CurrentVertices[Idx].Z = OriginalVertices[Idx].Z - DeformationDepth[Idx];
                    bVertexDirty = true;
                }*/
            }
        }
    }
}

void ASnowChunk::StampAtUV(
    const FVector2D& PrevUV,
    const FVector2D& CurrUV,
    float RadiusX,
    float RadiusY,
    float FootYaw)
{
    if (!bInitialized) return;

    // Write to RT for material appearance

    if (WriteMID && SurfaceMID && SnowRT_A && SnowRT_B)
    {
        UTextureRenderTarget2D* ReadRT = bUseA ? SnowRT_A : SnowRT_B;
        UTextureRenderTarget2D* WriteRT = bUseA ? SnowRT_B : SnowRT_A;

        WriteMID->SetScalarParameterValue(PrevStampUParamName, PrevUV.X);
        WriteMID->SetScalarParameterValue(PrevStampVParamName, PrevUV.Y);
        WriteMID->SetScalarParameterValue(StampUParamName, CurrUV.X);
        WriteMID->SetScalarParameterValue(StampVParamName, CurrUV.Y);
        WriteMID->SetTextureParameterValue(PreviousRTParamName, ReadRT);
        WriteMID->SetScalarParameterValue(RadiusXParamName, RadiusX);
        WriteMID->SetScalarParameterValue(RadiusYParamName, RadiusY);
        WriteMID->SetScalarParameterValue(FootYawParamName, FootYaw);

        UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, WriteRT, WriteMID);
        SurfaceMID->SetTextureParameterValue(SurfaceRTParamName, WriteRT);

        bUseA = !bUseA;
    }

    // Deform actual vertices
    ApplyVertexDeformation(PrevUV, CurrUV, RadiusX, RadiusY, MaxDepthCm);

    bHasDeformation = true;
}

void ASnowChunk::UpdateDeformation(float DeltaTime)
{
    if (!bVertexDirty) return;

    RecalculateNormals();

    ProcMesh->UpdateMeshSection(
        0,
        CurrentVertices,
        Normals,
        UVs,
        VertexColors,
        Tangents
    );

    bVertexDirty = false;
}

void ASnowChunk::ResetDeformation()
{
    if (SnowRT_A)
        UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT_A, FLinearColor(1, 0, 0, 1));
    if (SnowRT_B)
        UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT_B, FLinearColor(1, 0, 0, 1));

    bUseA = true;

    if (SurfaceMID)
        SurfaceMID->SetTextureParameterValue(TEXT("SnowDeformationRT"), SnowRT_A);

    // Reset vertex deformation
    if (OriginalVertices.Num() > 0)
    {
        for (int32 i = 0; i < CurrentVertices.Num(); i++)
        {
            CurrentVertices[i] = OriginalVertices[i];
            DeformationDepth[i] = 0.0f;
            RidgeHeight[i] = 0.0f;
        }
        bVertexDirty = true;
        bHasDeformation = false;
    }
}

FVector2D ASnowChunk::WorldToChunkUV(FVector2D WorldXY) const
{
    FVector2D Local = WorldXY - ChunkOrigin;
    return FVector2D(
        FMath::Clamp(Local.X / ChunkSize, 0.0f, 1.0f),
        FMath::Clamp(Local.Y / ChunkSize, 0.0f, 1.0f)
    );
}

bool ASnowChunk::ContainsWorldPosition(FVector2D WorldXY) const
{
    FVector2D Local = WorldXY - ChunkOrigin;
    return Local.X >= 0.0f && Local.X < ChunkSize &&
        Local.Y >= 0.0f && Local.Y < ChunkSize;
}