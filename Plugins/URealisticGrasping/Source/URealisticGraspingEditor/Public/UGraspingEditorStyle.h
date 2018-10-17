// Copyright 2018, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * Sets all of the styles for the new added botton.
 */
class UGraspingEditorStyle
{
public:
	static void Initialize();
	static void Shutdown();


	//* Reloads textures used by slate renderer
	static void ReloadTextures();

	// @return The Slate style set for the game
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:
	static TSharedRef< class FSlateStyleSet > Create();
	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};
