#include "HWPlayerController.h"
#include "EnhancedInputSubsystems.h"

AHWPlayerController::AHWPlayerController() : InputMappingContext(nullptr), MoveAction(nullptr), LookAction(nullptr)
{
}

void AHWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext) 
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}
