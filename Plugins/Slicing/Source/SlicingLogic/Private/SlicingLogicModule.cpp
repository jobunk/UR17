﻿// Copyright 2018, Institute for Artificial Intelligence

#include "SlicingLogicModule.h"

#include "StaticMeshResources.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "RawMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/ConvexElem.h"

#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"

#define LOCTEXT_NAMESPACE "FSlicingLogicModule"

void FSlicingLogicModule::StartupModule()
{
	// Empty as of right now
}

void FSlicingLogicModule::ShutdownModule()
{
	// Empty as of right now
}

UProceduralMeshComponent* FSlicingLogicModule::ConvertStaticToProceduralMeshComponent(
	UStaticMeshComponent* StaticMeshComponent, TArray<FStaticMaterial> StaticMaterials)
{
	// Needed so that the component can be cut/changed in runtime
	StaticMeshComponent->GetStaticMesh()->bAllowCPUAccess = true;

	UProceduralMeshComponent* ProceduralMeshComponent = NewObject<UProceduralMeshComponent>(StaticMeshComponent);
	ProceduralMeshComponent->SetRelativeTransform(StaticMeshComponent->GetRelativeTransform());
	ProceduralMeshComponent->RegisterComponent();
	ProceduralMeshComponent->SetCollisionProfileName(FName("PhysicsActor"));
	ProceduralMeshComponent->bUseComplexAsSimpleCollision = false;
	ProceduralMeshComponent->SetEnableGravity(true);
	ProceduralMeshComponent->SetSimulatePhysics(true);
	ProceduralMeshComponent->bGenerateOverlapEvents = true;
	ProceduralMeshComponent->ComponentTags = StaticMeshComponent->ComponentTags;

	// Copies the mesh, collision and currently used materials from the StaticMeshComponent
	UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(
		StaticMeshComponent, 0, ProceduralMeshComponent, true);

	// Give out a copy of the static material from the original component, as procedural meshes do not have one
	// and therefore lose out on information about the materials
	StaticMaterials = StaticMeshComponent->GetStaticMesh()->StaticMaterials;

	// Remove the old static mesh
	StaticMeshComponent->DestroyComponent();

	return ProceduralMeshComponent;
}

void FSlicingLogicModule::ConvertProceduralComponentToStaticMeshActor(UProceduralMeshComponent* ProceduralMeshComponent)
{
	///																   ///
	/// COPIED OVER FROM "ProceduralMeshComponentDetails.cpp l.118-212 ///
	///																   ///
	// Raw mesh data we are filling in
	FRawMesh RawMesh;
	// Materials to apply to new mesh
	TArray<UMaterialInterface*> MeshMaterials;
	
	const int32 NumSections = ProceduralMeshComponent->GetNumSections();
	int32 VertexBase = 0;
	for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
	{
		FProcMeshSection* ProcSection = ProceduralMeshComponent->GetProcMeshSection(SectionIdx);

		// Copy verts
		for (FProcMeshVertex& Vert : ProcSection->ProcVertexBuffer)
		{
			RawMesh.VertexPositions.Add(Vert.Position);
		}

		// Copy 'wedge' info
		int32 NumIndices = ProcSection->ProcIndexBuffer.Num();
		for (int32 IndexIdx = 0; IndexIdx < NumIndices; IndexIdx++)
		{
			int32 Index = ProcSection->ProcIndexBuffer[IndexIdx];

			RawMesh.WedgeIndices.Add(Index + VertexBase);

			FProcMeshVertex& ProcVertex = ProcSection->ProcVertexBuffer[Index];

			FVector TangentX = ProcVertex.Tangent.TangentX;
			FVector TangentZ = ProcVertex.Normal;
			FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal() * (ProcVertex.Tangent.bFlipTangentY ? -1.f : 1.f);

			RawMesh.WedgeTangentX.Add(TangentX);
			RawMesh.WedgeTangentY.Add(TangentY);
			RawMesh.WedgeTangentZ.Add(TangentZ);

			RawMesh.WedgeTexCoords[0].Add(ProcVertex.UV0);
			RawMesh.WedgeColors.Add(ProcVertex.Color);
		}

		// copy face info
		int32 NumTris = NumIndices / 3;
		for (int32 TriIdx = 0; TriIdx < NumTris; TriIdx++)
		{
			RawMesh.FaceMaterialIndices.Add(SectionIdx);
			RawMesh.FaceSmoothingMasks.Add(0); // Assume this is ignored as bRecomputeNormals is false
		}

		// Remember material
		MeshMaterials.Add(ProceduralMeshComponent->GetMaterial(SectionIdx));

		// Update offset for creating one big index/vertex buffer
		VertexBase += ProcSection->ProcVertexBuffer.Num();
	}

	// The new StaticMesh that is going to be filled with the scanned info
	UStaticMesh* StaticMesh = NewObject<UStaticMesh>();

	// If we got some valid data.
	if (RawMesh.VertexPositions.Num() >= 3 && RawMesh.WedgeIndices.Num() >= 3)
	{
		StaticMesh->InitResources();

		StaticMesh->LightingGuid = FGuid::NewGuid();

		// Add source to new StaticMesh
		FStaticMeshSourceModel* SrcModel = new (StaticMesh->SourceModels) FStaticMeshSourceModel();
		SrcModel->BuildSettings.bRecomputeNormals = false;
		SrcModel->BuildSettings.bRecomputeTangents = false;
		SrcModel->BuildSettings.bRemoveDegenerates = false;
		SrcModel->BuildSettings.bUseHighPrecisionTangentBasis = false;
		SrcModel->BuildSettings.bUseFullPrecisionUVs = false;
		SrcModel->BuildSettings.bGenerateLightmapUVs = true;
		SrcModel->BuildSettings.SrcLightmapIndex = 0;
		SrcModel->BuildSettings.DstLightmapIndex = 1;
		SrcModel->RawMeshBulkData->SaveRawMesh(RawMesh);

		// Copy materials to new mesh
		for (UMaterialInterface* Material : MeshMaterials)
		{
			StaticMesh->StaticMaterials.Add(FStaticMaterial(Material));
		}

		//Set the Imported version before calling the build
		StaticMesh->ImportVersion = EImportStaticMeshVersion::LastVersion;

		// Build mesh from source
		StaticMesh->Build();
		StaticMesh->PostEditChange();
	}

	///
	/// END OF COPY
	///

	// Get the correct (simple) collision from the procedural mesh
	StaticMesh->BodySetup->AddCollisionFrom(ProceduralMeshComponent->GetBodySetup()->AggGeom);
	// TODO: CHECK FOR NULL
	StaticMesh->Build();
	StaticMesh->PostEditChange();

	// Spawn the StaticMeshActor
	FVector Location = ProceduralMeshComponent->GetAttachmentRootActor()->GetActorLocation();
	FRotator Rotation = ProceduralMeshComponent->GetAttachmentRootActor()->GetActorRotation();
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Template = ProceduralMeshComponent->GetAttachmentRootActor();
	AStaticMeshActor* StaticMeshActor =
		ProceduralMeshComponent->GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation, SpawnInfo);

	// Edit the StaticMeshComponent to have the same properties as the old ProceduralMeshComponent
	UStaticMeshComponent* NewStaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();
	NewStaticMeshComponent->SetStaticMesh(StaticMesh);

	NewStaticMeshComponent->SetRelativeTransform(ProceduralMeshComponent->GetRelativeTransform());
	NewStaticMeshComponent->RegisterComponent();
	NewStaticMeshComponent->SetCollisionProfileName(FName("PhysicsActor"));
	NewStaticMeshComponent->SetEnableGravity(true);
	NewStaticMeshComponent->SetSimulatePhysics(true);
	NewStaticMeshComponent->bGenerateOverlapEvents = true;
	NewStaticMeshComponent->ComponentTags = ProceduralMeshComponent->ComponentTags;

	// Remove the old component
	ProceduralMeshComponent->DestroyComponent();
}

template<class ComponentType>
ComponentType* FSlicingLogicModule::GetSlicingComponent(UStaticMeshComponent* SlicingObject)
{
	TArray<USceneComponent*> SlicingComponents;
	SlicingObject->GetChildrenComponents(true, SlicingComponents);

	for (USceneComponent* Component : SlicingComponents)
	{
		if (ComponentType* TypedComponent = Cast<ComponentType>(Component))
		{
			// Only one slicing component of each type should exist
			return TypedComponent;
		}
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSlicingLogicModule, Slicing)