// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Waldemar Zeitler

#pragma once

#include "CoreMinimal.h"
#include "HUD/GameHUD.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "UGameModeBase.generated.h"

UCLASS()
class UGAME_API AUGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    AUGameModeBase();
};