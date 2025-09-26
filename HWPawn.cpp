#include "HWPawn.h"
#include "HWPlayerController.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"


AHWPawn::AHWPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	//ĸ�� ������Ʈ�� ��Ʈ������Ʈ�� ����
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetSimulatePhysics(false);
	SetRootComponent(CapsuleComp);
	//���̷�Ż�޽�������Ʈ�� ĸ��������Ʈ�� ����
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetSimulatePhysics(false);
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	//������ ���� ĸ���� ����
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(CapsuleComp);
	SpringArmComp->TargetArmLength = 350.0f; // ������ �� �Ÿ�
	SpringArmComp->bUsePawnControlRotation = true; // ��Ʈ�ѷ� ȸ���� ���� ȸ���ϵ��� ����
	//ī�޶� ������ �Ͽ� ����
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName); // ������ �� �� ���Ͽ� ����
	CameraComp->bUsePawnControlRotation = false;

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	FloatingPawnMovement->UpdatedComponent = CapsuleComp;
	//���̷�Ż �޽� ������Ʈ ����
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT("/Game/Resources/Characters/Meshes/SKM_Manny.SKM_Manny"));
	if (MeshAsset.Succeeded())
	{
		SkeletalMeshComp->SetSkeletalMesh(MeshAsset.Object);
	}
}

void AHWPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced InputComponent�� ĳ����
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA�� �������� ���� ���� ���� ���� Controller�� ASpartaPlayerController�� ĳ����
		if (AHWPlayerController* PlayerController = Cast<AHWPlayerController>(GetController()))
		{
			// IA_Move �׼� Ű�� ������ �ִ� ���� Move() ȣ��
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this, &AHWPawn::Move);
			}
			// IA_Look ���콺�� �������� �� Look() ȣ��
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &AHWPawn::Look);
			}
		}
	}
}

void AHWPawn::Move(const FInputActionValue& value)
{
	if (!Controller)
	{
		return;
	}

	const FVector2D MoveInput = value.Get<FVector2D>();

	// ��Ʈ�ѷ�(ī�޶�)�� ȸ�� ��������
	FRotator ControlRotation = Controller->GetControlRotation();
	// ī�޶� ȸ������ Pitch�� �����ϰ� Yaw�� ���
	FRotator YawRotation(0, ControlRotation.Yaw, 0);

	// ī�޶� ���� ���� ���Ϳ� ���� ���� ���
	//FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	//FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	FVector ForwardDirection = YawRotation.Quaternion().GetAxisX();
	FVector RightDirection = YawRotation.Quaternion().GetAxisY();

	// ���� �̵� (ī�޶� ����)
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(ForwardDirection, MoveInput.X);
	}

	// �¿� �̵� (ī�޶� ����)
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(RightDirection, MoveInput.Y);
	}
}

void AHWPawn::Look(const FInputActionValue& value)
{
	const FVector2D LookInput = value.Get<FVector2D>();
	
	if (Controller)
	{
		// 1. ���� ��Ʈ�ѷ��� ȸ�� ��������
		FRotator ControlRotation = Controller->GetControlRotation();

		// 2. Yaw ȸ�� (�¿�) ó��
		if (!FMath::IsNearlyZero(LookInput.X))
		{
			// ��Ʈ�ѷ� Yaw ȸ�� �� ����
			ControlRotation.Yaw += LookInput.X * 2.0f;
		}

		// 3. Pitch ȸ�� (����) ó�� - ��ȣ ������ ����
		if (!FMath::IsNearlyZero(LookInput.Y))
		{
			// ��Ʈ�ѷ� Pitch ȸ�� �� ���� (���Ʒ� ���� ����), Y���� ���� ����
			float Pitch = FMath::Clamp(ControlRotation.Pitch - LookInput.Y * 2.0f, -80.0f, 80.0f);
			ControlRotation.Pitch = Pitch;
		}

		// 4. ���� ȸ�� ���� ��Ʈ�ѷ��� ���� (ī�޶� ������)
		Controller->SetControlRotation(ControlRotation);

		// 5. Pawn�� Yaw ȸ���� ��Ʈ�ѷ��� Yaw ȸ���� ����ȭ (�޽� ȸ��)
		FRotator PawnRotation = GetActorRotation();
		PawnRotation.Yaw = ControlRotation.Yaw; // Pawn�� Yaw�� ��Ʈ�ѷ��� Yaw�� ���� ����
		SetActorRotation(PawnRotation); // Pawn�� ���ο� ȸ�� ����
	}
}
