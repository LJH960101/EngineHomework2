// Fill out your copyright notice in the Description page of Project Settings.

#include "ABCharacter.h"


// Sets default values
AABCharacter::AABCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// �������ϰ� ī�޶� ����
	springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	// ������ ����
	springArm->SetupAttachment(GetCapsuleComponent());
	camera->SetupAttachment(springArm);

	// �޽ø� �ʱ�ȭ
	mesh = GetMesh();
	mesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f),
		FRotator(0.0f, -90.0f, 0.0f));
	springArm->TargetArmLength = 400.0f;
	springArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

	// arm ���� �� �ʱ�ȭ
	armLengthSpeed = 3.0f;
	armRotationSpeed = 10.0f;

	// ���̷�Ż �޽� ����
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_CARDBOARD(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard"));
	if (SK_CARDBOARD.Succeeded()) {
		mesh->SetSkeletalMesh(SK_CARDBOARD.Object);
	}

	// �ִϸ��̼� ����
	mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIOR_ANIM(TEXT("/Game/Book/Animations/WarriorAnimBlueprint.WarriorAnimBlueprint_C"));
	if (WARRIOR_ANIM.Succeeded()) {
		mesh->SetAnimInstanceClass(WARRIOR_ANIM.Class);
	}

	// ��� �ʱ�ȭ
	SetControlMode(currentControlMode);
}

// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TargetArmLength�� armLengthTo�� õõ�� interp ��Ŵ.
	springArm->TargetArmLength = FMath::FInterpTo(springArm->TargetArmLength, armLengthTo, DeltaTime, armLengthSpeed);

	switch (currentControlMode)
	{
	case QUARTER:
		// ȸ���� ���ر��� õõ�� interp ��Ŵ.
		springArm->RelativeRotation = FMath::RInterpTo(springArm->RelativeRotation, armRotaionTo, DeltaTime, armRotationSpeed);

		// �����̴� ���̶��?
		if (directionToMove.SizeSquared() > 0.0f) {
			// ȸ���� ������ ä �����δ�.
			GetController()->SetControlRotation(FRotationMatrix::MakeFromX(directionToMove).Rotator());
			AddMovementInput(directionToMove);
		}
		break;
	case FPS:
		// �޽ð� ���̴� �����̰�, springArm �Ÿ��� ª�ٸ� �޽ø� �����ش�.
		if (mesh->bVisible && springArm->TargetArmLength <= 50.0f) mesh->SetVisibility(false);
		break;
	}
}

// Called to bind functionality to input
void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AABCharacter::UpDown);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AABCharacter::LeftRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AABCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AABCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Zoom"), this, &AABCharacter::Zoom);
	PlayerInputComponent->BindAction(TEXT("ViewChange"), EInputEvent::IE_Pressed, this, &AABCharacter::ViewChange);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AABCharacter::Jump);
}

void AABCharacter::UpDown(float newAxisVal)
{
	switch (currentControlMode)
	{
	case FPS:
	case TPS:
		AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), newAxisVal);
		break;
	case QUARTER:
		directionToMove.X = newAxisVal;
		break;
	}
}

void AABCharacter::LeftRight(float newAxisVal)
{
	switch (currentControlMode)
	{
	case FPS:
	case TPS:
		AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), newAxisVal);
		break;
	case QUARTER:
		directionToMove.Y = newAxisVal;
		break;
	}
}

void AABCharacter::LookUp(float newAxisVal)
{
	switch (currentControlMode)
	{
	case FPS:
	case TPS:
		AddControllerPitchInput(newAxisVal);
		break;
	}
}

void AABCharacter::Turn(float newAxisVal)
{
	switch (currentControlMode)
	{
	case FPS:
	case TPS:
		AddControllerYawInput(newAxisVal);
		break;
	}
}

void AABCharacter::Zoom(float newAxisVal)
{
	switch (currentControlMode)
	{
	case TPS:
		// �� �ప���� armLength�� ����, �ʹ� ������ FPS���� �����Ѵ�.
		armLengthTo = FMath::Clamp(armLengthTo + newAxisVal * armZoomPower * -1.0f, 0.0f, maxZoomLength);
		if (armLengthTo <= 50.0f) SetControlMode(EControlMode::FPS);
		break;
	case FPS:
		// Ȯ�� �Է��� ������, TPS�� �ٽ� ��ȯ�Ѵ�.
		if (newAxisVal < 0.0f) SetControlMode(EControlMode::TPS);
		break;
	}
}

void AABCharacter::ViewChange()
{
	switch (currentControlMode)
	{
	case FPS:
	case TPS:
		GetController()->SetControlRotation(GetActorRotation());
		SetControlMode(EControlMode::QUARTER);
		break;
	case QUARTER:
		GetController()->SetControlRotation(springArm->RelativeRotation);
		SetControlMode(EControlMode::TPS);
		break;
	default:
		ABLOG(Warning, TEXT("Unexpected Mode."));
		break;
	}
}

void AABCharacter::SetControlMode(EControlMode controlMode)
{
	currentControlMode = controlMode;
	switch (controlMode)
	{
	case TPS:
	{
		//springArm->TargetArmLength = 450.0f;
		//springArm->SetRelativeRotation(FRotator::ZeroRotator);
		mesh->SetVisibility(true);
		armLengthTo = 450.0f;
		springArm->bUsePawnControlRotation = true;
		springArm->bInheritPitch = true;
		springArm->bInheritRoll = true;
		springArm->bInheritYaw = true;
		springArm->bDoCollisionTest = true;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}
	break;
	case QUARTER:
	{
		//springArm->TargetArmLength = 800.0f;
		//springArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
		mesh->SetVisibility(true);
		armLengthTo = 800.0f;
		armRotaionTo = FRotator(-45.0f, 0.0f, 0.0f);
		springArm->bUsePawnControlRotation = false;
		springArm->bInheritPitch = false;
		springArm->bInheritRoll = false;
		springArm->bInheritYaw = false;
		springArm->bDoCollisionTest = false;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}
	break;
	case FPS:
	{
		// TPS�� ���� �����ϰ�, TPS���� ���̵ǹǷ�, ���� ������ �ʿ䰡 ����.
		mesh->SetVisibility(true);
		armLengthTo = 0.0f;
	}
	break;
	default:
		ABLOG(Warning, TEXT("Unexpected Mode."));
		break;
	}
}

