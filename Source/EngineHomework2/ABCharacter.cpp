// Fill out your copyright notice in the Description page of Project Settings.

#include "ABCharacter.h"


// Sets default values
AABCharacter::AABCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 스프링암과 카메라를 만듬
	springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	// 계층을 설정
	springArm->SetupAttachment(GetCapsuleComponent());
	camera->SetupAttachment(springArm);

	// 메시를 초기화
	mesh = GetMesh();
	mesh->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f),
		FRotator(0.0f, -90.0f, 0.0f));
	springArm->TargetArmLength = 400.0f;
	springArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));

	// arm 관련 값 초기화
	armLengthSpeed = 3.0f;
	armRotationSpeed = 10.0f;

	// 스켈레탈 메시 설정
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_CARDBOARD(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard"));
	if (SK_CARDBOARD.Succeeded()) {
		mesh->SetSkeletalMesh(SK_CARDBOARD.Object);
	}

	// 애니메이션 설정
	mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIOR_ANIM(TEXT("/Game/Book/Animations/WarriorAnimBlueprint.WarriorAnimBlueprint_C"));
	if (WARRIOR_ANIM.Succeeded()) {
		mesh->SetAnimInstanceClass(WARRIOR_ANIM.Class);
	}

	// 모드 초기화
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

	// TargetArmLength를 armLengthTo로 천천히 interp 시킴.
	springArm->TargetArmLength = FMath::FInterpTo(springArm->TargetArmLength, armLengthTo, DeltaTime, armLengthSpeed);

	switch (currentControlMode)
	{
	case QUARTER:
		// 회전을 기준까지 천천히 interp 시킴.
		springArm->RelativeRotation = FMath::RInterpTo(springArm->RelativeRotation, armRotaionTo, DeltaTime, armRotationSpeed);

		// 움직이는 중이라면?
		if (directionToMove.SizeSquared() > 0.0f) {
			// 회전은 고정한 채 움직인다.
			GetController()->SetControlRotation(FRotationMatrix::MakeFromX(directionToMove).Rotator());
			AddMovementInput(directionToMove);
		}
		break;
	case FPS:
		// 메시가 보이는 상태이고, springArm 거리가 짧다면 메시를 숨겨준다.
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
		// 휠 축값으로 armLength를 조절, 너무 작으면 FPS모드로 변경한다.
		armLengthTo = FMath::Clamp(armLengthTo + newAxisVal * armZoomPower * -1.0f, 0.0f, maxZoomLength);
		if (armLengthTo <= 50.0f) SetControlMode(EControlMode::FPS);
		break;
	case FPS:
		// 확대 입력이 들어오면, TPS로 다시 전환한다.
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
		// TPS와 값이 동일하고, TPS에서 전이되므로, 따로 설정할 필요가 없음.
		mesh->SetVisibility(true);
		armLengthTo = 0.0f;
	}
	break;
	default:
		ABLOG(Warning, TEXT("Unexpected Mode."));
		break;
	}
}

