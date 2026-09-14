// Copyright Epic Games, Inc. All Rights Reserved.


#include "SecondSemester_TeamPlayerController.h"
#include "CSHCrosshairWidget.h"
#include "CSHMainUIWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "SecondSemester_TeamCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "UObject/SoftObjectPath.h"
#include "SecondSemester_Team.h"
#include "Widgets/Input/SVirtualJoystick.h"

ASecondSemester_TeamPlayerController::ASecondSemester_TeamPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = ASecondSemester_TeamCameraManager::StaticClass();
}

void ASecondSemester_TeamPlayerController::SetWeaponHUDVisible(bool bVisible)
{
    if (MainUIWidget)
    {
        MainUIWidget->SetWeaponUIVisible(bVisible);
    }
}
void ASecondSemester_TeamPlayerController::SetWeaponHUDInfo(const FText& Name, const FText& Description)
{
    if (MainUIWidget)
    {
        MainUIWidget->SetWeaponInfo(Name, Description);
    }
}
void ASecondSemester_TeamPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		CrosshairWidget = CreateWidget<UCSHCrosshairWidget>(this, UCSHCrosshairWidget::StaticClass());
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToPlayerScreen(10);
		}

		const TSoftClassPtr<UCSHMainUIWidget> MainUIClass(
			FSoftObjectPath(TEXT("/Game/CSH/Buleprint/UI/WBP_CSH_MainUI.WBP_CSH_MainUI_C")));
		if (UClass* LoadedMainUIClass = MainUIClass.LoadSynchronous())
		{
			MainUIWidget = CreateWidget<UCSHMainUIWidget>(this, LoadedMainUIClass);
			if (MainUIWidget)
			{
				MainUIWidget->AddToPlayerScreen(9);
			}
		}
	}

	
	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogSecondSemester_Team, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void ASecondSemester_TeamPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
}

bool ASecondSemester_TeamPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
