// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Waldemar Zeitler

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Engine.h"
#include "GMovement.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UGAME_API UGMovement : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UGMovement();

    // Called when the game starts
    virtual void BeginPlay() override;

    // Handles moving forward/backward
    void MoveForward(const float Val);

    // Handles strafing Left/Right
    void MoveRight(const float Val);

    // The maximum (or default) speed
    UPROPERTY(EditAnywhere, Category = "CI - Speed Setup")
        float MaxMovementSpeed;

    // The minimum movement speed. Used if player picks up items which then effects speed
    UPROPERTY(EditAnywhere, Category = "CI - Speed Setup")
        float MinMovementSpeed;

    float CurrentSpeed;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void SetupKeyBindings(UInputComponent* PlayerInputComponent);

    // Renders the player movable/unmovable
    void SetMovable(bool bCanMove);

    bool bCanMove;

private:
    // Regulates the max speed
    float SpeedLimit;

    ACharacter* Character;

    // The item which is currently focused by the player
    AStaticMeshActor* ItemToHandle;

    // Stores the default speed set at start game
    float DefaultSpeed;

public:
    /**
    *  Calculates a rising speed up. This funktion is done with the PT1-Glied function.
    *  The "Sprungantwort" function is used: a(t) = K(1-e^(-t/T)).
    *  https://de.wikipedia.org/wiki/PT1-Glied
    *  @param TimeStep - Current time to rise the speed
    *  @return the new walking speed
    */
    float CalculateNewSpeed(float TimeStep);

private:
    // Time taken from the tick function, to calculate the speed up.
    float SpeedUpTime;

    // Bool to check if the character is moving.
    bool bIsMoving;

    /**
    *	Rises and sets new speed value.
    *	@param Direction - Direction in which to speed up.
    */
    void SpeedUp(const FVector Direction, const float Val);

};
