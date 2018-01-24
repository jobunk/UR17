// Copyright 2017, Institute for Artificial Intelligence

#include "SlicingEditorActionCallbacks.h"
#include "SlicingComponent.h"

#include "Core.h"
#include "Editor.h"
#include "Engine/Selection.h"

#include "Components/BoxComponent.h"

#define LOCTEXT_NAMESPACE "FSlicingEditorModule"

void FSlicingEditorActionCallbacks::ShowInstructions()
{
	FMessageDialog* Instructions = new FMessageDialog();
	const FText InstructionsTitle =
		LOCTEXT("SlicingInstructions_Title", "Instructions to edit an object to be able to slice");

	Instructions->Debugf(
		LOCTEXT("SlicingInstructions", "To alter an object to be able to slice other objects, the user needs to create multiple Sockets on the position - and with the size of - certain areas needed for cutting objects. These sockets need specific names to specify the area they are used for.\n\nCutting Blade: ...\nCutting Handle: ...\nCutting Exitpoint: ..."),
		&InstructionsTitle
	);
}

void FSlicingEditorActionCallbacks::OnEnableDebugConsoleOutput(bool* bButtonValue)
{
	*bButtonValue = !*bButtonValue;
}

bool FSlicingEditorActionCallbacks::OnIsEnableDebugConsoleOutputEnabled(bool* bButtonValue)
{
	return *bButtonValue;
}

void FSlicingEditorActionCallbacks::OnEnableDebugShowPlane(bool* bButtonValue)
{
	*bButtonValue = !*bButtonValue;
}

bool FSlicingEditorActionCallbacks::OnIsEnableDebugShowPlaneEnabled(bool* bButtonValue)
{
	return *bButtonValue;
}

void FSlicingEditorActionCallbacks::OnEnableDebugShowTrajectory(bool* bButtonValue)
{
	*bButtonValue = !*bButtonValue;
}

bool FSlicingEditorActionCallbacks::OnIsEnableDebugShowTrajectoryEnabled(bool* bButtonValue)
{
	return *bButtonValue;
}

void FSlicingEditorActionCallbacks::ReplaceSocketsWithComponents()
{
	USelection* Selection = GEditor->GetSelectedComponents();
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Selection->GetSelectedObject(0));
	if (Mesh != NULL && Mesh != nullptr)
	{
		if (Mesh->ComponentHasTag(FName("Knife")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Has Sockets = Success"));

			float BoxScale = 3.5;

			UBoxComponent* HandleBox = NewObject<UBoxComponent>(Mesh,FName("Handle"));
			HandleBox->RegisterComponent();
			HandleBox->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Handle"));
			HandleBox->SetWorldLocation(Mesh->GetSocketLocation("Handle"));
			FVector TempScale = Mesh->GetSocketTransform(FName("Handle")).GetScale3D();
			HandleBox->SetBoxExtent(FVector(BoxScale, BoxScale, BoxScale));
			HandleBox->SetCollisionProfileName(FName("BlockAll"));
			HandleBox->bGenerateOverlapEvents = false;
			
			USlicingComponent* BladeBox = NewObject<USlicingComponent>(Mesh, FName("BladeBox"));
			BladeBox->RegisterComponent();
			BladeBox->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("BladeBox"));
			BladeBox->SetWorldLocation(Mesh->GetSocketLocation("BladeBox"));
			TempScale = Mesh->GetSocketTransform(FName("BladeBox")).GetScale3D();
			BladeBox->SetBoxExtent(FVector(BoxScale, BoxScale, BoxScale));
			BladeBox->bGenerateOverlapEvents = true;
			BladeBox->SetCollisionProfileName(FName("OverlapAll"));
			BladeBox->bMultiBodyOverlap = false;

			UBoxComponent* CuttingExitpointBox = NewObject<UBoxComponent>(Mesh, FName("CuttingExitpointBox"));
			CuttingExitpointBox->RegisterComponent();
			CuttingExitpointBox->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetIncludingScale, FName("CuttingExitpointBox"));
			CuttingExitpointBox->SetWorldLocation(Mesh->GetSocketLocation("CuttingExitpointBox"));
			TempScale = Mesh->GetSocketTransform(FName("CuttingExitpointBox")).GetScale3D();
			CuttingExitpointBox->SetBoxExtent(FVector(BoxScale, BoxScale, BoxScale));
			CuttingExitpointBox->SetCollisionProfileName(FName("OverlapAll"));
			CuttingExitpointBox->bGenerateOverlapEvents = false;
		} 
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Has Sockets = FAILURE"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Null Error in Box Creation"));
	}
}

void FSlicingEditorActionCallbacks::ReplaceSocketsOfAllStaticMeshComponents()
{
	// To be implemented in the future
}

#undef LOCTEXT_NAMESPACE