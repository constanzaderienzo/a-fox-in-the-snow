#include "SnowChunk.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

ASnowChunk::ASnowChunk()
{
    PrimaryActorTick.bCanEverTick = false;

    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    RootComponent = ProcMesh;

    // Enable complex collision so character feet trace against actual geometry
    ProcMesh->bUseComplexAsSimpleCollision = true;
}

void ASnowChunk::BeginPlay()
{
    Super::BeginPlay();
}

void ASnowChunk::InitializeChunk(FVector2D InChunkOrigin, float InChunkSize, int32 InResolution, float InMaxHeightCm)
{
    ChunkOrigin = InChunkOrigin;
    ChunkSize = InChunkSize;
    Resolution = InResolution;
    MaxHeightCm = InMaxHeightCm;

    // Set world position
    SetActorLocation(FVector(ChunkOrigin.X, ChunkOrigin.Y, 0.0f));

    // Create ping pong render targets
    SnowRT_A = UKismetRenderingLibrary::CreateRenderTarget2D(
        this, 1024, 1024, ETextureRenderTargetFormat::RTF_RGBA8);
    SnowRT_B = UKismetRenderingLibrary::CreateRenderTarget2D(
        this, 1024, 1024, ETextureRenderTargetFormat::RTF_RGBA8);

    ResetDeformation();
    GenerateMesh();

    bInitialized = true;
}

void ASnowChunk::ResetDeformation()
{
    if (SnowRT_A)
        UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT_A, FLinearColor(1, 0, 0, 1));
    if (SnowRT_B)
        UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT_B, FLinearColor(1, 0, 0, 1));

    bUseA = true;

    // Update material to use RT_A
    if (SurfaceMID)
        SurfaceMID->SetTextureParameterValue(TEXT("SnowDeformationRT"), SnowRT_A);
}

float ASnowChunk::SampleHeight(float X, float Y) const
{
    // Layered sine waves for rolling hills
    // Using world position so hills are continuous across chunk boundaries
    float WorldX = ChunkOrigin.X + X;
    float WorldY = ChunkOrigin.Y + Y;

    float Scale1 = 0.003f;  // Large rolling hills
    float Scale2 = 0.008f;  // Medium variation
    float Scale3 = 0.02f;   // Small surface bumps

    float H1 = FMath::Sin(WorldX * Scale1) * FMath::Cos(WorldY * Scale1);
    float H2 = FMath::Sin(WorldX * Scale2 + 1.3f) * FMath::Cos(WorldY * Scale2 + 0.7f);
    float H3 = FMath::Sin(WorldX * Scale3 + 2.1f) * FMath::Cos(WorldY * Scale3 + 1.8f);

    // Weighted sum — large hills dominate
    float Height = (H1 * 0.6f + H2 * 0.3f + H3 * 0.1f);

    // Normalize from -1,1 to 0,MaxHeightCm
    return Height * MaxHeightCm;
}

void ASnowChunk::GenerateMesh()
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;

    int32 VertCount = Resolution + 1;
    float Step = ChunkSize / Resolution;

    // Generate vertices
    for (int32 Y = 0; Y <= Resolution; Y++)
    {
        for (int32 X = 0; X <= Resolution; X++)
        {
            float LocalX = X * Step;
            float LocalY = Y * Step;
            float Z = SampleHeight(LocalX, LocalY);

            Vertices.Add(FVector(LocalX, LocalY, Z));

            // UV is 0-1 across the chunk
            UVs.Add(FVector2D(
                (float)X / Resolution,
                (float)Y / Resolution
            ));

            Normals.Add(FVector(0, 0, 1)); // Will be recalculated
            VertexColors.Add(FColor::White);
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

            // Triangle 1
            Triangles.Add(BottomLeft);
            Triangles.Add(TopLeft);
            Triangles.Add(BottomRight);

            // Triangle 2
            Triangles.Add(BottomRight);
            Triangles.Add(TopLeft);
            Triangles.Add(TopRight);
        }
    }

    // Calculate smooth normals
    // First pass — zero out normals
    TArray<FVector> SmoothNormals;
    SmoothNormals.SetNumZeroed(Vertices.Num());

    // Accumulate face normals
    for (int32 i = 0; i < Triangles.Num(); i += 3)
    {
        FVector V0 = Vertices[Triangles[i]];
        FVector V1 = Vertices[Triangles[i + 1]];
        FVector V2 = Vertices[Triangles[i + 2]];

        FVector FaceNormal = FVector::CrossProduct(V1 - V0, V2 - V0).GetSafeNormal();

        SmoothNormals[Triangles[i]] += FaceNormal;
        SmoothNormals[Triangles[i + 1]] += FaceNormal;
        SmoothNormals[Triangles[i + 2]] += FaceNormal;
    }

    // Normalize
    for (int32 i = 0; i < SmoothNormals.Num(); i++)
    {
        Normals[i] = SmoothNormals[i].GetSafeNormal();
    }

    // Create the mesh section
    ProcMesh->CreateMeshSection(
        0,
        Vertices,
        Triangles,
        Normals,
        UVs,
        VertexColors,
        Tangents,
        true // Create collision
    );

    // Apply material if set
    if (SnowMaterial)
    {
        SurfaceMID = UMaterialInstanceDynamic::Create(SnowMaterial, this);
        ProcMesh->SetMaterial(0, SurfaceMID);

        // Pass chunk parameters to material
        SurfaceMID->SetScalarParameterValue(TEXT("ChunkOriginX"), ChunkOrigin.X);
        SurfaceMID->SetScalarParameterValue(TEXT("ChunkOriginY"), ChunkOrigin.Y);
        SurfaceMID->SetScalarParameterValue(TEXT("ChunkSize"), ChunkSize);
        SurfaceMID->SetTextureParameterValue(TEXT("SnowDeformationRT"), SnowRT_A);
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
    return Local.X >= 0.0f && Local.X <= ChunkSize &&
        Local.Y >= 0.0f && Local.Y <= ChunkSize;
}