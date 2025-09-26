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

	//캡슐 컴포넌트를 루트컴포넌트로 설정
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetSimulatePhysics(false);
	SetRootComponent(CapsuleComp);
	//스켈레탈메쉬컴포넌트를 캡슐컴포넌트에 부착
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetSimulatePhysics(false);
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
	//스프링 암을 캡슐에 부착
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(CapsuleComp);
	SpringArmComp->TargetArmLength = 350.0f; // 스프링 암 거리
	SpringArmComp->bUsePawnControlRotation = true; // 컨트롤러 회전에 따라 회전하도록 적용
	//카메라를 스프링 암에 부착
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName); // 스프링 암 끝 소켓에 부착
	CameraComp->bUsePawnControlRotation = false;

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	FloatingPawnMovement->UpdatedComponent = CapsuleComp;
	//스켈레탈 메쉬 컴포넌트 설정
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>MeshAsset(TEXT("/Game/Resources/Characters/Meshes/SKM_Manny.SKM_Manny"));
	if (MeshAsset.Succeeded())
	{
		SkeletalMeshComp->SetSkeletalMesh(MeshAsset.Object);
	}
}

void AHWPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced InputComponent로 캐스팅
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA를 가져오기 위해 현재 소유 중인 Controller를 ASpartaPlayerController로 캐스팅
		if (AHWPlayerController* PlayerController = Cast<AHWPlayerController>(GetController()))
		{
			// IA_Move 액션 키를 누르고 있는 동안 Move() 호출
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this, &AHWPawn::Move);
			}
			// IA_Look 마우스를 움직이일 때 Look() 호출
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

	// 컨트롤러(카메라)의 회전 가져오기
	FRotator ControlRotation = Controller->GetControlRotation();
	// 카메라 회전에서 Pitch는 무시하고 Yaw만 사용
	FRotator YawRotation(0, ControlRotation.Yaw, 0);

	// 카메라 기준 전방 벡터와 우측 벡터 계산
	//FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	//FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	FVector ForwardDirection = YawRotation.Quaternion().GetAxisX();
	FVector RightDirection = YawRotation.Quaternion().GetAxisY();

	// 전후 이동 (카메라 기준)
	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(ForwardDirection, MoveInput.X);
	}

	// 좌우 이동 (카메라 기준)
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
		// 1. 현재 컨트롤러의 회전 가져오기
		FRotator ControlRotation = Controller->GetControlRotation();

		// 2. Yaw 회전 (좌우) 처리
		if (!FMath::IsNearlyZero(LookInput.X))
		{
			// 컨트롤러 Yaw 회전 값 수정
			ControlRotation.Yaw += LookInput.X * 2.0f;
		}

		// 3. Pitch 회전 (상하) 처리 - 부호 반전된 상태
		if (!FMath::IsNearlyZero(LookInput.Y))
		{
			// 컨트롤러 Pitch 회전 값 수정 (위아래 제한 적용), Y축의 반전 적용
			float Pitch = FMath::Clamp(ControlRotation.Pitch - LookInput.Y * 2.0f, -80.0f, 80.0f);
			ControlRotation.Pitch = Pitch;
		}

		// 4. 계산된 회전 값을 컨트롤러에 적용 (카메라 움직임)
		Controller->SetControlRotation(ControlRotation);

		// 5. Pawn의 Yaw 회전을 컨트롤러의 Yaw 회전과 동기화 (메시 회전)
		FRotator PawnRotation = GetActorRotation();
		PawnRotation.Yaw = ControlRotation.Yaw; // Pawn의 Yaw를 컨트롤러의 Yaw와 같게 설정
		SetActorRotation(PawnRotation); // Pawn에 새로운 회전 적용
	}
}
