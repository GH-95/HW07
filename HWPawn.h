#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HWPawn.generated.h"

// 전방선언
class UCapsuleComponent;
class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;
//Enhanced Input에서 액션 값을 받을 때 사용하는 구조체
struct FInputActionValue;

UCLASS()
class HW_07_API AHWPawn : public APawn
{
	GENERATED_BODY()

public:
	AHWPawn();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capsule")
	UCapsuleComponent* CapsuleComp; // 캡슐 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capsule")
	USkeletalMeshComponent* SkeletalMeshComp; // 스켈레탈 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp; // 스프링 암 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp; // 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UFloatingPawnMovement* FloatingPawnMovement; //폰 무브먼트 컴포넌트

	//바인딩 함수
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Enhancede Input에서 액션 값은 FInputActionValue로 전달
	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	
};
