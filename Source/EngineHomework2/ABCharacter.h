// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineHomework2.h"
#include "GameFramework/Character.h"
#include "ABCharacter.generated.h"

UENUM()
enum  EControlMode {
	FPS,
	QUARTER,
	TPS
};
UCLASS()
class ENGINEHOMEWORK2_API AABCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AABCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* springArm;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* camera;

	USkeletalMeshComponent* mesh;

private:
	void UpDown(float newAxisVal);
	void LeftRight(float newAxisVal);
	void LookUp(float newAxisVal);
	void Turn(float newAxisVal);
	void Zoom(float newAxisVal);
	void ViewChange();

protected:
	void SetControlMode(EControlMode controlMode);
	EControlMode currentControlMode = EControlMode::QUARTER;
	FVector directionToMove = FVector::ZeroVector;

	float armLengthTo = 0.0f;
	FRotator armRotaionTo = FRotator::ZeroRotator;
	float armLengthSpeed = 0.0f;
	float armRotationSpeed = 0.0f;
	const float armZoomPower = 100.0f;
	const float maxZoomLength = 1000.0f;
};
