// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SecondSemester_TeamPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UCSHCrosshairWidget;
class UCSHWeaponHUDWidget;
class UCSHMainUIWidget;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config="Game")
class SECONDSEMESTER_TEAM_API ASecondSemester_TeamPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	ASecondSemester_TeamPlayerController();

	UFUNCTION(BlueprintCallable, Category="CSH|UI")
	void SetWeaponHUDVisible(bool bVisible);
	void SetWeaponHUDInfo(const FText& Name, const FText& Description);

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Always-visible center-screen aiming reticle. */
	UPROPERTY(Transient)
	TObjectPtr<UCSHCrosshairWidget> CrosshairWidget;

	/** Main HUD. It owns the reusable weapon UI child widget. */
	UPROPERTY(Transient)
	TObjectPtr<UCSHMainUIWidget> MainUIWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;
};
